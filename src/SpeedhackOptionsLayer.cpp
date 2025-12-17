#include "../vendor/robtop/SpeedhackOptionsLayer.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/ButtonSprite.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include "../vendor/robtop/CCTextInputNode.hpp"
#include <cocos2d.h>
#include <cstdlib> 
#include <cstdio>
bool hasChanged = false;
bool doReset = false;

CCTextInputNode* ExtraLayer::m_speedHackInput = nullptr;

#define MEMBER_BY_OFFSET(type, var, offset) \
    (*reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(var) + static_cast<uintptr_t>(offset)))


CCSprite* getToggleSpriteee(CCSprite* on, CCSprite* off, bool state) { return (state) ? on : off; }
CCMenuItemSprite* getMenuToggleSpriteee(CCMenuItemSprite* on, CCMenuItemSprite* off, bool state) { return (state) ? on : off; }

void SpeedhackOptionsLayer::decrement() {
  ExtraLayer::m_speedhack -= 0.1f;
  if (ExtraLayer::m_speedhack < 0.1f) {
    ExtraLayer::m_speedhack = 0.1f;
  }
  ExtraLayer::m_speedHackInput->setString(CCString::createWithFormat("%.1f", ExtraLayer::m_speedhack)->getCString());
  hasChanged = true;
}

void SpeedhackOptionsLayer::increment() {
  ExtraLayer::m_speedhack += 0.1f;
  if (ExtraLayer::m_speedhack > 10.f) {
    ExtraLayer::m_speedhack = 9.9f;
  }
  ExtraLayer::m_speedHackInput->setString(CCString::createWithFormat("%.1f", ExtraLayer::m_speedhack)->getCString());
  hasChanged = true;
}

 SpeedhackOptionsLayer* SpeedhackOptionsLayer::create(CCLayer* referrer) {
    auto ret = new SpeedhackOptionsLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool SpeedhackOptionsLayer::init(CCLayer* referrer) {
    if (!CCBlockLayer::init()) {
        return false;
    }

    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    ExtraLayer::m_speedhack = def->getFloatForKey("speedhackInt", 1.0f);

    CCNode* leftParent = CCNode::create();
    auto win_size = CCDirector::sharedDirector()->getWinSize();
     CCLayerColor *overlay = CCLayerColor::layerWithColor(ccc4(0, 0, 0, 127),
                                                win_size.width,
                                                 win_size.height);
    overlay->setPosition(0,0);
    this->addChild(overlay);
    this->addChild(leftParent);
    leftParent->setPosition(win_size.width / 2, win_size.height / 2);

    CCRect rect = CCRectMake(0, 0, 80, 80);
    cocos2d::extension::CCScale9Sprite* panel = cocos2d::extension::CCScale9Sprite::create("square02_001.png", rect);
    panel->setOpacity(127);
    panel->setContentSize(CCSizeMake(win_size.width / 2, win_size.height - 25));
    leftParent->addChild(panel);
    panel->setPosition({0.f, 0.f});

    CCLayer* mainLayoutLayer = CCLayer::create();
mainLayoutLayer->setAnchorPoint(ccp(0.5f, 0.5f));

this->setTouchEnabled(true);

auto titleLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Speedhack Options")->getCString(), 
            "goldFont-hd.fnt"
        );

  titleLabel->setPosition({win_size.width / 2, win_size.height - 35});
  titleLabel->setScale(1.0f);
  this->addChild(titleLabel);

    auto menu1 = CCMenu::create();
  auto noclipBtnSprite = ButtonSprite::create(
    "OK", 50, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto noclipButton = CCMenuItemSpriteExtra::create(
    noclipBtnSprite,
    noclipBtnSprite,
    this,
    menu_selector(SpeedhackOptionsLayer::onClose)
  );

  noclipButton->setScale(0.8f);
  menu1->addChild(noclipButton);

  auto bg = extension::CCScale9Sprite::create("square02_001.png", CCRectMake(0,0,80,80));
    bg->setContentSize(CCSizeMake(240, 60));
    bg->_setZOrder(-1);
    bg->setPosition({win_size.width / 2, win_size.height - 100});
    bg->setOpacity(127);
    this->addChild(bg);
    bg->setScale(0.9);

  CCTextInputNode* textInput = CCTextInputNode::create(100.0, 40.0, "1.0", "Thonburi", 12, "bigFont.fnt");
    ExtraLayer::m_speedHackInput = textInput;
    textInput->setString(CCString::createWithFormat("%.1f", ExtraLayer::m_speedhack)->getCString());
    MEMBER_BY_OFFSET(cocos2d::CCTextFieldDelegate*, textInput, 0x16c) = this;
    textInput->setPosition(ccp(win_size.width / 2, win_size.height - 100));
    textInput->setMaxLabelScale(0.7);
    textInput->setLabelPlaceholderScale(0.7);
    MEMBER_BY_OFFSET(int, textInput, 0x170) = 4; // char limit
    // textInput->setAllowedChars("1234567890."); // set allowed chars
    // textInput->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,+-/!?");
    textInput->setAnchorPoint({0, 0.5});

    auto menu2 = CCMenu::create();
  auto leftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
  
  auto leftButton = CCMenuItemSpriteExtra::create(
    leftSprite,
    leftSprite,
    this,
    menu_selector(SpeedhackOptionsLayer::decrement)
  );

  leftButton->setScale(0.8f);
  menu2->addChild(leftButton);

  auto menu3 = CCMenu::create();
  auto rightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
  
  auto rightButton = CCMenuItemSpriteExtra::create(
    rightSprite,
    rightSprite,
    this,
    menu_selector(SpeedhackOptionsLayer::increment)
  );

  rightButton->setScale(0.8f);
  menu3->addChild(rightButton);

  this->addChild(textInput);

  menu1->setPosition({win_size.width / 2, 20});
  menu2->setPosition({win_size.width / 2 - 130, win_size.height - 100});
  menu3->setPosition({win_size.width / 2 + 130, win_size.height - 100});

  mainLayoutLayer->addChild(menu1);
  mainLayoutLayer->addChild(menu2);
  mainLayoutLayer->addChild(menu3);
  this->addChild(mainLayoutLayer);
  this->setKeypadEnabled(true);

    return true;
}

  bool SpeedhackOptionsLayer::onTextFieldReturn(cocos2d::CCTextFieldTTF* pSender, const char * text, int nLen) {
    hasChanged = true;
    ExtraLayer::m_speedhack = atof(MEMBER_BY_OFFSET(cocos2d::CCTextFieldTTF*, ExtraLayer::m_speedHackInput, 0x168)->getString());
    std::string currentText = MEMBER_BY_OFFSET(cocos2d::CCTextFieldTTF*, ExtraLayer::m_speedHackInput, 0x168)->getString();
    std::string insertedChar(text, nLen);
    if (isdigit(insertedChar.at(0))) {
        return false; 
    }
    if (insertedChar == ".") {
        if (std::count(currentText.begin(), currentText.end(), '.') < 1) {
            return false;
        }
    }
    return true;
  }

 void SpeedhackOptionsLayer::onClose(CCObject* sender) {
    if (sender) this->retain();
    const char* str = MEMBER_BY_OFFSET(cocos2d::CCTextFieldTTF*, ExtraLayer::m_speedHackInput, 0x168)->getString();
    std::string cppString = str;
    auto number = atof(str);
    if(!hasChanged && (number - ExtraLayer::m_speedhack) != 0) {
    bool valid = true;
    bool belowOne = false;
    for (char c : cppString) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            valid = false;
        }
        if(c == 0) belowOne = true;
    }
    if(!cppString.empty() && cppString[0] == '0') belowOne = true;
    if (((number >= 10 && number <= 99) || belowOne) && std::stod(str) > 1.0) { ExtraLayer::m_speedhack = number / 10;
      ExtraLayer::m_speedHackInput->setString(CCString::createWithFormat("%.1f", ExtraLayer::m_speedhack / 10)->getCString());
 } else if(!valid || number < 0.1) {
      ExtraLayer::m_speedhack = 1.0f;
      ExtraLayer::m_speedHackInput->setString("1.0");
 } else ExtraLayer::m_speedhack = number;
} else ExtraLayer::m_speedhack = number;
cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
  def->setFloatForKey("speedhackInt", ExtraLayer::m_speedhack);
    def->flush();
hasChanged = false;
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
  }

  void SpeedhackOptionsLayer::keyBackClicked() {
    onClose(nullptr);
  }