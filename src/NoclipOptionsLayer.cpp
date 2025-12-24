#include "../vendor/robtop/NoclipOptionsLayer.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/ButtonSprite.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include <cocos2d.h>

CCSprite* getToggleSpritee(CCSprite* on, CCSprite* off, bool state) { return (state) ? on : off; }
CCMenuItemSprite* getMenuToggleSpritee(CCMenuItemSprite* on, CCMenuItemSprite* off, bool state) { return (state) ? on : off; }

class ToggleHack {
    public:
    void toggleDeaths() {
      ExtraLayer::m_deaths = !ExtraLayer::m_deaths;
    }

    void toggleFlash() {
      ExtraLayer::m_flash = !ExtraLayer::m_flash;
    }
};

 NoclipOptionsLayer* NoclipOptionsLayer::create(CCLayer* referrer) {
    auto ret = new NoclipOptionsLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool NoclipOptionsLayer::init(CCLayer* referrer) {
    if (!CCBlockLayer::init()) {
        return false;
    }

    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    ExtraLayer::m_deaths = def->getBoolForKey("noclipdeaths", false);
  ExtraLayer::m_flash = def->getBoolForKey("noclipflash", false);

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
            CCString::createWithFormat("No-Clip Options")->getCString(), 
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
    menu_selector(NoclipOptionsLayer::onClose)
  );

  noclipButton->setScale(0.8f);
  menu1->addChild(noclipButton);

  auto toggleOffSprite2 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite2 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff2 = CCMenuItemSprite::create(toggleOffSprite2, toggleOffSprite2, nullptr, nullptr);
auto itemOn2 = CCMenuItemSprite::create(toggleOnSprite2,  toggleOnSprite2,  nullptr, nullptr);

auto deaths = ExtraLayer::m_deaths;
  auto DEATHSButton = CCMenuItemToggler::create(
    getMenuToggleSpritee(itemOn2, itemOff2, deaths),
    getMenuToggleSpritee(itemOff2, itemOn2, deaths),  
    this, 
    menu_selector(ToggleHack::toggleDeaths) 
);

  auto menu2 = CCMenu::create();
   auto counterLabel2 = CCLabelBMFont::create(
            CCString::createWithFormat("NoClip Deaths")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel2->setScale(0.5f);
        auto labelMenuItem2 = CCMenuItemLabel::create(
    counterLabel2, 
    this, 
    menu_selector(ExtraLayer::dummy) 
);
  DEATHSButton->setScale(0.8f);
  menu2->addChild(DEATHSButton);
  menu2->addChild(labelMenuItem2);

  menu2->alignItemsHorizontally();

  auto toggleOffSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

  auto itemOff3 = CCMenuItemSprite::create(toggleOffSprite3, toggleOffSprite3, nullptr, nullptr);
auto itemOn3 = CCMenuItemSprite::create(toggleOnSprite3,  toggleOnSprite3,  nullptr, nullptr);

auto flash = ExtraLayer::m_flash;
  auto FLASHButton = CCMenuItemToggler::create(
    getMenuToggleSpritee(itemOn3, itemOff3, flash),
    getMenuToggleSpritee(itemOff3, itemOn3, flash),  
    this, 
    menu_selector(ToggleHack::toggleFlash) 
);

  auto menu3 = CCMenu::create();
   auto counterLabel3 = CCLabelBMFont::create(
            CCString::createWithFormat("NoClip Flash")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel3->setScale(0.5f);
        auto labelMenuItem3 = CCMenuItemLabel::create(
    counterLabel3, 
    this, 
    menu_selector(ExtraLayer::dummy) 
);
  FLASHButton->setScale(0.8f);
  menu3->addChild(FLASHButton);
  menu3->addChild(labelMenuItem3);

  menu3->alignItemsHorizontally();

  menu1->setPosition({win_size.width / 2, 20});
  menu2->setPosition({win_size.width / 2 + 10, win_size.height / 2 + 50});
  menu3->setPosition({win_size.width / 2, win_size.height / 2});

  labelMenuItem2->setPosition(12.5, 7);
  labelMenuItem3->setPosition(12.5, 7);

  mainLayoutLayer->addChild(menu1);
  mainLayoutLayer->addChild(menu2);
  mainLayoutLayer->addChild(menu3);
  this->addChild(mainLayoutLayer);
  this->setKeypadEnabled(true);

    return true;
}

 void NoclipOptionsLayer::onClose(CCObject* sender) {
    if (sender) this->retain();
    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    def->setBoolForKey("noclipdeaths", ExtraLayer::m_deaths);
    def->setBoolForKey("noclipflash", ExtraLayer::m_flash);
    def->flush();
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
  }

  void NoclipOptionsLayer::keyBackClicked() {
    onClose(nullptr);
  }