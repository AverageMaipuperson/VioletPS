#include "../vendor/robtop/HidePauseLayer.hpp"
#include "../vendor/robtop/PauseLayer.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/ButtonSprite.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include "../vendor/robtop/CCTextInputNode.hpp"
#include "../vendor/robtop/FLAlertLayer.hpp"
#include "../vendor/robtop/PlayLayer.hpp"
#include <cocos2d.h>
#include <cstdlib> 
#include <cstdio>
#include <map>
std::map<int, cocos2d::CCPoint> HidePauseLayer::s_activeTouches;

CCLayer* HidePauseLayer::m_playLayer = nullptr;

 HidePauseLayer* HidePauseLayer::create(CCLayer* referrer) {
    auto ret = new HidePauseLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

PauseLayer* PauseLayer::create(void*) {
    auto ret = new PauseLayer();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

cocos2d::CCPoint getTouchGL(cocos2d::CCTouch* pTouch) {
    return cocos2d::CCDirector::sharedDirector()->convertToGL(pTouch->locationInView());
}

cocos2d::CCPoint getTouchPrevGL(cocos2d::CCTouch* pTouch) {
    return cocos2d::CCDirector::sharedDirector()->convertToGL(pTouch->previousLocationInView());
}


CCMenu* HidePauseLayer::createButtonWithSprite(const char* spriteName, SEL_MenuHandler func) {
  auto sprite = CCSprite::createWithSpriteFrameName(spriteName);
  
  auto menu = CCMenu::create();
  auto btn = CCMenuItemSpriteExtra::create(
    sprite,
    sprite,
    this,
    func
  );

  menu->addChild(btn);
  menu->setPosition(CCPointZero);
  return menu;
}

CCMenu* HidePauseLayer::createNewButtonWithSprite(const char* spriteName, SEL_MenuHandler func) {
  auto sprite = ButtonSprite::create(
    CCSprite::createWithSpriteFrameName(spriteName), 50, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto menu = CCMenu::create();
  auto btn = CCMenuItemSpriteExtra::create(
    sprite,
    sprite,
    this,
    func
  );

  menu->addChild(btn);
  menu->setPosition(CCPointZero);
  return menu;
}

void HidePauseLayer::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -128, true);
}

bool HidePauseLayer::init(CCLayer* self) {
  this->setTouchEnabled(true);
    CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
    CCDirector::sharedDirector()->getTouchDispatcher()->addStandardDelegate(this, 0);
  auto win_size = CCDirector::sharedDirector()->getWinSize();
  auto exitBtn = this->createButtonWithSprite("GJ_arrow_03_001.png", menu_selector(HidePauseLayer::keyBackClicked));
  exitBtn->setPosition(30, win_size.height - 30);
  this->addChild(exitBtn);
  this->setKeypadEnabled(true);
  self->setKeypadEnabled(false);
  this->setTouchEnabled(true);
  return true;
}

bool HidePauseLayer::ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent) {
    s_activeTouches[pTouch->getID()] = getTouchGL(pTouch);
    return true; 
}
void HidePauseLayer::ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent) {
    s_activeTouches[pTouch->getID()] = getTouchGL(pTouch);
    if (s_activeTouches.size() == 1) {
        // calculates how much the finger has moved:
        CCPoint currentPos = getTouchGL(pTouch);
        CCPoint prevPos = getTouchPrevGL(pTouch);
        CCPoint delta = ccpSub(getTouchGL(pTouch), getTouchPrevGL(pTouch));
        HidePauseLayer::m_playLayer->setPosition(ccpAdd(HidePauseLayer::m_playLayer->getPosition(), delta));
        // move the layer by that delta
        CCPoint layerPos = HidePauseLayer::m_playLayer->getPosition();
        HidePauseLayer::m_playLayer->setPosition(ccpAdd(layerPos, delta));
    }
    if (s_activeTouches.size() == 2 && HidePauseLayer::m_playLayer != NULL && HidePauseLayer::m_playLayer != nullptr) {
        auto it = s_activeTouches.begin();
        int id1 = it->first;
        cocos2d::CCPoint pos1 = it->second;
        it++;
        int id2 = it->first;
        cocos2d::CCPoint pos2 = it->second;
        float currentDist = ccpDistance(pos1, pos2);
        cocos2d::CCPoint prevPos;
        cocos2d::CCPoint otherPos;
        
        // for good measure
        if (pTouch->getID() == id1) {
            prevPos = getTouchPrevGL(pTouch);
            otherPos = pos2;
        } else {
            prevPos = getTouchPrevGL(pTouch);
            otherPos = pos1;
        }
        float prevDist = ccpDistance(prevPos, otherPos);
        if (prevDist > 0) {
            float scaleDelta = currentDist / prevDist;
            float newScale = HidePauseLayer::m_playLayer->getScale() * scaleDelta;

            if (newScale > 1.f && newScale < 5.f) HidePauseLayer::m_playLayer->setScale(newScale); // really buggy either way
        }
    }
}

void HidePauseLayer::ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent) {
    s_activeTouches.erase(pTouch->getID()); // erase all the touches for better memory management
}



void HidePauseLayer::onClose() {
  if(m_pauseLayer != nullptr) {
  CCObject* child;
    CCARRAY_FOREACH(m_pauseLayer->getChildren(), child) {
        auto node = dynamic_cast<CCNode*>(child);
        if (node) node->setVisible(true);
        auto menu = dynamic_cast<CCMenu*>(node);
                if (menu) {
                    menu->setTouchEnabled(true);
                }
    }
    m_pauseLayer->setTouchEnabled(true);
    m_pauseLayer->setKeypadEnabled(true);
  }
  HidePauseLayer::m_playLayer->setScale(1.f);
  HidePauseLayer::m_playLayer->setPosition(0, 0);
  this->removeFromParentAndCleanup(true);
}

void HidePauseLayer::keyBackClicked() {
  onClose();
}