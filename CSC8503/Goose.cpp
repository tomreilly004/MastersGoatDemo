#include "Goose.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourAction.h"
#include "PhysicsObject.h"


Goose::Goose(string filePath, GameWorld* world, GameObject* goat, Vector3 initialPosition) : GameObject()
{
	grid = new NavigationGrid(filePath);
	scaryGoat = goat;
	this->world = world;
	grid->FindPath(initialPosition, scaryGoat->GetTransform().GetPosition(), path);
	path.PopWaypoint(targetPosition);

	BehaviourAction* PatrolMaze = new BehaviourAction("PatrolMaze",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Goose Starting Patrol!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				Vector3 goatPos = scaryGoat->GetTransform().GetPosition();
				Vector3 goosePos = this->GetTransform().GetPosition();
				Vector3 direction = goatPos - goosePos;
				direction.Normalise();
				Ray ray(GetTransform().GetPosition(), direction);
				RayCollision closestCollision;
				if (this->world->Raycast(ray, closestCollision, true, this)) {
					if (closestCollision.node == scaryGoat) {
						std::cout << "Spotted Goat!\n";
						return Success;
					}
					else {
						nodes.clear();
						path.Clear();
						grid->FindPath(GetTransform().GetPosition(), Vector3(60, 5, 10), path);
						while (path.PopWaypoint(nextDest)) {
							nodes.push_back(nextDest);
						}
						drawPath();
						if (nodes.size() >= 2) {
							Vector3 direction = nodes[1] - nodes[0];
							direction.Normalise();
							GetPhysicsObject()->SetForce(direction * speed * dt);
						}
					}
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);
	BehaviourAction* ChaseGoat = new BehaviourAction("ChaseGoat",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Goose Starting Chase\n";
				GetPhysicsObject()->AddForce(Vector3(0, 10, 0) * 100);
				state = Ongoing;
			}
			else if (state == Ongoing) {
				nodes.clear();
				path.Clear();
				if (grid->FindPath(GetTransform().GetPosition(), scaryGoat->GetTransform().GetPosition(), path)) {
					while (path.PopWaypoint(nextDest)) {
						nodes.push_back(nextDest);
					}
					drawPath();
					if (nodes.size() >= 2) {
						Vector3 direction = nodes[1] - nodes[0];
						direction.Normalise();
						GetPhysicsObject()->SetForce(direction * speed * 2 * dt);
					}
				}
				// if lost goat or caught
				if (GoatCaught) {
					std::cout << "Caught Goat!\n";
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		}
	);

	BehaviourAction* BringGoatHome = new BehaviourAction("TakingGoatHome",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Taking Goat Home!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				// add taking goat home behaviour
				if (found) {
					std::cout << "I found some treasure !\n";
					return Success;
				}
				return Failure;
			}
			return state;
		}
	);

	BehaviourAction* ReturnHome = new BehaviourAction("ReturnHome",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Goose Returning home!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				nodes.clear();
				path.Clear();
				targetPosition = origin;
				targetPosition.y = 0;
				if (grid->FindPath(GetTransform().GetPosition(), origin, path)) {
					Vector3 pos;
					while (path.PopWaypoint(pos)) {
						nodes.push_back(pos);
					}
					drawPath();
					if (nodes.size() >= 2) {
						Vector3 direction = nodes[1] - nodes[0];
						direction.Normalise();
						GetPhysicsObject()->SetForce(direction * speed * dt);
					}
				}
				if (GetTransform().GetPosition() == origin) {
					std::cout << "Returned Home\n";
					return Success;
				}
				return Ongoing;
			}
			return state;
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Patrol and Chase");
	sequence->AddChild(PatrolMaze);
	sequence->AddChild(ChaseGoat);

	BehaviourSequence* selection = new BehaviourSequence("Return and Home");
	//selection->AddChild(BringGoatHome);
	selection->AddChild(ReturnHome);

	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);
	rootSequence->Reset();
}

void Goose::UpdatePosition(GameObject* goat, float dt)
{
	scaryGoat = goat;
	
	BehaviourState state = Ongoing;
	state = rootSequence->Execute(dt); 
	if (state == Success) {
		std::cout << "GOOSE SUCCESS\n";
		rootSequence->Reset();
	}
	else if (state == Failure) {
		std::cout << "GOOSE FAILURE\n";
		rootSequence->Reset();
	}
}

void Goose::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "Goat"){
		world->RemoveGameObject(scaryGoat);
		GoatCaught = true;
	}
}


void Goose::drawPath()
{
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		Debug::DrawLine(nodes[i] + Vector3(0, 1, 0), nodes[i + 1] + Vector3(0, 1, 0), Vector4(1, 0, 1, 1));
	}
}

