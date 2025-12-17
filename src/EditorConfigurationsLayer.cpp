// NOTE: this file is abandoned, the declarations for ExtraLayer are in epic.cpp (for no reason)
#include "FLAlertLayer.hpp"
#include "CCMenuItemToggler.hpp"
#include "CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
using namespace cocos2d;

EditorConfigurationsLayer* EditorConfigurationsLayer::create(CCLayer* referrer) {
    auto ret = new EditorConfigurationsLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool EditorConfigurationsLayer::init(CCLayer* referrer) {
auto win_size = CCDirector::sharedDirector()->getWinSize();
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto button = CCMenuItemToggler::create(
    getToggleSprite(toggleOn, toggleOff, noclip),
    getToggleSprite(toggleOff, toggleOn, noclip),
    self,
    menu_selector(ToggleHack::toggleNoclip)
  );
  auto menu = CCMenu::create();
   auto counterLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Safe No-Clip")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel->setScale(0.5f);
        counterLabel->setAnchorPoint({0.f, 0.5f});
        counterLabel->setPosition(180,85);
        self->addChild(counterLabel);
  menu->addChild(button);
  menu->setPosition({(win_size.width / 5) - 6, (win_size.height / 2) - 100});
  menu->setScale(0.85f);
  button->setPosition(0,0);
  
  self->addChild(menu);
}

void EditorConfigurationsLayer::keyBackClicked() {
    onClose(nullptr);

}
