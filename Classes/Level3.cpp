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

	//���D : ��ܥثe BOX2D �Ҥ��Ъ��\��
	_titleLabel = Label::createWithTTF("Level_four", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_titleLabel->getContentSize().width * 0.5f + 125.f, _visibleSize.height - _titleLabel->getContentSize().height * 0.5f - 5.f);
	this->addChild(_titleLabel, 2);
	
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new (std::nothrow) b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("Level_three.csb");
	_oneSec = 0;
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
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
	//�˼�
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
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(Level_three::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(Level_three::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(Level_three::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
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

	b2BodyDef bodyDef;	//���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(ballLoc.x / PTM_RATIO, ballLoc.y / PTM_RATIO);
	bodyDef.userData = startBallSprite;	// �]�w�ϥ�
	_StartBallBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;			// �]�w���z�ʽ�
	fixtureDef.shape = &circleShape;	// �ŧi���骺�~�������ܼƬO��Ϊ���
	fixtureDef.restitution = 0.75f;			// �]�w��_�Y��
	fixtureDef.density = 5.0f;				// �]�w�K��
	fixtureDef.friction = 0.15f;			// �]�w�����Y��
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
		SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
		SensorFixtureDef.density = 9999 + i; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
		SensorBody->CreateFixture(&SensorFixtureDef);
	}
	
}
void Level_three::setupShoot()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	shootbody = _b2World->CreateBody(&bodyDef);
	b2PolygonShape triShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &triShape;
	Point pntLoc = _csbRoot->getPosition();
	for (size_t i = 1; i <= 1; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		std::ostringstream ostr;
		std::string objname;
		ostr << "shoot_start"; objname = ostr.str();

		const auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float angle = triSprite->getRotation();
		float scaleX = triSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = triSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
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
		if (_bSparking) { //�i�H�Q�o�A��{�o�����Q�o
			_tdelayTime = 0; // �ɶ����s�]�w�A
			_bSparking = false; // �}�l�p��
			for (int i = 0; i < 20; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
				auto sparkSprite = Sprite::createWithSpriteFrameName("spark.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//���ͤp������
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

				//���O�q
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
			_tdelayTime = 0; // �k�s
			_bSparking = true; // �i�i��U�@�����Q�o
		}
	}
	if (_fShoottime <= 0&& _iBubblenum<20) {
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		shootSprite[_iBubblenum] = Sprite::createWithSpriteFrameName("bubble.png");
		
		shootSprite[_iBubblenum]->setPosition(Vec2(shootSprite[_iBubblenum]->getPosition().x, shootSprite[_iBubblenum]->getPosition().y));
		this->addChild(shootSprite[_iBubblenum], 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = shootSprite[_iBubblenum];	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(100/ PTM_RATIO, 600 / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body* ballBody = _b2World->CreateBody(&bodyDef);
		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = shootSprite[_iBubblenum]->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.75f * (ballsize.width - 4) * 0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;	 // �T�w�˸m
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.75f;			// �]�w��_�Y��
		fixtureDef.density = 5.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
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

bool Level_three::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
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

void  Level_three::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bstartDraw&& drawnum<100&& _pvLoc!= touchLoc) {
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
		drawrectSprite[drawnum] = Sprite::createWithSpriteFrameName("rect01.png");
		drawrectSprite[drawnum]->setScale(0.5);
		this->addChild(drawrectSprite[drawnum], 2);
		drawbodyDef[drawnum].type = b2_staticBody; // �]�w�o�� Body �� �R�A��
		drawbodyDef[drawnum].userData = drawrectSprite[drawnum];
		drawbodyDef[drawnum].position.Set(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
		_drawbody[drawnum] = _b2World->CreateBody(&drawbodyDef[drawnum]);
		
		
		
		Size ts = drawrectSprite[drawnum]->getContentSize();
		Point loc = drawrectSprite[drawnum]->getPosition();
		float angle = drawrectSprite[drawnum]->getRotation();
		float scaleX = drawrectSprite[drawnum]->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = drawrectSprite[drawnum]->getScaleY();	// ����	���u�q�ϥܰ��]���u���� X �b��j

		// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;
		
		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = loc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
		drawfixtureDef; // ���� Fixture
		drawfixtureDef.shape = &rectShape;
		rectShape.Set(vecs, 4);
		_drawbody[drawnum]->CreateFixture(&drawfixtureDef);
		drawnum++;
		_pvLoc = touchLoc;
	}
}

void  Level_three::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
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
				_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��
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
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
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

	// Ū���Ҧ� wall1_ �}�Y���ϥ� ���O�R�A����
	// �{���X�Ӧ� StaticDynamicScene.cpp
	char tmp[20] = "";

	// ���� EdgeShape �� body
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 1; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		std::ostringstream ostr;
		std::string objname;
		ostr << "road_0" << i; objname = ostr.str();
		auto edgeSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = loc.y; //�]�w Translation�A�ۤv���[�W���˪�

											 // ���ͨ�Ӻ��I
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
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		std::ostringstream ostr;
		std::string objname;
		ostr << "end_0" << i; objname = ostr.str();

		const auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float angle = triSprite->getRotation();
		float scaleX = triSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = triSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		std::ostringstream ostr;
		std::string objname;
		ostr << "static_0" << i; objname = ostr.str();

		auto const rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = rectSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
//��gø�s��k
void Level_three::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif