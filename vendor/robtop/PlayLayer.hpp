#pragma once

#include <cocos2d.h>
#include "PlayerObject.hpp"
#include "GJGameLevel.hpp"

class PlayLayer : public cocos2d::CCLayer {
public:
    static PlayLayer* create(GJGameLevel*);
    bool init(void*);

    void resetLevel();
    void levelComplete();
    void removeLastCheckpoint();
	void getPlayLayer();
	void triggerRedPulse(float duration);
	void pulseLabelRed(CCLabelBMFont* label, float duration);
	virtual void draw() override;
    CCNode* getLastCheckpoint();
};
