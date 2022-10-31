//
// CGJ: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "geometry.h"

#include "avtFreeType.h"

#include "MyObject.h"
#include "Texture_Loader.h"

using namespace std;

#define CAPTION "CGJ Micromachines"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

//Vector with meshes
vector<struct MyMesh> myMeshes;


//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;

//Texture stuff
GLint tex_loc, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5, tex_loc6, tex_loc7, tex_loc8;
GLint texMode_uniformId;
unsigned int texture;
GLuint TextureArray[7];

//fog
GLint fogActivation_uniformId;
bool fogActivation = false;

//lights
bool directionalLight = false;
bool pointLight = false;
bool spotlights = false;

GLint spotlight_mode;
GLint directional_mode;
GLint pointlight_mode;

GLint ldDirection_uniformId;



string txt;
string camType = "main";
bool mouseControlActive = true;




// Mouse Tracking Variables
int startX, startY, tracking = 0;



// Camera Spherical Coordinates
float alpha = 0.0f, beta = 15;
float r = 8.0f;
float xUp = 0, yUp = 1, zUp = 0;

int deltaX, deltaY;
float alphaAux = alpha, betaAux = beta;
float rAux;

// Camera Position
float camX, camY, camZ;
float orthoHeight = BOARDSIZE / 2;
float ratio;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float lightPos[4] = { 0.0f, 5.0f, 0.0f, 1.0f };


//MyObjects
MyCar car;
MyTable table;
std::vector<MyOrange> oranges;
std::vector<MyTree> trees;
std::vector<MyButter> butters;
std::vector<MyCandle> candles;
MyRoad road;
MyTree tree;


float numOranges;
float numTree = 20;
float numButter = 4;
float numCandles = 6;
bool acceleration;
bool breaks = false;


void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeProjection() {
	loadIdentity(PROJECTION);
	if (camType.compare("orthogonal")==0) 
		ortho(-orthoHeight * ratio, orthoHeight * ratio, -orthoHeight, orthoHeight, -0.1f, 1000.0f);
	else
		perspective(53.13f, ratio, 0.1f, 1000.0f);
}

void changeSize(int w, int h) {
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	changeProjection();

}

//CHECK FOR CAR COLLISIONS

void carCollisions() {
	//create car AABB
	std::vector<My3DVector> carAABBPosition = car.AABB(); //first pos = min, second pos = max

	//Oranges
	//AABB vs Sphere 
	for (int i = 0; i < numOranges; i++) {
		//idea: var x = Math.max(box.minX, Math.min(sphere.x, box.maxX));
		float x = max(carAABBPosition[0].x, min(oranges[i].orange.position.x, carAABBPosition[1].x));
		float y = max(carAABBPosition[0].y, min(oranges[i].orange.position.y, carAABBPosition[1].y));
		float z = max(carAABBPosition[0].z, min(oranges[i].orange.position.z, carAABBPosition[1].z));

		float distance = sqrt(pow((x - oranges[i].orange.position.x), 2) +
			pow((y - oranges[i].orange.position.y), 2) +
			pow((z - oranges[i].orange.position.z), 2));

		if (distance < 1) { //distance < sphere.radius; COLLISION
			car.stop();
			car.restart();
		}
	}

	//Butter
	//AABB vs AABB 
	for (int i = 0; i < numButter; i++) {
		std::vector<My3DVector> ButterAABBPosition = butters[i].AABB(); //first pos = min, second pos = max
		if ((carAABBPosition[0].x <= ButterAABBPosition[1].x && carAABBPosition[1].x >= ButterAABBPosition[0].x) &&
			(carAABBPosition[0].y <= ButterAABBPosition[1].y && carAABBPosition[1].y >= ButterAABBPosition[0].y) &&
			(carAABBPosition[0].z <= ButterAABBPosition[1].z && carAABBPosition[1].z >= ButterAABBPosition[0].z)) {
			butters[i].updatePosition(car.direction, car.velocity);
			car.stop();
			acceleration = false;

		}
	}



	//Cheerio
	//AABB vs AABB 
	for (int i = 0; i < road.cheerios.size(); i++) {
		std::vector<My3DVector> CheerioAABBPosition = road.cheerios[i].AABB(); //first pos = min, second pos = max

		if ((carAABBPosition[0].x <= CheerioAABBPosition[1].x && carAABBPosition[1].x >= CheerioAABBPosition[0].x) &&
			(carAABBPosition[0].y <= CheerioAABBPosition[1].y && carAABBPosition[1].y >= CheerioAABBPosition[0].y) &&
			(carAABBPosition[0].z <= CheerioAABBPosition[1].z && carAABBPosition[1].z >= CheerioAABBPosition[0].z)) {
			road.cheerios[i].updatePosition(car.direction, car.velocity);
			car.stop();
			acceleration = false;
		}


	}


}

// ------------------------------------------------------------
//
// Render stufff
//

void renderScene(void) {
	float x = car.lights[0].position.x;
	float y = car.lights[0].position.y;
	float z = car.lights[0].position.z;

	GLint loc;
	float res[4];

	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	// set the camera using a function similar to gluLookAt

	if (camType == "main") {
		// set the camera position based on its spherical coordinates
		camX = r * sin((alphaAux + car.body.angle) * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camZ = r * cos((alphaAux + car.body.angle) * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
		camY = r * sin(betaAux * 3.14f / 180.0f);

		//update lookat
		lookAt(camX + car.body.position.x, camY + car.body.position.y, camZ + car.body.position.z, car.body.position.x, car.body.position.y, car.body.position.z, xUp, yUp, zUp);
	}
	else { //perspective e orthogonal
		camX = r * sin(0 * 3.14f / 180.0f) * cos(90 * 3.14f / 180.0f);
		camZ = r * cos(0 * 3.14f / 180.0f) * cos(90 * 3.14f / 180.0f);

		lookAt(camX, BOARDSIZE, camZ, 0, 0, 0, xUp, yUp, zUp);
	}


	// use our shader
	glUseProgram(shader.getProgramIndex());
	
	//fog
	fogActivation_uniformId = glGetUniformLocation(shader.getProgramIndex(), "fogActivation");
	if (fogActivation) glUniform1i(fogActivation_uniformId, 1);
	else glUniform1i(fogActivation_uniformId, 0);

	//headlights/spotlights
	loc = glGetUniformLocation(shader.getProgramIndex(), "spotlight_mode");
	if (spotlights) glUniform1i(loc, 1);
	else glUniform1i(loc, 0);

	//pointlight
	loc = glGetUniformLocation(shader.getProgramIndex(), "pointlight_mode");
	if (pointLight) glUniform1i(loc, 1);
	else glUniform1i(loc, 0);

	//Directional Light
	loc = glGetUniformLocation(shader.getProgramIndex(), "directional_mode");

	if (directionalLight) glUniform1i(loc, 1);
	else glUniform1i(loc, 0);
	
	float res_cone_dir[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float coneDir[4] = { -cos((car.body.angle - 90.0f) * 3.14 / 180), 0 , sin((car.body.angle - 90.0f) * 3.14 / 180), 0.0f };
	
	multMatrixPoint(VIEW, coneDir, res_cone_dir);
	loc = glGetUniformLocation(shader.getProgramIndex(), "coneDir");
	glUniform4fv(loc, 1, res_cone_dir);

	loc = glGetUniformLocation(shader.getProgramIndex(), "spotCosCutOff");
	glUniform1f(loc, 0.93f);

	float res_pos[8] = { 0.0 };


	// Spotlights
	for (int i = 0; i < 2; i++) {
		float light_pos[4] = { car.lights[i].position.x, car.lights[i].position.y, car.lights[i].position.z, 1.0f };
		multMatrixPoint(VIEW, light_pos, res_pos + 4*i);   //lightPos definido em World Coord so is converted to eye space
	}

	GLint sl_pos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "sl_pos");
	glUniform4fv(sl_pos_uniformId, 2, res_pos);

	float res_dl_dir[4] = { 0.0 };

	//Directional Light
	for (int i = 0; i < 1; i++) {
		float light_pos[4] = { 0.0f, -1.0f, 0.0f, 0.0f };
		multMatrixPoint(VIEW, light_pos, res_dl_dir);
	}

	GLint dl_dir_uniformId = glGetUniformLocation(shader.getProgramIndex(), "dl_dir");
	glUniform4fv(dl_dir_uniformId, 1, res_dl_dir);

	// Pointlightts

	float res_pl_pos[4 * 6] = { 0.0 };

	for (int i = 0; i < numCandles; i++) {
		float light_pos[4] = { candles[i].light_position[0] , candles[i].light_position[1], candles[i].light_position[2] , candles[i].light_position[3] };
		multMatrixPoint(VIEW, light_pos, res_pl_pos + 4*i);
	}

	GLint pp_pos_unfiromId = glGetUniformLocation(shader.getProgramIndex(), "pl_dir");
	glUniform4fv(pp_pos_unfiromId, 6, res_pl_pos);


	/*for (int i = 0; i < 2; i++) {
		//2 linhas seguintes necessário para spotlight
		//float coneDir[4] = { -cos((car.body.angle - 90.0f) * 3.14 / 180), 0 , sin((car.body.angle - 90.0f) * 3.14 / 180), 0.0f };
		//float coneDir[4] = { -car.direction.x, car.direction.y, car.direction.z, 0.0f };  //already in eye coordinates
		float coneDir[4] = { -car.lights[i].position.x, car.lights[i].position.y, car.lights[i].position.z, 0.0f };  //already in eye coordinates
		multMatrixPoint(VIEW, coneDir, res);
		loc = glGetUniformLocation(shader.getProgramIndex(), "coneDir");
		glUniform4fv(loc, 1, res);

		loc = glGetUniformLocation(shader.getProgramIndex(), "spotCosCutOff");
		glUniform1f(loc, 0.93f);
	}*/


	//Associar os Texture Units aos Objects Texture
	//road loaded in TU0;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, TextureArray[6]);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, TextureArray[7]);

	//Indicar ao sampler do GLSL quais os Texture Units a serem usados (1 por textura)
	glUniform1i(tex_loc, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_loc4, 4);
	glUniform1i(tex_loc5, 5);
	glUniform1i(tex_loc6, 6);
	glUniform1i(tex_loc7, 7);

	car.updatePosition();
	car.render(shader);
	road.render(shader);
	for (int i = 0; i < numOranges; i++) {
		if (oranges[i].outOfLimits()) {
			float velocity = rand() % (MAX_VELOCITY - MIN_VELOCITY + 1) + MIN_VELOCITY;
			oranges[i] = MyOrange::MyOrange(My3DVector(10, 0.5, 10), velocity);
		}
		oranges[i].updatePosition();
		oranges[i].render(shader);
	}
	table.render(shader);
	for (int i = 0; i < numButter; i++) {
		butters[i].render(shader);
	}
	for (int i = 0; i < numTree; i++) {
		if (trees[i].outOfLimits()) {
			trees[i] = MyTree::MyTree(My3DVector(10, 0, 10));
		}
		trees[i].render(shader);
	}
	for (int i = 0; i < numCandles; i++) {
		candles[i].render(shader);
	}

	carCollisions();
	if (breaks)
		car.breaks(acceleration);


	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer at origin looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	RenderText(shaderText, txt, 25.0f, 25.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}


// ------------------------------------------------------------
//
// Events from the Keyboard
//


void processKeysPressed(unsigned char key, int xx, int yy)
{


	switch (key) {
	case 27:
		glutLeaveMainLoop();
		break;

	case 'O':
	case 'o':
		printf("%d\n", acceleration);
		//check key to simulate it
		if (car.velocity != 0) {
			if (acceleration)
				car.accelerate();
			else
				car.deccelerate();

			car.turnLeft();
		}
		break;

	case 'P':
	case 'p':
		//check key to simulate it
		if (car.velocity != 0) {
			if (acceleration)
				car.accelerate();
			else
				car.deccelerate();

			car.turnRight();
		}
		break;

	case 'Q':
	case 'q':
		car.accelerate();
		breaks = false;
		acceleration = true;
		break;

	case 'A':
	case 'a':
		car.deccelerate();
		breaks = false;
		acceleration = false;
		break;

	case 'F':
	case 'f':
		fogActivation = !fogActivation;
		break;
	//directional light
	case 'N':
	case 'n':
		if (!directionalLight) directionalLight = true;
		else directionalLight = false;
		break;
	//Point Lights
	case 'C':
	case 'c':
		if (!pointLight) pointLight = true;
		else pointLight = false;
		break;

	case 'H':
	case 'h':
		if (!spotlights) spotlights = true;
		else spotlights = false;
		break;
	//cams
	case '1':
		mouseControlActive = false;
		camType = "orthogonal";
		changeProjection();
		break;

	case '2':
		mouseControlActive = false;
		camType = "perspective";
		changeProjection();
		break;

	case '3':
		mouseControlActive = true;
		camType = "main";
		changeProjection();
		break;
	}

}

void processKeysReleased(unsigned char key, int xx, int yy) {
	switch (key) {

	case 'Q':
	case 'q':
	case 'A':
	case 'a':
		breaks = true;
		break;
	}

}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{
	if (mouseControlActive == true) {

		deltaX = -xx + startX;
		deltaY = yy - startY;

		// left mouse button: move camera
		if (tracking == 1) {

			alphaAux = alpha + deltaX;
			betaAux = beta + deltaY;

			if (betaAux > 85.0f)
				betaAux = 85.0f;
			else if (betaAux < -85.0f)
				betaAux = -85.0f;
			rAux = r;

		}


		// right mouse button: zoom
		else if (tracking == 2) {

			alphaAux = alpha;
			betaAux = beta;
			rAux = r + (deltaY * 0.01f);
			if (rAux < 0.1f)
				rAux = 0.1f;
		}

		//  uncomment this if not using an idle or refresh func
		//	glutPostRedisplay();
	}
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	//shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight.vert");
	//shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight.frag");
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/texture_demo.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/texture_demo.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	ldDirection_uniformId = glGetUniformLocation(shader.getProgramIndex(), "ld_directions");

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");

	// Textures Get UniformID
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");
	tex_loc6 = glGetUniformLocation(shader.getProgramIndex(), "texmap6");
	tex_loc7 = glGetUniformLocation(shader.getProgramIndex(), "texmap7");

	printf("InfoLog for Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);
	srand(time(NULL));


	//Texture Object definition------------------------------------------------------------------------------------------
	glGenTextures(7, TextureArray);
	Texture2D_Loader(TextureArray, "./Texture_Materials/grass 1.jpg", 0);
	Texture2D_Loader(TextureArray, "./Texture_Materials/road.png", 1);
	Texture2D_Loader(TextureArray, "./Texture_Materials/cereal.jpg", 2);
	Texture2D_Loader(TextureArray, "./Texture_Materials/orange.jpg", 3);
	Texture2D_Loader(TextureArray, "./Texture_Materials/tree.png", 4);
	Texture2D_Loader(TextureArray, "./Texture_Materials/finish.png", 5);
	Texture2D_Loader(TextureArray, "./Texture_Materials/butter.jpg", 6);
	Texture2D_Loader(TextureArray, "./Texture_Materials/candle.jpg", 7);

	//objects
	car = MyCar::MyCar(My3DVector(CAR_INITIAL_X, CAR_INITIAL_Y, CAR_INITIAL_Z));
	table = MyTable::MyTable(My3DVector(-BOARDSIZE / 2, -1, -BOARDSIZE / 2));
	numOranges = rand() % (MAX_ORANGES - MIN_ORANGES + 1) + MIN_ORANGES;

	road = MyRoad::MyRoad(BOARDSIZE / 2 - 10, BOARDSIZE / 2 - 40);
	oranges = {};
	for (int i = 0; i < numOranges; i++) {
		float velocity = rand() % (MAX_VELOCITY - MIN_VELOCITY + 1) + MIN_VELOCITY;
		oranges.push_back(MyOrange::MyOrange(My3DVector(10, 2, 10), velocity));
	}
	trees = {};
	for (int i = 0; i < numOranges; i++) {
		trees.push_back(MyTree::MyTree(My3DVector(0, 0, 20)));
	}
	butters = { MyButter::MyButter(My3DVector(-70, 0, 50)), MyButter::MyButter(My3DVector(-40, 0, 80)), MyButter::MyButter(My3DVector(-40, 0, -80)), MyButter::MyButter(My3DVector(70, 0, 10)) };
	candles = {};
	candles.push_back(MyCandle::MyCandle(My3DVector(40, 0, 0)));
	candles.push_back(MyCandle::MyCandle(My3DVector(-40, 0, 0)));
	candles.push_back(MyCandle::MyCandle(My3DVector(40, 0, -40)));
	candles.push_back(MyCandle::MyCandle(My3DVector(-40, 0, -40)));
	candles.push_back(MyCandle::MyCandle(My3DVector(40, 0, 40)));
	candles.push_back(MyCandle::MyCandle(My3DVector(-40, 0 , 40)));



	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeysPressed);
	glutKeyboardUpFunc(processKeysReleased);

	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}







