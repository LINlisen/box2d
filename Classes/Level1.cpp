#include "Level1.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#define  CREATED_REMOVED
#ifdef CREATED_REMOVED
int g_totCreated = 0, g_totRemoved = 0;
#endif
USING_NS_CC;
using namespace cocostudio::timeline;
Color3B filterColor[3] = { Color3B(208,45,45), Color3B(77,204,42), Color3B(14,201,220) };

#ifndef MAX_CIRCLE_OBJECTS
#define MAX_CIRCLE_OBJECTS  11
#endif
extern char g_CircleObject[MAX_CIRCLE_OBJECTS][20]; 

Level_one::~Level_one()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level_one::createScene()
{
	return Level_one::create();
}

bool Level_one::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("Level_one.csb");
	pntLoc = _csbRoot->getPosition();
	_titleLabel = Label::createWithTTF("Level_two", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_titleLabel->getContentSize().width * 0.5f + 100.f, _visibleSize.height - _titleLabel->getContentSize().height * 0.5f - 5.f);
	this->addChild(_titleLabel, 2);
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);
#endif
	addChild(_csbRoot, 1);
	_igreen = 2;
	_ired = 1;
	_bminus = false;
	r[1] = 100;
	r[2] = 100;
	r[3] = 100;
	g[1] = 100;
	g[2] = 100;
	g[3] = 100;
	_brzero = false;
	_bgzero = false;
	_fbluedisscale=1.0f;
	_freddisscale = 1.0f;
	_fgreendisscale = 1.0f;
	//setStaticWalls();
	createStaticBoundary();
	setupDesnity();
	setupFrictionAndFilter();
	//setupSensorAndCollision();
	setupGearJoint();
	setupSensor();
	_fcount=0.0f;
	_fduration=1.0f;
	setReplay();
	
#ifdef BOX2D_DEBUG
	
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level_one::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level_one::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level_one::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level_one::doStep));

	return true;
}

void Level_one::setupDesnity() {
	Point pntLoc = _csbRoot->getPosition();

	b2BodyDef bodyDef;
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2FixtureDef fixtureDef; // 產生 Fixture
	std::ostringstream ostr;
	std::string objname;

	// 產生三角形靜態物體所需要的 triShape
	// 產生蹺蹺板底座的三角形
	b2Body* seesawBasedbody;
	b2PolygonShape triShape;
	fixtureDef.shape = &triShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.8f;
	for (size_t i = 1; i <= 1; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		ostr.str("");
		ostr << "triangle1_0" << i; objname = ostr.str();
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float scaleX = triSprite->getScaleX();	// 對 X 軸放大
		float scaleY = triSprite->getScaleY();	// 對 Y 軸放大

		Point lep[3], wep[3];	// triShape 的三個頂點, 0 頂點、 1 左下、 2 右下
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;

		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		for (size_t j = 0; j < 3; j++)
		{   // 納入縮放與旋轉的 local space 的座標計算
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO)
		};
		triShape.Set(vecs, 3);
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		seesawBasedbody = _b2World->CreateBody(&bodyDef);
		seesawBasedbody->CreateFixture(&fixtureDef);
	}
		// 產生蹺蹺板，此為動態，每一個動態物體在 b2World 中都必須建立實體
	// 設定這個 Body 為 動態的，並在 _b2World 中建立實體，
		bodyDef.type = b2_dynamicBody;
		b2PolygonShape seesawBoardShape;
		fixtureDef.shape = &seesawBoardShape;
		fixtureDef.density = 3.0f;
		fixtureDef.friction = 0.1f;
		fixtureDef.restitution = 0.1f;
		// 取得代表蹺蹺板的圖示，並設定成 sprite

		auto boardSprite = (Sprite*)_csbRoot->getChildByName("seesawBoard");
		
		
		bodyDef.userData = boardSprite;
		Size ts = boardSprite->getContentSize();
		Point loc = boardSprite->getPosition();
		float scaleX = boardSprite->getScaleX();	// 對矩形圖示 X 軸縮放值
		float scaleY = boardSprite->getScaleY();	// 對矩形圖示 Y 軸縮放值
		// 設定板子所在的位置，因為是使用 joint 可以不用設定位置
//	bodyDef.position.Set(loc.x/ PTM_RATIO, loc.y/ PTM_RATIO); 
		b2Body* seesawBoardbody = _b2World->CreateBody(&bodyDef); // 在 b2World 中建立實體

		// 算出 seesawBoard 的縮放後的寬高, 4 為預留的寬度，不跟其他的圖片重疊
		float bw = (ts.width - 4) * scaleX;
		float bh = (ts.height - 4) * scaleY;

		// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
		seesawBoardShape.SetAsBox(bw * 0.5f / PTM_RATIO, bh * 0.5f / PTM_RATIO);
		seesawBoardbody->CreateFixture(&fixtureDef);

		// 建立與基底三角形的 Joint 連結
		b2RevoluteJointDef seesawJoint;
		seesawJoint.bodyA = seesawBasedbody;
		seesawJoint.localAnchorA.Set(0, 1.4f);
		seesawJoint.bodyB = seesawBoardbody;
		seesawJoint.localAnchorB.Set(-1.5f, 0);
		_b2World->CreateJoint(&seesawJoint);

		
		//put
		
}
void Level_one::setupFrictionAndFilter() {
	//score
	for (int i = 0; i < 2; i++) {
		std::ostringstream ostr;
		std::string objname;
		if (i == 0) {
			ostr << "red:" << _ired;
		}
		else
		{
			ostr << "green:" << _igreen;
		}
		objname = ostr.str();
		_scoreLabel[i] = Label::createWithTTF(objname, "fonts/Marker Felt.ttf", 32);
		_scoreLabel[i]->setPosition(_visibleSize.width / 2 - 200.0f, _visibleSize.height * 0.95f - i * 30);
		this->addChild(_scoreLabel[i], 1);
	}
	//bt1
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("putbt_01"));
	_rectButton_one = CButton::create();
	_rectButton_one->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_rectButton_one->setScale(btnSprite->getScale());
	this->addChild(_rectButton_one, 3);
	btnSprite->setVisible(false);
	_iNumofRect_one = 0;
	//bt2
	auto btnSprite2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("putbt_02"));
	_rectButton_two = CButton::create();
	_rectButton_two->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite2->getPosition());
	_rectButton_two->setScale(btnSprite2->getScale());
	this->addChild(_rectButton_two, 3);
	btnSprite2->setVisible(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Body* rectbody;
	b2FixtureDef fixtureDef; // 產生 Fixture
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	/*std::ostringstream ostr;
	std::string objname;*/

	// 設定三個不同顏色代表三個碰撞測試的群組
	for (int i = 1; i <= 2; i++)
	{
		std::ostringstream ostr;
		std::string objname;

		ostr.str("");
		ostr << "filter1_0" << i; objname = ostr.str();
		// 取得三個設定碰撞過濾器的靜態物體圖示
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		auto rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[(i - 1)]);	// 使用 filterColor 已經建立的顏色
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();	// 讀取矩形畫框有對 X 軸縮放
		float scaleY = rectSprite->getScaleY();	// 讀取矩形畫框有對 Y 軸縮放

		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO); // 設定板子所在的位置
		rectbody = _b2World->CreateBody(&bodyDef); // 在 b2World 中建立實體
		float bw = (ts.width - 4) * scaleX;
		float bh = (ts.height - 4) * scaleY;
		// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
		rectShape.SetAsBox(bw * 0.5f / PTM_RATIO, bh * 0.5f / PTM_RATIO);
		fixtureDef.filter.categoryBits = 1 << i;
		rectbody->CreateFixture(&fixtureDef);
	}
}
void Level_one::changeNum() {
	for (int i = 0; i < 2; i++) {
		std::ostringstream ostr;
		std::string objname;
		if (i == 0) {
			ostr << "red:" << _ired;
			objname = ostr.str();
			_scoreLabel[i]->setString(objname);
		}
		else
		{
			ostr << "green:" << _igreen;
			objname = ostr.str();
			_scoreLabel[i]->setString(objname);
		}
		
		
	}
}
void Level_one::doStep(float dt)
{

	update(dt);
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	_b2World->Step(dt, velocityIterations, positionIterations);
	/*log("green:%d", _igreen);
	log("red:%d", _ired);*/
	if (_igreen <= 0 && _ired <= 0) {
		log("pass");
		this->unschedule(schedule_selector(Level_one::update));
		Director::getInstance()->replaceScene(Level_two::createScene());
	}
	for (b2Body* body = _b2World->GetBodyList(); body; )
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite* spriteData = (Sprite*)(body->GetUserData());
			spriteData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}


		// 跑出螢幕外面就讓物體從 b2World 中移除
		if (body->GetType() == b2BodyType::b2_dynamicBody) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;
			if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite*)(body->GetUserData());
					this->removeChild(spriteData);
				}
				b2Body* nextbody = body->GetNext(); // 取得下一個 body
				_b2World->DestroyBody(body); // 釋放目前的 body
				body = nextbody;  // 讓 body 指向剛才取得的下一個 body
#ifdef CREATED_REMOVED
				g_totRemoved++;
				//CCLOG("Removing %4d Particles", g_totRemoved);
#endif
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}

		else body = body->GetNext(); //否則就繼續更新下一個Body
	}


	g_totCreated += _contactListener._NumOfSparks;
	//CCLOG("Creating %4d Particles", g_totCreated);
	
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//產生完關閉
		if (_bSparking) { //可以噴發，實現這次的噴發
			_tdelayTime = 0; // 時間重新設定，
			_bSparking = false; // 開始計時
			for (int j = 0; j < 10; j++) {
				// 建立 Spark Sprite 並與目前的物體結合
				SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
				auto sparkSprite = Sprite::createWithSpriteFrameName("circle.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//產生小方塊資料
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(_storePos.x + 0, _storePos.y + (-30) / PTM_RATIO);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = sparkSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture* RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//給力量
				RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50 + rand() % 30), _storePos + b2Vec2(0, -30 / PTM_RATIO), true);
			}
#ifdef CREATED_REMOVED
			g_totCreated += _contactListener._NumOfSparks;
			CCLOG("Creating %4d Particles", g_totCreated);
#endif
		}
	}
	if (!_bSparking) {
		_tdelayTime += dt;
		if (_tdelayTime >= 0.075f) {
			_tdelayTime = 0; // 歸零
			_bSparking = true; // 可進行下一次的噴發
		}
	}
}
			
void Level_one::update(float dt) {
	for (int i = 0; i < _iNumofRect_one; i++) {
		if (PutrectSprite[i] != NULL && _btest[i] == false) {


			float Sensor1minX = sensorSprite[1]->getPosition().x - (Sensorsize[1].width) / 2;
			float Sensor1maxX = sensorSprite[1]->getPosition().x + (Sensorsize[1].width) / 2;
			float Sensor1minY = sensorSprite[1]->getPosition().y - (Sensorsize[1].height) / 2;
			float Sensor1maxY = sensorSprite[1]->getPosition().y + (Sensorsize[1].height) / 2;
			float PutRectPosX = PutrectSprite[i]->getPosition().x;
			float PutRectPosY = PutrectSprite[i]->getPosition().y;

			if (PutRectPosX > Sensor1minX && PutRectPosX<Sensor1maxX && PutRectPosY>Sensor1minY && PutRectPosY < Sensor1maxY) {
				if (_bluefcount < _fduration) {
					_bluefcount += dt;
					if (_fbluedisscale > 0) {
						_fbluedisscale -= _bluefcount;
					}
					else if (_fbluedisscale <= 0) {
						_fbluedisscale = 0;
					}
					PutrectSprite[i]->setScale(_fbluedisscale);
				}
				else
				{
					log("disappear");
					Putbody[i]->SetActive(false);
					PutrectSprite[i]->setVisible(false);
					_bluefcount = 0;
					_fbluedisscale = 0.75;
				}
			}
			_brecyle = false;
			float Sensor2minX = sensorSprite[2]->getPosition().x - (Sensorsize[2].width) / 2;
			float Sensor2maxX = sensorSprite[2]->getPosition().x + (Sensorsize[2].width) / 2;
			float Sensor2minY = sensorSprite[2]->getPosition().y - (Sensorsize[2].height) / 2;
			float Sensor2maxY = sensorSprite[2]->getPosition().y + (Sensorsize[2].height) / 2;
			if (PutRectPosX > Sensor2minX && PutRectPosX<Sensor2maxX && PutRectPosY>Sensor2minY && PutRectPosY < Sensor2maxY) {
				/*if (_fcount < _fduration) {
					_fcount += dt;
					PutrectSprite[i]->setScale(1.0f - _fcount);
				}*/
				/*else {
					log("disappear");
					Putbody[i]->SetActive(false);
					PutrectSprite[i]->setVisible(false);
					_fcount = 0;
				}*/
				Putbody[i]->ApplyLinearImpulse(b2Vec2(-10, 2), Putbody[i]->GetWorldCenter(), true);
			}

			float Sensor3minX = sensorSprite[3]->getPosition().x - (Sensorsize[3].width) / 2;
			float Sensor3maxX = sensorSprite[3]->getPosition().x + (Sensorsize[3].width) / 2;
			float Sensor3minY = sensorSprite[3]->getPosition().y - (Sensorsize[3].height) / 2;
			float Sensor3maxY = sensorSprite[3]->getPosition().y + (Sensorsize[3].height) / 2;
			float Sensor4minX = sensorSprite[4]->getPosition().x - (Sensorsize[4].width) / 2;
			float Sensor4maxX = sensorSprite[4]->getPosition().x + (Sensorsize[4].width) / 2;
			float Sensor4minY = sensorSprite[4]->getPosition().y - (Sensorsize[4].height) / 2;
			float Sensor4maxY = sensorSprite[4]->getPosition().y + (Sensorsize[4].height) / 2;
			if (PutRectPosX > Sensor3minX && PutRectPosX<Sensor3maxX && PutRectPosY>Sensor3minY && PutRectPosY < Sensor3maxY && i != g[1] && i != g[2] && i != g[3]) {
				if (!_bminus) {
					_igreen -= 1;
					_bminus = true;
					_bSparking = true;
					log("green--");
					_storePos = Putbody[i]->GetWorldCenter();
					_fdefstoreX = PutbodyDef->position.x;
					_fdefstoreY = PutbodyDef->position.y;
					_contactListener.setCollisionTarget(*PutrectSprite[i]);
					_tdelayTime = 0; // 第一次一定可以噴發

				}
				std::ostringstream ostr;
				std::string objname;

				/*ostr << "green" << _igreen;
				objname = ostr.str();
				_scoreLabel[i] = Label::createWithTTF(objname, "fonts/Marker Felt.ttf", 32);*/

				if (_fcount < _fduration) {
					_fcount += dt;
					if (_fgreendisscale > 0) {
						_fgreendisscale -= _fcount;
					}
					else if (_fgreendisscale <= 0) {
						_fgreendisscale = 0;
					}
					PutrectSprite[i]->setScale(_fgreendisscale);
				}
				else {
					Putbody[i]->SetActive(false);
					PutrectSprite[i]->setVisible(false);
					PutrectSprite[i]->removeChild(PutrectSprite[i], true);
					_fcount = 0;
					_bminus = false;
					g[_igreen] = i;
					_btest[i] = true;
					_fgreendisscale = 0.75f;
					/*if (_igreen == 0) {
						_brzero = false;
					}*/
					changeNum();
				}
			}
			if (PutRectPosX > Sensor4minX && PutRectPosX<Sensor4maxX && PutRectPosY>Sensor4minY && PutRectPosY < Sensor4maxY && i != r[1] && i != r[2] /*&& i != r[3]*/) {
				if (!_bminus) {
					_bminus = true;
					_ired -= 1;
					_bSparking = true;
					log("red--");
					_storePos = Putbody[i]->GetWorldCenter();
					_fdefstoreX = PutbodyDef->position.x;
					_fdefstoreY = PutbodyDef->position.y;
					_contactListener.setCollisionTarget(*PutrectSprite[i]);
					_tdelayTime = 0; // 第一次一定可以噴發

				}

				/*std::ostringstream ostr;
				std::string objname;
				ostr << "red" << _ired;
				objname = ostr.str();
				_scoreLabel[i] = Label::createWithTTF(objname, "fonts/Marker Felt.ttf", 32);*/
				if (_redfcount < _fduration) {
					_redfcount += dt;
					if (_freddisscale > 0) {
						_freddisscale -= _redfcount;
					}
					else if (_freddisscale <= 0) {
						_freddisscale = 0;
					}
					PutrectSprite[i]->setScale(_freddisscale);
				}
				else {
					Putbody[i]->SetActive(false);
					PutrectSprite[i]->setVisible(false);
					PutrectSprite[i]->removeChild(PutrectSprite[i], true);
					_redfcount = 0;
					_freddisscale = 0.75f;
					_bminus = false;
					r[_ired] = i;
					_btest[i] = true;
					/*if (_ired == 0) {
						_brzero = false;
					}*/
					changeNum();
				}
			}
		}
	}
}
//ratation
void Level_one::setupGearJoint()
{
	Sprite* gearSprite[9] = { nullptr };
	Point loc[9];
	Size size[9];
	float scale[9] = { 0 };
	float angle[9] = { 0 };
	b2Body* staticBody[9] = { nullptr };
	b2Body* dynamicBody[9] = { nullptr };
	b2RevoluteJoint* RevJoint[9] = { nullptr };
	b2PrismaticJoint* PriJoint[9] = { nullptr };

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;
	b2PolygonShape polyShape;
	
	for (int i = 0; i < 3; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "rotation01_0" << i; objname = ostr.str();
		gearSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();
		scale[i] = gearSprite[i]->getScale();
		angle[i] = gearSprite[i]->getRotation();
		if (i < 1)
		{
			staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
			staticBody[i] = _b2World->CreateBody(&staticBodyDef);
			staticBody[i]->CreateFixture(&fixtureDef);
		}
		else {
			b2BodyDef bodyDef;
			bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
			bodyDef.userData = NULL;
			b2Body* body = _b2World->CreateBody(&bodyDef);
			b2EdgeShape edgeShape;
			b2FixtureDef fixtureDef; // 產生 Fixture
			fixtureDef.shape = &edgeShape;
			Point lep1, lep2, wep1, wep2; // EdgeShape 的兩個端點 // world edge point1 & point2 
			lep1.y = 0; lep1.x = -(size[i].width - 4) / 2.0f;  // local edge point1 & point2
			lep2.y = 0; lep2.x = (size[i].width - 4) / 2.0f;

			// 所有的線段圖示都是是本身的中心點為 (0,0)，
			// 根據縮放、旋轉產生所需要的矩陣
			// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
			// 然後進行旋轉，
			// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scale[i];  // 先設定 X 軸的縮放
			cocos2d::Mat4::createRotationZ(angle[i] * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix); //  modelMatrix = rotMatrix*modelMatrix
			modelMatrix.m[3] = pntLoc.x + loc[i].x; //設定 Translation，自己的加上父親的, (pntLoc = csbRoot->getPosition())
			modelMatrix.m[7] = pntLoc.y + loc[i].y; //設定 Translation，自己的加上父親的

			// 產生兩個端點
			wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
			wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
			wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
			wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

			// bottom edge
			edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
			staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
			staticBody[i] = _b2World->CreateBody(&staticBodyDef);
			staticBody[i]->CreateFixture(&fixtureDef);
			
		}
	}
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;

	b2CircleShape circleShape;
	
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;




	// 第六個是矩形要另外處裡
	for (int i = 0; i < 3; i++)
	{
		if (i < 1)
		{
			circleShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
			dynamicBodyDef.userData = gearSprite[i];
			dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
			dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
			dynamicBody[i]->CreateFixture(&fixtureDef);
		}
		else {
			/*staticBodyDef.userData = gearSprite[i];*/
			float sx = gearSprite[i]->getScaleX();
			float sy = gearSprite[i]->getScaleY();
			fixtureDef.shape = &polyShape;
			fixtureDef.density = 10.0f;
			fixtureDef.friction = 1.0f;
			polyShape.SetAsBox((size[i].width - 4) * 0.5f * sx / PTM_RATIO, (size[i].height - 4) * 0.5f * sy / PTM_RATIO);
			dynamicBodyDef.userData = gearSprite[i];
			dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
			dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
			dynamicBody[i]->CreateFixture(&fixtureDef);
		}
	}
	b2RevoluteJointDef RJoint;
	b2RevoluteJointDef PrJoint;
	for (int i = 0; i < 3; i++)
	{
		if (i < 1) {
			RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
			RevJoint[i] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
		}
		else {
			PrJoint.Initialize(staticBody[0], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
			PriJoint[i] = (b2PrismaticJoint*)_b2World->CreateJoint(&PrJoint);
			if (i == 1) {
				b2GearJointDef GJoint;
				GJoint.bodyA = dynamicBody[0];
				GJoint.bodyB = dynamicBody[i];
				GJoint.joint1 = RevJoint[0];
				GJoint.joint2 = PriJoint[i];
				GJoint.ratio = 1;
				_b2World->CreateJoint(&GJoint);
			}
			else {
				b2GearJointDef GJoint;
				GJoint.bodyA = dynamicBody[0];
				GJoint.bodyB = dynamicBody[i];
				GJoint.joint1 = RevJoint[0];
				GJoint.joint2 = PriJoint[i];
				GJoint.ratio = 1;
				_b2World->CreateJoint(&GJoint);
			}

			/*b2RevoluteJointDef rotationJoint;
			rotationJoint.bodyA = dynamicBody[0];
			rotationJoint.bodyB = staticBody[i];
			_b2World->CreateJoint(&RJoint);*/
			/*b2GearJointDef GJoint;
			GJoint.bodyA = dynamicBody[0];
			GJoint.bodyB = dynamicBody[i];
			GJoint.joint1 = RevJoint[0];
			GJoint.joint2 = RevJoint[i];
			GJoint.ratio = 1;
			_b2World->CreateJoint(&GJoint);*/
		}
	}
	
}

bool Level_one::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton_one->touchesBegin(touchLoc));

	if (_rectButton_two->touchesBegin(touchLoc));
	if (Replaybt->touchesBegin(touchLoc))
	{
		this->unschedule(schedule_selector(Level_one::update));
		Director::getInstance()->replaceScene(Level_one::createScene());
	}
	
	return true;
}

void  Level_one::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	

}

void  Level_one::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	if (Replaybt->touchesEnded(touchLoc));
	if (_rectButton_one->touchesEnded(touchLoc) && _iNumofRect_one < 50) { // 釋放一個長方形
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		PutrectSprite[_iNumofRect_one] = Sprite::createWithSpriteFrameName("frame04.png");
		PutrectSprite[_iNumofRect_one]->setScale(0.75f);
		PutrectSprite[_iNumofRect_one]->setColor(filterColor[2]);
		this->addChild(PutrectSprite[_iNumofRect_one], 2);

		
		PutbodyDef[_iNumofRect_one].type = b2_dynamicBody;
		PutbodyDef[_iNumofRect_one].userData = PutrectSprite[_iNumofRect_one];
		PutbodyDef[_iNumofRect_one].position.Set(850.0f / PTM_RATIO, 680.0f / PTM_RATIO);
		Putbody[_iNumofRect_one] = _b2World->CreateBody(&PutbodyDef[_iNumofRect_one]);
		// Define poly shape for our dynamic body.
		b2PolygonShape rectShape;
		Size rectSize = PutrectSprite[_iNumofRect_one]->getContentSize();
		rectShape.SetAsBox((rectSize.width - 4) * 0.5f * 0.75f / PTM_RATIO, (rectSize.height - 4) * 0.5f * 0.75f / PTM_RATIO);
		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &rectShape;
		fixtureDef.restitution = 0.1f;
		fixtureDef.density = 300.0f;
		fixtureDef.friction = (_iNumofRect_one + 1) * 0.1f;

		// 所有 BOX2D 物件的 filter.categoryBits 預設都是 1
		fixtureDef.filter.maskBits = 1 << (_iNumofRect_one % 3 + 1) | 1;
		Putbody[_iNumofRect_one]->CreateFixture(&fixtureDef);

		_iNumofRect_one++;
	}
	if (_rectButton_two->touchesEnded(touchLoc) && _iNumofRect_one < 50) { // 釋放一個長方形
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		PutrectSprite[_iNumofRect_one] = Sprite::createWithSpriteFrameName("frame04.png");
		PutrectSprite[_iNumofRect_one]->setScale(0.75f);
		
			PutrectSprite[_iNumofRect_one]->setColor(filterColor[_iNumofRect_one % 2]);
		
		this->addChild(PutrectSprite[_iNumofRect_one], 2);

		PutbodyDef[_iNumofRect_one].type = b2_dynamicBody;
		PutbodyDef[_iNumofRect_one].userData = PutrectSprite[_iNumofRect_one];
		PutbodyDef[_iNumofRect_one].position.Set(1000.0f / PTM_RATIO, 680.0f / PTM_RATIO);
		Putbody[_iNumofRect_one] = _b2World->CreateBody(&PutbodyDef[_iNumofRect_one]);
		// Define poly shape for our dynamic body.
		b2PolygonShape rectShape;
		Size rectSize = PutrectSprite[_iNumofRect_one]->getContentSize();
		rectShape.SetAsBox((rectSize.width - 4) * 0.5f * 0.75f / PTM_RATIO, (rectSize.height - 4) * 0.5f * 0.75f / PTM_RATIO);
		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &rectShape;
		fixtureDef.restitution = 0.1f;
		fixtureDef.density = 4.0f;
		fixtureDef.friction = 0.1f;

		// 所有 BOX2D 物件的 filter.categoryBits 預設都是 1
		
			fixtureDef.filter.maskBits = 1 << (_iNumofRect_one % 2+ 1) | 1;
		
		Putbody[_iNumofRect_one]->CreateFixture(&fixtureDef);

		_iNumofRect_one++;
	}
}
void Level_one::setupSensor() {
	std::ostringstream ostr;
	std::string objname;
	for (int i = 1; i <=4; i++)
	{
		ostr.str("");
		ostr << "sensor_0" << i; objname = ostr.str();
		sensorSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Sensorloc[i] = sensorSprite[i]->getPosition();
		/*log("locdisapear(x:%f,y:%f)", Sensorloc[i].x, Sensorloc[i].y);*/
		Sensorsize[i] = sensorSprite[i]->getContentSize();
		float scale = sensorSprite[i]->getScale();
		sensorSprite[i]->setVisible(true);
		b2BodyDef sensorBodyDef;
		sensorBodyDef.position.Set(Sensorloc[i].x / PTM_RATIO, Sensorloc[i].y / PTM_RATIO);
		sensorBodyDef.type = b2_staticBody;

		b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
		b2PolygonShape sensorShape;
		sensorShape.SetAsBox(Sensorsize[i].width * 0.5f * scale / PTM_RATIO, Sensorsize[i].height * 0.5f * scale / PTM_RATIO);

		b2FixtureDef SensorFixtureDef;
		SensorFixtureDef.shape = &sensorShape;
		SensorFixtureDef.isSensor = true;	// 設定為 Sensor
		SensorFixtureDef.density = 9999 + i; // 故意設定成這個值，方便碰觸時候的判斷
		SensorBody->CreateFixture(&SensorFixtureDef);
	}
}
void Level_one::setReplay() {
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("replay"));
	Replaybt = CButton::create();
	Replaybt->setButtonInfo("replay.png", "replay.png", btnSprite->getPosition());
	Replaybt->setScale(btnSprite->getScale());
	this->addChild(Replaybt, 3);

}
void Level_one::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
	edgeFixtureDef.shape = &edgeShape;
	edgeFixtureDef.density = 10.0f;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
}
void Level_one::removeRect(Sprite* spriteData) {
	this->removeChild(spriteData, true);

}
#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level_one::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

Level_one_CContactListener::Level_one_CContactListener()
{

	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;
}
void Level_one_CContactListener::setCollisionTarget(cocos2d::Sprite& targetSprite)
{
	_targetSprite = &targetSprite;
}
void  Level_one_CContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	log("A Density:%f", BodyA->GetFixtureList()->GetDensity());
	log("B Density:%f", BodyB->GetFixtureList()->GetDensity());
	// check 是否為落下的球經過 sensor1 ，只要經過就立刻讓他彈出去
	if (BodyA->GetFixtureList()->GetDensity() == 1003.0f) { // 代表 sensor1
		Sprite* spriteData = (Sprite*)(BodyB->GetUserData());
		BodyB->DestroyFixture(BodyB->GetFixtureList());
		log("touch");
	}
	if (BodyA->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		log("lengthV:%f", lengthV);
		if (lengthV >=2.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
	else if (BodyB->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 2.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
}
//碰撞結束
void  Level_one_CContactListener::EndContact(b2Contact* contact)
{

}