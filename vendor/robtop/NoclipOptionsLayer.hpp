#pragma once
#include <cocos2d.h>
#include "CCBlockLayer.h"

class NoclipOptionsLayer : public CCBlockLayer {

public:
    static NoclipOptionsLayer* create(CCLayer* referrer);
	void onClose(CCObject*);
	bool init(CCLayer* referrer);
    void keyBackClicked();
};
