#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

#include <iostream>
#include <sstream>
#include <fstream>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "VSShaderlib.h"
#include "MyObject.h"



extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];



My3DVector::My3DVector() {}

My3DVector::My3DVector(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

// Creates a vector with an angle
My3DVector::My3DVector(float angle, float x, float y, float z) {
	this->angle = angle;
	this->x = x;
	this->y = y;
	this->z = z;
}

// Normalizes the vector
void My3DVector::normalize2D() {
	float normalize_constant = sqrt(pow(this->x, 2) + pow(this->z, 2));
	this->x = this->x / normalize_constant;
	this->z = this->z / normalize_constant;
}

My3DVector My3DVector::rotatearoundX(float angle) {
	float new_angle = fmod(angle, 360.0f) * (3.14f / 180.f);
	float new_y = cos(new_angle) * this->y - sin(new_angle) * this->z;
	float new_z = sin(new_angle) * this->y + cos(new_angle) * this->z;
	return My3DVector(this->x, new_y, new_z);
}

My3DVector My3DVector::rotatearoundY(float angle) {
	float new_angle = fmod(angle, 360.0f) * (3.14f / 180.f);
	float new_x = cos(new_angle) * this->y + sin(new_angle) * this->z;
	float new_z = -1 * sin(new_angle) * this->y + cos(new_angle) * this->z;
	return My3DVector(new_x, this->y, new_z);
}

My3DVector My3DVector::rotatearoundZ(float angle) {
	float new_angle = fmod(angle, 360.0f) * (3.14f / 180.f);
	float new_x = cos(new_angle) * this->x - sin(new_angle) * this->y;
	float new_y = sin(new_angle) * this->x + cos(new_angle) * this->y;
	return My3DVector(new_x, new_y, this->z);
}

My3DVector My3DVector::inverseRotationZ(float angle) {
	float new_angle = fmod(angle, 360.0f) * (3.14f / 180.f);
	float new_x = cos(new_angle) * this->x + sin(new_angle) * this->y;
	float new_y = -1 * sin(new_angle) * this->x + cos(new_angle) * this->y;
	return My3DVector(new_x, new_y, this->z);
}

MyObject::MyObject() {
}

/*
An Object has:
A Mesh that represents the shape
Pos that represents the position
Scale that represents the scale
A rotationList that represents the rotations.

*/
MyObject::MyObject(MyMesh objectMesh, My3DVector pos, My3DVector scale, std::vector<My3DVector> rotationsList) {
	mesh = objectMesh;
	position = pos;
	scaleSize = scale;
	rotations = rotationsList;

}

My3DVector MyObject::updateVertice(My3DVector vertice) {
	pushMatrix(MODEL);
	translate(MODEL, position.x, position.y, position.z);

	for (My3DVector MyRotation : rotations) {
		rotate(MODEL, MyRotation.angle, MyRotation.x, MyRotation.y, MyRotation.z);
	}

	scale(MODEL, scaleSize.x, scaleSize.y, scaleSize.z);

	for (My3DVector MyCenter : center) {
		translate(MODEL, MyCenter.x, MyCenter.y, MyCenter.z);
	}


	//UPDATE VERTICES
	float transformVertice[4] = { vertice.x,vertice.y,vertice.z, 1.0 };

	float updatedVertice[4];

	// multiply vec4 vertex by the model matrix (mul. order is important)
	multMatrixPoint(MODEL, transformVertice, updatedVertice);

	popMatrix(MODEL);

	// return vector with new vertices
	return My3DVector{ updatedVertice[0],updatedVertice[1],updatedVertice[2] };
}



MyCar::MyCar() {

}

/*
A car receieves it's position
*/

MyCar::MyCar(My3DVector position) {

	// O nosso carro começa a olhar para cima.
	direction = My3DVector(0, 0, -1);
	// -1 no Z = Para cima em top view
	// 1 no Z = Para baixo em top view.
	// -1 no x = Para esquerda em top view.
	// 1 no x = Para direita em top view.


	//Body
	MyMesh bodyMesh = createCube();


	// Defines the materials for the car's body mesh
	float bodyamb[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float bodydiff[] = { 0.9f, 0.1f, 0.0f, 1.0f };
	float bodyspec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	float bodyemissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float bodyshininess = 500.0;
	int bodytexcount = 0;

	// Binds the materials to the mesh.
	memcpy(bodyMesh.mat.ambient, bodyamb, 4 * sizeof(float));
	memcpy(bodyMesh.mat.diffuse, bodydiff, 4 * sizeof(float));
	memcpy(bodyMesh.mat.specular, bodyspec, 4 * sizeof(float));
	memcpy(bodyMesh.mat.emissive, bodyemissive, 4 * sizeof(float));
	bodyMesh.mat.shininess = bodyshininess;
	bodyMesh.mat.texCount = bodytexcount;

	//bodyPosition = My3DVector(position.x, position.y, position.z);

	// Create the body's object
	body = MyObject(bodyMesh, position, My3DVector(1.0f, 1.0f, 2.0f), { My3DVector(0, 0.0, 1.0, 0.0) });
	// Normalmente seria -0.5, 0, -1, mas como tamos a usar o scale = 2, fica em -0.5
	My3DVector centerVector = My3DVector(-0.5, 0, -0.5);
	// Body's center
	body.center = { centerVector };
	//Window
	MyMesh windowMesh = createCube();


	// Defines the materials for the windows's body mesh
	float windowamb[] = { 0.0f, 0.0f, 0.9f, 1.0f };
	float windowdiff[] = { 0.0f, 0.1f, 0.9f, 1.0f };
	float windowspec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	float windowemissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float windowshininess = 500.0;
	int windowtexcount = 0;

	// Binds the materials to the mesh.
	memcpy(windowMesh.mat.ambient, windowamb, 4 * sizeof(float));
	memcpy(windowMesh.mat.diffuse, windowdiff, 4 * sizeof(float));
	memcpy(windowMesh.mat.specular, windowspec, 4 * sizeof(float));
	memcpy(windowMesh.mat.emissive, windowemissive, 4 * sizeof(float));
	windowMesh.mat.shininess = windowshininess;
	windowMesh.mat.texCount = windowtexcount;


	My3DVector windowposition = My3DVector(position.x, position.y + 1, position.z);
	//Creates the windows object
	window = MyObject(windowMesh, windowposition, My3DVector(1, 0.5, 1), { My3DVector(0, 0, 1, 0) });
	My3DVector centerWindowVector = My3DVector(-0.5, 0, -0.5);
	window.center = { centerWindowVector };


	//Wheels
	MyMesh wheelMesh;

	// Defines the materials for the wheels's body mesh
	float wheelamb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float wheeldiff[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float wheelspec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float wheelemissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float wheelshininess = 1.0f;
	int wheeltexcount = 0;
	// Top Right Wheel, Bottom Right Wheel, Bottom Left Wheel, Top Left Wheel
	//float wheelx[] = {0.5, 0.5, -0.5, -0.5};
	//float wheelz[] = { -0.5, 0.5, 0.5, -0.5 };
	
	float wheelx[] = { position.x,position.x,position.x,position.x};
	float wheely[] = { 0.5,0.5,0.5,0.5 };
	float wheelz[] = { position.z, position.z, position.z,position.z };


	// -1 no Z = Para cima em top view
	// 1 no Z = Para baixo em top view.
	// -1 no x = Para esquerda em top view.
	// 1 no x = Para direita em top view.
	//float wheelz[] = { -0.5, 0.5, 0.5, -0.5 };
	std::vector<My3DVector> wheelcenters;
	wheelcenters.push_back(My3DVector(-0.5, 0.5, 0.5));
	wheelcenters.push_back(My3DVector(-0.5, 0.5, -0.5));
	wheelcenters.push_back(My3DVector(-0.5, -0.5, -0.5));
	wheelcenters.push_back(My3DVector(-0.5, -0.5, 0.5));

	for (int i = 0; i < 4; i++) {

		wheelMesh = createTorus(0.25, 0.5, 20, 20);
		memcpy(wheelMesh.mat.ambient, wheelamb, 4 * sizeof(float));
		memcpy(wheelMesh.mat.diffuse, wheeldiff, 4 * sizeof(float));
		memcpy(wheelMesh.mat.specular, wheelspec, 4 * sizeof(float));
		memcpy(wheelMesh.mat.emissive, wheelemissive, 4 * sizeof(float));
		wheelMesh.mat.shininess = wheelshininess;
		wheelMesh.mat.texCount = wheeltexcount;


		wheels[i] = MyObject(wheelMesh, My3DVector(wheelx[i], wheely[i], wheelz[i]), My3DVector(1, 1, 1), { My3DVector(float(90), 0, 0, 1), My3DVector(0, 1, 0, 0) });
		//wheels[i] = MyObject(wheelMesh, My3DVector(wheelx[i], wheely[i], wheelz[i]), My3DVector(1, 1, 1), { My3DVector(90, 0, 0, 1) });
		//wheels[i] = MyObject(wheelMesh, My3DVector(wheelx[i], wheely[i], wheelz[i]), My3DVector(1, 1, 1), {});
		//wheels[i].center = {};
		wheels[i].center = { wheelcenters[i] };
	}

	//Lights
	MyMesh lightsMesh;
	// Defines the materials for the wheels's body mesh
	float lightamb[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float lightdiff[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float lightspec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightemissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float lightshininess = 1.0f;
	int lighttexcount = 0;
	std::vector<My3DVector> lightcenter = { My3DVector(-2,0,-5), My3DVector(1,0,-5) };
	My3DVector lightsScale = My3DVector(0.25, 0.25, 0.25);

	std::vector<My3DVector> lightspos = { My3DVector(position.x, position.y + 0.75,position.z), My3DVector(position.x, position.y + 0.75, position.z) };


	for (int i = 0; i < 2; i++) {

		lightsMesh = createCube();
		memcpy(lightsMesh.mat.ambient, lightamb, 4 * sizeof(float));
		memcpy(lightsMesh.mat.diffuse, lightdiff, 4 * sizeof(float));
		memcpy(lightsMesh.mat.specular, lightspec, 4 * sizeof(float));
		memcpy(lightsMesh.mat.emissive, lightemissive, 4 * sizeof(float));
		lightsMesh.mat.shininess = lightshininess;
		lightsMesh.mat.texCount = lighttexcount;

		lights[i] = MyObject(lightsMesh, lightspos[i], lightsScale, { My3DVector(0, 0, 1, 0) });
		lights[i].center = { lightcenter[i] };

	}

}

void MyCar::render(VSShaderLib& shader) {
	body.render(shader,"body");
	window.render(shader, "window");
	for (int i = 0; i < 4; i++) {
		wheels[i].render(shader, "wheels");
	}
	for (int i = 0; i < 2; i++) {
		lights[i].render(shader,"wheels");
	}
}

void MyCar::turnRight() {
	body.angle -= 10 * velocity;
	body.angle = fmod(body.angle, 360);
	body.rotations[0].angle = body.angle;
	window.rotations[0].angle = body.angle;
	for (int i = 0; i < 4; i++) {
		wheels[i].angle -= 10 * velocity;
		wheels[i].rotations[1].angle = body.angle;
	}
	for (int i = 0; i < 2; i++) {
		lights[i].angle -= 10 * velocity;
		lights[i].rotations[0].angle = body.angle;
	}
	updateDirection();

}

void MyCar::turnLeft() {
	body.angle += 10 * velocity;
	body.angle = fmod(body.angle, 360);
	body.rotations[0].angle = body.angle;
	window.rotations[0].angle = body.angle;
	for (int i = 0; i < 4; i++) {
		wheels[i].angle += 10 * velocity;
		wheels[i].rotations[1].angle = body.angle;
	}
	for (int i = 0; i < 2; i++) {
		lights[i].angle += 10 * velocity;
		lights[i].rotations[0].angle = body.angle;
	}
	updateDirection();

}

void MyCar::updateDirection() {
	// cos needs radians so we convert degrees into radians
	// sen needs radians so we convert degrees into radians

	direction.x = 0 * cos(body.angle * 3.14f / 180.0f) - -1 * sin(body.angle * 3.14f / 180.0f);
	direction.z = 0 * sin(body.angle * 3.14f / 180.0f) + -1 * cos(body.angle * 3.14f / 180.0f);
	direction.normalize2D();
}

void MyCar::updatePosition() {

	body.position.x += -1 * direction.x * velocity;
	body.position.z += direction.z * velocity;
	window.position.x += -1 * direction.x * velocity;
	window.position.z += direction.z * velocity;
	for (int i = 0; i < 4; i++) {
		wheels[i].position.x += -1 * direction.x * velocity;
		wheels[i].position.z += direction.z * velocity;
	}
	for (int i = 0; i < 2; i++) {
		lights[i].position.x += -1 * direction.x * velocity;
		lights[i].position.z += direction.z * velocity;
	}

}

void MyCar::accelerate() {
	if (velocity <= MAX_VELOCITY) {
		velocity += 0.01f;
	}
}

void MyCar::deccelerate() {
	if (velocity >= -MAX_VELOCITY) {
		velocity -= 0.01f;
	}
}

void MyCar::breaks(bool acceleration) {

	if (acceleration && velocity >= 0.0f) {
		velocity -= 0.01f;
		if (velocity < 0.0f)
			velocity = 0.0f;
	}
	else if (!acceleration && abs(velocity) >= 0.0f) {
		velocity += 0.01f;
		if (velocity > 0.0f)
			velocity = 0.0f;
	}

}

void MyCar::stop() {
	velocity = 0.0f;

}


void MyCar::restart() {
	velocity = 0.0f;
	direction = My3DVector{ 0, 0, -1 };
	body.position = My3DVector{ CAR_INITIAL_X, CAR_INITIAL_Y, CAR_INITIAL_Z };
	body.angle = 0.0f;
	body.rotations[0].angle = 0.0f;
	window.position = My3DVector{ CAR_INITIAL_X, CAR_INITIAL_Y + 1, CAR_INITIAL_Z };
	window.angle = 0.0f;
	window.rotations[0].angle = 0.0f;

	for (int i = 0; i < 4; i++) {
		//wheels[i].position = My3DVector(wheelx[i], wheely[i], wheelz[i]);
		wheels[i].position = My3DVector(CAR_INITIAL_X, CAR_INITIAL_Y + 0.5, CAR_INITIAL_Z);
		wheels[i].angle = 0.0f;
		wheels[i].rotations[1].angle = 0.0f;

	}

	for (int i = 0; i < 2; i++) {
		lights[i].position = My3DVector{ CAR_INITIAL_X, CAR_INITIAL_Y + 0.75, CAR_INITIAL_Z };
		lights[i].angle = 0.0f;
		lights[i].rotations[0].angle = 0.0f;
	}
}

std::vector<My3DVector> MyCar::AABB() {
	//GATHER ALL VERTICES
	std::vector<My3DVector> vertices = { My3DVector{0.0, 1.0, 1.0}, My3DVector{0.0, 0.0, 1.0},
										My3DVector{1.0, 0.0, 1.0}, My3DVector{1.0, 1.0, 1.0},
										My3DVector{1.0, 1.0, 0.0}, My3DVector{1.0, 0.0, 0.0},
										My3DVector{0.0, 0.0, 0.0}, My3DVector{0.0, 1.0, 0.0} };

	My3DVector minPos = body.updateVertice(vertices[0]);
	My3DVector maxPos = body.updateVertice(vertices[0]);

	for (My3DVector vertices : vertices) {
		My3DVector updatedVertices = body.updateVertice(vertices);

		minPos = My3DVector{ std::min(minPos.x, updatedVertices.x), std::min(minPos.y, updatedVertices.y), std::min(minPos.z, updatedVertices.z) };
		maxPos = My3DVector{ std::max(maxPos.x, updatedVertices.x), std::max(maxPos.y, updatedVertices.y), std::max(maxPos.z, updatedVertices.z) };

	}

	//return min and max position of vertices
	return { minPos, maxPos };
}


MyTable::MyTable() {}

MyTable::MyTable(My3DVector position) {

	MyMesh tableMesh;

	float amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 500.0;
	int texcount = 0;


	tableMesh = createCube();
	memcpy(tableMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(tableMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(tableMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(tableMesh.mat.emissive, emissive, 4 * sizeof(float));
	tableMesh.mat.shininess = shininess;
	tableMesh.mat.texCount = texcount;

	table = MyObject(tableMesh, position, My3DVector(BOARDSIZE, 0.5, BOARDSIZE), {});

}

void MyTable::render(VSShaderLib& shader) {
	table.render(shader, "table");
}
MyOrange::MyOrange() {
}

MyOrange::MyOrange(My3DVector position, float velocity) {


	MyMesh orangeMesh;

	float amb[] = { 1.0f, 0.65f, 0.0f, 0.7f };
	float diff[] = { 1.0f, 0.65f, 0.0f, 0.7f };
	float spec[] = { 1.0f, 0.65f, 0.0f, 0.7f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 0.7f };
	float shininess = 500.0;
	int texcount = 0;

	orangeMesh = createSphere(2, 20);
	//orangeMesh = createCube();
	memcpy(orangeMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(orangeMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(orangeMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(orangeMesh.mat.emissive, emissive, 4 * sizeof(float));
	orangeMesh.mat.shininess = shininess;
	orangeMesh.mat.texCount = texcount;

	My3DVector orangeDirection = My3DVector((rand() % BOARDSIZE) - BOARDSIZE / 2, 0, (rand() % BOARDSIZE) - BOARDSIZE / 2);
	orangeDirection.normalize2D();
	direction = orangeDirection;
	carVelocity = velocity;
	position = My3DVector((rand() % BOARDSIZE) - BOARDSIZE / 2, 1.5, (rand() % BOARDSIZE) - BOARDSIZE / 2);

	orange = MyObject(orangeMesh, position, My3DVector(1, 1, 1), { My3DVector(0, -orangeDirection.x, 1, orangeDirection.z) });
}

void MyOrange::render(VSShaderLib& shader) {
	orange.render(shader,"orange");
}

bool MyOrange::outOfLimits() {
	if (abs(orange.position.x) > BOARDSIZE / 2 || abs(orange.position.z) > BOARDSIZE / 2) {
		return true;
	}
	return false;

}

void MyOrange::updatePosition() {

	orange.angle = fmod(orange.angle + 10, 360);
	orange.rotations[0].angle = orange.angle;
	carVelocity += 0.05;
	orange.position.x += 0.1 * direction.x * carVelocity;
	orange.position.z += 0.1 * direction.z * carVelocity;
}

//My3DVector MyOrange::boundingBox() {

//}

MyTree::MyTree() {

}

MyTree::MyTree(My3DVector position) {

	MyMesh treeMesh;

	float treeamb[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float treediff[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float treespec[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float treeemissive[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float treeshininess = 500.0;
	int treetexcount = 0;

	treeMesh = createCube();
	memcpy(treeMesh.mat.ambient, treeamb, 4 * sizeof(float));
	memcpy(treeMesh.mat.diffuse, treediff, 4 * sizeof(float));
	memcpy(treeMesh.mat.specular, treespec, 4 * sizeof(float));
	memcpy(treeMesh.mat.emissive, treeemissive, 4 * sizeof(float));
	treeMesh.mat.shininess = treeshininess;
	treeMesh.mat.texCount = treetexcount;

	position = My3DVector((rand() % BOARDSIZE) - BOARDSIZE / 2, 0, (rand() % BOARDSIZE) - BOARDSIZE / 2);

	tree = MyObject(treeMesh, My3DVector(position.x, position.y, position.z), My3DVector(10, 10, 0.01), {});


}


void MyTree::render(VSShaderLib& shader) {
	tree.render(shader, "tree");
}

bool MyTree::outOfLimits() {
	if (abs(tree.position.x) > BOARDSIZE / 2 || abs(tree.position.z) > BOARDSIZE / 2) {
		return true;
	}
	return false;

}


MyButter::MyButter() {

}

MyButter::MyButter(My3DVector position) {

	MyMesh butterMesh;

	float butteramb[] = { 1.0f, 1.0f, 0.0f, 0.0f };
	float butterdiff[] = { 1.0f, 1.0f, 0.0f, 0.0f };
	float butterspec[] = { 1.0f, 1.0f, 0.0f, 0.0f };
	float butteremissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float buttershininess = 1.0;
	int buttertexcount = 0;

	butterMesh = createCube();
	memcpy(butterMesh.mat.ambient, butteramb, 4 * sizeof(float));
	memcpy(butterMesh.mat.diffuse, butterdiff, 4 * sizeof(float));
	memcpy(butterMesh.mat.specular, butterspec, 4 * sizeof(float));
	memcpy(butterMesh.mat.emissive, butteremissive, 4 * sizeof(float));
	butterMesh.mat.shininess = buttershininess;
	butterMesh.mat.texCount = buttertexcount;

	butter = MyObject(butterMesh, My3DVector(position.x, position.y, position.z), My3DVector(3, 1.5, 2), {});

}


void MyButter::updatePosition(My3DVector direction, float velocity) {
	butter.position.x += direction.x * velocity * 3;
	butter.position.z += direction.z * velocity * 3;
}


void MyButter::render(VSShaderLib& shader) {
	butter.render(shader, "butter");
}

std::vector<My3DVector> MyButter::AABB() {
	//GATHER ALL VERTICES OF BUTTER CUBE
	std::vector<My3DVector> vertices = { My3DVector{0.0, 1.0, 1.0}, My3DVector{0.0, 0.0, 1.0},
										My3DVector{1.0, 0.0, 1.0}, My3DVector{1.0, 1.0, 1.0},
										My3DVector{1.0, 1.0, 0.0}, My3DVector{1.0, 0.0, 0.0},
										My3DVector{0.0, 0.0, 0.0}, My3DVector{0.0, 1.0, 0.0} };

	My3DVector minPos = butter.updateVertice(vertices[0]);
	My3DVector maxPos = butter.updateVertice(vertices[0]);

	for (My3DVector vertices : vertices) {
		My3DVector updatedVertices = butter.updateVertice(vertices);

		minPos = My3DVector{ std::min(minPos.x, updatedVertices.x), std::min(minPos.y, updatedVertices.y), std::min(minPos.z, updatedVertices.z) };
		maxPos = My3DVector{ std::max(maxPos.x, updatedVertices.x), std::max(maxPos.y, updatedVertices.y), std::max(maxPos.z, updatedVertices.z) };
	}

	//return min and max position of vertices
	return { minPos, maxPos };
}


MyCheerio::MyCheerio() {
}

MyCheerio::MyCheerio(My3DVector position) {

	MyMesh cheerioMesh;

	direction = My3DVector(0.0f, 0.0f, 0.0f);

	float amb[] = { 1.0f, 0.65f, 0.0f, 1.0f };
	float diff[] = { 1.0f, 0.65f, 0.0f, 1.0f };
	float spec[] = { 1.0f, 0.65f, 0.0f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 500.0;
	int texcount = 0;

	cheerioMesh = createTorus(MIN_CHEERIO_RADIUS, MAX_CHEERIO_RADIUS, 20, 20);
	memcpy(cheerioMesh.mat.ambient, amb, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.diffuse, diff, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.specular, spec, 4 * sizeof(float));
	memcpy(cheerioMesh.mat.emissive, emissive, 4 * sizeof(float));
	cheerioMesh.mat.shininess = shininess;
	cheerioMesh.mat.texCount = texcount;

	cheerio = MyObject(cheerioMesh, position, My3DVector(1, 1, 1), {});

}

void MyCheerio::updatePosition(My3DVector direction, float velocity) {
	cheerio.position.x += direction.x * velocity * 3;
	cheerio.position.z += direction.z * velocity * 3;

}

void MyCheerio::render(VSShaderLib& shader) {
	cheerio.render(shader,"cheerio");
}


std::vector<My3DVector> MyCheerio::AABB() {
	//GATHER ALL VERTICES OF BUTTER CUBE
	std::vector<My3DVector> vertices = { My3DVector{ MAX_CHEERIO_RADIUS, (MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, MAX_CHEERIO_RADIUS },
											My3DVector{ MAX_CHEERIO_RADIUS, (MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, -MAX_CHEERIO_RADIUS },
											My3DVector{ MAX_CHEERIO_RADIUS, -(MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, MAX_CHEERIO_RADIUS },
											My3DVector{ MAX_CHEERIO_RADIUS, -(MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, -MAX_CHEERIO_RADIUS },
											My3DVector{ -MAX_CHEERIO_RADIUS, (MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, MAX_CHEERIO_RADIUS },
											My3DVector{ -MAX_CHEERIO_RADIUS, (MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, -MAX_CHEERIO_RADIUS },
											My3DVector{ -MAX_CHEERIO_RADIUS, -(MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, MAX_CHEERIO_RADIUS },
											My3DVector{ -MAX_CHEERIO_RADIUS, -(MAX_CHEERIO_RADIUS - MIN_CHEERIO_RADIUS) / 2, -MAX_CHEERIO_RADIUS } };

	My3DVector minPos = cheerio.updateVertice(vertices[0]);
	My3DVector maxPos = cheerio.updateVertice(vertices[0]);

	for (My3DVector vertices : vertices) {
		My3DVector updatedVertices = cheerio.updateVertice(vertices);

		minPos = My3DVector{ std::min(minPos.x, updatedVertices.x), std::min(minPos.y, updatedVertices.y), std::min(minPos.z, updatedVertices.z) };
		maxPos = My3DVector{ std::max(maxPos.x, updatedVertices.x), std::max(maxPos.y, updatedVertices.y), std::max(maxPos.z, updatedVertices.z) };
	}

	//return min and max position of vertices
	return { minPos, maxPos };
}

MyRoad::MyRoad() {

}

MyRoad::MyRoad(float radius, float innerRadius) {
	MyCheerio newcheerio;
	My3DVector cheerioposition;

	for (int i = -radius; i <= radius; i+=CHEERIO_SPACE) {
		cheerioposition = My3DVector(i, 0, radius);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
		cheerioposition = My3DVector(i, 0, -radius);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
	}

	for (int j = -radius; j <= radius; j+=CHEERIO_SPACE) {
		cheerioposition = My3DVector(radius, 0, j);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
		cheerioposition = My3DVector(-radius, 0, j);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
	}

	for (int i = -innerRadius; i <= innerRadius; i+=CHEERIO_SPACE) {
		cheerioposition = My3DVector(i, 0, innerRadius);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
		cheerioposition = My3DVector(i, 0, -innerRadius);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
	}

	for (int j = -innerRadius; j <= innerRadius; j+=CHEERIO_SPACE) {
		cheerioposition = My3DVector(innerRadius, 0, j);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
		cheerioposition = My3DVector(-innerRadius, 0, j);
		newcheerio = MyCheerio(cheerioposition);
		cheerios.push_back(newcheerio);
	}

}

void MyRoad::render(VSShaderLib& shader) {
	for (MyCheerio current_cheerio : cheerios) {
		current_cheerio.render(shader);
	}
}


MyCandle::MyCandle() {
}

MyCandle::MyCandle(My3DVector position) {

		MyMesh candleMesh;

		float candleamb[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		float candlediff[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		float candlespec[] = { 1.0f, 1.0f, 1.0f, 0.0f };
		float candleemissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		float candleshininess = 1.0;
		int candletexcount = 0;

		candleMesh = createCylinder(30, 2, 20);
		memcpy(candleMesh.mat.ambient, candleamb, 4 * sizeof(float));
		memcpy(candleMesh.mat.diffuse, candlediff, 4 * sizeof(float));
		memcpy(candleMesh.mat.specular, candlespec, 4 * sizeof(float));
		memcpy(candleMesh.mat.emissive, candleemissive, 4 * sizeof(float));
		candleMesh.mat.shininess = candleshininess;
		candleMesh.mat.texCount = candletexcount;

		candle = MyObject(candleMesh, My3DVector(position.x, position.y, position.z), My3DVector(1, 1, 1), {});

		light_position[0] = position.x;
		light_position[1] = 30 + 2;
		light_position[2] = position.z;
		light_position[3] = 1.0;
}

void MyCandle::render(VSShaderLib& shader) {
	candle.render(shader, "candle");
}



void MyObject::render(VSShaderLib& shader, std::string objectType) {
	GLint loc;

	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mesh.mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mesh.mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mesh.mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mesh.mat.shininess);

	pushMatrix(MODEL);
	translate(MODEL, position.x, position.y, position.z);
	for (My3DVector MyRotation : rotations) {
		rotate(MODEL, MyRotation.angle, MyRotation.x, MyRotation.y, MyRotation.z);
	}
	scale(MODEL, scaleSize.x, scaleSize.y, scaleSize.z);
	for (My3DVector MyCenter : center) {
		translate(MODEL, MyCenter.x, MyCenter.y, MyCenter.z);
	}

	GLint pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	GLint vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	GLint normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	GLint texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");// different modes of texturing

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	//TEXTURES FOR OBJECT
	if (objectType.compare("table") == 0) glUniform1i(texMode_uniformId, 0); //road + grass 0+1
	
	else if (objectType.compare("cheerio") == 0) glUniform1i(texMode_uniformId, 2);
	else if (objectType.compare("orange") == 0) glUniform1i(texMode_uniformId, 3);
	else if (objectType.compare("tree") == 0) glUniform1i(texMode_uniformId, 4);
	else if (objectType.compare("butter") == 0) glUniform1i(texMode_uniformId, 6);
	else if (objectType.compare("candle") == 0) glUniform1i(texMode_uniformId, 7);
	else glUniform1i(texMode_uniformId, 5);


	

	// Render mesh
	glBindVertexArray(mesh.vao);

	if (!shader.isProgramValid()) {
		printf("Program Not Valid!\n");
		exit(1);
	}

	glDrawElements(mesh.type, mesh.numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	popMatrix(MODEL);

}