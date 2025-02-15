#include "objects.h"

#include <vector>

using std::vector;

Mesh *obj3D::combineMeshes(const std::string &name, std::initializer_list<Mesh *> meshes)
{
	vector<VertexFormat> vRes;
	vector<unsigned int> iRes;
	size_t off = 0;

	for (Mesh *mesh : meshes) {
		vRes.insert(vRes.end(), mesh->vertices.begin(), mesh->vertices.end());

		for (unsigned int idx : mesh->indices) {
			iRes.push_back(idx + off);
		}

		off = vRes.size();
	}

	Mesh *res = new Mesh(name);
	res->InitFromData(vRes, iRes);

	return res;
}

/**
 * Center is at bottom left corner
 */
Mesh *obj3D::createRectangle(const std::string &name, glm::vec3 corner, float h, float L, glm::vec3 color)
{
	vector<VertexFormat> vertices =
	{
		VertexFormat(corner + glm::vec3(0, 0, h), color),
		VertexFormat(corner + glm::vec3(L, 0, h), color),
		VertexFormat(corner + glm::vec3(L, 0, 0), color),
		VertexFormat(corner + glm::vec3(0, 0, 0), color)
	};

	Mesh *rectangle = new Mesh(name);
	vector<unsigned int> indices = { 0, 1, 2, 3, 0, 2 };

	rectangle->InitFromData(vertices, indices);
	return rectangle;
}

Mesh *obj3D::createCylinder(const std::string &name, glm::vec3 center,
	float h, float r, glm::vec3 color)
{
	vector<VertexFormat> vertices;
	vector<unsigned int> indices;

	int nrSegments = 36;
	float angleStep = 2.0f * glm::pi<float>() / nrSegments;

	for (int i = 0; i <= nrSegments; i++) {
		float angle = i * angleStep;
		float x = cos(angle) * r;
		float z = sin(angle) * r;

		vertices.push_back(VertexFormat(center + glm::vec3(x, h, z), color));
		vertices.push_back(VertexFormat(center + glm::vec3(x, 0, z), color));
	}

	for (int i = 0; i < nrSegments; i++) {
		int topCurrent = i * 2;
		int bottomCurrent = topCurrent + 1;
		int topNext = ((i + 1) % nrSegments) * 2;
		int bottomNext = topNext + 1;

		indices.push_back(topCurrent);
		indices.push_back(bottomCurrent);
		indices.push_back(topNext);

		indices.push_back(bottomCurrent);
		indices.push_back(bottomNext);
		indices.push_back(topNext);
	}

	for (int i = 0; i < nrSegments; i++) {
		int topCenter = 0;
		int bottomCenter = 1;

		indices.push_back(topCenter + (i * 2));
		indices.push_back(topCenter + ((i + 1) * 2));
		indices.push_back(topCenter);

		indices.push_back(bottomCenter + 1);
		indices.push_back(bottomCenter + ((i + 1) * 2) + 1);
		indices.push_back(bottomCenter + ((i) * 2));
	}

	Mesh *cylinder = new Mesh(name);
	cylinder->InitFromData(vertices, indices);
	cylinder->SetDrawMode(GL_TRIANGLES);

	return cylinder;
}

Mesh *obj3D::createCone(const std::string &name, glm::vec3 center,
	float h, float r, glm::vec3 color)
{
	vector<VertexFormat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(VertexFormat(center, color));
	vertices.push_back(VertexFormat(center + glm::vec3(0, h, 0), color - glm::vec3(0.15f)));

	int nrSegments = 36;
	float angleStep = 2.0f * glm::pi<float>() / nrSegments;

	for (int i = 0; i <= nrSegments; i++) {
		float angle = i * angleStep;
		float x = cos(angle) * r;
		float z = sin(angle) * r;

		vertices.push_back(VertexFormat(center + glm::vec3(x, 0, z), color));

		if (i > 0) {
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i + 2);

			indices.push_back(1);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}
	}

	Mesh *circle = new Mesh(name);
	circle->InitFromData(vertices, indices);
	circle->SetDrawMode(GL_TRIANGLES);

	return circle;
}

Mesh *obj3D::createRectangleParallelepiped(const std::string &name, glm::vec3 center,
	float l, float L, float h, glm::vec3 color, float angle)
{
	vector<VertexFormat> vertices;
	vector<unsigned int> indices;

	float halfL = L / 2.0f;
	float halfl = l / 2.0f;
	float halfH = h / 2.0f;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

	vertices = {
		VertexFormat(glm::vec3(-halfL, -halfH, -halfl)),
		VertexFormat(glm::vec3(-halfL, -halfH, halfl)),
		VertexFormat(glm::vec3(halfL, -halfH, -halfl)),
		VertexFormat(glm::vec3(halfL, -halfH, halfl)),

		VertexFormat(glm::vec3(-halfL, halfH, -halfl)),
		VertexFormat(glm::vec3(-halfL, halfH, halfl)),
		VertexFormat(glm::vec3(halfL, halfH, -halfl)),
		VertexFormat(glm::vec3(halfL, halfH, halfl))
	};

	for (auto &&vertex : vertices) {
		auto rotated = rotationMatrix * glm::vec4(vertex.position, 1.f);

		vertex.position = center + glm::vec3(rotated);
		vertex.color = color;
	}

	indices = {
		0, 2, 1, 1, 2, 3, // top
		4, 6, 5, 5, 6, 7, // bottom

		0, 4, 1, 1, 5, 4, // left
		2, 0, 6, 0, 4, 6, // right

		1, 3, 5, 3, 7, 5, // front
		3, 2, 7, 2, 6, 7 // back
	};

	Mesh *rectangle = new Mesh(name);
	rectangle->InitFromData(vertices, indices);
	rectangle->SetDrawMode(GL_TRIANGLES);

	return rectangle;
}
