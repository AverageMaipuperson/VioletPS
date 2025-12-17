#pragma once

#include <cocos2d.h>

class DS_Dictionary {
public:
    int getIntegerForKey(const char* key);
    void setIntegerForKey(const char* key, int value);
};