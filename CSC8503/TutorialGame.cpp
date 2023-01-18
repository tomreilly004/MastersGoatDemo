#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "HingeConstraint.h"
#include "HingeHeightConstraint.h"
#include "Assets.h"
#include <fstream>


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;
	GoatNotSelected = true;
	grappled = false;
	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	gooseMesh = renderer->LoadMesh("goose.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::GrapplingHook() {
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::RIGHT)) {

		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true) && !grappled) {
			selectionObject = (GameObject*)closestCollision.node;
			grappleConstraint = new PositionConstraint(playerCharacter, selectionObject, 20);
			selectionObject->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
			world->AddConstraint(grappleConstraint);
			grappled = true;
		} 
	}
	if (grappled && Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::SPACE)) {
		grappled = false;
		selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		selectionObject = nullptr;
		world->RemoveConstraint(grappleConstraint);
	}
	else if (grappled) {
		Debug::DrawLine(playerCharacter->GetTransform().GetPosition(), 
						selectionObject->GetTransform().GetPosition(), Vector4(1,1,0,1));
	}
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (lockedObject != nullptr && lockedObject->GetName() == "Goat") {
		GoatNotSelected = false;
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + playerCharacter->GetTransform().GetOrientation() * lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetYaw(angles.y);
		MovePlayerCharacter(dt);
		GrapplingHook();
	}
	movingBall->Update(dt);
	if (goose->GoatCaught == true)
	{
		//do {
		//	Debug::Print("You lost to a goose press F to start over", Vector2(5, 65), Debug::RED);
		//} while (!Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::F));

		InitWorld();
		InitCamera();
	}

	if (this->world->sm->GetScore() == 20)
	{
		Debug::Print("You have won the game! Hooray", Vector2(5, 45), Debug::YELLOW);
	}

	if (testStateObject) {
		testStateObject->Update(dt);
	}

	if (farmer1 && playerCharacter) {
		farmer1->UpdatePosition(playerCharacter, dt);
	}
	if (farmer2 && playerCharacter) {
		farmer2->UpdatePosition(playerCharacter, dt);
	}
	if (farmer3 && playerCharacter) {
		farmer3->UpdatePosition(playerCharacter, dt);
	}
	if (farmer4 && playerCharacter) {
		farmer4->UpdatePosition(playerCharacter, dt);
	}
	if (goose && playerCharacter) {
		goose->UpdatePosition(playerCharacter, dt);
	}
	UpdateKeys();

	int score = world->sm->GetScore();
	
	Debug::Print("Score " + std::to_string(score), Vector2(50, 10), Debug::WHITE);

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;
			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));

	SelectObject();
	MoveSelectedObject();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);
}


void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		GoatNotSelected = true;
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		GoatNotSelected = true;
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		//LockedObjectMovement();
	}
	else {
		//DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	InitCamera(); 
	world->sm->ResetScore();
	world->ClearAndErase();
	physics->Clear();

	useGravity = false;
	inSelectionMode = true;
	GoatNotSelected = true;
	grappled = false;
	selectionObject = nullptr;
	lockedObject = nullptr;

	InitDefaultFloor();
	AddOBBCube({ -10, 20, 0 }, { 1, 1, 1 }, 1);
	AddOBBCube({ -10, 20, 5 }, { 1, 1, 1 }, 1);
	AddMazeToWorld();
}

void TutorialGame::Reset() {
	world->ClearAndErase();
	physics->Clear();
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	movingBall = new StateGameObject();

	AABBVolume* volume = new AABBVolume({5,5,5 });
	movingBall->SetBoundingVolume((CollisionVolume*)volume);
	movingBall->GetTransform().SetScale(Vector3(3, 3, 3)).SetPosition(position);

	movingBall->SetRenderObject(new RenderObject(&movingBall->GetTransform(), sphereMesh, nullptr, basicShader));
	movingBall->SetPhysicsObject(new PhysicsObject(&movingBall->GetTransform(), movingBall->GetBoundingVolume()));

	movingBall->GetPhysicsObject()->SetInverseMass(1.0f);
	movingBall->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(movingBall);

	return movingBall;
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();
	floor->GetPhysicsObject()->SetStiffness(600);

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform().SetScale(sphereSize).SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	//cube->GetPhysicsObject()->SetElasticity(1.0f);
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCube(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform().SetPosition(position).SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	cube->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(1+rand()%100, 1 + rand() % 100, 1 + rand() % 100));
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	//cube->GetPhysicsObject()->SetElasticity(1.0f);
	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 1.0f;

	playerCharacter = new GameObject("Goat");
	SphereVolume* volume = new SphereVolume(1.0);
	playerCharacter->SetBoundingVolume((CollisionVolume*)volume);

	playerCharacter->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	playerCharacter->SetRenderObject(new RenderObject(&playerCharacter->GetTransform(), charMesh, nullptr, basicShader));
	playerCharacter->SetPhysicsObject(new PhysicsObject(&playerCharacter->GetTransform(), playerCharacter->GetBoundingVolume()));

	playerCharacter->GetPhysicsObject()->SetInverseMass(inverseMass);
	playerCharacter->GetPhysicsObject()->InitSphereInertia();
	playerCharacter->GetPhysicsObject()->SetStiffness(0.1);
	playerCharacter->GetPhysicsObject()->SetElasticity(1.0f);
	playerCharacter->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	world->AddGameObject(playerCharacter);
	
	return playerCharacter;
}

void TutorialGame::MovePlayerCharacter(float dt){
	float rotationSpeed = 60.0f;
	Vector3 pyr = playerCharacter->GetTransform().GetOrientation().ToEuler();

	yaw -= (Window::GetMouse()->GetRelativePosition().x);
	Quaternion q = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), yaw);
	playerCharacter->GetTransform().SetOrientation(q);
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::A))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() *
			Vector3(-1, 0, 0) * forceMagnitude);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::W))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * Vector3(0, 0, -1) * forceMagnitude );
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::S))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * Vector3(0, 0, 1) * forceMagnitude /** playerCharacter->GetSpeedMultiplier()*/);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::D))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * 
			Vector3(1, 0, 0) * forceMagnitude );
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::SPACE) && playerCharacter->isTouchingFloor())
	{
		playerCharacter->GetPhysicsObject()->AddForce(Vector3(0, 10, 0) * 50);
	}
}

GameObject* TutorialGame::AddEnemy1ToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 1.0f;

	farmer1 = new Farmer("TestGrid1.txt", playerCharacter, position);
	farmer1->SetOrigin(position);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	farmer1->SetBoundingVolume((CollisionVolume*)volume);

	farmer1->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	farmer1->SetRenderObject(new RenderObject(&farmer1->GetTransform(), enemyMesh, nullptr, basicShader));
	farmer1->SetPhysicsObject(new PhysicsObject(&farmer1->GetTransform(), farmer1->GetBoundingVolume()));

	farmer1->GetPhysicsObject()->SetInverseMass(inverseMass);
	farmer1->GetPhysicsObject()->InitSphereInertia();
	farmer1->frictionOn = true;
	farmer1->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	world->AddGameObject(farmer1);

	return farmer1;
}

GameObject* TutorialGame::AddEnemy2ToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 1.0f;

	farmer2 = new Farmer("TestGrid1.txt", playerCharacter, position);
	farmer2->SetOrigin(position);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	farmer2->SetBoundingVolume((CollisionVolume*)volume);

	farmer2->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	farmer2->SetRenderObject(new RenderObject(&farmer2->GetTransform(), enemyMesh, nullptr, basicShader));
	farmer2->SetPhysicsObject(new PhysicsObject(&farmer2->GetTransform(), farmer2->GetBoundingVolume()));

	farmer2->GetPhysicsObject()->SetInverseMass(inverseMass);
	farmer2->GetPhysicsObject()->InitSphereInertia();
	farmer2->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	farmer2->frictionOn = true;
	world->AddGameObject(farmer2);

	return farmer2;
}

GameObject* TutorialGame::AddEnemy3ToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 1.0f;

	farmer3 = new Farmer("TestGrid1.txt", playerCharacter, position);
	farmer3->SetOrigin(position);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	farmer3->SetBoundingVolume((CollisionVolume*)volume);

	farmer3->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	farmer3->SetRenderObject(new RenderObject(&farmer3->GetTransform(), enemyMesh, nullptr, basicShader));
	farmer3->SetPhysicsObject(new PhysicsObject(&farmer3->GetTransform(), farmer3->GetBoundingVolume()));

	farmer3->GetPhysicsObject()->SetInverseMass(inverseMass);
	farmer3->GetPhysicsObject()->InitSphereInertia();
	farmer3->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	farmer3->frictionOn = true;
	world->AddGameObject(farmer3);

	return farmer3;
}

GameObject* TutorialGame::AddEnemy4ToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 1.0f;

	farmer4 = new Farmer("TestGrid1.txt", playerCharacter, position);
	farmer4->SetOrigin(position);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	farmer4->SetBoundingVolume((CollisionVolume*)volume);

	farmer4->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	farmer4->SetRenderObject(new RenderObject(&farmer4->GetTransform(), enemyMesh, nullptr, basicShader));
	farmer4->SetPhysicsObject(new PhysicsObject(&farmer4->GetTransform(), farmer4->GetBoundingVolume()));

	farmer4->GetPhysicsObject()->SetInverseMass(inverseMass);
	farmer4->GetPhysicsObject()->InitSphereInertia();
	farmer4->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	farmer4->frictionOn = true;
	world->AddGameObject(farmer4);

	return farmer4;
}

GameObject* TutorialGame::AddGooseToWorld(const Vector3& position) {
	float meshSize = 1.0;
	float inverseMass = 1.0f;

	goose = new Goose("TestGrid1.txt", world, playerCharacter, position);
	goose->SetOrigin(position);

	SphereVolume* volume = new SphereVolume(1.0);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();
	goose->frictionOn = false;
	goose->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	world->AddGameObject(goose);

	return goose;
}

Apple* TutorialGame::AddBonusToWorld(const Vector3& position) {
	Apple* apple = new Apple(world);

	SphereVolume* volume = new SphereVolume();
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetScale(Vector3(1, 1, 1)).SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), sphereMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->GetRenderObject()->SetColour({ 1,0,1,1 });
	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::AddMazeToWorld() {
	int nodeSize;
	int gridWidth;
	int gridHeight;

	std::ifstream infile(Assets::DATADIR + "TestGrid1.txt");

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	navGrid = new NavigationGrid("TestGrid1.txt");

	GridNode* nodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = nodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3((float)(x * nodeSize ), 7, (float)(y * nodeSize ));
			// x = 120 in decimal
			if (type == 120)maze.emplace_back(AddCubeToWorld(n.position, { (float)nodeSize / 2,(float)nodeSize / 2,(float)nodeSize / 2 }, 0));
			// h = 104 gate
			if (type == 104)AddGateToWorld(Vector3(n.position.x, n.position.y -2, n.position.z + 4), Vector3(0,90,0));
			if (type == 106)AddGateToWorld(Vector3(n.position.x - 4, n.position.y -2, n.position.z), Vector3(0,0,0));
			// g = 103 goose
			if (type == 103)maze.emplace_back(AddGooseToWorld(n.position));
			if (type == 105)maze.emplace_back(AddStateObjectToWorld(Vector3(n.position.x, n.position.y + 40, n.position.z)));
			// p = 112 goat
			if (type == 112)maze.emplace_back(AddPlayerToWorld(n.position));
			// farmers
			if (type == 97)maze.emplace_back(AddEnemy1ToWorld(n.position));
			if (type == 98)maze.emplace_back(AddEnemy2ToWorld(n.position));
			if (type == 99)maze.emplace_back(AddEnemy3ToWorld(n.position));
			if (type == 100)maze.emplace_back(AddEnemy4ToWorld(n.position));
			if (type == 115)maze.emplace_back(AddBonusToWorld(n.position));
		}
	}
	return;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, 0, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(30, 5, 10));
	//AddEnemyToWorld(Vector3(30, 5, 80));
	AddBonusToWorld(Vector3(10, 5, 10));
	AddGooseToWorld(Vector3(10, 10, 80));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);
	//float inverseMass = 1;

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT) && GoatNotSelected) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 10.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(5, 0, 0));
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(-5, 0, 0));
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -5));
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 5));
	}

	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}


void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(4, 4, 4);

	float invCubeMass = 2; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 25; // distance between links

	Vector3 startPos = Vector3(0, 0, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		OrientationConstraint* oriConst = new OrientationConstraint(block, Vector3(0, 0, 1));
		world->AddConstraint(constraint);
		world->AddConstraint(oriConst);
		previous = block;
	}

	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

GameObject* TutorialGame::AddFenceToWorld(const Vector3& position, const Vector3& scale, const Vector3& rotation, const float inverseMass)
{
	GameObject* ramp = new GameObject();

	OBBVolume* volume = new OBBVolume(scale);
	ramp->SetBoundingVolume((CollisionVolume*)volume);
	ramp->GetTransform().SetScale(scale * 2);
	ramp->GetTransform().SetPosition(position);
	ramp->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation.x, rotation.y, rotation.z));

	ramp->SetRenderObject(new RenderObject(&ramp->GetTransform(), cubeMesh, basicTex, basicShader));
	ramp->SetPhysicsObject(new PhysicsObject(&ramp->GetTransform(), ramp->GetBoundingVolume()));

	ramp->GetPhysicsObject()->SetInverseMass(inverseMass);
	ramp->GetPhysicsObject()->InitCubeInertia();

	ramp->GetRenderObject()->SetColour(Vector4(1, 0.9, 0.5, 1));

	world->AddGameObject(ramp);

	return ramp;
}


void TutorialGame::AddGateToWorld(const Vector3& position, const Vector3& rotation)
{
	GameObject* fence1 = AddFenceToWorld(position, Vector3(0.2, 2, 0.3), rotation, 0);
	GameObject* gate = AddFenceToWorld(position + Quaternion::EulerAnglesToQuaternion(rotation.x, rotation.y, rotation.z) 
										* Vector3(12, 0, 0), Vector3(8, 1.8, 0.2), rotation, 1);

	PositionConstraint* constraint = new PositionConstraint(fence1, gate, 10);
	HingeConstraint* rot = new HingeConstraint(fence1, gate);
	HingeHeightConstraint* height = new HingeHeightConstraint(gate, gate->GetTransform().GetPosition().y);

	world->AddConstraint(constraint);
	world->AddConstraint(rot);
	world->AddConstraint(height);
}


