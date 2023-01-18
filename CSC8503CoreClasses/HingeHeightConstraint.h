#pragma once
#include "Constraint.h"

using namespace NCL::Maths;

namespace NCL
{
	namespace CSC8503
	{
		class GameObject;

		class HingeHeightConstraint : public Constraint
		{
		public:
			HingeHeightConstraint(GameObject* gameObject, const float heightValue) {
				object = gameObject;
				height = heightValue;
			}
			~HingeHeightConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* object;

			float height;
		};
	}
}