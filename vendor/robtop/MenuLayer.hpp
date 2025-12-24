#pragma once

#include "FLAlertLayer.hpp"
#include <atomic>

class MenuLayer : public cocos2d::CCLayer, public FLAlertLayerProtocol {
public:
static MenuLayer* sharedLayer;
static std::atomic<int> s_newVersion;
static std::atomic<bool> s_hasVersionData;
    static MenuLayer* node();
    static cocos2d::CCScene* scene() ;

    void onMenuInfo();
    void onOpenMenu();
	void onOptions();
	void checkVersionIsOutdated();
	void showUpdateAlert(float dt);
	void onVersionReceived(CCObject*);
	void endGame();
	void versionCheckPoll(float dt) ;

    virtual bool init();
    virtual void keyBackClicked();
    virtual void googlePlaySignedIn();
    virtual void FLAlert_Clicked(FLAlertLayer* alert, bool btn2);
};