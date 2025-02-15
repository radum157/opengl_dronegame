#pragma once

#include "utils/glm_utils.h"
#include "utils/math_utils.h"

namespace implemented {
	class GameCamera {
	public:
		GameCamera()
		{
			forward = glm::vec3(0, 0, -1);
			up = glm::vec3(0, 1, 0);
			right = glm::vec3(1, 0, 0);

			distanceToTarget = 2;
		}

		GameCamera(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
		{
			Set(position, center, up);
		}

		~GameCamera()
		{}

		// Update camera
		void Set(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
		{
			this->position = position;
			forward = glm::normalize(center - position);
			right = glm::cross(forward, up);
			this->up = glm::cross(right, forward);
		}

		void MoveForward(float distance)
		{
			glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
			position += dir * distance;
		}

		void TranslateForward(float distance)
		{
			position += distance * glm::normalize(forward);
		}

		void TranslateUpward(float distance)
		{
			position += distance * glm::normalize(up);
		}

		void TranslateRight(float distance)
		{
			position += distance * glm::normalize(glm::cross(up, forward));
		}

		void RotateFirstPerson_OX(float angle)
		{
			auto rfwd = glm::rotate(glm::mat4(1), angle, glm::normalize(right)) * glm::vec4(forward, 1);
			forward = glm::normalize(glm::vec3(rfwd));

			auto rup = glm::rotate(glm::mat4(1), angle, glm::normalize(right)) * glm::vec4(up, 1);
			up = glm::normalize(glm::vec3(rup));
		}

		void RotateFirstPerson_OY(float angle)
		{
			auto rfwd = glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) * glm::vec4(forward, 1);
			forward = glm::normalize(glm::vec3(rfwd));

			auto rright = glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0)) * glm::vec4(right, 1);
			right = glm::normalize(glm::vec3(rright));

			up = glm::cross(right, forward);
		}

		void RotateFirstPerson_OZ(float angle)
		{
			auto rright = glm::rotate(glm::mat4(1), angle, glm::normalize(forward)) * glm::vec4(right, 1);
			right = glm::normalize(glm::vec3(rright));

			auto rup = glm::rotate(glm::mat4(1), angle, glm::normalize(forward)) * glm::vec4(up, 1);
			up = glm::normalize(glm::vec3(rup));
		}

		void RotateThirdPerson_OX(float angle)
		{
			TranslateForward(distanceToTarget);

			RotateFirstPerson_OX(angle);

			TranslateForward(-distanceToTarget);
		}

		void RotateThirdPerson_OY(float angle)
		{
			TranslateForward(distanceToTarget);

			RotateFirstPerson_OY(angle);

			TranslateForward(-distanceToTarget);
		}

		void RotateThirdPerson_OZ(float angle)
		{
			TranslateForward(distanceToTarget);

			RotateFirstPerson_OZ(angle);

			TranslateForward(-distanceToTarget);
		}

		glm::mat4 GetViewMatrix()
		{
			// Returns the view matrix
			return glm::lookAt(position, position + forward, up);
		}

		glm::vec3 GetTargetPosition()
		{
			return position + forward * distanceToTarget;
		}

	public:
		float distanceToTarget;
		glm::vec3 position;
		glm::vec3 forward;
		glm::vec3 right;
		glm::vec3 up;
	};
}   // namespace implemented
