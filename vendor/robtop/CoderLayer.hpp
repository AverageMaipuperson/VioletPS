#pragma once

#include "cocos2d.h"
#include "GJGameLevel.hpp"
#include "FLAlertLayer.hpp"
#include "GJUserScore.hpp"
#include "CCMenuItemToggler.hpp"
#include "../cocos/CocosDenshion/android/jni/SimpleAudioEngineJni.h"
#include "../cocos/cocos2dx/extensions/network/HttpClient.h"
#include "../cocos/cocos2dx/extensions/network/HttpRequest.h"
#include "../cocos/cocos2dx/extensions/network/HttpResponse.h"
#include "cocos2dExt.h"

using namespace cocos2d;

#define INPUT_TAG 63584276

class CoderLayer : public CCLayer, public CCTextFieldDelegate {
    public:
	int starAmount = 0;
	int coinAmount = 0;
	int oldStarAmount = 0;
	int oldCoinAmount = 0;
	bool doorIsClosed = false;
	CCLabelBMFont* label = nullptr;
	CCSprite* starSprite = nullptr;
	static CoderLayer* sharedState();
    static CCLayer* create(CCLayer* referrer);
    void onClose(CCObject* sender);
	void onCloseWithDelay(CCObject* sender);
	static void reward(int type, int amount);
	static int getStat(int type);
	static void countUpScore(float dt, int amount);
	void animationWithSprite(CCSprite* sprite, float duration, CCPoint destination, int type = 0);
	void animationWithLabel(CCLabelBMFont* sprite, float duration, CCPoint destination, int type = 0);
	void initializeVariables();
	void countUpScore(float dt);
	void starAnimationBack();
	static bool checkIfTheDoorInCreateLayerInitIsClosedOrNot();
	bool cift();
	static CCScene* scene(CCLayer* referrer);
	virtual void fadeOutAndTransitionMusic();
	virtual ~CoderLayer();
	CCMenu* createButtonWithFunction(const char* name, CCPoint position, cocos2d::SEL_MenuHandler selector);
	CCMenu* createSpriteButtonWithFunction(CCSprite* sprite, CCPoint position, cocos2d::SEL_MenuHandler selector);
	CCLabelBMFont* newFont(const char* body, CCPoint position, const char* font, float scale);
	char* randomString();
	void updateLabel(CCLabelBMFont* label);
	void updateMainLabel();
	void showStarAnimation(int amount);
	void showCoinAnimation(int amount);
	virtual bool onTextFieldReturn(cocos2d::CCTextFieldTTF* pSender);
	void delayMenuLoop();
	void sendDataToServer();
	void onHttpRequestCompleted(cocos2d::extension::CCHttpClient *sender, cocos2d::extension::CCHttpResponse *response);
    protected:
    void keyBackClicked() override;
	bool init(CCLayer* self);
	void updateMusicVolume(float dt);
};