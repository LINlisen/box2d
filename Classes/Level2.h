#pragma once
//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Level1.h"
#include "Level3.h"
#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class Level_two : public cocos2d::Scene
{
public:

	~Level_two();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;
	//for car body
	Sprite* carSprite;
	Point locCar;
	Size sizeCar;
	b2Body* bodyCar;

	// for MouseJoint
	b2Body* _bottomBody; // 底部的 edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;

	// Box2D Examples
	//void readBlocksCSBFile(const std::string &);
	//void readSceneFile(const std::string &);
	void createStaticBoundary();

	void setupMouseJoint();
	void setupDistanceJoint();
	void setupPrismaticJoint();
	void setupPulleyJoint();
	void setupGearJoint();
	void setupWeldJoint();
	void setupRopeJoint();
	void setupCar();
	b2Body* bodyA;
	b2Body* wheelbodyB;
	bool _bSmoking;
	//replay
	CButton* Replaybt;
	void setReplay();
	//fire
	void setFire();
	CButton* Firebt;
	int _iFirenum;
	b2BodyDef _PutBall[3];
	cocos2d::Sprite* _sPutBall[3];
	b2Body* FireballBody[3];
	//sensor
	void setupSensor();
	Sprite* sensorSprite[3];
	Point Sensorloc[3];
	Size Sensorsize[3];
	//pushbar
	void setupPushBar();
	//sliderrope
	void setSliderRope();

#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	void update(float dt);

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

	// implement the "static create()" method manually
	CREATE_FUNC(Level_two);
};