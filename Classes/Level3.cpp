#include "Level3.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;

Level_three::~Level_three()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	//	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level_three::createScene()
{
	return Level_three::create();
}

// on "init" you need to initialize your instance
bool Level_three::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("number.plist");
	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	//標題 : 顯示目前 BOX2D 所介紹的功能
	_titleLabel = Label::createWithTTF("Level_four", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_titleLabel->getContentSize().width * 0.5f + 125.f, _visibleSize.height - _titleLabel->getContentSize().height * 0.5f - 5.f);
	this->addChild(_titleLabel, 2);
	
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new (std::nothrow) b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("Level_three.csb");
	_oneSec = 0;
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);
	
	createStaticBoundary();
	setupStartBall();
	setupSensor();
	_bstartDraw = false;
	drawnum = 0;
	setupShoot();
	_fShoottime = 10.0f;
	_iBubblenum = 0;
	//倒數
	std::ostringstream ostr;
	std::string objname;
	ostr.str("");
	ostr << "Countdown:" << _fShoottime; objname = ostr.str();
	_countLabel = Label::createWithTTF(objname, "fonts/Marker Felt.ttf", 32);
	_countLabel->setPosition(_countLabel->getContentSize().width * 0.5f+500, _visibleSize.height - _countLabel->getContentSize().height * 0.5f-10);
	this->addChild(_countLabel, 2);
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

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(Level_three::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(Level_three::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(Level_three::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level_three::update));

	return true;
}
void Level_three::setupStartBall() {
	startBallSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("start_ball"));
	//auto circleSprite =dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel_01"));
	ballLoc = startBallSprite->getPosition();
	ballSize = startBallSprite->getContentSize();
	float scale = startBallSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = ballSize.width * 0.5f * scale / PTM_RATIO;

	b2BodyDef bodyDef;	//先以結構 b2BodyDef 宣告一個 Body 的變數
	bodyDef.type = b2_dynamicBody;		// 設定為動態物體
	bodyDef.position.Set(ballLoc.x / PTM_RATIO, ballLoc.y / PTM_RATIO);
	bodyDef.userData = startBallSprite;	// 設定圖示
	_StartBallBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;			// 設定物理性質
	fixtureDef.shape = &circleShape;	// 宣告物體的外型物件變數是圓形物體
	fixtureDef.restitution = 0.75f;			// 設定恢復係數
	fixtureDef.density = 5.0f;				// 設定密度
	fixtureDef.friction = 0.15f;			// 設定摩擦係數
	_StartBallBody->CreateFixture(&fixtureDef);
}
void Level_three::setupSensor() {
	std::ostringstream ostr;
	std::string objname;
	for (int i = 1; i <= 2; i++)
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
void Level_three::setupShoot()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	shootbody = _b2World->CreateBody(&bodyDef);
	b2PolygonShape triShape;
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &triShape;
	Point pntLoc = _csbRoot->getPosition();
	for (size_t i = 1; i <= 1; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		std::ostringstream ostr;
		std::string objname;
		ostr << "shoot_start"; objname = ostr.str();

		const auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float angle = triSprite->getRotation();
		float scaleX = triSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = triSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep[3], wep[3];	// triShape 的三個頂點, 0 頂點、 1 左下、 2 右下
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = pntLoc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 3; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO) };

		triShape.Set(vecs, 3);
		shootbody->CreateFixture(&fixtureDef);
	}
}
void Level_three::setReplay() {
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("replay"));
	Replaybt = CButton::create();
	Replaybt->setButtonInfo("replay.png", "replay.png", btnSprite->getPosition());
	Replaybt->setScale(btnSprite->getScale());
	this->addChild(Replaybt, 3);

}
void Level_three::update(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite* ballData = (Sprite*)(body->GetUserData());
			ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
	
		

	float Sensor1minX = sensorSprite[1]->getPosition().x - (Sensorsize[1].width) / 2;
	float Sensor1maxX = sensorSprite[1]->getPosition().x + (Sensorsize[1].width) / 2;
	float Sensor1minY = sensorSprite[1]->getPosition().y - (Sensorsize[1].height) / 2;
	float Sensor1maxY = sensorSprite[1]->getPosition().y + (Sensorsize[1].height) / 2;
	float PutRectPosX = startBallSprite->getPosition().x;
	float PutRectPosY = startBallSprite->getPosition().y;
			
	if (PutRectPosX > Sensor1minX && PutRectPosX<Sensor1maxX && PutRectPosY>Sensor1minY && PutRectPosY < Sensor1maxY)
	{
		_StartBallBody->ApplyLinearImpulse(b2Vec2(-120, 1000), _StartBallBody->GetWorldCenter(), true);
	}
	float Sensor2minX = sensorSprite[2]->getPosition().x - (Sensorsize[2].width) / 2;
	float Sensor2maxX = sensorSprite[2]->getPosition().x + (Sensorsize[2].width) / 2;
	float Sensor2minY = sensorSprite[2]->getPosition().y - (Sensorsize[2].height) / 2;
	float Sensor2maxY = sensorSprite[2]->getPosition().y + (Sensorsize[2].height) / 2;
	if (PutRectPosX > Sensor2minX && PutRectPosX<Sensor2maxX && PutRectPosY>Sensor2minY && PutRectPosY < Sensor2maxY) {
		log("pass");
		_bSparking = true;
		if (_bSparking) { //可以噴發，實現這次的噴發
			_tdelayTime = 0; // 時間重新設定，
			_bSparking = false; // 開始計時
			for (int i = 0; i < 20; i++) {
				// 建立 Spark Sprite 並與目前的物體結合
				SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
				auto sparkSprite = Sprite::createWithSpriteFrameName("spark.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//產生小方塊資料
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(600 / PTM_RATIO,400 / PTM_RATIO);
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
				RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50 + rand() % 30), RectBody->GetWorldCenter(), true);
			}
		}
		Finish = Label::createWithTTF("Finish the Game", "fonts/Marker Felt.ttf", 64);
		Finish->setPosition(Finish->getContentSize().width * 0.5f + 500, _visibleSize.height - Finish->getContentSize().height * 0.5f - 300);
		this->addChild(Finish, 2);
	}
	if (!_bSparking) {
		_tdelayTime += dt;
		if (_tdelayTime >= 0.075f) {
			_tdelayTime = 0; // 歸零
			_bSparking = true; // 可進行下一次的噴發
		}
	}
	if (_fShoottime <= 0&& _iBubblenum<20) {
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		shootSprite[_iBubblenum] = Sprite::createWithSpriteFrameName("bubble.png");
		
		shootSprite[_iBubblenum]->setPosition(Vec2(shootSprite[_iBubblenum]->getPosition().x, shootSprite[_iBubblenum]->getPosition().y));
		this->addChild(shootSprite[_iBubblenum], 2);

		// 建立一個簡單的動態球體
		b2BodyDef bodyDef;	// 先以結構 b2BodyDef 宣告一個 Body 的變數
		bodyDef.type = b2_dynamicBody; // 設定為動態物體
		bodyDef.userData = shootSprite[_iBubblenum];	// 設定 Sprite 為動態物體的顯示圖示
		bodyDef.position.Set(100/ PTM_RATIO, 600 / PTM_RATIO);
		// 以 bodyDef 在 b2World  中建立實體並傳回該實體的指標
		b2Body* ballBody = _b2World->CreateBody(&bodyDef);
		// 設定該物體的外型
		b2CircleShape ballShape;	//  宣告物體的外型物件變數，此處是圓形物體
		Size ballsize = shootSprite[_iBubblenum]->getContentSize();	// 根據 Sprite 圖形的大小來設定圓形的半徑
		ballShape.m_radius = 0.75f * (ballsize.width - 4) * 0.5f / PTM_RATIO;
		// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
		b2FixtureDef fixtureDef;	 // 固定裝置
		fixtureDef.shape = &ballShape;			// 指定剛體的外型為圓形
		fixtureDef.restitution = 0.75f;			// 設定恢復係數
		fixtureDef.density = 5.0f;				// 設定密度
		fixtureDef.friction = 0.15f;			// 設定摩擦係數
		ballBody->CreateFixture(&fixtureDef);	// 在 Body 上產生這個剛體的設定
		 ballBody->ApplyLinearImpulse(b2Vec2(300, 20), ballBody->GetWorldCenter(), true);
		// GetWorldCenter(): Get the world position of the center of mass
		_iBubblenum++;
		_fShoottime = 5.0f;
	}	
	_fShoottime -= dt;
	_oneSec += dt;
	if (_oneSec >= 0.5) {
		std::ostringstream ostr;
		std::string objname;
		int time = (int)_fShoottime;
		ostr.str("");
		ostr << "Countdown:" << (int)_fShoottime; objname = ostr.str();
		_countLabel->setString(objname);
		_oneSec = 0;
	}
	
}

bool Level_three::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_bstartDraw = true;
	if (Replaybt->touchesBegin(touchLoc))
	{
		this->unschedule(schedule_selector(Level_three::update));
		Director::getInstance()->replaceScene(Level_three::createScene());
	}
	return true;

}

void  Level_three::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	if (_bstartDraw&& drawnum<100&& _pvLoc!= touchLoc) {
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		drawrectSprite[drawnum] = Sprite::createWithSpriteFrameName("rect01.png");
		drawrectSprite[drawnum]->setScale(0.5);
		this->addChild(drawrectSprite[drawnum], 2);
		drawbodyDef[drawnum].type = b2_staticBody; // 設定這個 Body 為 靜態的
		drawbodyDef[drawnum].userData = drawrectSprite[drawnum];
		drawbodyDef[drawnum].position.Set(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
		_drawbody[drawnum] = _b2World->CreateBody(&drawbodyDef[drawnum]);
		
		
		
		Size ts = drawrectSprite[drawnum]->getContentSize();
		Point loc = drawrectSprite[drawnum]->getPosition();
		float angle = drawrectSprite[drawnum]->getRotation();
		float scaleX = drawrectSprite[drawnum]->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = drawrectSprite[drawnum]->getScaleY();	// 水平	的線段圖示假設都只有對 X 軸放大

		// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;
		
		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = loc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };
		b2PolygonShape rectShape;
		drawfixtureDef; // 產生 Fixture
		drawfixtureDef.shape = &rectShape;
		rectShape.Set(vecs, 4);
		_drawbody[drawnum]->CreateFixture(&drawfixtureDef);
		drawnum++;
		_pvLoc = touchLoc;
	}
}

void  Level_three::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_bstartDraw = false;
	
	
		for (int i = _ipvdrawnum; i < drawnum; i++)
		{
			_drawbody[i]->SetType(b2_dynamicBody);
			b2WeldJointDef JointDef;
			if (i + 1 != drawnum)
			{
				JointDef.Initialize(_drawbody[i], _drawbody[i + 1], _drawbody[i]->GetWorldCenter() + b2Vec2(10 / PTM_RATIO, 0));
				_b2World->CreateJoint(&JointDef); // 使用預設值焊接
			}
			else 
			{
				_ipvdrawnum = drawnum;
				_botherdraw = true;
			}

		}
	
	if (Replaybt->touchesEnded(touchLoc));
	

	
	
}
void Level_three::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
	edgeFixtureDef.shape = &edgeShape;
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

	// 讀取所有 wall1_ 開頭的圖示 當成是靜態物體
	// 程式碼來自 StaticDynamicScene.cpp
	char tmp[20] = "";

	// 產生 EdgeShape 的 body
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	body = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 1; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		std::ostringstream ostr;
		std::string objname;
		ostr << "road_0" << i; objname = ostr.str();
		auto edgeSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep1, lep2, wep1, wep2; // EdgeShape 的兩個端點
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // 先設定 X 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = loc.y; //設定 Translation，自己的加上父親的

											 // 產生兩個端點
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}
	b2PolygonShape triShape;
	fixtureDef.shape = &triShape;
	Point pntLoc = _csbRoot->getPosition();
	for (size_t i = 1; i <= 2; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		std::ostringstream ostr;
		std::string objname;
		ostr << "end_0" << i; objname = ostr.str();

		const auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float angle = triSprite->getRotation();
		float scaleX = triSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = triSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep[3], wep[3];	// triShape 的三個頂點, 0 頂點、 1 左下、 2 右下
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = pntLoc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 3; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO) };

		triShape.Set(vecs, 3);
		body->CreateFixture(&fixtureDef);
	}
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	for (size_t i = 1; i <= 4; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		std::ostringstream ostr;
		std::string objname;
		ostr << "static_0" << i; objname = ostr.str();

		auto const rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = rectSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

		// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = pntLoc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };

		rectShape.Set(vecs, 4);
		body->CreateFixture(&fixtureDef);
	}
}
#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level_three::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif