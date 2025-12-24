#include <cocos2d.h>

class HitboxLayer : public cocos2d::CCLayer {
public:
	virtual bool init();
    virtual void draw(); 
	void drawNodeHitbox(cocos2d::CCNode* node);
    static HitboxLayer* create() {
    HitboxLayer *pRet = new HitboxLayer();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}
    
    cocos2d::CCSpriteBatchNode* l_targetLayer; 
};