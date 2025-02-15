#pragma once

#include "core/gpu/mesh.h"

#include "../../objects.h"
#include "../../../color.h"

#define DRONE_l 0.25f
#define DRONE_L 1.75f
#define DRONE_h 0.25f
#define DRONE_SIZE 0.4f

namespace obj3D {

	class Target {
	public:
		glm::vec3 pos;
		glm::vec3 sendPos;

		float size = 0;
		float angle = 0;

		float distance;

		glm::mat4 getMatrix() const;
		glm::mat4 getDeliverMatrix() const;

		inline bool deliver() const
		{
			return glm::distance(pos, sendPos) <= size;
		}
	};

	class Drone {
	public:

		static inline std::pair<Mesh *, Mesh *> createDroneMeshes(const std::string &name1,
			const std::string &name2, glm::vec3 center)
		{
			return {
				createBaseMesh(name1, center, DRONE_l, DRONE_L, DRONE_h),
				createRectangleParallelepiped(name2, center, DRONE_l / 3.f, DRONE_L / 7.f, DRONE_h / 4.f, COLOR_BLACK)
			};
		}

		glm::mat4 getBaseMatrix() const;

		std::vector<glm::mat4> getBladeMatrices() const;

		glm::vec3 pos;
		float size = 1;

		float angle = 0;
		float bladeAngle = 0;

		Target *target = nullptr;

		void acquireTarget(Target &target);

	private:
		static Mesh *createBaseMesh(const std::string &name, glm::vec3 center,
			float l, float L, float h);
	};

} // namespace obj3D
