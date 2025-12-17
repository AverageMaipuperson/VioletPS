#pragma once

#include <cocos2d.h>
#include "FLAlertLayer.hpp"
#include "GameObject.hpp"
#include "CCMenuItemSpriteExtra.hpp"
#include <utility>

class CreateMenuItem : public CCMenuItemSpriteExtra {};

class EditorUI : public cocos2d::CCLayer, public cocos2d::CCTargetedTouchDelegate
{
public:
    static EditorUI* create();
    bool init();

// #if GAME_VERSION < 7
//     void keyBackClicked();
// #endif
    void onPause();
    void deselectAll();
    void selectObject(GameObject*);
    void onDeleteSelected();
    void updateCreateMenu();
    CreateMenuItem* getCreateBtn(const char*, int);
	void showDeleteConfirmation();
	void toggleDuplicateButton();
	void setFilterType();
	void createFilterBtn();
	static EditorUI& sharedInstance();

    CCMenuItemSpriteExtra* getSpriteButton(const char* name, SEL_MenuHandler callback, CCMenu* menu, float scale);
    CCMenuItemSpriteExtra* getSpriteButton2(const char* name, SEL_MenuHandler callback, CCMenu* menu, float scale);
    CCMenuItemSpriteExtra* getSpriteButton3(const char* name, SEL_MenuHandler callback, CCMenu* menu, float scale, float scale2);
    void moveObjectCall(CCNode* sender);
    void transformObjectCall(CCNode* sender);
    void moveObject(GameObject* obj, CCPoint transform);
    void editObject();
	void deleteAll();
	virtual bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    virtual void ccTouchMoved(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);
    virtual void ccTouchEnded(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);

    void moveObjectCall2(CCNode* sender);
    void transformObjectCall2(CCNode* sender);
};