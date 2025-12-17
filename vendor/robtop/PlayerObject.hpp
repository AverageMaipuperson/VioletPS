#pragma once

#include <cocos2d.h>
#include "EndPortalObject.hpp"

using namespace cocos2d;

class PlayerObject : public cocos2d::CCSprite {
public:
    static PlayerObject* create(int, cocos2d::CCLayer*);
    bool init(void*);
    void lockPlayer();
    void gravityDown();
    void gravityUp();
    void speedDown();
    void speedUp();
    void yStartDown();
    void yStartUp();
    void logValues();

    void deactivateParticle();
    void flipMod();
    virtual CCLayer* getGameLayer() const;
    virtual EndPortalObject* getPortalObject();
};
