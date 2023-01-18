#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;
		using namespace Maths;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, Vector3 orientation);
			~OrientationConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;

			Vector3 orientation;
		};
	}
}

