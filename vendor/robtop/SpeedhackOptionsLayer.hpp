#pragma once
#include <cocos2d.h>
#include "CCBlockLayer.h"
#include "CCTextInputNode.hpp"

class SpeedhackOptionsLayer : public CCBlockLayer, public CCTextFieldDelegate {

public:
    static SpeedhackOptionsLayer* create(CCLayer* referrer);
	void onClose(CCObject*);
	bool init(CCLayer* referrer);
    void keyBackClicked();
	virtual bool onTextFieldReturn(cocos2d::CCTextFieldTTF* pSender, const char * text, int nLen);
	virtual bool onTextFieldInsertText(cocos2d::CCTextFieldTTF* pSender, const char * text, int nLen) { return true; }
	void increment();
	void decrement();
};
