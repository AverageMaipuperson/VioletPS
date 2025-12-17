#pragma once

#include <cocos2d.h>
#include "FLAlertLayer.hpp"

class PauseLayer : public cocos2d::CCLayer, public FLAlertLayerProtocol {
public:
    static PauseLayer* create(void*);
	virtual void FLAlert_Clicked(FLAlertLayer* self, bool btn2) override;
    void onOpenMenu();
};