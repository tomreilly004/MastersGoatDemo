#include "Vector3.h"
#include "GameObject.h"
#include "HingeHeightConstraint.h"
#include "Maths.h"

using namespace NCL;
using namespace CSC8503;


void HingeHeightConstraint::UpdateConstraint(float dt)
{
	Vector3 position = object->GetTransform().GetPosition();
	position.y = height;
	object->GetTransform().SetPosition(position);
}