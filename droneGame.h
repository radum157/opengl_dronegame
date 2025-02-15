#pragma once

#include "components/simple_scene.h"
#include "components/text_renderer.h"

#include "lab_m1/tema2/gameCamera.h"
#include "3D/objects.h"
#include "3D/assets/terrain/terrain.h"
#include "3D/assets/drone/drone.h"

using obj3D::Drone;

namespace m1 {
	class DroneGame : public gfxc::SimpleScene {
	public:
		DroneGame();
		~DroneGame();

		void Init() override;

	private:
		struct ViewportArea {
			ViewportArea() : x(0), y(0), width(1), height(1) {}
			ViewportArea(int x, int y, int width, int height)
				: x(x), y(y), width(width), height(height)
			{}
			int x;
			int y;
			int width;
			int height;
		};

		void FrameStart() override;
		void Update(float deltaTimeSeconds) override;
		void FrameEnd() override;

		void RenderMesh(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix) override;

		void OnInputUpdate(float deltaTime, int mods) override;
		void OnKeyPress(int key, int mods) override;
		void OnKeyRelease(int key, int mods) override;
		void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
		void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
		void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
		void OnWindowResize(int width, int height) override;

		void RenderScene(float scale = 1.f);

		void moveInput(float deltaTime);

		void addShaders();
		void addMeshes();

		void moveForward(float distance);
		void moveRight(float distance);
		void moveUp(float distance);

		void restart();

		void checkPosHit(glm::vec3 oldPos);

		void displayIndicator();

		glm::vec3 keepInBounds(glm::vec3 pos);
		void updateShaders();

	protected:
		implemented::GameCamera *camera;
		bool fstPerson;

		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

		ViewportArea miniViewport;

		obj3D::Terrain terrain;

		Drone drone;
		float speedFactor;

		std::string fowShader;

		bool enableUI;

		gfxc::TextRenderer textRenderer;
		int feedback;
		int score;
	};
}   // namespace m1
