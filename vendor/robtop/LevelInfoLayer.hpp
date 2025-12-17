#pragma once

#include <cocos2d.h>
#include "GJGameLevel.hpp"

class LevelInfoLayer : public CCLayer {
	public:
	static LevelInfoLayer* create(GJGameLevel*);
	void onClone();
};