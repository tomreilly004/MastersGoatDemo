#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"
#include <GameWorld.h>


using namespace std;

namespace NCL {
    namespace CSC8503 {
        class StateMachine;
        class Farmer : public GameObject {
        public:
            Farmer(std::string filePath, GameObject* goat, Vector3 position);
            ~Farmer() { delete grid; delete stateMachine; }

            virtual void UpdatePosition(GameObject* goat, float dt);

			void SetOrigin(Vector3 position){ origin = position; }

			virtual void OnCollisionBegin(GameObject* otherObject);
			void drawPath();
        protected:

			void Chase(float dt);
			void RunAway(float dt);

            StateMachine* stateMachine;
            float counter;
			NavigationGrid* grid;
			vector<Vector3> nodes;

			float speed = 600.0f;

			Vector3 origin;

			Vector3 targetPosition;

			NavigationPath path;

			Vector3 currentPosition;

			std::string filename;

			Vector3 lastKnownGoatPosition;

			GameObject* scaryGoat = nullptr;

			bool stateSwitch;

			Vector3 destPos;

			bool pathNotFound = true;

			int j = 0;
        };
    }
}
