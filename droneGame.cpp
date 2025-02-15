#include "lab_m1/tema2/droneGame.h"

#include <vector>
#include <string>

using namespace std;
using namespace m1;

#define MAP_SIZE_Z 100
#define MAP_SIZE_X (MAP_SIZE_Z + 10)
#define MAP_SIZE_Y 15

#define MAP_OBSTACLES (MAP_SIZE_X * MAP_SIZE_Z / 40)

#define FONT_SIZE 18


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */

void makeFirstPerson(implemented::GameCamera *camera, glm::vec3 target)
{
	auto fwd = camera->forward;
	fwd.y = 0;
	fwd = glm::normalize(fwd);

	camera->distanceToTarget = 0.f;
	camera->Set(target + glm::vec3(0, 0.2f, 0), target + fwd, glm::vec3(0, 1, 0));
}

void makeThirdPerson(implemented::GameCamera *camera, glm::vec3 target)
{
	auto fwd = camera->forward;
	fwd.y = 0;
	fwd = glm::normalize(fwd);

	camera->distanceToTarget = sqrt(5);
	camera->Set(target - 2.f * fwd + glm::vec3(0, 1.f, 0), target, glm::vec3(0, 1, 0));
}

void DroneGame::restart()
{
	score = 0;
	feedback = 0;

	fstPerson = true;
	enableUI = true;

	drone.pos = glm::vec3(0, 5, 0);
	drone.size = DRONE_SIZE;

	drone.angle = 0;
	drone.bladeAngle = 0;

	drone.target = nullptr;

	if (camera != nullptr) {
		delete camera;
	}
	camera = new implemented::GameCamera();
	makeFirstPerson(camera, drone.pos);

	terrain.generate(MAP_SIZE_X, MAP_SIZE_Z, MAP_OBSTACLES, shaders["TerrainShader"]);
}

DroneGame::DroneGame()
	: textRenderer(gfxc::TextRenderer(window->props.selfDir, 0, 0))
{}


DroneGame::~DroneGame()
{}

void DroneGame::addShaders()
{
	{
		Shader *shader = new Shader("TerrainShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1,
			"tema2", "shaders", "terrain/VertexShader.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1,
			"tema2", "shaders", "terrain/FragmentShader.glsl"), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
	{
		Shader *shader = new Shader("FOWShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1,
			"tema2", "shaders", "fow/VertexShader.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1,
			"tema2", "shaders", "fow/FragmentShader.glsl"), GL_FRAGMENT_SHADER);

		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}
}

void DroneGame::addMeshes()
{
	{
		Mesh *mesh = obj3D::createRectangle("TerrainTile", glm::vec3(0), 1, 2, glm::vec3(0));
		AddMeshToList(mesh);
	}
	{
		Mesh *mesh = obj3D::Tree::createTree("Tree", glm::vec3(0), 1, 0.5f);
		AddMeshToList(mesh);
	}
	{
		Mesh *mesh = obj3D::Building::createBuilding("Building", glm::vec3(0));
		AddMeshToList(mesh);
	}
	{
		auto meshes = drone.createDroneMeshes("Base", "Blade", glm::vec3(0));
		AddMeshToList(meshes.first);
		AddMeshToList(meshes.second);
	}
	{
		Mesh *mesh = obj3D::createRectangleParallelepiped("Target", glm::vec3(0), 1, 1, 1, COLOR_RED);
		AddMeshToList(mesh);
	}
	{
		Mesh *mesh = obj3D::createRectangleParallelepiped("Delivery", glm::vec3(0), 1, 1, 1, COLOR_BLUE);
		AddMeshToList(mesh);
	}
	{
		Mesh *mesh = obj3D::createCone("Indicator", glm::vec3(0), 1, 1, COLOR_YELLOW);
		AddMeshToList(mesh);
	}
}

void DroneGame::Init()
{
	camera = nullptr;
	speedFactor = 1.f;

	window->DisablePointer();
	fowShader = "VertexColor";

	restart();

	addMeshes();
	addShaders();

	textRenderer = gfxc::TextRenderer(window->props.selfDir, window->GetResolution().x, window->GetResolution().y);
	textRenderer.Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), FONT_SIZE);
}


void DroneGame::FrameStart()
{
	float viewX = window->props.resolution.x / 3.f;
	float viewY = viewX / window->props.aspectRatio;

	miniViewport = ViewportArea(0, 0, viewX, viewY);

	viewMatrix = camera->GetViewMatrix();
	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

	// Clears the color buffer (using the previously set color) and depth buffer
	if (feedback > 0) {
		glClearColor(0.1f, 1.f, 0.1f, 1);
		feedback--;
	} else {
		if (fowShader == "FOWShader") {
			glClearColor(0.01f, 0.01f, 0.06f, 1);
		} else {
			glClearColor(0.3f, 0.3f, 0.8f, 1);
		}
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}


void DroneGame::displayIndicator()
{
	auto targetPos = (drone.target == nullptr)
		? terrain.target.pos : drone.target->sendPos;
	auto fwd = glm::normalize(targetPos - drone.pos);

	float angle = atan2(fwd.x, fwd.z);

	glm::mat4 modelMatrix(1);
	modelMatrix = glm::translate(modelMatrix, drone.pos + glm::vec3(0, drone.size * DRONE_h * 1.f + 1, 0));
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
	modelMatrix = glm::rotate(modelMatrix, RADIANS(110), glm::vec3(1, 0, 0));
	modelMatrix = glm::scale(modelMatrix, drone.size / 2.f * glm::vec3(0.5f, 2.5f, 0.5f));

	RenderMesh(meshes["Indicator"], shaders["VertexColor"], modelMatrix);
}

void DroneGame::updateShaders()
{
	{
		glUseProgram(shaders["TerrainShader"]->program);

		auto loc = glGetUniformLocation(shaders["TerrainShader"]->program, "dronePos");
		glUniform3fv(loc, 1, glm::value_ptr(drone.pos));

		loc = glGetUniformLocation(shaders["TerrainShader"]->program, "fow");
		glUniform1i(loc, fowShader == "FOWShader");

		glUseProgram(0);
	}
	{
		glUseProgram(shaders["FOWShader"]->program);

		auto loc = glGetUniformLocation(shaders["FOWShader"]->program, "dronePos");
		glUniform3fv(loc, 1, glm::value_ptr(drone.pos));

		glUseProgram(0);
	}
}

void DroneGame::RenderScene(float scale)
{
	updateShaders();

	if (fowShader == "FOWShader") {
		auto voidMatrix = glm::mat4(1);
		voidMatrix = glm::translate(voidMatrix, glm::vec3(-MAP_SIZE_X * 2, -0.01f, -MAP_SIZE_Z * 2));
		voidMatrix = glm::scale(voidMatrix, glm::vec3(MAP_SIZE_X * 2, 0, MAP_SIZE_Z * 4));

		RenderMesh(meshes["TerrainTile"], shaders["VertexColor"], voidMatrix);
	}

	auto baseMatrix = drone.getBaseMatrix();
	baseMatrix = glm::scale(baseMatrix, glm::vec3(scale));
	RenderMesh(meshes["Base"], shaders["VertexColor"], baseMatrix);

	for (auto &&bladeMatrix : drone.getBladeMatrices()) {
		baseMatrix = glm::scale(baseMatrix, glm::vec3(scale));
		RenderMesh(meshes["Blade"], shaders["VertexColor"], bladeMatrix);
	}

	auto obstacles = terrain.getObstacleData();
	for (auto &&kv : obstacles) {
		RenderMesh(meshes[kv.first], shaders[fowShader], kv.second);
	}

	auto tiles = terrain.getTileMatrices();
	for (auto &&matrix : tiles) {
		RenderMesh(meshes["TerrainTile"], shaders["TerrainShader"], matrix);
	}

	auto targetMatrix = glm::scale(terrain.target.getMatrix(), glm::vec3(scale));
	RenderMesh(meshes["Target"], shaders[fowShader], targetMatrix);

	if (drone.target != nullptr) {
		auto deliverMatrix = glm::scale(drone.target->getDeliverMatrix(), glm::vec3(scale));
		RenderMesh(meshes["Delivery"], shaders[fowShader], deliverMatrix);

		if (enableUI) {
			targetMatrix = glm::translate(drone.target->getDeliverMatrix(), glm::vec3(0, 50, 0));
			targetMatrix = glm::scale(targetMatrix, glm::vec3(0.07f, 100.f, 0.07f));

			RenderMesh(meshes["Delivery"], shaders["VertexColor"], targetMatrix);
		}
	} else if (enableUI) {
		targetMatrix = glm::translate(terrain.target.getMatrix(), glm::vec3(0, 50, 0));
		targetMatrix = glm::scale(targetMatrix, glm::vec3(0.07f, 100.f, 0.07f));

		RenderMesh(meshes["Target"], shaders["VertexColor"], targetMatrix);
	}

	if (!enableUI) {
		return;
	}

	if (fowShader == "FOWShader") {
		textRenderer.RenderText("Score: " + std::to_string(score),
			window->GetResolution().x * 9.f / 10.f, 1, 1);
	} else {
		textRenderer.RenderText("Score: " + std::to_string(score),
			window->GetResolution().x * 9.f / 10.f, 1, 1, COLOR_BLACK);
	}
}

void DroneGame::Update(float deltaTimeSeconds)
{
	RenderScene();

	if (!enableUI) {
		return;
	}

	displayIndicator();

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(miniViewport.x, miniViewport.y, miniViewport.width, miniViewport.height);

	glm::vec3 topDownPosition = glm::vec3(0, 25.f, 0);
	glm::vec3 topDownTarget = glm::vec3(0, 0, 0);
	glm::vec3 upDirection = glm::vec3(0, 0, -1);

	glm::mat4 topDownView = glm::lookAt(topDownPosition, topDownTarget, upDirection);
	glm::mat4 orthoProjection = glm::ortho(-MAP_SIZE_X / 2.f, MAP_SIZE_X / 2.f,
		-MAP_SIZE_Z / 2.f, MAP_SIZE_Z / 2.f, 0.1f, 100.0f);

	viewMatrix = topDownView;
	projectionMatrix = orthoProjection;

	RenderScene(3.f);
}


void DroneGame::FrameEnd()
{}


void DroneGame::RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix)
{
	if (!mesh || !shader || !shader->program)
		return;

	// Render an object using the specified shader and the specified position
	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	mesh->Render();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

glm::vec3 DroneGame::keepInBounds(glm::vec3 pos)
{
	glm::vec3 res;

	res.x = std::max(-MAP_SIZE_X / 2.f, pos.x);
	res.x = std::min(MAP_SIZE_X / 2.f, res.x);

	res.z = std::max(-MAP_SIZE_Z / 2.f, pos.z);
	res.z = std::min(MAP_SIZE_Z / 2.f, res.z);

	float droneRY = drone.size * DRONE_h * 1.5f / 2.f;
	if (drone.target != nullptr) {
		droneRY += drone.target->size / 1.5f;
	}

	res.y = std::max(terrain.getTerrainY(res.x, res.z)
		+ droneRY + 0.05f, res.y);
	res.y = std::min(static_cast<float>(MAP_SIZE_Y), res.y);

	return res;
}

void DroneGame::moveForward(float distance)
{
	auto dVec = camera->forward;
	dVec.y = 0;
	dVec = glm::normalize(dVec);

	auto oldPos = drone.pos;
	drone.pos = keepInBounds(drone.pos + distance * dVec);

	checkPosHit(oldPos);
}

void DroneGame::moveRight(float distance)
{
	auto dVec = camera->right;

	auto oldPos = drone.pos;
	drone.pos = keepInBounds(drone.pos + distance * dVec);

	checkPosHit(oldPos);
}

void DroneGame::moveUp(float distance)
{
	auto dVec = glm::vec3(0, 1, 0);

	auto oldPos = drone.pos;
	drone.pos = keepInBounds(drone.pos + 1.2f * distance * dVec);

	checkPosHit(oldPos);
}

void DroneGame::checkPosHit(glm::vec3 oldPos)
{
	glm::vec3 dVec = drone.pos - oldPos;
	if (terrain.hit(drone)) {
		drone.pos -= dVec * 1.2f;
	}
	camera->position += drone.pos - oldPos;
}

void DroneGame::moveInput(float deltaTime)
{
	float step = deltaTime * 3.f;
	float dAngle = deltaTime * 15;

	// Move
	if (window->KeyHold(GLFW_KEY_W)) {
		moveForward(step);
		dAngle = deltaTime * 25;
	} else if (window->KeyHold(GLFW_KEY_S)) {
		moveForward(-step);
		dAngle = deltaTime * 25;
	}

	if (window->KeyHold(GLFW_KEY_D)) {
		moveRight(step);
		dAngle = deltaTime * 25;
	} else if (window->KeyHold(GLFW_KEY_A)) {
		moveRight(-step);
		dAngle = deltaTime * 25;
	}

	if (window->KeyHold(GLFW_KEY_LEFT_SHIFT)) {
		moveUp(step);
		dAngle = deltaTime * 25;
	} else if (window->KeyHold(GLFW_KEY_LEFT_CONTROL)) {
		moveUp(-step);
		dAngle = deltaTime * 25;
	}

	drone.bladeAngle += dAngle;
}

void DroneGame::OnInputUpdate(float deltaTime, int mods)
{
	speedFactor = 1.f;
	if (window->KeyHold(GLFW_KEY_SPACE)) {
		speedFactor = MAP_SIZE_Y / 5.f;
	}

	deltaTime *= speedFactor;

	moveInput(deltaTime);

	// Rotate
	float angleStep = deltaTime * 1.5f;

	if (window->KeyHold(GLFW_KEY_Q)) {
		drone.angle += angleStep;
		if (fstPerson) {
			camera->RotateThirdPerson_OY(angleStep);
		}
	} else if (window->KeyHold(GLFW_KEY_E)) {
		drone.angle -= angleStep;
		if (fstPerson) {
			camera->RotateThirdPerson_OY(-angleStep);
		}
	}

	drone.acquireTarget(terrain.target);
	if (drone.target != nullptr && drone.target->deliver()) {
		score += floor(drone.target->distance);

		feedback = 15;

		drone.target = nullptr;
		terrain.generateTarget();
	}
}

void DroneGame::OnKeyPress(int key, int mods)
{
	// Add key press event
	if (key == GLFW_KEY_R) {
		restart();
		return;
	}

	if (key == GLFW_KEY_TAB) {
		fstPerson = !fstPerson;

		if (fstPerson) {
			makeFirstPerson(camera, drone.pos);
		} else {
			makeThirdPerson(camera, drone.pos);
		}
	}

	if (key == GLFW_KEY_F) {
		if (fowShader == "FOWShader") {
			fowShader = "VertexColor";
		} else {
			fowShader = "FOWShader";
		}
	}

	if (key == GLFW_KEY_P) {
		window->DisablePointer();
	}

	if (key == GLFW_KEY_U) {
		enableUI = !enableUI;
	}
}


void DroneGame::OnKeyRelease(int key, int mods)
{
	// Add key release event
}


void DroneGame::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event
	float sensivityOX = 0.001f;
	float sensivityOY = 0.001f;

	camera->RotateThirdPerson_OY(-sensivityOX * deltaX);
	camera->RotateThirdPerson_OX(-sensivityOY * deltaY);
}


void DroneGame::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
}


void DroneGame::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}


void DroneGame::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{}


void DroneGame::OnWindowResize(int width, int height)
{
	textRenderer = gfxc::TextRenderer(window->props.selfDir, window->GetResolution().x, window->GetResolution().y);
	textRenderer.Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), FONT_SIZE);
}
