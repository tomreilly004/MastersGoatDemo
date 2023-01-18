#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, Vector3 o)
{
	objectA = a;
	orientation = o;
}

OrientationConstraint::~OrientationConstraint()
{

}

void OrientationConstraint::UpdateConstraint(float dt) {
    Vector3 oriB = objectA->GetTransform().GetOrientation() * orientation;

    Vector3 offsetDir = (oriB).Normalised();
    float offset = 1 - Vector3::Dot(offsetDir, orientation);

    if (abs(offset) > 0.0f) {
        PhysicsObject* physB = objectA->GetPhysicsObject();

        Vector3 relativeVelocity = Vector3::Cross(-physB->GetAngularVelocity(), orientation);

        float constraintMass = physB->GetInverseMass();

        if (constraintMass > 0.0f) {
            float velocityDot = Vector3::Dot(relativeVelocity, oriB);
            float biasFactor = 0.01f;
            float bias = -(biasFactor / dt) * offset;

            Vector3 correction = Vector3::Cross(orientation, offsetDir);

            float lambda = -(velocityDot + bias) / constraintMass;

            Vector3 aImpulse = correction * lambda;
            Vector3 bImpulse = -correction * lambda;

            physB->ApplyAngularImpulse(bImpulse);
        }
    }
}