#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "NavigationGrid.h"
#include "NavigationPath.h"
#include "NavigationMap.h"
#include "StateGameObject.h"
#include "Farmer.h"
#include "Goose.h"
#include "PositionConstraint.h"



namespace NCL {
	namespace CSC8503 {
		class Apple : public GameObject {
		public:
			Apple(GameWorld* world, std::string name = "") : GameObject(name) {
				this->world = world;
				triggerDelete = true;
			};
			void OnCollisionBegin(GameObject* otherObject) override {
				if (otherObject->GetName() == "Goat") {
					this->world->sm->UpdateScore();
					std::cout << this->world->sm->GetScore() << "\n";
					world->RemoveGameObject(this, true);
				}
			}
			GameWorld* world;			
		};

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();
			
			void InitWorld();
			virtual void UpdateGame(float dt);

		protected:

			StateGameObject* AddStateObjectToWorld(const Vector3& position);

			StateGameObject* testStateObject;

			Farmer* farmer1;
			Farmer* farmer2;
			Farmer* farmer3;
			Farmer* farmer4;
			Goose* goose;
			StateGameObject* movingBall;

			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			GameObject* AddOBBCube(const Vector3& position, Vector3 dimensions, float inverseMass);
			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void GrapplingHook();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemy1ToWorld(const Vector3& position);
			GameObject* AddEnemy2ToWorld(const Vector3& position);
			GameObject* AddEnemy3ToWorld(const Vector3& position);
			GameObject* AddEnemy4ToWorld(const Vector3& position);
			GameObject* AddGooseToWorld(const Vector3& position);
			Apple* AddBonusToWorld(const Vector3& position);

			void MovePlayerCharacter(float dt);

			void Reset();

			void AddMazeToWorld();
			vector<Vector3> mazeNodes;
			NavigationGrid* navGrid;

			void BridgeConstraintTest();

			GameObject* AddFenceToWorld(const Vector3& position, const Vector3& scale, const Vector3& rotation, const float inverseMass);

			void AddGateToWorld(const Vector3& position, const Vector3& rotation);

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;
			bool GoatNotSelected;
			bool grappled;

			float		forceMagnitude;

			std::vector<GameObject*> maze;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;
			MeshGeometry*	gooseMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			GameObject* playerCharacter = nullptr;
			//Vector3 lockedOffset		= Vector3(0, 14, 20);
			Vector3 lockedOffset		= Vector3(0, 5, 10);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			float	yaw;
			PositionConstraint* grappleConstraint;

			bool playerisTouchingFloor;
			GameObject* objClosest = nullptr;
		};
	}
}

