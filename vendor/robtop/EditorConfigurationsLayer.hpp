#pragma once

#include <cocos2d.h>
#include "CCBlockLayer.h"
#include "GameStatsManager.hpp"
#include "CCTextInputNode.hpp"

namespace ExtraLayerInfo {
	enum class InfoType {
        PMH,
        HA,
        SEI,
		SF,
		FPS,
		NP,
		NDE,
		NOCLIP,
		DEATHS,
		FLASH,
		PEE,
        UNKNOWN
    };
}

class ExtraLayer : public CCBlockLayer {

public:
	static bool m_deaths;
	static bool m_flash;
	static float m_speedhack;
	static bool m_speedhackEnabled;
	static CCTextInputNode* m_speedHackInput;
    static ExtraLayer* create(CCLayer* referrer);
	void onClose(CCObject*);
	void dummy(CCObject* pSender);
	void readSettings();
	void showHA();
	void showEI();
	void showPMH();
	void showSF();
	void showFPS();
	void showNP();
	void showNDE();
	void showNOCLIP();
	void showDEATHS();
	void showFLASH();
	void showPEE();
	void showInfo(ExtraLayerInfo::InfoType type);
	static void incrementStat(GameStatsManager* self, char* type, int amount);
	static int getStat(GameStatsManager* self, char* type);
	static bool saveSettingsToFile();
	static bool loadSettingFromFile(const char* settingname);
	static void onLoadSettings();
	void onNoclipOptions();
	void onPEEOptions();
protected:
    bool init(CCLayer* referrer);
    void keyBackClicked();
};