#pragma once

#include "core/gpu/mesh.h"

namespace obj3D {

	Mesh *combineMeshes(const std::string &name, std::initializer_list<Mesh *> meshes);

	/**
	 * Center is at half the left side (height)
	 */
	Mesh *createRectangle(const std::string &name, glm::vec3 corner, float h, float L, glm::vec3 color);
	Mesh *createRectangleParallelepiped(const std::string &name, glm::vec3 center,
		float l, float L, float h, glm::vec3 color, float angle = 0);

	inline Mesh *createCube(const std::string &name, glm::vec3 center, float l,
		glm::vec3 color, float angle)
	{
		return createRectangleParallelepiped(name, center, l, l, l, color, angle);
	}

	Mesh *createCylinder(const std::string &name, glm::vec3 center,
		float h, float r, glm::vec3 color);
	Mesh *createCone(const std::string &name, glm::vec3 center,
		float h, float r, glm::vec3 color);

} // namespace obj3D
