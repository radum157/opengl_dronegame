#include "drone.h"

using namespace obj3D;

Mesh *Drone::createBaseMesh(const std::string &name, glm::vec3 center,
	float l, float L, float h)
{
	float angles[2] = { RADIANS(45), RADIANS(-45) };

	Mesh *ends[4];

	for (int i = 0; i < 4; i++) {
		float di = (i > 1) ? 1 : -1;
		float dj = 1 - (i % 2) * 2;
		float angle = angles[(i > 1) ? 1 - i % 2 : i % 2];

		float dx = di * abs(cos(angle)) * (L / 2.f);
		float dz = dj * abs(sin(angle)) * (L / 2.f);

		ends[i] = createRectangleParallelepiped("", center + glm::vec3(dx, 0, dz),
			l + 0.1f, L / 10.f, h * 1.5f, COLOR_LIGHT_GREY, angle);
	}

	auto p1 = createRectangleParallelepiped("", center, l, L, h, COLOR_LIGHT_GREY, angles[0]);
	auto p2 = createRectangleParallelepiped("", center, l, L, h, COLOR_LIGHT_GREY, angles[1]);

	auto res = combineMeshes(name, { ends[0], ends[1], ends[2], ends[3], p1, p2 });
	res->SetDrawMode(GL_TRIANGLES);

	return res;
}

glm::mat4 Drone::getBaseMatrix() const
{
	if (target != nullptr) {
		target->angle = angle;
		target->pos = pos - glm::vec3(0, DRONE_h * size / 2.f + target->size / 3.f + 0.01f, 0);
	}

	auto modelMatrix = glm::mat4(1);

	modelMatrix = glm::translate(modelMatrix, pos);
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(size));

	return modelMatrix;
}

std::vector<glm::mat4> Drone::getBladeMatrices() const
{
	std::vector<glm::mat4> res;
	float halfL = DRONE_L * size / 2.f;
	float h = size * (DRONE_L / 10.f + DRONE_h / 8.f + 0.02f);

	float angleSgn[2] = { 1, -1 };

	for (int i = 0; i < 4; i++) {
		float di = (i > 1) ? 1 : -1;
		float dj = 1 - (i % 2) * 2;

		float sideAngle = RADIANS(45) +
			angleSgn[(i > 1) ? 1 - i % 2 : i % 2] * angle;

		float dx = di * cos(sideAngle) * halfL;
		float dz = dj * sin(sideAngle) * halfL;

		auto posi = pos + glm::vec3(dx, h, dz);
		auto mat = glm::mat4(1);

		mat = glm::translate(mat, posi);
		mat = glm::rotate(mat, angle + bladeAngle, glm::vec3(0, 1, 0));
		mat = glm::scale(mat, glm::vec3(size * 2.f, size, size));

		res.push_back(mat);
	}

	return res;
}

glm::mat4 Target::getMatrix() const
{
	auto modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, pos);
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(size, size / 1.5f, size));

	return modelMatrix;
}

glm::mat4 Target::getDeliverMatrix() const
{
	auto modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, sendPos);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(size, size / 1.5f, size));

	return modelMatrix;
}

void Drone::acquireTarget(Target &target)
{
	if (this->target != nullptr) {
		return;
	}

	float halfH = DRONE_h * size / 2.f;
	float targetH = target.size / 1.5f;
	float targetL = target.size / 2.f;

	if (target.pos.y + targetH >= pos.y) {
		return;
	}

	if (pos.y - halfH > target.pos.y + targetH + 0.2f) {
		return;
	}

	if (abs(pos.x - target.pos.x) <= targetL + 0.2f
		&& abs(pos.z - target.pos.z) <= targetL + 0.2f) {
		this->target = &target;
	}
}
