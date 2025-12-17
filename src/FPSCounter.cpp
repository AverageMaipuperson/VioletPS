#include "../vendor/robtop/FPSCounter.hpp"
#include <iostream>
#include <ctime>

FPSCounter* FPSCounter::s_sharedCounter = nullptr;

FPSCounter::FPSCounter()
: m_fpsMeasurePeriod(0.5f)
, m_fpsAccumulator(0)
, m_currentFps(0)
{
    this->scheduleUpdate();
    m_fpsLastMeasureTime = std::clock();
}


