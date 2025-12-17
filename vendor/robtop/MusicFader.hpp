#include "cocos2d.h"

class MusicFader {
public:
    virtual void fadeOutAndTransitionMusic() = 0; 
    virtual ~MusicFader() = default;
};