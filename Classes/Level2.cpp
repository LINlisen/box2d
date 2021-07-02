#include "Level2.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;

Level_two::~Level_two()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	//	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level_two::createScene()
{
	return Level_two::create();
}

// on "init" you need to initialize your instance
bool Level_two::init()
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
	_titleLabel = Label::createWithTTF("Level_three", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_titleLabel->getContentSize().width * 0.5f + 225.f, _visibleSize.height - _titleLabel->getContentSize().height * 0.5f - 5.f);
	this->addChild(_titleLabel, 2);

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new (std::nothrow) b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("Level_two.csb");

#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	createStaticBoundary();
	setupMouseJoint();
	//setupDistanceJoint();
	//setupPrismaticJoint();
	//setupPulleyJoint();
	setupGearJoint();
	//setupWeldJoint();
	setupRopeJoint();
	setupCar();
	setFire();
	setupSensor();
	_iFirenum = 0;
	setupPushBar();
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
	listener->onTouchBegan = CC_CALLBACK_2(Level_two::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(Level_two::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(Level_two::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level_two::update));

	return true;
}

void Level_two::setupMouseJoint()
{
	//// ���o�ó]�w frame01 �e�عϥܬ��ʺA����
	//auto frameSprite = _csbRoot->getChildByName("frame01");
	//Point loc = frameSprite->getPosition();

	//b2BodyDef bodyDef;
	//bodyDef.type = b2_dynamicBody;
	//bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	//bodyDef.userData = frameSprite;
	//b2Body* body = _b2World->CreateBody(&bodyDef);

	//// Define poly shape for our dynamic body.
	//b2PolygonShape rectShape;
	//Size frameSize = frameSprite->getContentSize();
	//rectShape.SetAsBox((frameSize.width - 4) * 0.5f / PTM_RATIO, (frameSize.height - 4) * 0.5f / PTM_RATIO);
	//// Define the dynamic body fixture.
	//b2FixtureDef fixtureDef;
	//fixtureDef.shape = &rectShape;
	//fixtureDef.restitution = 0.1f;
	//fixtureDef.density = 1.0f;
	//fixtureDef.friction = 0.1f;
	//body->CreateFixture(&fixtureDef);
	//_bTouchOn = false;
}



void Level_two::setupCar() {
	
	auto circleSprite = _csbRoot->getChildByName("wheel_01");
	//auto circleSprite =dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel_01"));
	Point locA = circleSprite->getPosition();
	Size size = circleSprite->getContentSize();
	float scale = circleSprite->getScale();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width * 0.5f * scale / PTM_RATIO;

	b2BodyDef bodyDef;	//���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(locA.x / PTM_RATIO, locA.y / PTM_RATIO);
	bodyDef.userData = circleSprite;	// �]�w�ϥ�
	bodyA = _b2World->CreateBody(&bodyDef);
	b2FixtureDef fixtureDef;			// �]�w���z�ʽ�
	fixtureDef.shape = &circleShape;	// �ŧi���骺�~�������ܼƬO��Ϊ���
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.2f;			// �]�w�����Y��
	bodyA->CreateFixture(&fixtureDef);

	// ���o�ó]�w wheel_2 ���i�ʺA����B�j
	circleSprite = _csbRoot->getChildByName("wheel_02");
	//auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wheel_02"));
	Point locB = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	scale = circleSprite->getScale();
	circleShape.m_radius = size.width * 0.5f * scale / PTM_RATIO;

	bodyDef.type = b2_dynamicBody;		// �]�w���ʺA����
	bodyDef.position.Set(locB.x / PTM_RATIO, locB.y / PTM_RATIO);
	bodyDef.userData = circleSprite;	// �]�w�ϥ�
	wheelbodyB = _b2World->CreateBody(&bodyDef);
	wheelbodyB->CreateFixture(&fixtureDef);	// �P�W��bodyA�ۦP
	
	//carbody
	carSprite=dynamic_cast<Sprite*>(_csbRoot->getChildByName("car"));
	locCar = carSprite->getPosition();
	sizeCar = carSprite->getContentSize();
	float scaleCar = carSprite->getScale();
	b2BodyDef carDef;
	carDef.type = b2_dynamicBody;
	carDef.position.Set(locCar.x / PTM_RATIO, locCar.y / PTM_RATIO);
	carDef.userData = carSprite;
	bodyCar = _b2World->CreateBody(&carDef);

	b2PolygonShape carShape;
	carShape.SetAsBox(sizeCar.width* scaleCar / 2.5 / PTM_RATIO,sizeCar.height* scaleCar / 2.5 / PTM_RATIO);

	b2FixtureDef carFixture;
	carFixture.shape = &carShape;
	carFixture.density = 100.0f;
	carFixture.friction = 0.5f;

	bodyCar->CreateFixture(&carFixture);

	b2RevoluteJointDef revJoint, revJoint2;
	revJoint.bodyA = bodyA;
	revJoint.localAnchorA.Set(0, 0);
	revJoint.bodyB = bodyCar;
	revJoint.localAnchorB.Set(0.65, -0.6);
	_b2World->CreateJoint(&revJoint);

	revJoint2.bodyA = wheelbodyB;
	revJoint2.localAnchorA.Set(0, 0);
	revJoint2.bodyB = bodyCar;
	revJoint2.localAnchorB.Set(-0.75, -0.6);
	_b2World->CreateJoint(&revJoint2);

}
// ���d�Ҧ@�����ӷ|�ʪ��ʺA����A���ӬO��Τ@�ӬO�����
void Level_two::setupGearJoint()
{
	// �������ͤ��դ���ܪ��i��Ρj�R�A����A�H�T�w�i�H��ʪ����ӰʺA����
	// �]�����ӰʺA����w�g�b�ù��W�A�i�H�����o�� Sprite �A�P�ɨ��o�y��
	char tmp[20] = "";
	Sprite* gearSprite[6] = { nullptr };
	Point loc[6];
	Size  size[6];
	float scale[6] = { 0 };
	b2Body* staticBody[6] = { nullptr };
	b2Body* dynamicBody[6] = { nullptr };
	b2RevoluteJoint* RevJoint[5] = { nullptr };
	b2PrismaticJoint* PriJoint = { nullptr };

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;

	// �إߤ����R�A����� Body
	// �P�ɫإߤ��ӰʺA��Body�A�H�j�� gear01_01 ~  gear01_06 ���ϥ�
	for (int i = 0; i < 4; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "gear01_0" << i + 1; objname = ostr.str();

		gearSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();
		scale[i] = gearSprite[i]->getScale();

		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;

	b2CircleShape circleShape;
	b2PolygonShape polyShape;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;
	// �Ĥ��ӬO�x�έn�t�~�B��
	for (int i = 0; i < 4; i++)
	{
		if (i < 3) circleShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
		else {
			float sx = gearSprite[i]->getScaleX();
			float sy = gearSprite[i]->getScaleY();
			fixtureDef.shape = &polyShape;
			polyShape.SetAsBox((size[i].width - 4) * 0.5f * sx / PTM_RATIO, (size[i].height - 4) * 0.5f * sy / PTM_RATIO);
		}
		dynamicBodyDef.userData = gearSprite[i];
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}

	b2RevoluteJointDef RJoint;	// �������`
	b2PrismaticJointDef PrJoint; // �������`
	for (int i = 0; i < 4; i++)
	{
		if (i < 3) {
			RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
			RevJoint[i] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
		}
		else {
			PrJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter(), b2Vec2(1.0f, 0));
			PriJoint = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));
		}
	}
	
	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicBody[0];
	GJoint.bodyB = dynamicBody[1];
	GJoint.joint1 = RevJoint[0];
	GJoint.joint2 = RevJoint[1];
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);
	//���;������`(A �� B ���⭿�֦P�V)
	/*GJoint.bodyA = dynamicBody[2];
	GJoint.bodyB = dynamicBody[3];
	GJoint.joint1 = RevJoint[2];
	GJoint.joint2 = RevJoint[3];
	GJoint.ratio = -2;
	_b2World->CreateJoint(&GJoint);*/
	
	GJoint.bodyA = dynamicBody[2];
	GJoint.bodyB = dynamicBody[3];
	GJoint.joint1 = RevJoint[2];
	GJoint.joint2 = PriJoint;
	GJoint.ratio = 1;
	_b2World->CreateJoint(&GJoint);
}

//void Level_two::setupWeldJoint()
//{
//	// ���o�ó]�w frame01_weld ���i�R�A����j
//	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("frame01_weld"));
//	Point loc = frameSprite->getPosition();
//	Size size = frameSprite->getContentSize();
//	b2BodyDef staticBodyDef;
//	staticBodyDef.type = b2_staticBody;
//	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
//	staticBodyDef.userData = frameSprite;
//	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
//	b2PolygonShape boxShape;
//	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO, size.height * 0.5f / PTM_RATIO);
//	b2FixtureDef fixtureDef;
//	fixtureDef.shape = &boxShape;
//	staticBody->CreateFixture(&fixtureDef);
//
//	//���o�ó]�w circle01_weld ���i�ʺA����j
//	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("circle01_weld"));
//	loc = circleSprite->getPosition();
//	size = circleSprite->getContentSize();
//
//	b2BodyDef dynamicBodyDef;
//	dynamicBodyDef.type = b2_dynamicBody;
//	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
//	dynamicBodyDef.userData = circleSprite;
//	b2Body* dynamicBody1 = _b2World->CreateBody(&dynamicBodyDef);
//	b2CircleShape circleShape;
//	circleShape.m_radius = (size.width - 4) * 0.5f / PTM_RATIO;
//	fixtureDef.shape = &circleShape;
//	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
//	dynamicBody1->CreateFixture(&fixtureDef);
//
//	//���o�ó]�w frame02_weld ���i�ʺA����j
//	frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("frame02_weld"));
//	loc = frameSprite->getPosition();
//	size = frameSprite->getContentSize();
//
//	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
//	dynamicBodyDef.userData = frameSprite;
//	b2Body* dynamicBody2 = _b2World->CreateBody(&dynamicBodyDef);
//	boxShape.SetAsBox(size.width * 0.5f / PTM_RATIO, size.height * 0.5f / PTM_RATIO);
//	fixtureDef.shape = &boxShape;
//	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
//	dynamicBody2->CreateFixture(&fixtureDef);
//
//	////���Ͳk�����`(�T�w)
//	b2WeldJointDef JointDef;
//	JointDef.Initialize(staticBody, dynamicBody2, staticBody->GetPosition() + b2Vec2(-30 / PTM_RATIO, -30 / PTM_RATIO));
//	_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��
//
//	//���Ͳk�����`(�i���)
//	JointDef.Initialize(staticBody, dynamicBody1, staticBody->GetPosition() + b2Vec2(30 / PTM_RATIO, 30 / PTM_RATIO));
//	JointDef.frequencyHz = 1.0f;
//	JointDef.dampingRatio = 0.125f;
//	_b2World->CreateJoint(&JointDef);
//}

void Level_two::setupRopeJoint()
{
	// ���o�ó]�w frame01_rope �e�عϥܬ��i�R�A����j
	auto frameSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("frame01_rope"));
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox(sizeHead.width * 0.5f / PTM_RATIO, sizeHead.height * 0.5f / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//���o�ó]�w circle01_rope ���i�ʺA����j
	auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("circle01_rope"));
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	b2Body* donutsBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4) * 0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 0.01f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	donutsBody->CreateFixture(&fixtureDef);
	donutsBody->ApplyLinearImpulse(b2Vec2(-0.5,0), donutsBody->GetWorldCenter(), true);

	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = donutsBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 30.0f / PTM_RATIO);
	JointDef.maxLength = (locHead.y - locTail.y - 30) / PTM_RATIO;
	JointDef.collideConnected = true;
	b2RopeJoint* J = dynamic_cast<b2RopeJoint*>(_b2World->CreateJoint(&JointDef));

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite* ropeSprite[14] = { nullptr };
	Point loc[14];
	Size  size[15];
	b2Body* ropeBody[14] = { nullptr };

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
	for (int i = 0; i < 14; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "rope01_"; ostr.width(2); ostr.fill('0'); ostr << i + 1; objname = ostr.str();

		ropeSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4) * 0.5f / PTM_RATIO, (size[i].height - 4) * 0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f * (size[0].height - 5) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;
	revJoint.localAnchorA.Set(0, -0.9f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 13; i++) {
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[13];
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = donutsBody;
	revJoint.localAnchorB.Set(0, 0.85f);
	_b2World->CreateJoint(&revJoint);
}
void Level_two::setReplay() {
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("replay"));
	Replaybt = CButton::create();
	Replaybt->setButtonInfo("replay.png", "replay.png", btnSprite->getPosition());
	Replaybt->setScale(btnSprite->getScale());
	this->addChild(Replaybt, 3);

}
void Level_two::setFire() {
	//fire bt
	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("fire_01"));
	Firebt = CButton::create();
	Firebt->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	Firebt->setScale(btnSprite->getScale());
	log("rotation:%f", Firebt->getRotation());
	this->addChild(Firebt, 3);
	btnSprite->setVisible(false);
	_iFirenum = 0;

}
void Level_two::setupSensor() {
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
void Level_two::setupPushBar() {
	Sprite* rotation_bar[2] = { nullptr };
	Point loc[2];
	Size  size[2];
	float scale[2] = { 0 };
	b2Body* staticBody[2];
	b2Body* dynamicBody[2];
	b2RevoluteJoint* RevJoint[2] = { nullptr };
	b2PrismaticJoint* PriJoint = { nullptr };

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;
	for (int i = 0; i < 2; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr.str("");
		ostr << "rotation_bar_0" << i; objname = ostr.str();

		rotation_bar[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		loc[i] = rotation_bar[i]->getPosition();
		size[i] = rotation_bar[i]->getContentSize();
		scale[i] = rotation_bar[i]->getScale();
		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;

	b2CircleShape circleShape;
	b2PolygonShape polyShape;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;
	for (int i = 0; i < 2; i++)
	{
		if (i < 1) circleShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
		else {
			float sx = rotation_bar[i]->getScaleX();
			float sy = rotation_bar[i]->getScaleY();
			rotation_bar[i]->getRotation();
			fixtureDef.shape = &polyShape;
			polyShape.SetAsBox((size[i].width - 4) * 0.5f * sx / PTM_RATIO, (size[i].height - 4) * 0.5f * sy / PTM_RATIO);
		}
		dynamicBodyDef.userData = rotation_bar[i];
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}
	b2Body* pushdynamicBody[3];
	Sprite* pushSprite[3] = { nullptr };
	Point pushloc[3];
	Size  pushsize[3];
	float pushscale[3] = { 0 };
	b2BodyDef pushdynamicBodyDef;
	pushdynamicBodyDef.type = b2_dynamicBody;
	for (int j = 1; j <= 2; j++) {
		std::ostringstream ostr;
		std::string objname;
		ostr.str("");
		ostr << "push_0" << j; objname = ostr.str();
		pushSprite[j] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		pushloc[j] = pushSprite[j]->getPosition();
		pushsize[j] = pushSprite[j]->getContentSize();
		pushscale[j] = pushSprite[j]->getScale();
		float sx = pushSprite[j]->getScaleX();
		float sy = pushSprite[j]->getScaleY();
		fixtureDef.shape = &polyShape;
		fixtureDef.density = 10.0f;
		polyShape.SetAsBox((pushsize[j].width - 4) * 0.5f * sx / PTM_RATIO, (pushsize[j].height - 4) * 0.5f * sy / PTM_RATIO);
		pushdynamicBodyDef.userData = pushSprite[j];
		pushdynamicBodyDef.position.Set(pushloc[j].x / PTM_RATIO, pushloc[j].y / PTM_RATIO);
		pushdynamicBody[j] = _b2World->CreateBody(&pushdynamicBodyDef);
		pushdynamicBody[j]->CreateFixture(&fixtureDef);
	}
	b2RevoluteJointDef RJoint;	// �������`
	b2PrismaticJointDef PrJoint; // �������`
	for (int i = 0; i < 2; i++)
	{
		if (i < 1) {
			RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
			RevJoint[i] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
		}
		else {
			PrJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter(), b2Vec2(0.0f, 1.0f));
			PriJoint = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));
		}
	}
	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicBody[0];
	GJoint.bodyB = dynamicBody[1];
	GJoint.joint1 = RevJoint[0];
	GJoint.joint2 = PriJoint;
	GJoint.ratio = 1;
	_b2World->CreateJoint(&GJoint);

	b2WeldJointDef JointDef;
	//JointDef.Initialize(staticBody[1], pushdynamicBody[1], staticBody[1]->GetWorldCenter() + b2Vec2(0, +60 / PTM_RATIO));
	//_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��
	b2WeldJointDef JointDef2;
	JointDef2.Initialize(staticBody[1], pushdynamicBody[2], staticBody[0]->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO));
	_b2World->CreateJoint(&JointDef2); // �ϥιw�]�Ȳk��
	JointDef.Initialize(staticBody[1], pushdynamicBody[1], staticBody[1]->GetPosition() + b2Vec2(30 / PTM_RATIO, 70 / PTM_RATIO));
	JointDef.frequencyHz = 1.0f;
	JointDef.dampingRatio = 0.125f;
	_b2World->CreateJoint(&JointDef);
}

//void Level_two::readBlocksCSBFile(const std::string& csbfilename)
//{
//	auto csbRoot = CSLoader::createNode(csbfilename);
//	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
//	addChild(csbRoot, 1);
//	char tmp[20] = "";
//	for (size_t i = 1; i <= 3; i++)
//	{
//		// ���ͩһݭn�� Sprite file name int plist 
//		sprintf(tmp, "block1_%02d", i); 
//	}
//}
//
//void Level_two::readSceneFile(const std::string &csbfilename)
//{
//	auto csbRoot = CSLoader::createNode(csbfilename);
//	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
//	addChild(csbRoot, 1);
//	char tmp[20] = "";
//	for (size_t i = 1; i <= 3; i++)
//	{
//		// ���ͩһݭn�� Sprite file name int plist 
//		sprintf(tmp, "XXX_%02d", i);
//	}
//}

void Level_two::update(float dt)
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
			Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
			ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
	for (int i = 0; i < _iFirenum; i++) {
		if (_sPutBall[i] != NULL) {
			
				float Sensor1minX = sensorSprite[1]->getPosition().x - (Sensorsize[1].width) / 2;
				float Sensor1maxX = sensorSprite[1]->getPosition().x + (Sensorsize[1].width) / 2;
				float Sensor1minY = sensorSprite[1]->getPosition().y - (Sensorsize[1].height) / 2;
				float Sensor1maxY = sensorSprite[1]->getPosition().y + (Sensorsize[1].height) / 2;
				float PutRectPosX = _sPutBall[i]->getPosition().x;
				float PutRectPosY = _sPutBall[i]->getPosition().y;
				if (PutRectPosX > Sensor1minX && PutRectPosX<Sensor1maxX && PutRectPosY>Sensor1minY && PutRectPosY < Sensor1maxY) {
					FireballBody[i]->ApplyLinearImpulse(b2Vec2(5, 40), FireballBody[i]->GetWorldCenter(), true);
				}
				PutRectPosX = carSprite->getPosition().x;
				PutRectPosY= carSprite->getPosition().y;
				float Sensor2minX = sensorSprite[2]->getPosition().x - (Sensorsize[2].width) / 2;
				float Sensor2maxX = sensorSprite[2]->getPosition().x + (Sensorsize[2].width) / 2;
				float Sensor2minY = sensorSprite[2]->getPosition().y - (Sensorsize[2].height) / 2;
				float Sensor2maxY = sensorSprite[2]->getPosition().y + (Sensorsize[2].height) / 2;
				if (PutRectPosX > Sensor2minX && PutRectPosX<Sensor2maxX && PutRectPosY>Sensor2minY && PutRectPosY < Sensor2maxY) {
					
						log("pass");
						this->unschedule(schedule_selector(Level_two::update));
						Director::getInstance()->replaceScene(Level_three::createScene());
						
				
			}
		}
	}
	float lengthF = bodyA->GetLinearVelocity().x;
	float lengthB = wheelbodyB->GetLinearVelocity().y;
	log("lengthF:%f", lengthF);
	log("lengthB:%f", lengthB);
	if (lengthF > 4.5f || lengthB < -4.5f) {
		_bSmoking = true;
		if (_bSmoking)
		{ 
			_bSmoking = false; // �}�l�p��
			for (int i = 0; i < 2; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto smokeSprite = Sprite::createWithSpriteFrameName("cloud.png");
				smokeSprite->setColor(Color3B(255, 255, 255));
				smokeSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(smokeSprite, 5);
				//���ͤp������
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(wheelbodyB->GetPosition().x, wheelbodyB->GetPosition().y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = smokeSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture* RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//���O�q
				RectBody->ApplyForce(b2Vec2(-lengthF * (int)(rand() % 20) + 25, -lengthB * 10), wheelbodyB->GetPosition(), true);
			}
		}
	}
}

bool Level_two::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();

	// For Mouse Joiint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // �R�A���餣�B�z
		// �P�_�I����m�O�_���b�ʺA����@�w���d��
		Sprite* spriteObj = static_cast<Sprite*>(body->GetUserData());
		Size objSize = spriteObj->getContentSize();
		float fdist = MAX_2(objSize.width, objSize.height) / 2.0f;
		float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
		float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
		float tpdist = x * x + y * y;
		if (tpdist < fdist * fdist)
		{
			_bTouchOn = true;
			b2MouseJointDef mouseJointDef;
			mouseJointDef.bodyA = _bottomBody;
			mouseJointDef.bodyB = body;
			mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
			mouseJointDef.collideConnected = true;
			mouseJointDef.maxForce = 1000.0f * body->GetMass();
			_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // �s�W Mouse Joint
			body->SetAwake(true);
			break;
		}
	}
	if (Firebt->touchesBegin(touchLoc)) {
		if (_iFirenum < 3) {
			log("fire");
			SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
			// ���إ� ballSprite �� Sprite �å[�J������
			_sPutBall[_iFirenum] = Sprite::createWithSpriteFrameName("dount04.png");
			_sPutBall[_iFirenum]->setScale(0.75f);
			//	ballSprite->setPosition(touchLoc);
			this->addChild(_sPutBall[_iFirenum], 2);
			_PutBall[_iFirenum].type = b2_dynamicBody; // �]�w���ʺA����
			_PutBall[_iFirenum].userData = _sPutBall[_iFirenum];	// �]�w Sprite ���ʺA���骺��ܹϥ�
			_PutBall[_iFirenum].position.Set(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
			// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
			FireballBody[_iFirenum] = _b2World->CreateBody(&_PutBall[_iFirenum]);
			// �]�w�Ӫ��骺�~��
			b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
			Size ballsize = _sPutBall[_iFirenum]->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
			ballShape.m_radius = 0.75f * (ballsize.width - 4) * 0.5f / PTM_RATIO;
			// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
			b2FixtureDef fixtureDef;	 // �T�w�˸m
			fixtureDef.shape = &ballShape;			// ���w���骺�~�������
			fixtureDef.restitution = 0.75f;			// �]�w��_�Y��
			fixtureDef.density = 5.0f;				// �]�w�K��
			fixtureDef.friction = 0.15f;			// �]�w�����Y��
			FireballBody[_iFirenum]->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
			FireballBody[_iFirenum]->ApplyLinearImpulse(b2Vec2(-120, 1), FireballBody[_iFirenum]->GetWorldCenter(), true);
			// GetWorldCenter(): Get the world position of the center of mass
			_iFirenum++;
		}
	}
	if (Replaybt->touchesBegin(touchLoc))
	{
		this->unschedule(schedule_selector(Level_two::update));
		Director::getInstance()->replaceScene(Level_two::createScene());
	}
	return true;
	
}

void  Level_two::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
}

void  Level_two::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
	if (Firebt->touchesEnded(touchLoc));
	if (Replaybt->touchesEnded(touchLoc));
}

void Level_two::createStaticBoundary()
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

	for (size_t i = 1; i <= 4; i++)
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
}

#ifdef BOX2D_DEBUG
//��gø�s��k
void Level_two::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif