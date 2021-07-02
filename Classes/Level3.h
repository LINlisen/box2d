#pragma once


#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
//#define BOX2D_DEBUG 1
#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class Level_three : public cocos2d::Scene
{
public:

	~Level_three();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node* _csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;
	cocos2d::Label* _countLabel;
	cocos2d::Label* Finish;
	void createStaticBoundary();
	b2Body* _bottomBody; // 底部的 edgeShape
//start ball
	void setupStartBall();
	cocos2d::Sprite* startBallSprite;
	b2Body* _StartBallBody;
	cocos2d::Point ballLoc;
	cocos2d::Size  ballSize;
//sensor
	void setupSensor();
	cocos2d::Sprite* sensorSprite[3];
	cocos2d::Point Sensorloc[3];
	cocos2d::Size Sensorsize[3];
//draw
	cocos2d::Sprite* drawrectSprite[3000];
	bool _bstartDraw;
	b2Body* _drawbody[3000];
	b2FixtureDef drawfixtureDef;
	b2BodyDef drawbodyDef[3000];
	int drawnum;
	int _ipvdrawnum;
	bool _botherdraw;
	cocos2d::Point _pvLoc;
	int _pvlocnum;
	int _nowlocnum;
//start shoot
	void setupShoot();
	b2Body* shootbody;
	float _fShoottime;
	cocos2d::Sprite* shootSprite[20];
	int _iBubblenum;
	float _oneSec;
	bool _bSparking;
	float _tdelayTime;
	CButton* Replaybt;
	void setReplay();
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
	CREATE_FUNC(Level_three);
};