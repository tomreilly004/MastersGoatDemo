#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Goat : public GameObject {
		public:
			Goat(std::string name = "");
			~Goat();

			virtual void OnCollisionBegin(GameObject* otherObject) {
				if (otherObject->GetName() == "floor") {
					touchingFloor = true;
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				if (otherObject->GetName() == "floor") {
					touchingFloor = false;
				}
			}
		protected:
			bool touchingFloor;
		};
	}
}