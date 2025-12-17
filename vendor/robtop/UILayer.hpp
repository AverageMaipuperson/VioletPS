#pragma once

#include <cocos2d.h>

class UILayer : public cocos2d::CCLayer {
public:
    static UILayer* create(void*);
    bool init(void*);

    void pCommand(CCNode*);

    void speedUp();
    void speedDown();
    void gravityUp();
    void gravityDown();
    void yStartUp();
    void yStartDown();

    void createCheatIndicator();
    void createPercentageLabel();
    void createPCommand();
    void createLabel();
    void updateLabel();
    void createSwitcher();
    void pickStartPos(int index);
    void onBackSP();
    void onForwardSP();

	void noclipLabel();

    void toggleCheckpointsMenu(bool toggle);
};