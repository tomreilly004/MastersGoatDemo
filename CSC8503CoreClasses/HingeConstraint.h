#pragma once
#include "Constraint.h"

namespace NCL
{
	

	namespace CSC8503
	{
		using namespace Maths;
		class GameObject;

		class HingeConstraint : public Constraint
		{
		public:
			HingeConstraint(GameObject* a, GameObject* b) 
				{
					objectA = a;
					objectB = b;
				}
			
			~HingeConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
		};
	}
}

