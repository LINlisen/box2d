#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CLight.h"
#include "Level2.h"
//#define BOX2D_DEBUG 1

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class Level_one_CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite* _targetSprite;
	bool _bCreateSpark;
	bool _bApplyImpulse;
	b2Vec2 _createLoc;
	int _NumOfSparks;
	Level_one_CContactListener();
	// 碰撞
	virtual void BeginContact(b2Contact* contact);
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite& targetSprite);
	
};
class Level_one :public cocos2d::Scene
{
public:
	~Level_one();
	static cocos2d::Scene* createScene();
	Node* _csbRoot;
	//Box2D
	b2World* _b2World;
	cocos2d::Label* _titleLabel;
	cocos2d::Size _visibleSize;
	//
	Level_one_CContactListener _contactListener;
	//put
	Sprite* PutrectSprite[50];
	b2BodyDef PutbodyDef[50];
	b2Body* Putbody[50];
	Point pntLoc;
	int _iNumofRect_one;
	CButton* _rectButton_one;
	CButton* _rectButton_two;
	CButton* _rectButton_three;
	CButton* _rectButton_four;
	CButton* _rectButton_five;
	//disappear
	float _bluefcount;
	float _redfcount;
	float _fcount;
	float _fduration;
	bool _brecyle;
	//sensor
	Sprite* sensorSprite[5];
	Point Sensorloc[5];
	Size Sensorsize[5];
	float _fbluedisscale;
	float _freddisscale;
	float _fgreendisscale;
	//sensor sparking
	bool _bSparking;
	float _tdelayTime;
	b2Vec2 _storePos;
	float _fdefstoreX;
	float _fdefstoreY;
	//score
	Label* _scoreLabel[2];
	int _ired;
	int _igreen;
	bool _bminus;
	int r[4];
	int g[4];
	bool _bgzero;
	bool _brzero;
	bool _btest[100] = { false };
	//
	void setStaticWalls();
	void setupDesnity();
	void setupFrictionAndFilter();
	void setupSensorAndCollision();
	void createStaticBoundary();
	void setupGearJoint();
	void setupSensor();
	void removeRect(Sprite* spriteData);
	void changeNum();
	CButton* Replaybt;
	void setReplay();
	void update(float dt);
#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif
	virtual bool init();
	void doStep(float dt);

	bool onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent); //觸碰結束事件 

	CREATE_FUNC(Level_one);
};

