#ifndef MYOBJECT
#define MYOBJECT

#define BOARDSIZE 200
#define MAX_VELOCITY 5
#define MIN_VELOCITY 1
#define CAR_INITIAL_X -75
#define CAR_INITIAL_Y 0
#define CAR_INITIAL_Z 0
#define MAX_ORANGES 30
#define MIN_ORANGES 25
#define MAX_CHEERIO_RADIUS 2
#define MIN_CHEERIO_RADIUS 0.5
#define CHEERIO_SPACE 5


class My3DVector {
public:
	float x;
	float y;
	float z;
	float angle;

	My3DVector();
	My3DVector(float x, float y, float z);
	My3DVector(float angle, float x, float y, float z);

	void normalize2D();

	My3DVector rotatearoundX(float angle);
	My3DVector rotatearoundY(float angle);
	My3DVector rotatearoundZ(float angle);
	My3DVector inverseRotationZ(float angle);
};

class MyObject {
public:
	MyMesh mesh;
	My3DVector position;
	My3DVector scaleSize;
	std::vector<My3DVector> center;
	std::vector<My3DVector> rotations;
	float angle = 0;

	MyObject();
	MyObject(MyMesh objectMesh, My3DVector pos, My3DVector scale, std::vector<My3DVector> rotationsList);

	My3DVector updateVertice(My3DVector vertice);

	void render(VSShaderLib& shader, std::string objectType);

	//void render(VSShaderLib& shader);

};


class MyCar {
public:
	My3DVector direction;
	MyObject body;
	MyObject window;
	MyObject wheels[4];
	MyObject lights[2];
	float rotation = 0.0;
	float velocity = 0.0f;


	MyCar();
	MyCar(My3DVector position);

	void render(VSShaderLib& shader);
	void turnLeft();
	void turnRight();
	void updatePosition();
	void accelerate();
	void deccelerate();
	void breaks(bool acceleration);
	void restart();
	std::vector<My3DVector> AABB();
	void stop();
	void updateDirection();
};

class MyTable {
public:
	MyObject table;

	MyTable();
	MyTable(My3DVector position);

	void render(VSShaderLib& shader);

};

class MyOrange {
public:
	My3DVector direction;
	MyObject orange;
	float carVelocity;


	MyOrange();
	MyOrange(My3DVector position, float velocity);


	void render(VSShaderLib& shader);
	bool outOfLimits();
	void updatePosition();

};

class MyButter {
public:
	MyObject butter;

	MyButter();
	MyButter(My3DVector position);

	void updatePosition(My3DVector direction, float velocity);

	void render(VSShaderLib& shader);
	std::vector<My3DVector> AABB();
};

class MyTree {
public:
	MyObject tree;

	MyTree();
	MyTree(My3DVector position);


	void render(VSShaderLib& shader);
	bool outOfLimits();
};

class MyCheerio {
public:
	MyObject cheerio;
	My3DVector direction;

	MyCheerio();
	MyCheerio(My3DVector position);

	void render(VSShaderLib& shader);

	std::vector<My3DVector> AABB();
	void updatePosition(My3DVector direction, float velocity);

};

class MyCandle {
public:
	MyObject candle;
	float light_position[4] = { 0.0 };

	MyCandle();
	MyCandle(My3DVector position);

	void render(VSShaderLib& shader);

};

class MyRoad {
public:
	std::vector<MyCheerio> cheerios;

	MyRoad();
	MyRoad(float radius, float innerRadius);

	void render(VSShaderLib& shader);


};
#endif 