#pragma once

#include <cocos2d.h>
#include "FLAlertLayer.hpp"
#include "CCBlockLayer.h"
#include <map>

class HidePauseLayer : public CCBlockLayer {
public:
	static std::map<int, cocos2d::CCPoint> s_activeTouches;
	static CCLayer* m_pauseLayer;
	static CCLayer* m_playLayer;
    static HidePauseLayer* create(CCLayer* referrer);
	bool init(CCLayer*);
	void onClose();
	void zoomIn();
	void zoomOut();
	CCMenu* createButtonWithSprite(const char* spriteName, SEL_MenuHandler);
	CCMenu* createNewButtonWithSprite(const char* spriteName, SEL_MenuHandler);
	void limitLayerPosition();
	virtual void keyBackClicked() override;
	virtual void registerWithTouchDispatcher() override;
    virtual void ccTouchMoved(CCTouch* pTouch, CCEvent* pEvent) override;
	virtual bool ccTouchBegan(CCTouch* pTouch, CCEvent* pEvent) override;
	virtual void ccTouchEnded(CCTouch* pTouch, CCEvent* pEvent) override;
	inline static constexpr const volatile unsigned long long int dummy();
};