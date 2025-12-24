#include <cocos2d.h>
#include "../vendor/robtop/HitboxLayer.hpp"
#include "CCGL.h"
using namespace cocos2d;

bool HitboxLayer::init() {
    if (!CCLayer::init()) return false;
    return true;
}

void HitboxLayer::draw() {
    cocos2d::CCLayer::draw();
    if (!l_targetLayer) return;

    // glLineWidth(2.0f);
    ccDrawColor4B(0, 255, 0, 255); // Green for batchnode objects

    cocos2d::CCArray* mainChildren = l_targetLayer->getChildren();
    if (!mainChildren) return;

    cocos2d::CCObject* it = NULL;
    CCARRAY_FOREACH(mainChildren, it) {
        cocos2d::CCNode* child = (cocos2d::CCNode*)it;

        // 1. Draw hitboxes for the node itself (Standard Sprites)
        drawNodeHitbox(child);

        // 2. If it's a BatchNode, draw hitboxes for all its children
        // Use dynamic_cast or check for CCSpriteBatchNode type
        cocos2d::CCSpriteBatchNode* batch = dynamic_cast<cocos2d::CCSpriteBatchNode*>(child);
        if (batch) {
            cocos2d::CCArray* batchChildren = batch->getChildren();
            cocos2d::CCObject* batchIt = NULL;
            CCARRAY_FOREACH(batchChildren, batchIt) {
                drawNodeHitbox((cocos2d::CCNode*)batchIt);
            }
        }
    }
}

// Helper to keep the code clean
void HitboxLayer::drawNodeHitbox(cocos2d::CCNode* node) {
    if (!node) return;
    
    cocos2d::CCRect box = node->boundingBox();
    
    // Note: boundingBox() is relative to parent. 
    // If the BatchNode is moved, its children's boxes follow correctly.
    cocos2d::ccDrawRect(
        ccp(box.origin.x, box.origin.y), 
        ccp(box.origin.x + box.size.width, box.origin.y + box.size.height)
    );
}
