#include "Farmer.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "PhysicsObject.h"


Farmer::Farmer(string filePath, GameObject* goat, Vector3 initialPosition) : GameObject()
{
	scaryGoat = goat;
	lastKnownGoatPosition = scaryGoat->GetTransform().GetPosition();
	filename = filePath;
	grid = new NavigationGrid(filename);
	grid->FindPath(initialPosition, scaryGoat->GetTransform().GetPosition(), path);
	path.PopWaypoint(destPos);

	stateMachine = new StateMachine();

	State* ChaseState = new State([&](float dt) -> void
		{
			//std::cout << "Chasing!!!!!!!!!!!!!";
			this->Chase(dt);
		}
	);
	State* RunState = new State([&](float dt) -> void
		{
			//std::cout << "Running!!!!!!!!!!!!!!";
			this->RunAway(dt);
		}
	);

	stateMachine->AddState(ChaseState);
	stateMachine->AddState(RunState);

	stateMachine->AddTransition(new StateTransition(ChaseState, RunState,
		[&]() -> bool
		{
			if (this->counter > 30.0f) {
				return true;
			}
			else {
				return false;
			}
		}
	));

	stateMachine->AddTransition(new StateTransition(RunState, ChaseState,
		[&]() -> bool
		{
			if (this->counter < 2.0f) {
				return true;
			}
			else {
				return false;
			}
		}
	));
}

void Farmer::UpdatePosition(GameObject* goat, float dt)
{
	scaryGoat = goat;
	currentPosition = GetTransform().GetPosition();
	stateMachine->Update(dt);
}

void Farmer::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "Goat"){
		std::cout << "Caught";
	}
}

void Farmer::Chase(float dt)
{
	nodes.clear();
	path.Clear();
	grid->FindPath(currentPosition, scaryGoat->GetTransform().GetPosition(), path);
	while (path.PopWaypoint(destPos))
	{
		nodes.push_back(destPos);
	}
	//drawPath();
	if (nodes.size() >= 2){
		Vector3 direction = nodes[1] - nodes[0];
		direction.Normalise();
		direction.y = 0;
		GetPhysicsObject()->SetForce(direction * speed *dt);
	}

	counter += dt;
}

void Farmer::RunAway(float dt)
{
	nodes.clear();
	path.Clear();
	grid->FindPath(currentPosition, origin, path);
	while (path.PopWaypoint(destPos))
	{
		nodes.push_back(destPos);
	}
	drawPath();
	if (nodes.size() >= 2) {
		Vector3 direction = nodes[1] - nodes[0];
		direction.Normalise();
		direction.y = 0;
		GetPhysicsObject()->SetForce(direction * speed * dt);
	}

	counter += dt;
}

void Farmer::drawPath()
{
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		Debug::DrawLine(nodes[i] + Vector3(0, 1, 0), nodes[i + 1] + Vector3(0, 1, 0), Vector4(1, 0, 0, 1));
	}
}