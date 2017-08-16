#include "GameUILayer.h"
#include "SimpleAudioEngine.h"

#include "GameScene.h"
#include "GameLayer.h"

USING_NS_CC;

// on "init" you need to initialize your instance
bool GameUILayer::init()
{
    //////////////////////////////
    // 1. super init first
    if(!Node::init())
    {
        return false;
    }

    CCLOG("Game UI Layer loaded");

    _visibleSize = Director::getInstance()->getVisibleSize();


	// add touch listeners
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(GameUILayer::OnTouchBegan, this);
	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);


    setupScoreLabel();
    setupHealthBar();
    setupGauge();
	setupSkillButton();

    _instructionSprite = Sprite::create("PNG/instruction.png");
    // not recommended butfor now..
    _instructionSprite->setPosition(_visibleSize.width * .5f - 315, _visibleSize.height * .5f + 88.f - 148.f);
    this->addChild(_instructionSprite);


    return true;
}

void GameUILayer::update(float dt)
{

    updateScore();
    updateGauge();


}

void GameUILayer::updateScore()
{
    std::stringstream ss;

    ss <<(int)*_score;

    _scoreLabel->setString(ss.str());

}

void GameUILayer::setupScoreLabel()
{
    //_scoreLabel = Label::createWithTTF("Score: 0", "FONTS/kenpixel_blocks.ttf", 60.f, Size::ZERO, TextHAlignment::LEFT, TextVAlignment::CENTER);
    //    _scoreLabel->setColor(Color3B::BLACK);
    _scoreLabel = Label::createWithCharMap("PNG/HUD/hudNumbers.png", 64, 128, 48);
    _scoreLabel->setAlignment(TextHAlignment::LEFT, TextVAlignment::CENTER);
    _scoreLabel->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
    _scoreLabel->setPosition(_visibleSize.width * 0.03f, _visibleSize.height * .98f);
    this->addChild(_scoreLabel, 0);


}

void GameUILayer::updateHealth()
{
    CCLOG("player health on ui: %d", _gameLayer->_playerCharacter->getHealth());

    if(*_playerHealth % 2 == 0){
        _healthIcons.at(*_playerHealth / 2)->setSpriteFrame(Sprite::create("PNG/HUD/hudHeart_empty.png")->getSpriteFrame());

    }
    else{
        _healthIcons.at(*_playerHealth / 2)->setSpriteFrame(Sprite::create("PNG/HUD/hudHeart_half.png")->getSpriteFrame());
    }

    // shaky heart? cam?
    auto shake1 = MoveBy::create(.05f, Vec2(random(-3, 3), random(-3, 3)));
    auto shake2 = MoveBy::create(.05f, Vec2(random(-3, 3), random(-3, 3)));
    auto shake3 = MoveBy::create(.05f, Vec2(random(-3, 3), random(-3, 3)));

    auto sequence = Sequence::create(shake1, shake1->reverse(), shake2, shake2->reverse(), shake3, shake3->reverse(), nullptr);

    //_healthIcons.at(*_playerHealth / 2)->runAction(sequence);

    GameScene* scene = (GameScene*)this->getParent();
    scene->getDefaultCamera()->runAction(sequence);
}

void GameUILayer::setupHealthBar()
{
    // player has 100 hp = 5 hearts
    auto heart = Sprite::create("PNG/HUD/hudHeart_full.png");

    for(int i = 0; i < 5; i++){
        auto heartClone = Sprite::createWithSpriteFrame(heart->getSpriteFrame());
        heartClone->setAnchorPoint(Vec2::ANCHOR_TOP_RIGHT);
        heartClone->setPosition(_visibleSize.width - heartClone->getContentSize().width * .8f * (i), _visibleSize.height * .98f);
        _healthIcons.pushBack(heartClone);
        this->addChild(heartClone);
    }
}

void GameUILayer::resetHealth()
{
    for(auto spr : _healthIcons){
        spr->setSpriteFrame(Sprite::create("PNG/HUD/hudHeart_full.png")->getSpriteFrame());
    }
}

void GameUILayer::updateGauge()
{
    _gaugeBar->getChildByName<ProgressTimer*>("bar")->setPercentage(_gameLayer->_playerCharacter->getGauge());
}

void GameUILayer::setupGauge()
{
    // Sprite node for background
    _gaugeBar = Sprite::create("PNG/UI_pack/yellow_button13.png");
    _gaugeBar->setPosition(_visibleSize.width * .1f, _visibleSize.height * .03f);
    _gaugeBar->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

    // progess bar?
    auto gaugeBar = ProgressTimer::create(Sprite::create("PNG/UI_pack/yellow_button_fill.png"));
    gaugeBar->setType(ProgressTimer::Type::BAR);
    gaugeBar->setBarChangeRate(Vec2(1.f, 0.f));
    gaugeBar->setMidpoint(Vec2(0.f, .5f));
    gaugeBar->setPercentage(0.f);
    gaugeBar->setName("bar");
    gaugeBar->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

    _gaugeBar->addChild(gaugeBar);

    // icon
    auto icon = Sprite::create("PNG/skillIcon.png");
    icon->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
    icon->setPosition(_gaugeBar->getPositionX(), _gaugeBar->getPositionY() + _gaugeBar->getContentSize().height * .5f + 2.f);
    this->addChild(icon);

    this->addChild(_gaugeBar);
}

void GameUILayer::setupSkillButton()
{
	
}


bool GameUILayer::OnTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{
	switch (*_gameState){
		case GAME_STATE::PLAYING:
			if (!(_gameLayer->_playerCharacter->isMidAir())){
				// jump
				// reverse fall direction
				_gameLayer->_reverseFall *= -1;
				// jump player 
				_gameLayer->_fallSpeed = 4.f;
				_gameLayer->_playerCharacter->setPositionY(_gameLayer->_playerCharacter->getPositionY() + _gameLayer->_reverseFall);

				// flip sprite
				_gameLayer->_playerCharacter->setFlippedY(!_gameLayer->_playerCharacter->isFlippedY());
			}
			break;

		case GAME_STATE::PAUSED:
			// start game
			_gameLayer->startGame();
			_gameLayer->_gameUILayer->setInstruction(false);

			// set obstacle spawn schedule
			_gameLayer->schedule(CC_SCHEDULE_SELECTOR(GameLayer::scheduleObstacleSpawns), random(5.f, 7.f));
			_gameLayer->schedule(CC_SCHEDULE_SELECTOR(GameLayer::scheduleRandomGust), _gameLayer->_gustSpawnRate);

			break;

		case GAME_STATE::OVER:

			/************************************************************************/
			/* 사실 게임 오버때는 여기 말고 UI에서 터치 처리해야됨                     */
			/************************************************************************/
			CCLOG("Game over and touched");

			// clean up and reset componets
			this->removeAllChildrenWithCleanup(true);

			_gameLayer->_backgrounds.clear();
			_gameLayer->_tilePlatforms.clear();
			_gameLayer->_obstacles.clear();

			_gameLayer->restartComponents();
			resetHealth();

			*_gameState = GAME_STATE::PAUSED;

			break;

	}


	return false;
}

