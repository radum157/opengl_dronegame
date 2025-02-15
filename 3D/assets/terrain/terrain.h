#pragma once

#include <vector>
#include <set>
#include <memory>

#include "core/gpu/mesh.h"
#include "core/gpu/shader.h"

#include "../drone/drone.h"

#define TERRAIN_MAX_Y 0.5f

#define BUILDING_h 1.f
#define BUILDING_L 0.5f

typedef std::pair<float, float> Point;

namespace obj3D {

	float distance(const Point &p1, const Point &p2);
	inline glm::vec3 pointAsVec3(const Point &p)
	{
		return glm::vec3(p.first, 0, p.second);
	}

	class Obstacle {
	public:
		Point pos;

		Obstacle() {}
		virtual ~Obstacle() = default;
		Obstacle(const Point &pos) : pos(pos) {}

		/**
		 * !!! No 2 obstacles should have the same @a pos !!!
		 */
		inline bool operator<(const Obstacle &o) const
		{
			return pos < o.pos;
		}

		virtual bool hit(const Drone &drone) const = 0;
		virtual bool intersect(const Obstacle &o) const = 0;

		virtual bool intersectTree(const Obstacle &t) const = 0;
		virtual bool intersectBuilding(const Obstacle &b) const = 0;
	};

	class Tree : public Obstacle {
	public:
		float r = 0;
		float h = 0;

		Tree() : Obstacle() {};
		Tree(const Point &pos, float r, float h) : Obstacle(pos), r(r), h(h) {}

		bool hit(const Drone &drone) const override;
		inline bool intersect(const Obstacle &o) const override
		{
			return o.intersectTree(*this);
		}

		static Mesh *createTree(const std::string &name, glm::vec3 corner, float h, float r);

		bool intersectTree(const Obstacle &t) const override;
		bool intersectBuilding(const Obstacle &b) const override;
	};

	class Building : public Obstacle {
	public:
		float h = 1.f;
		float l = 1.f;

		Building() : Obstacle() {};
		Building(const Point &pos, float h, float l) : Obstacle(pos), h(h), l(l) {}

		bool hit(const Drone &drone) const override;
		inline bool intersect(const Obstacle &o) const override
		{
			return o.intersectBuilding(*this);
		}

		inline static Mesh *createBuilding(const std::string &name, glm::vec3 center)
		{
			return createRectangleParallelepiped(name, center, BUILDING_L, BUILDING_L, BUILDING_h, COLOR_DARK_GREY, 0);
		}

		bool intersectTree(const Obstacle &t) const override;
		bool intersectBuilding(const Obstacle &b) const override;
	};

	struct ObstacleComp {
		inline bool operator()(const std::shared_ptr<Obstacle> &lhs, const std::shared_ptr<Obstacle> &rhs) const
		{
			return lhs < rhs;
		}
	};

	typedef std::shared_ptr<Obstacle> ObstaclePtr;
	typedef std::set<ObstaclePtr, ObstacleComp> ObstacleSet;

	typedef std::pair<std::string, glm::mat4> ObstacleData;

	class Terrain {
	public:
		Target target;

		Terrain() {}

		void generate(int nrTilesX, int nrTilesZ, int nrTrees, Shader *shader);

		inline const std::vector<glm::mat4> &getTileMatrices() const
		{
			return tileMatrices;
		}
		inline const std::vector<ObstacleData> &getObstacleData() const
		{
			return obstacleData;
		}

		inline bool hit(const Drone &drone) const
		{
			return std::find_if(obstacles.begin(), obstacles.end(), [&drone](const ObstaclePtr &o) {
				return o->hit(drone);
				}) != obstacles.end();
		}

		void generateTarget(float size = 0.3f);
		float getTerrainY(float x, float z) const;

	private:
		std::vector<glm::mat4> tileMatrices;
		std::vector<ObstacleData> obstacleData;
		ObstacleSet obstacles;

		float creationTime = 0;

		int sizeX = 0;
		int sizeZ = 0;

		void generateTiles();
		void generateObstacles(int nrObstacles);
	};

} // namespace obj3D
