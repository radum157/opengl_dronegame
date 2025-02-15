#include "terrain.h"

#include <random>
#include <set>
#include <memory>

#include "core/gpu/shader.h"
#include "core/engine.h"

#include "../../objects.h"
#include "../../../color.h"

using namespace obj3D;

void Terrain::generateTiles()
{
	for (float dx = -sizeX / 2.f; dx <= sizeX / 2.f; dx++) {
		for (float dz = -sizeZ / 2.f; dz <= sizeZ / 2.f; dz++) {
			auto modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(dx, 0, dz));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 1, 1));

			tileMatrices.push_back(modelMatrix);
		}
	}
}

float obj3D::distance(const Point &p1, const Point &p2)
{
	float dx = abs(p1.first - p2.first);
	float dy = abs(p1.second - p2.second);

	return sqrt(dx * dx + dy * dy);
}

bool Tree::intersectTree(const Obstacle &o) const
{
	auto t = dynamic_cast<const Tree *>(&o);
	if (t != nullptr) {
		return distance(pos, t->pos) <= r + t->r + 0.1f;
	}
	throw std::exception();
}

bool Tree::intersectBuilding(const Obstacle &o) const
{
	auto b = dynamic_cast<const Building *>(&o);
	if (b != nullptr) {
		return distance(pos, b->pos) <= r + b->l / 3.f + 0.1f;
	}
	throw std::exception();
}

bool Building::intersectTree(const Obstacle &o) const
{
	auto t = dynamic_cast<const Tree *>(&o);
	if (t != nullptr) {
		return distance(pos, t->pos) <= l / 3.f + t->r + 0.1f;
	}
	throw std::exception();
}

bool Building::intersectBuilding(const Obstacle &o) const
{
	auto b = dynamic_cast<const Building *>(&o);
	if (b != nullptr) {
		return distance(pos, b->pos) <= (l + b->l) / 3.f + 0.1f;
	}
	throw std::exception();
}

bool checkPosition(const Obstacle &o, const ObstacleSet &obstacles)
{
	if (obstacles.empty()) {
		return true;
	}

	auto it = std::min_element(obstacles.begin(), obstacles.end(),
		[&o](const ObstaclePtr &a, const ObstaclePtr &b) {
			return distance(a->pos, o.pos) < distance(b->pos, o.pos);
		});
	return !o.intersect(**it);
}

void Terrain::generateObstacles(int nrObstacles)
{
	Drone mock;
	mock.size = DRONE_SIZE;
	mock.pos = glm::vec3(0, 5, 0);

	float rangeX = sizeX / 2.f;
	float rangeZ = sizeZ / 2.f;
	float baseScale = 2.f;

	float maxRY = 2.75f * baseScale / 2.f;
	float maxRXZ = maxRY * 3.f / 5.f;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> distX(-rangeX + maxRXZ / 2.f, rangeX - maxRXZ / 2.f);
	std::uniform_real_distribution<> distZ(-rangeZ + maxRXZ / 2.f, rangeZ - maxRXZ / 2.f);
	std::uniform_real_distribution<> distScale(2.f, 5.f);

	int fail = 0;

	for (int i = 0; i < nrObstacles; i++) {
		if (fail > 50) {
			fail = 0;
			continue;
		}

		float scaleY = distScale(gen) * baseScale;
		float scaleXZ = scaleY * 3.f / 5.f;

		Point pos(distX(gen), distZ(gen));
		std::string name;

		if (i % 6 == 0) {
			name = "Building";

			Building building(pos, scaleY, scaleXZ);
			if (building.hit(mock) || !checkPosition(building, obstacles)) {
				fail++;
				i--;
				continue;
			}

			fail = 0;
			obstacles.insert(std::make_shared<Building>(building));
		} else {
			name = "Tree";

			float r = scaleXZ * 0.5f;
			Tree tree(pos, r, scaleY);

			if (tree.hit(mock) || !checkPosition(tree, obstacles)) {
				fail++;
				i--;
				continue;
			}

			fail = 0;
			obstacles.insert(std::make_shared<Tree>(tree));
		}

		auto modelMatrix = glm::mat4(1);
		if (i % 6 == 0) {
			modelMatrix = glm::translate(modelMatrix, glm::vec3(0, scaleY / 2.f, 0));
		}
		modelMatrix = glm::translate(modelMatrix, glm::vec3(pos.first, 0, pos.second));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleXZ, scaleY, scaleXZ));

		obstacleData.push_back({ name, modelMatrix });
	}
}

void Terrain::generateTarget(float size)
{
	float rangeX = sizeX / 2.f;
	float rangeZ = sizeZ / 2.f;

	std::random_device rd;
	std::mt19937 gen(rd());

	float fact = 9.f / 10.f;

	std::uniform_real_distribution<> distX(-rangeX * fact, rangeX * fact);
	std::uniform_real_distribution<> distZ(-rangeZ * fact, rangeZ * fact);

	target.size = size;
	target.angle = 0;

	float h = target.size / 3.f;

	while (true) {
		Point pos(distX(gen), distZ(gen));
		Building mock(pos, target.size / 2.f, target.size * 3.f / 10.f);

		if (checkPosition(mock, obstacles)) {
			target.pos = glm::vec3(pos.first,
				getTerrainY(pos.first, pos.second) + h, pos.second);
			break;
		}
	}

	while (true) {
		Point pos(distX(gen), distZ(gen));
		Building mock(pos, target.size / 2.f, target.size * 3.f / 10.f);

		auto posV3 = glm::vec3(pos.first, 0, pos.second);

		if (checkPosition(mock, obstacles) && glm::distance(target.pos, posV3) >= 5) {
			target.sendPos = glm::vec3(pos.first,
				getTerrainY(pos.first, pos.second) + h, pos.second);
			target.distance = glm::distance(target.pos, target.sendPos);
			return;
		}
	}
}

void Terrain::generate(int nrTilesX, int nrTilesZ, int nrObstacles, Shader *shader)
{
	tileMatrices.clear();
	obstacleData.clear();
	obstacles.clear();

	sizeX = nrTilesX;
	sizeZ = nrTilesZ;

	generateTiles();
	generateObstacles(nrObstacles);
	generateTarget();

	if (shader != nullptr) {
		creationTime = static_cast<float>(glfwGetTime());

		glUseProgram(shader->program);

		auto loc = glGetUniformLocation(shader->program, "time");
		glUniform1f(loc, creationTime);

		glUseProgram(0);
	}
}

Mesh *Tree::createTree(const std::string &name, glm::vec3 corner, float h, float r)
{
	Mesh *base = createCylinder("", corner, 4.f * h / 5.f, r / 5.f, COLOR_DARK_BROWN);
	Mesh *leaf1 = createCone("", corner + glm::vec3(0, 2.f * h / 5.f, 0),
		3.f * h / 5.f, r, COLOR_GREEN);
	Mesh *leaf2 = createCone("", corner + glm::vec3(0, 4.f * h / 5.f, 0),
		2.f * h / 5.f, r / 2.f, COLOR_GREEN);

	auto tree = combineMeshes(name, { base, leaf1, leaf2 });
	tree->SetDrawMode(GL_TRIANGLES);

	return tree;
}

bool coneHitDrone(glm::vec3 conePos, float coneR, float coneH,
	glm::vec3 dronePos, float droneRXZ, float droneRY)
{
	if (dronePos.y < conePos.y - droneRY
		|| dronePos.y > conePos.y + coneH + droneRY / 2.f) {
		return false;
	}

	float d = abs(dronePos.y - conePos.y);

	float m = (0.05f - coneR) / coneH;
	float n = coneR + 0.01f;

	float r = abs(m * d + n);

	return glm::distance(dronePos, glm::vec3(conePos.x, dronePos.y, conePos.z))
		<= r + droneRXZ;
}

bool cylinderHitDrone(glm::vec3 cylPos, float cylR, float cylH,
	glm::vec3 dronePos, float droneRXZ, float droneRY)
{
	if (dronePos.y < cylPos.y - droneRY
		|| dronePos.y > cylPos.y + cylH + droneRY) {
		return false;
	}
	return glm::distance(dronePos, glm::vec3(cylPos.x, dronePos.y, cylPos.z))
		<= cylR + droneRXZ;
}

bool Building::hit(const Drone &drone) const
{
	float droneRXZ = drone.size * DRONE_L / 2.f;
	float droneRY = drone.size * DRONE_h * 0.75f;

	if (drone.pos.y > h + droneRY && drone.target != nullptr) {
		droneRY += drone.target->size / 1.5f;
	}
	if (drone.pos.y > h + droneRY) {
		return false;
	}

	float actualL = l / 4.f;
	return abs(drone.pos.x - pos.first) <= actualL + droneRXZ
		&& abs(drone.pos.z - pos.second) <= actualL + droneRXZ;
}

bool Tree::hit(const Drone &drone) const
{
	float droneRXZ = drone.size * DRONE_L / 2.f;
	float droneRY = drone.size * DRONE_h * 0.75f;

	auto pos1 = pointAsVec3(pos) + glm::vec3(0, 2.f * h / 5.f, 0);
	auto pos2 = pointAsVec3(pos) + glm::vec3(0, 4.f * h / 5.f, 0);

	if (drone.pos.y > pos1.y && drone.target != nullptr) {
		float targetH = drone.target->size / 1.5f;
		if (coneHitDrone(pos1, r, 3.f * h / 5.f, drone.pos, droneRXZ, droneRY + targetH)) {
			return true;
		}
	} else if (coneHitDrone(pos1, r, 3.f * h / 5.f, drone.pos, droneRXZ, droneRY)) {
		return true;
	}

	if (drone.pos.y > pos1.y && drone.target != nullptr) {
		float targetH = drone.target->size / 1.5f;
		if (coneHitDrone(pos2, r / 2.f, 2.f * h / 5.f, drone.pos, droneRXZ, droneRY + targetH)) {
			return true;
		}
	} else if (coneHitDrone(pos2, r / 2.f, 2.f * h / 5.f, drone.pos, droneRXZ, droneRY)) {
		return true;
	}

	auto pos3 = pointAsVec3(pos);
	return cylinderHitDrone(pos3, r / 5.f, 4.f * h / 5.f, drone.pos, droneRXZ, droneRY);
}

inline float random(glm::vec2 st)
{
	return glm::fract(glm::sin(st.x) + glm::cos(st.y));
}

float makeNoise(glm::vec2 coord, float time)
{
	glm::vec2 i = glm::floor(coord);
	glm::vec2 f = glm::fract(coord);

	// Four corners in 2D of a tile
	float a = random(i + glm::sin(time));
	float b = random(i + glm::sin(time) + glm::vec2(1.0f, 0.0f));
	float c = random(i + glm::sin(time) + glm::vec2(0.0f, 1.0f));
	float d = random(i + glm::sin(time) + glm::vec2(1.0f, 1.0f));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	glm::vec2 u = f * f * (3.0f - 2.0f * f);
	// glm::vec2 u = glm::smoothstep(0.0, 1.0, f);

	// Mix 4 corners percentages
	return glm::mix(a, b, u.x) +
		(c - a) * u.y * (1.0f - u.x) +
		(d - b) * u.x * u.y;
}

inline float Terrain::getTerrainY(float x, float z) const
{
	float noise = makeNoise(glm::vec2(x, z) * 0.5f, creationTime);
	return glm::mix(0.f, TERRAIN_MAX_Y, 1.f - noise);
}
