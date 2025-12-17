#ifndef __FPS_COUNTER_H__
#define __FPS_COUNTER_H__

#include "cocos2d.h"

USING_NS_CC;

class FPSCounter : public cocos2d::CCNode
{
public:
    FPSCounter();
	static FPSCounter* sharedCounter();
    virtual void update(float dt) override;
    
    // Public function to retrieve the last calculated FPS
	static FPSCounter* s_sharedCounter; 
    float m_fpsMeasurePeriod;
    int m_fpsAccumulator;
    float m_fpsLastMeasureTime;
    int m_currentFps;
	int _frames;
    float _accumulatedTime;
    float _currentFPS;
	FPSCounter* m_fpsCounter = nullptr;
};

#endif // __FPS_COUNTER_H__
