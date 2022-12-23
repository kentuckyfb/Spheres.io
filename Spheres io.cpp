//Nathan Himesh Kammanankada

#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <iostream>
#include <vector>
#include<cmath>


using namespace std;
using namespace tle;

//player movement keys
const EKeyCode up = Key_W;
const EKeyCode r_left = Key_A;
const EKeyCode down = Key_S;
const EKeyCode r_right = Key_D;
//camera movement keys
const EKeyCode camUp = Key_Up;
const EKeyCode camDown = Key_Down;
const EKeyCode camLeft = Key_Left;
const EKeyCode camRight = Key_Right;
//pause and escape keys
const EKeyCode gamepause = Key_P;
const EKeyCode quitgame = Key_Escape;
//camera switch keys
const EKeyCode UpDowncam_key = Key_1;
const EKeyCode IsonometricCam_key = Key_2;
//game variables
int maxscore = 120;
float kSphereSpeed = 0.3f;
float kRotationSpeed = 0.2f;
float scaleValue = 1.2f;
int cubecount = 0;
int hypercube;
int hypertime = 0.0f;
float GroundLevel = 0.0f;
float countdown = 0.0f;
int playerPoints = 0;
int enemyPoints = 0;
bool display = true;
float slowed = 0.3;

//enum to hold game states
enum gamestate { pregame, playing, gameover, gamewon, pause, resume, quit};

// data struct for game objects
struct object{
	//x,y,z floats to hold vectors and coordinates
	float x;
	float y;
	float z;

	//maximum and minimum values for sphere to box collision detection
	float Xmax;
	float Xmin;
	float Ymin;
	float Ymax;
	float Zmin;
	float Zmax;

	//length is associated with dimensions of a model or lenght of a vector
	float length;
	float diameter = length * 2;
	bool IsActive;
	int points = 0;
	int cubecount = 0;
	bool hyper = false;
	bool scale = false;
	bool Static = false;

	//to reference cubes collided with the player or enemy sphere
	bool taggedBYplayer = false;
	IModel* model;
};

//function to calculate the distance from one point to another
object get_length(object from, object to) {
	object distance;
	distance.x = to.x - from.x;
	distance.y = to.y - from.y;
	distance.z = to.z - from.z;
	distance.length = sqrt((distance.x * distance.x) + (distance.y * distance.y) + (distance.z * distance.z));
	return distance;
}

//get positions of a model and put it into a vector
object getposition(IModel* gameobject, object playerVector, bool minmax) {

	object obj;
	float total;
	float lengthOfCube = 2.5f;
	float lengthOfSphere = playerVector.length;

	if (playerVector.hyper == true) {
		total = 50.0f;
	}
	else {
		total = lengthOfCube + lengthOfSphere;
	}
	obj.x = gameobject->GetLocalX();
	obj.y = gameobject->GetLocalY();
	obj.z = gameobject->GetLocalZ();

	//get minimun and maximun coordinates for sphere to box collisions
	if (minmax == true) {
		obj.Xmax = obj.x + (total);
		obj.Xmin = obj.x - (total);
		obj.Ymax = obj.y + (total);
		obj.Ymin = obj.y - (total);
		obj.Zmax = obj.z + (total);
		obj.Zmin = obj.z - (total);
	}
	return obj;
}
object random_pos() {
	object random;
	// generate a random number from 1 to 4
	int val = rand() % 4 + 1;

	//switch case statment to randomly spawn cubes all over the plane
	switch (val) {
		//top right
	case 1:
		random.x = rand() % 90 + 1;
		random.y = 5;
		random.z = rand() % 90 + 1;
		break;
		//bottom right
	case 2:
		random.x = rand() % 90 + 1;
		random.y = 5;
		random.z = rand() % 90 + -90;
		break;
		//top left
	case 3:
		random.x = rand() % 90 + -90;
		random.y = 5;
		random.z = rand() % 90 + 1;
		break;
	case 4:
		//bottom left
		random.x = rand() % 90 + -90;
		random.y = 5;
		random.z = rand() % 90 + -90;
		break;
	default:
		random.x = rand() % (170 + (-80));
		random.y = 5;
		random.z = rand() % (170 + (-80));
		break;
	}

	return random;
}
//update cubes when 
vector<int> hyper_UpdateCubes(vector <IModel*> cubes, vector <object> cubeVectors, object sphereobject) {
	vector<int> reactive;
	int size = cubes.size();

	//increase spawn distance to not collide with hyperspheres collision distance
	float bounds = 50.0f;
	object sub;
	bool found = false;
	int count = 0;
	
	for (int i = 0; i < size; i++) {
		if (cubeVectors[i].IsActive == false) {

			//while loop to generate random number that is 50 units away from the player and 15 units away from other cubes
			while (found == false) {

				sub = random_pos();
				object len = get_length(sub, sphereobject);
				sub.length = len.length;
				count = 0;

				if (sub.length > bounds) {
					for (int num = 0; num < size; num++) {
						object temp_obj = get_length(cubeVectors[num], sub);

						if (temp_obj.length > 15) {
							count += 1;
						}
					}
					if (count == size) {
						found = true;
					}
				}
			}
			cubes[i]->SetSkin("minicube.jpg");
			cubes[i]->SetLocalX(sub.x);
			cubes[i]->SetLocalY(sub.y);
			cubes[i]->SetLocalZ(sub.z);
			cubeVectors[i].x = sub.x;
			cubeVectors[i].y = sub.y;
			cubeVectors[i].z = sub.z;
			reactive.push_back(i);
		}
	}
	return reactive;
}
vector<int> UpdateCubes(vector <IModel*> cubes, vector <object> cubeVectors, object sphereobject) {

	int size = cubes.size();
	object substitute;
	//vector to update cube varaibles after respawn
	vector<int> reactive;
	int count = 0;
	bool found = false;

	//to randomly spawn hyper cubes
	int ran = rand() % 100;
	
	//iterate through all cubes
	for (int i = 0; i < size; i++) {
		object temp_obj = cubeVectors[i];
		IModel* temp_Model = cubes[i];
		float bounds = sphereobject.length + temp_obj.length + 10;
		int temp = 0;
		
		if (temp_obj.IsActive == false) {
			found = false;
			//while loop to generate random number that is 22.5(sphere radius + cube length + 10 units) units away from the player and 15 units away from other cubes
			while (found == false) {
				count = 0;
				substitute = random_pos();
				object len = get_length(substitute, sphereobject);
				//substitute.length = len.length;
				if (len.length > bounds) {
					for (int num = 0; num < size; num++) {
							object temp = get_length(substitute, cubeVectors[num]);
							if (temp.length > bounds) {
								count += 1;
							}
					}
					if (count == size) {
						found = true;
					}
					else {
						found = false;
					}
				}
			}
			//spawn hypercube
			if (ran > 55 && ran < 65) {
				temp_Model->SetSkin("hypercube.jpg");
				temp = i + 12;
			}
			else {
				temp_Model->SetSkin("minicube.jpg");
				temp = i;
			}
			temp_Model->SetLocalX(substitute.x);
			temp_Model->SetLocalY(substitute.y);
			temp_Model->SetLocalZ(substitute.z);
			temp_obj.x = substitute.x;
			temp_obj.y = substitute.y;
			temp_obj.z = substitute.z;
		}
		reactive.push_back(temp);
	}
	return reactive;
}

//function to check position against the land
bool check_position(object player) {

	float x = player.x;
	float y = player.y;
	float z = player.z;

	if ((x > 95 || x < -95)||(z > 95 || z < -95)){
		return true;
	}
	else { return false; }
}

//print texts on screen
void printScore(IFont* firstmessage, IFont* secondmessage, IFont* gameoverText, int playerpoints, int enemypoints, gamestate game, bool player_outofbounds) {

	string gametext;
	if (game == gameover) {
		gametext = "Game Over";
	}
	if (game == gamewon) {
		gametext = "Game Won";
	}
	if (playerpoints > enemypoints) {
		gameoverText->Draw(gametext, 500, 400);
		firstmessage->Draw("Player Points : " + to_string(playerpoints), 1020, 0);
		secondmessage->Draw("Enemy Points : " + to_string(enemypoints), 1020, 50);

	}
	else if (playerpoints < enemypoints) {
		gameoverText->Draw(gametext, 500, 400);
		firstmessage->Draw("Enemy Points : " + to_string(enemypoints), 1020, 0);
		secondmessage->Draw("Player Points : " + to_string(playerpoints), 1020, 50);
	}
	else if ((playerpoints == enemypoints) && player_outofbounds == false) {
		gameoverText->Draw("Game Draw", 500, 400);
		firstmessage->Draw("Player Points : " + to_string(playerpoints), 1020, 0);
		secondmessage->Draw("Enemy Points : " + to_string(enemypoints), 1020, 50);
	}
	else {
		gameoverText->Draw(gametext, 500, 400);
		firstmessage->Draw("Player Points : " + to_string(playerpoints), 1020, 0);
		secondmessage->Draw("Enemy Points : " + to_string(enemypoints), 1020, 50);
	}
}

//sphere to box collision detection
bool SphereToBoxCollisonDetection(object playervector, object cubeobject, IModel* player, IModel* cubeModel) {

	object cube = getposition(cubeModel, playervector, true);
	bool collision = false;
	if ((playervector.x < cube.Xmax && playervector.x > cube.Xmin) && (playervector.z < cube.Zmax && playervector.z > cube.Zmin)) {
		collision = true;
	}
	else {
		collision = false;
	}
	return collision;
}

//point to point collision detection
bool P2PCollisionDetection(object playervector, object cube, IModel* player, int i) {

	object PlayerToCube;
	//calculate minimum distance
	float bounds;
	bounds = playervector.length + cube.length;
	PlayerToCube = get_length(cube, playervector);

	if (PlayerToCube.length <= bounds && cube.IsActive == true) {
		return true;
	}
	else {
		return false;
	}
}

void main()
{	

	object playerVector;
	gamestate game = pregame;
	
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed();

	IFont* firstmessage = myEngine->LoadFont("Comic Sans MS", 36);
	IFont* secondmessage = myEngine->LoadFont("Comic Sans MS", 36);
	IFont* gameoverText = myEngine->LoadFont("my game", 50);

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder("..\\resources");
	
	/**** Set up your scene here ****/
	//water
	IMesh* watermesh = myEngine->LoadMesh("water.x");
	IModel* watermodel = watermesh->CreateModel(0, -5, 0); 

	//island
	IMesh* islandmesh = myEngine->LoadMesh("island.x");
	IModel* islandmodel = islandmesh->CreateModel(0, -5, 0);

	//player
	IMesh* playermesh = myEngine->LoadMesh("spheremesh.x");
	playerVector.length = 10;
	IModel* player = playermesh->CreateModel(0, GroundLevel+playerVector.length, 0);
	playerVector.model = player;
	playerVector.x = 0;
	playerVector.y = GroundLevel + playerVector.length;
	playerVector.z = 0;
	playerVector.hyper = false;
	playerVector.scale = false;

	//enemy
	IMesh* enemymesh = myEngine->LoadMesh("spheremesh.x");
	IModel* enemy = enemymesh->CreateModel();
	enemy->SetSkin("enemysphere.jpg");

	object enemyVector;
	enemyVector.x = 0;
	enemyVector.z = 0;
	object enemyToplayer;

	//get random co ordinates for enemy spawn
	do {
		enemyVector = random_pos();
		enemyToplayer = get_length(enemyVector, playerVector);
	}while (enemyToplayer.length < 65);

	enemyVector.y = 10;
	enemy->SetLocalX(enemyVector.x);
	enemy->SetLocalY(enemyVector.y);
	enemy->SetLocalZ(enemyVector.z);
	enemyVector.model = enemy;
	enemyVector.length = 10.0f;
	enemyVector.IsActive = false;
	enemyVector.scale = false;

	//camera
	ICamera* myCamera;
	myCamera = myEngine->CreateCamera(kManual, 0, 200, 0);
	myCamera->RotateLocalX(90);

	//cubes
	IMesh* cubemesh;
	IModel* cube1;
	IModel* cube2;
	IModel* cube3;
	IModel* cube4;
	IModel* cube5;
	IModel* cube6;
	IModel* cube7;
	IModel* cube8;
	IModel* cube9;
	IModel* cube10;
	IModel* cube11;
	IModel* cube12;

	cubemesh = myEngine->LoadMesh("minicube.x");
	cube1 = cubemesh->CreateModel();
	cube2 = cubemesh->CreateModel();
	cube3 = cubemesh->CreateModel();
	cube4 = cubemesh->CreateModel();
	cube5 = cubemesh->CreateModel();
	cube6 = cubemesh->CreateModel();
	cube7 = cubemesh->CreateModel();
	cube8 = cubemesh->CreateModel();
	cube9 = cubemesh->CreateModel();
	cube10 = cubemesh->CreateModel();
	cube11 = cubemesh->CreateModel();
	cube12 = cubemesh->CreateModel();

	//cube vectors
	object cube1_vector;
	object cube2_vector;
	object cube3_vector;
	object cube4_vector;
	object cube5_vector;
	object cube6_vector;
	object cube7_vector;
	object cube8_vector;
	object cube9_vector;
	object cube10_vector;
	object cube11_vector;
	object cube12_vector;


	//cube arrays
	vector <IModel*> cubes = { cube1, cube2, cube3, cube4, cube5, cube6, cube7, cube8, cube9, cube10, cube11, cube12 };
	vector <object> cube_vectors = { cube1_vector ,cube2_vector ,cube3_vector ,cube4_vector,cube5_vector, cube6_vector,cube7_vector,cube8_vector,cube9_vector,cube10_vector,cube11_vector,cube12_vector };
	vector <object> pos;

	//set cube object varaibles
	for (int cube = 0; cube < cubes.size(); cube++) {
		cube_vectors[cube].model = cubes[cube];
		cube_vectors[cube].IsActive = true;

	}

	//spawning cubes in random positions 
	int NumberOfCubes = cubes.size();
	for (int i = 0; i < NumberOfCubes; i++) {
		object random;
		if (i == 0) {
			random = random_pos();
		}
		else {
			bool end = false;
			int count = 0;
			while (end == false) {
				count = 0;
				random = random_pos();
				object ranTosphere = get_length(random, playerVector);
				if (ranTosphere.length > 15.0f) {
					for (int j = 0; j < pos.size(); j++) {
						object ranTocube = get_length(random, pos[j]);
						if (ranTocube.length > 20.0f) {
							count += 1;
						}
					}
					if (count == pos.size()) {
						end = true;
					}
				}
			}
		}

		//object random = setposition(cubes[i], playerVector, cube_vectors[i], pos);
		cubes[i]->SetLocalX(random.x);
		cubes[i]->SetLocalY(random.y);
		cubes[i]->SetLocalZ(random.z);

		pos.push_back(random);
	}

	//sky
	IMesh* skymesh = myEngine->LoadMesh("sky.x");
	IModel* sky = skymesh->CreateModel(0, -960, 0);
	
	bool player_outofbounds = false;
	bool enemy_outofbounds = false;
	string scoreText;
	string enemyScore;
	object updowncam_obj;
	updowncam_obj.IsActive = true;
	object isocam_obj;
	bool start = false;
	playerVector.points = 0;
	enemyVector.points = 0;
	int cubenumber;
	object movementdif;
	movementdif.Static = false;
	game = playing;

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{	
		myEngine->DrawScene();

		if (game == pause) {
			if (myEngine->KeyHit(gamepause)) {
			game = playing;
			}
		}
		else if (game == gamewon) {
			kSphereSpeed = 0;
			kRotationSpeed = 0;
			printScore(firstmessage, secondmessage, gameoverText, playerPoints, enemyPoints, game, player_outofbounds);
		}
		else if (game == quit) {
			myEngine->Stop();
		}
		else if (game == gameover) {
			kSphereSpeed = 0;
			kRotationSpeed = 0;
			printScore(firstmessage, secondmessage, gameoverText, playerPoints, enemyPoints, game, player_outofbounds);
		}
		else if (game == playing) {
			
			float timer = myEngine->Timer();

			/**** Update your scene each frame here ****/
			//isonometric camera and topdown camera controls
			if (myEngine->KeyHit(IsonometricCam_key) && updowncam_obj.IsActive == true) {

				updowncam_obj.IsActive = false;
				isocam_obj.IsActive = true;
				myCamera->ResetOrientation();
				myCamera->SetPosition(150, 150, -150);
				myCamera->RotateLocalY(-45);
				myCamera->RotateLocalX(45);
			}
			if (myEngine->KeyHit(UpDowncam_key) && isocam_obj.IsActive == true) {

				updowncam_obj.IsActive = true;
				isocam_obj.IsActive = false;
				myCamera->SetPosition(0, 200, 0);
				myCamera->ResetOrientation();
				myCamera->RotateLocalX(90);
			}

			//player movement 
			if (myEngine->KeyHeld(up)) {
				player->MoveLocalZ(kSphereSpeed);
			}

			if (myEngine->KeyHeld(down)) {
				player->MoveLocalZ(-kSphereSpeed);
			}

			if (myEngine->KeyHeld(r_left)) {
				player->RotateLocalY(-kRotationSpeed);
			}

			if (myEngine->KeyHeld(r_right)) {
				player->RotateLocalY(kRotationSpeed);
			}

			//camera movement 
			if (myEngine->KeyHeld(camUp)) {
				myCamera->MoveLocalZ(kSphereSpeed);
			}

			if (myEngine->KeyHeld(camDown)) {
				myCamera->MoveLocalZ(-kSphereSpeed);
			}

			if (myEngine->KeyHeld(camLeft)) {
				myCamera->MoveLocalX(-kRotationSpeed);
			}

			if (myEngine->KeyHeld(camRight)) {
				myCamera->MoveLocalX(kRotationSpeed);
			}

			//pause
			if (myEngine->KeyHit(gamepause)) {
				game = pause;
			}

			//quit game
			if (myEngine->KeyHit(quitgame)) {
				game = quit;
			}

			//get enemy position
			object temp2 = getposition(enemy, enemyVector, false);
			enemyVector.x = temp2.x;
			enemyVector.y = temp2.y;
			enemyVector.z = temp2.z;

			//get player position
			object temp = getposition(player, playerVector, false);
			playerVector.x = temp.x;
			playerVector.y = temp.y;
			playerVector.z = temp.z;

			//get cube positions
			for (int l = 0; l < 12; l++) {
				object randomCubePosition = getposition(cubes[l], cube_vectors[l], false);
				cube_vectors[l].x = randomCubePosition.x;
				cube_vectors[l].y = randomCubePosition.y;
				cube_vectors[l].z = randomCubePosition.z;
			}
			object enemyTocube;
			//pick random cube
			if (enemyVector.IsActive == false) {
				int randomcube = rand() % 12;
				enemyTocube = get_length(enemyVector, cube_vectors[randomcube]);
				enemyVector.IsActive = true;
				cubenumber = randomcube;

			}

			enemyTocube = get_length(enemyVector, cube_vectors[cubenumber]);

			//move npc to cubes
			if (enemyVector.IsActive == true && enemyVector.Static==false) {

				if (enemyTocube.length <= enemyVector.length) {
					enemyVector.IsActive = false;
				}
				else if (enemyTocube.length > enemyVector.length) {
				
					enemy->MoveX(timer * enemyTocube.x * slowed);
					enemy->MoveZ(timer * enemyTocube.z * slowed);
				}
			}

		
			//check player position if its out of bounds
			player_outofbounds = check_position(playerVector);
			if (player_outofbounds == true) {
				game = gameover;
			}

			//collision detection
			for (int i = 0; i < NumberOfCubes; i++) {
				bool collision = false;

				cube_vectors[i].length = 5;
				collision = SphereToBoxCollisonDetection(playerVector, cube_vectors[i], player, cubes[i]);

				if (collision == true && cube_vectors[i].IsActive == true) {
					cube_vectors[i].taggedBYplayer = true;
					//check if collided cube is a hypercube
					if (cube_vectors[i].hyper == true) {
						cube_vectors[i].hyper = false;
						playerVector.hyper = true;
						start = true;
						hypertime += 5.0f;
					}
					playerVector.points += 10;
					playerVector.cubecount += 1;
					cube_vectors[i].IsActive = false;
				}
			}

			//check for collisions in enemy sphere
			for (int k = 0; k < NumberOfCubes; k++) {
				bool enemyCollision = false;
				enemyCollision = SphereToBoxCollisonDetection(enemyVector, cube_vectors[k], enemy, cubes[k]);
				if (enemyCollision == true && cube_vectors[k].IsActive == true) {
					cube_vectors[k].taggedBYplayer = false;
					enemyVector.points += 10;
					enemyVector.cubecount += 1;
					cube_vectors[k].IsActive = false;
				}
			}
			object playerToenemy = get_length(enemyVector, playerVector);
			object enemyToplayer = get_length(playerVector, enemyVector);
			if (playerToenemy.length <= enemyVector.length + playerVector.length && playerVector.Static == false && enemyVector.Static == false) {
				int pointDif = playerVector.points - enemyVector.points;

				if (pointDif >= 40) {
					playerVector.scale = true;
					enemyVector.scale = false;
					enemy->SetY(-1000);
					enemyVector.Static = true;
					playerVector.points += 40;
				}
				else if (pointDif <= -40) {
					enemyVector.scale = true;
					playerVector.scale = false;
					player->SetY(-1000);
					playerVector.Static = true;
					enemyVector.points += 40;
				}
				else {
					if (movementdif.Static == false) {
						movementdif.Static = true;
						movementdif.x = playerToenemy.x;
						movementdif.z = playerToenemy.z;
						movementdif.length = playerToenemy.length;
					}
					player->MoveX(timer * (movementdif.x) * movementdif.length);
					player->MoveZ(timer * (movementdif.z) * movementdif.length);

					enemy->MoveX(timer * (-1 * movementdif.x) * movementdif.length);
					enemy->MoveZ(timer * (-1 * movementdif.z) * movementdif.length);
					movementdif.length -= 1.5f;
				}
			}

			//scalling enemy
			if (enemyVector.cubecount == 4 || enemyVector.scale == true) {
				float y;
				enemyVector.cubecount = 0;
				enemy->Scale(scaleValue);
				y = (playerVector.length * 1.2);
				enemyVector.length = y;
				enemyVector.y = y + GroundLevel;
				enemy->SetY(y + GroundLevel);
				enemyVector.scale = false;
			}

			//scaling player
			if (playerVector.cubecount == 4 || playerVector.scale == true) {
				float y;
				playerVector.cubecount = 0;
				player->Scale(scaleValue);
				y = (playerVector.length * 1.2);
				playerVector.length = y;
				playerVector.y = y + GroundLevel;
				player->SetY(y + GroundLevel);
				playerVector.scale = false;
			}

			//start timer for hypersphere 
			if (start == true) {
				countdown += timer;
				player->SetSkin("hypersphere.jpg");
			}
			else {
				countdown = 0;
				player->SetSkin("regularsphere.jpg");
			}

			//check time for hypersphere
			if (countdown >= hypertime) {
				playerVector.hyper = false;
				hypertime = 0.0f;
				start = false;
			}

			//vector to reset cube object variables
			vector<int> reactivate;
			vector<int> hyper_reactivate;

			//respawn cubes in regular mode
			if (playerVector.hyper == false) {
				reactivate = UpdateCubes(cubes, cube_vectors, playerVector);
				for (auto num : reactivate) {
					int count = num;
					if (num >= 12) {
						count = num - 12;
						cube_vectors[count].hyper = true;
					}
					cube_vectors[count].IsActive = true;
				}
			}
			//respawning cubes in hypermode
			else if (playerVector.hyper == true) {
				for (int m = 0; m < NumberOfCubes; m++) {
					if (cube_vectors[m].IsActive == false && cube_vectors[m].taggedBYplayer == true) {
						object cubeToplayer = get_length(cube_vectors[m], playerVector);
						//attract cubes in a 50 unit distance from the sphere towards it
						if (cubeToplayer.length > playerVector.length) {
							cubes[m]->MoveLocalX(timer * cubeToplayer.x);
							cubes[m]->MoveLocalZ(timer * cubeToplayer.z);
						}
						//respawn after cube attracts
						if (cubeToplayer.length <= playerVector.length) {
							hyper_reactivate = hyper_UpdateCubes(cubes, cube_vectors, playerVector);
							for (int hypercount = 0; hypercount < hyper_reactivate.size(); hypercount++) {
								int pos = hyper_reactivate[hypercount];
								cube_vectors[pos].IsActive = true;
							}
						}
					}
					else if (cube_vectors[m].IsActive == false && cube_vectors[m].taggedBYplayer == false) {
						reactivate = UpdateCubes(cubes, cube_vectors, playerVector);
						for (auto num : reactivate) {
							int count = num;
							if (num >= 12) {
								count = num - 12;
								cube_vectors[count].hyper = true;
							}
							cube_vectors[count].IsActive = true;
						}
					}
				}
			}
			
			//game win condition
			if (playerVector.points >= maxscore) {
					game = gamewon;
					display = false;
			}
			if (enemyVector.points >= maxscore) {

					game = gameover;
					display = false;
			}

			//displayer score on screen
			if (display == true) {
				scoreText = "Score : " + to_string(playerVector.points);
				firstmessage->Draw(scoreText, 1100, 0, kBlack);
			}
		}
		playerPoints = playerVector.points;
		enemyPoints = enemyVector.points;

		
	}
	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}
