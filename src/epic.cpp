#include "../vendor/cocos/cocos2dx/include/cocos2d.h"
#include "../vendor/cocos/CocosDenshion/include/SimpleAudioEngine.h"
#include <iostream>
#include <string>
#include "../vendor/other/hooking.h"
#include "../vendor/cocos/cocos2dx/platform/CCPlatformMacros.h"
// #include "../vendor/robtop/GJGameLevel.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include "../vendor/robtop/FPSCounter.hpp"
#include "../vendor/robtop/OptionsLayer.hpp"
#include "../vendor/robtop/FLAlertLayer.hpp"
#include "../vendor/robtop/GameManager.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/ButtonSprite.hpp"
#include "../vendor/robtop/MenuLayer.hpp"
#include "platform/android/jni/JniHelper.h"
#include <typeinfo>
#include <jni.h>
#include <cmath>
#include <map>
#include <cstdint>
#include <unordered_map>
#include <ctime>
#include <random>
#include <utility>
#include "../vendor/robtop/NoclipOptionsLayer.hpp"
#include "../vendor/robtop/SpeedhackOptionsLayer.hpp"
#include "../vendor/robtop/LevelInfoLayer.hpp"
#include "../vendor/robtop/DS_Dictionary.hpp"
// #include "../vendor/robtop/patch.h"
#include "../vendor/robtop/PlayLayer.hpp"
#include "../vendor/robtop/LevelEditorLayer.hpp"
#include "../vendor/robtop/CoderLayer.hpp"
#include "../vendor/robtop/CreatorLayer.hpp"
#include "../vendor/robtop/GJGameLevel.hpp"
#include "../vendor/robtop/EditorUI.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/GJComment.hpp"
#include "../vendor/robtop/PauseLayer.hpp"
#include "../vendor/cocos/cocos2dx/cocoa/CCDictionary.h"
#include "../vendor/cocos/cocos2dx/cocoa/CCDictionary.cpp"
#include "../vendor/cocos/cocos2dx/cocoa/CCString.h"
#include "../vendor/cocos/cocos2dx/cocoa/CCString.cpp"

#define PAGE_1 = 743276
#define PAGE_2 = 743277
using namespace cocos2d;

#define MEMBER_BY_OFFSET(type, var, offset) \
    (*reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(var) + static_cast<uintptr_t>(offset)))

bool noclip = false;
bool extraLayerCreated = false;
bool elc = false;
bool hasActivatedNoclip = false;
bool safeMode = false;
int deaths = 0;
int atts = 0;
int jumps = 0;
bool hideatts = false;
bool extrainfo = false;
GJGameLevel* gjlvl = nullptr;
int song = 0;
bool pmh = false;
const char* displayText;
const char* titleText;
bool doorClosed = false;
bool hitbox = true;
bool filter = false;
bool filterOption = false;
bool filterShowedMsg = false;
bool fps = false;
bool added = false;
bool noParticles = false;
bool noDeathEffect = false;
bool confirmExit = false;
bool iconHack = false;
bool speedhack = false;
int lastDeadFrame = 0;
int frameCount = 0;
cocos2d::CCLabelBMFont* deathsLabel;

static JavaVM* g_vm = nullptr;

// Definition for the static member declared in `EditorConfigurationsLayer.hpp`
bool ExtraLayer::m_deaths = false;
bool ExtraLayer::m_flash = false;
float ExtraLayer::m_speedhack = 1.0f;
bool ExtraLayer::m_speedhackEnabled = false;

static JNIEnv* getEnv() {
  if (!g_vm) return nullptr;
  JNIEnv* env = nullptr;
  jint res = g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
  if (res == JNI_OK) return env;
  if (res == JNI_EDETACHED) {
    if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return nullptr;
    return env;
  }
  return nullptr;
}

CCSprite* getToggleSprite(CCSprite* on, CCSprite* off, bool state) { return (state) ? on : off; }
CCMenuItemSprite* getMenuToggleSprite(CCMenuItemSprite* on, CCMenuItemSprite* off, bool state) { return (state) ? on : off; }

template <typename T, size_t size>
int random_array_index(const T (&array)[size]) {
    int num = std::rand() % size; 

    return num;
}

void seekBackgroundMusicTo(int ms) {
    JNIEnv* env = getEnv();

    if (!env) {
        cocos2d::CCLog("Failed to get JNI environment");
        return;
    }
    jclass Cocos2dxActivity = env->FindClass("org/cocos2dx/lib/Cocos2dxActivity");
    if (Cocos2dxActivity == nullptr) {
        cocos2d::CCLog("Failed to find Cocos2dxActivity class");
        return;
    }
    // some cocos2d java class names have been obfuscated, but not all
    jfieldID fieldID_backgroundMusicPlayer = env->GetStaticFieldID(Cocos2dxActivity, "backgroundMusicPlayer", "Lorg/cocos2dx/lib/p;");
    if (fieldID_backgroundMusicPlayer == nullptr) {
        cocos2d::CCLog("Failed to get field ID of backgroundMusicPlayer");
        return;
    }
    jobject backgroundMusicPlayer = env->GetStaticObjectField(Cocos2dxActivity, fieldID_backgroundMusicPlayer);
    if (backgroundMusicPlayer == nullptr) {
        cocos2d::CCLog("Failed to get backgroundMusicPlayer");
        return;
    }

    jclass Cocos2dxMusic = env->FindClass("org/cocos2dx/lib/p");
    if (Cocos2dxMusic == nullptr) {
        cocos2d::CCLog("Failed to get Cocos2dxMusic");
        return;
    }
    jfieldID fieldID_mBackgroundMediaPlayer = env->GetFieldID(Cocos2dxMusic, "mBackgroundMediaPlayer", "Landroid/media/MediaPlayer;");
    if (fieldID_mBackgroundMediaPlayer == nullptr) {
        cocos2d::CCLog("Failed to get field ID of mBackgroundMediaPlayer");
        return;
    }
    jobject mBackgroundMediaPlayer = env->GetObjectField(backgroundMusicPlayer, fieldID_mBackgroundMediaPlayer);
    if (mBackgroundMediaPlayer == nullptr) {
        cocos2d::CCLog("Failed to get mBackgroundMediaPlayer");
        return;
    }

    jclass MediaPlayer = env->GetObjectClass(mBackgroundMediaPlayer);
    if (MediaPlayer == nullptr) {
        cocos2d::CCLog("Failed to get MediaPlayer");
        return;
    }
    
    bool useNew = true;
    jclass versionClass;
    jfieldID sdkIntFieldID;
    jint sdkJint;
    int sdkInt;
    jmethodID seekTo;
    do {
        versionClass = env->FindClass("android/os/Build$VERSION");
        if (versionClass == nullptr) {
            cocos2d::CCLog("Warning: Failed to get class Build$VERSION. Using old \"broken\" method.");
            useNew = false;
            break;
        }
        sdkIntFieldID = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
        if (sdkIntFieldID == nullptr) {
            cocos2d::CCLog("Warning: Failed to get field ID of SDK_INT. Using old \"broken\" method.");
            useNew = false;
            break;
        }

        sdkJint = env->GetStaticIntField(versionClass, sdkIntFieldID);
        sdkInt = static_cast<int>(sdkJint);
        if (sdkInt < 26) {
            cocos2d::CCLog("sdkInt: %i < 26. Using old \"broken\" method.", sdkInt);
            useNew = false;
            break;
        }
        seekTo = env->GetMethodID(MediaPlayer, "seekTo", "(JI)V");
        if (seekTo == nullptr) {
            cocos2d::CCLog("Warning: Failed to get method ID of seekTo(long, int). Using old \"broken\" method.");
            useNew = false;
            break;
        }
    } while (0);

    if (useNew) {
        env->CallVoidMethod(mBackgroundMediaPlayer, seekTo, static_cast<jlong>(static_cast<long>(ms)), static_cast<jint>(2));
    } else {
        jmethodID oldSeekTo = env->GetMethodID(MediaPlayer, "seekTo", "(I)V");
        if (oldSeekTo == nullptr) {
            cocos2d::CCLog("Failed to get method ID of seekTo(int)");
            return;
        }
        env->CallVoidMethod(mBackgroundMediaPlayer, oldSeekTo, static_cast<jint>(ms));
    } 
}

template <typename T>
std::string ToString(T val)
{
    std::stringstream stream;
    stream << val;
    return stream.str();
}

int stoi(const std::string& s) {
    int result = 0;
    int multiplier = 1;
    if (s.empty()) return 0;
    for (size_t i = s.length(); i-- > 0;) {
        result += multiplier * (s[i] - '0');
        multiplier *= 10;
    }
    return result;
}

template <class R2>
R2 callFunctionFromSymbol(const char* symbol) {
    return reinterpret_cast<R2>(HookManager::getPointerFromSymbol(dlopen("libgame.so", RTLD_LAZY), symbol));
}

template <class R, class T>
R& from(T base, intptr_t offset) {
    return *reinterpret_cast<R*>(reinterpret_cast<uintptr_t>(base) + offset);
}

// test shitttt
#define cpatch(addr, val) addPatch("libgame.so", addr, val)

class LevelCell : public CCLayer {
public:
  virtual ~LevelCell() = default;
};

class SimpleAudioEngine {
  public:
  virtual ~SimpleAudioEngine() = default;
};

void PlayLayer::triggerRedPulse(float duration)
{
    CCSize visibleSize = CCDirector::sharedDirector()->getWinSize();
    CCLayerColor* redPulseLayer = CCLayerColor::create(ccc4(255, 0, 0, 255), visibleSize.width, visibleSize.height);
    redPulseLayer->setOpacity(200);
    redPulseLayer->setBlendFunc({GL_ONE, GL_ONE_MINUS_SRC_ALPHA});
    redPulseLayer->setAnchorPoint(ccp(0, 0));
    redPulseLayer->setPosition(ccp(0, 0));
    this->addChild(redPulseLayer, 10);
    CCActionInterval* fadeOut = CCFadeOut::create(duration);
    CCCallFunc* removeLayer = CCCallFunc::create(redPulseLayer, callfunc_selector(CCLayerColor::removeFromParentAndCleanup));
    auto sequence = CCSequence::create(fadeOut, removeLayer, NULL);
    redPulseLayer->runAction(sequence);
}

void PlayLayer::pulseLabelRed(CCLabelBMFont* label, float duration)
{
    ccColor3B originalColor = ccc3(255, 255, 255); 
    ccColor3B pulseColor = ccc3(255, 0, 0);
    CCTintTo* tintToRed = CCTintTo::create(duration / 2.0f, pulseColor.r, pulseColor.g, pulseColor.b);
    CCTintTo* tintToOriginal = CCTintTo::create(duration / 2.0f, originalColor.r, originalColor.g, originalColor.b);
    auto sequence = CCSequence::create(tintToRed, tintToOriginal, NULL);
    label->runAction(sequence);
}


char * (*LevelTools_getAudioTitle)(int ID);
char * LevelTools_getAudioTitle_H(int ID) {
  switch (ID) {
    case -1: return "Practice: Stay Inside Me";
    case 0: return "Stereo Madness";
    case 1: return "Back On Track";
    case 2: return "Polargeist";
    case 3: return "Dry Out";
    case 4: return "Base After Base";
    case 5: return "Cant Let Go";
    case 6: return "Jumper";
    case 7: return "Time Machine";
    case 8: return "Cycles";
    case 9: return "xStep";
    case 10: return "Clutterfunk";
    case 11: return "Theory Of Everything";
    case 12: return "Electroman Adventures";
    case 13: return "Clubstep";
    case 14: return "Active";
    case 15: return "Electrodynamix";
    case 16: return "Hexagon Force";
    case 17: return "Blast Processing";
    case 18: return "Theory Of Everything 2";
    case 19: return "Cosmic Dreamer";
    case 20: return "Sky Fortress";
    case 21: return "Sound of Infinity";
    case 22: return "Rupture";
    case 23: return "Stalemate";
    case 24: return "Glorious Morning";
    default: return "Unknown";
  }
}

class ToggleHack {
  public:
    void toggleNoclip(CCObject*, GameManager* self) {noclip = !noclip;
    if(!hasActivatedNoclip && noclip) {
      FLAlertLayer::create(
            nullptr,
            "Warning",
            CCString::createWithFormat("Noclip will turn on Safe Mode and it will kick you out of the level if you try beating it.")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
      hasActivatedNoclip = true;
    }
    return;
    }

    void patchNotes() {
      FLAlertLayer::create(
            nullptr,
            "Patch Notes",
            CCString::createWithFormat("1. Added <cr>Patch Notes</c>\n2. <cl>Practice Music Hack</c>\n3. <cg>Icon Hack</c>")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
    }

    void showAtts() {
      float normalPercentageF = MEMBER_BY_OFFSET(float, gjlvl, 0x170);
      int normalPercentage = (int)normalPercentageF;
      FLAlertLayer::create(
            nullptr,
            "Info",
            CCString::createWithFormat("<cy>Attempts:</c> %i\n<cg>Jumps:</c> %i\n<cp>Debug Normal Percentage</c> %i", atts, jumps, normalPercentage)->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
    }

    void showLevelInfo(GJGameLevel* lvl) {
      int attempts = MEMBER_BY_OFFSET(int, lvl, 0x168);
      int totaljumps = MEMBER_BY_OFFSET(int, lvl, 0x16c);
      int normalpercent = MEMBER_BY_OFFSET(int, lvl, 0x170);
      int practicepercent = MEMBER_BY_OFFSET(int, lvl, 0x174);
      FLAlertLayer::create(
            nullptr,
            "Level Info",
            CCString::createWithFormat("<cg>Total Attempts:</c> %i\n<cl>Total Jumps:</c> %i\n<cp>Normal:</c> %i%%\n<co>Practice:</c> %i%%", attempts, totaljumps, normalpercent, practicepercent)->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
    }

    void showMoreLevelInfo(GJGameLevel* lvl) {
      auto fVar2 = (float)(int)lvl / 311.0 * 1000.f;
      auto length = (int)(float)(int)floorf(fVar2);

      const char* audioTrack = LevelTools_getAudioTitle_H(MEMBER_BY_OFFSET(int, lvl, 0x148));
      FLAlertLayer::create(
            nullptr,
            "Level Info",
            CCString::createWithFormat("<cg>Length:</c> %i\n<cl>Song:</c> %s", length, audioTrack)->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
    }

    void nothingHere() {
      FLAlertLayer::create(
            nullptr,
            "Patch Notes",
            CCString::createWithFormat("Nothing here..\n\n :)")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
    }

    void hideAttempts() {hideatts = !hideatts;}

    void extraInfo() {extrainfo = !extrainfo;}

    void pmhf() {pmh = !pmh;}

    void hit() {hitbox = !hitbox;}

    void sf() {filterOption = !filterOption;
    if(!filterShowedMsg){
      FLAlertLayer::create(
            nullptr,
            "Warning",
            CCString::createWithFormat("Select Filter is really <cr>limited and buggy</c>, you should use it considering these facts.")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
        filterShowedMsg = true;
    }}

    void dfps() {fps = !fps;}

    void np() {noParticles = !noParticles;}

    void nde() {noDeathEffect = !noDeathEffect;}

    void iconHackF() {iconHack = !iconHack;}

    void speedhackT() {speedhack = !speedhack;}

    void dummy(CCObject* pSender) {};

};

void (*GameStatsManager_incrementStat)(GameStatsManager* self, char* type, int amount);
void GameStatsManager_incrementStat_H(GameStatsManager* self, char* type, int amount) {
    GameStatsManager_incrementStat(self, type, amount);
}

  void ExtraLayer::incrementStat(GameStatsManager* self, char* type, int amount) {
    GameStatsManager_incrementStat(self, type, amount);
  } 
  bool ExtraLayer::saveSettingsToFile() {
    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    
    def->setBoolForKey("noclip", noclip);
    def->setBoolForKey("hideatts", hideatts);
    def->setBoolForKey("extrainfo", extrainfo);
    def->setBoolForKey("fps", fps);
    def->setBoolForKey("noParticles", noParticles);
    def->setBoolForKey("noDeathEffect", noDeathEffect);
    def->setBoolForKey("speedhack", speedhack);
    def->setBoolForKey("noclipdeaths", ExtraLayer::m_deaths);
    def->setBoolForKey("noclipflash", ExtraLayer::m_flash);
    def->setFloatForKey("speedhackInt", ExtraLayer::m_speedhack);

    def->flush();
    return true;
  }

  void ExtraLayer::onLoadSettings() {
    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
  noclip = def->getBoolForKey("noclip", false);
  hideatts = def->getBoolForKey("hideatts", false);
  extrainfo = def->getBoolForKey("extrainfo", false);
  doorClosed = def->getBoolForKey("doorClosed", false);
  filterOption = def->getBoolForKey("filterOption", false);
  fps = def->getBoolForKey("fps", false);
  noParticles = def->getBoolForKey("noParticles", false);
  noDeathEffect = def->getBoolForKey("noDeathEffect", false);
  speedhack = def->getBoolForKey("speedhack", false);
  ExtraLayer::m_deaths = def->getBoolForKey("noclipdeaths", false);
  ExtraLayer::m_flash = def->getBoolForKey("noclipflash", false);
  ExtraLayer::m_speedhack = def->getFloatForKey("speedhackInt", 1.0f);
  }

  ExtraLayer* ExtraLayer::create(CCLayer* referrer) {
    auto ret = new ExtraLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

void ExtraLayer::onNoclipOptions() {
  auto extra = NoclipOptionsLayer::create(this);
    this->addChild(extra, 1000);
}

void ExtraLayer::onPEEOptions() {
  auto extra = SpeedhackOptionsLayer::create(this);
    this->addChild(extra, 1000);
}

  bool ExtraLayer::init(CCLayer* self) {
    CCNode* leftParent = CCNode::create();
    auto win_size = CCDirector::sharedDirector()->getWinSize();
     CCLayerColor *overlay = CCLayerColor::layerWithColor(ccc4(0, 0, 0, 127),
                                                win_size.width,
                                                 win_size.height);
    overlay->setPosition(0,0);
    this->addChild(overlay);
    this->addChild(leftParent);
    leftParent->setPosition(win_size.width / 2, win_size.height / 2);


    CCRect rect = CCRectMake(0, 0, 80, 80);
    cocos2d::extension::CCScale9Sprite* panel = cocos2d::extension::CCScale9Sprite::create("GJ_square01-hd.png", rect);
    panel->setContentSize(CCSizeMake(win_size.width - 50, win_size.height - 25));
    leftParent->addChild(panel);
    panel->setPosition({0.f, 0.f});

    CCLayer* mainLayoutLayer = CCLayer::create();
mainLayoutLayer->setAnchorPoint(ccp(0.5f, 0.5f));


    auto buttonMenu = CCMenu::create();
auto button = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, this, menu_selector(ExtraLayer::keyBackClicked));
menuBtn->setPosition({-25,-25});
buttonMenu->setPosition({win_size.width, win_size.height});
buttonMenu->addChild(menuBtn);
this->addChild(buttonMenu);
this->setTouchEnabled(true);
self->setTouchEnabled(false);

auto titleLabel = CCLabelBMFont::create(
            CCString::createWithFormat("More Options")->getCString(), 
            "goldFont-hd.fnt"
        );

  titleLabel->setPosition({win_size.width / 2, win_size.height - 35});
  titleLabel->setScale(1.0f);
  this->addChild(titleLabel);

  auto toggleOffSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  CCAssert(toggleOffSprite != nullptr, "FATAL error: GJ_checkOff_001.png missing from the cache");
  auto toggleOnSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  CCAssert(toggleOnSprite != nullptr, "FATAL error: GJ_checkOn_001.png missing from the cache");
  auto itemOff = CCMenuItemSprite::create(toggleOffSprite, toggleOffSprite, nullptr, nullptr);
auto itemOn  = CCMenuItemSprite::create(toggleOnSprite,  toggleOnSprite,  nullptr, nullptr);

// 3. Create the Toggler using the *menu items*, the target, and the selector.
auto hideAttsbutton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn, itemOff, hideatts), // Default (initial) item
    getMenuToggleSprite(itemOff, itemOn, hideatts),  // Second item (toggled state)  // MUST end the variable argument list with nullptr
    this, // Target
    menu_selector(ToggleHack::hideAttempts) // Selector
);

  auto menu1 = CCMenu::create();
   auto counterLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Hide Attempts")->getCString(), 
            "bigFont.fnt"
        );
        CCAssert(counterLabel != nullptr, "FATAL error: bigFont.fnt missing");
        counterLabel->setScale(0.5f);
        auto labelMenuItem = CCMenuItemLabel::create(
    counterLabel, 
    this, 
    menu_selector(ExtraLayer::showHA) 
);
  hideAttsbutton->setScale(0.8f);
  menu1->addChild(hideAttsbutton);
  menu1->addChild(labelMenuItem);
  menu1->setPosition({50, win_size.height - 25});

  menu1->alignItemsHorizontally();

  
auto toggleOffSprite2 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  CCAssert(toggleOffSprite2 != nullptr, "FATAL error: GJ_checkOff_001.png missing from the cache");
  auto toggleOnSprite2 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  CCAssert(toggleOnSprite2 != nullptr, "FATAL error: GJ_checkOn_001.png missing from the cache");
  auto itemOff2 = CCMenuItemSprite::create(toggleOffSprite2, toggleOffSprite2, nullptr, nullptr);
auto itemOn2  = CCMenuItemSprite::create(toggleOnSprite2,  toggleOnSprite2,  nullptr, nullptr);

  auto extraInfoButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn2, itemOff2, extrainfo),
    getMenuToggleSprite(itemOff2, itemOn2, extrainfo),  
    this, 
    menu_selector(ToggleHack::extraInfo) 
);

  auto menu2 = CCMenu::create();
   auto counterLabel2 = CCLabelBMFont::create(
            CCString::createWithFormat("Show Extra Info")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel2->setScale(0.5f);
        auto labelMenuItem2 = CCMenuItemLabel::create(
    counterLabel2, 
    this, 
    menu_selector(ExtraLayer::showEI) 
);
  extraInfoButton->setScale(0.8f);
  menu2->addChild(extraInfoButton);
  menu2->addChild(labelMenuItem2);
  menu2->setPosition({200, win_size.height - 100});

  menu2->alignItemsHorizontally();

  auto toggleOffSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  CCAssert(toggleOffSprite3 != nullptr, "FATAL error: GJ_checkOff_001.png missing from the cache");
  auto toggleOnSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  CCAssert(toggleOnSprite3 != nullptr, "FATAL error: GJ_checkOn_001.png missing from the cache");
  auto itemOff3 = CCMenuItemSprite::create(toggleOffSprite3, toggleOffSprite3, nullptr, nullptr);
auto itemOn3  = CCMenuItemSprite::create(toggleOnSprite3,  toggleOnSprite3,  nullptr, nullptr);

  auto PMHButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn3, itemOff3, pmh),
    getMenuToggleSprite(itemOff3, itemOn3, pmh),  
    this, 
    menu_selector(ToggleHack::pmhf) 
);

  auto menu3 = CCMenu::create();
   auto counterLabel3 = CCLabelBMFont::create(
            CCString::createWithFormat("Practice Music Hack")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel3->setScale(0.5f);
        auto labelMenuItem3 = CCMenuItemLabel::create(
    counterLabel3, 
    this, 
    menu_selector(ExtraLayer::showPMH) 
);
  PMHButton->setScale(0.8f);
  menu3->addChild(PMHButton);
  menu3->addChild(labelMenuItem3);
  menu3->setPosition({200, win_size.height - 100});

  menu3->alignItemsHorizontally();

  auto toggleOffSprite4 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  
  auto toggleOnSprite4 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff4 = CCMenuItemSprite::create(toggleOffSprite4, toggleOffSprite4, nullptr, nullptr);
auto itemOn4  = CCMenuItemSprite::create(toggleOnSprite4,  toggleOnSprite4,  nullptr, nullptr);

  auto FPSButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn4, itemOff4, fps),
    getMenuToggleSprite(itemOff4, itemOn4, fps),  
    this, 
    menu_selector(ToggleHack::dfps) 
);

  auto menu4 = CCMenu::create();
   auto counterLabel4 = CCLabelBMFont::create(
            CCString::createWithFormat("Show FPS")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel4->setScale(0.5f);
        auto labelMenuItem4 = CCMenuItemLabel::create(
    counterLabel4, 
    this, 
    menu_selector(ExtraLayer::showFPS) 
);
  FPSButton->setScale(0.8f);
  menu4->addChild(FPSButton);
  menu4->addChild(labelMenuItem4);

  menu4->alignItemsHorizontally();

  auto toggleOffSprite5 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite5 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff5 = CCMenuItemSprite::create(toggleOffSprite5, toggleOffSprite5, nullptr, nullptr);
auto itemOn5  = CCMenuItemSprite::create(toggleOnSprite5,  toggleOnSprite5,  nullptr, nullptr);

  auto NPButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn5, itemOff5, noParticles),
    getMenuToggleSprite(itemOff5, itemOn5, noParticles),  
    this, 
    menu_selector(ToggleHack::np) 
);

  auto menu5 = CCMenu::create();
   auto counterLabel5 = CCLabelBMFont::create(
            CCString::createWithFormat("Show Percentage")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel5->setScale(0.5f);
        auto labelMenuItem5 = CCMenuItemLabel::create(
    counterLabel5, 
    this, 
    menu_selector(ExtraLayer::showNP) 
);
  NPButton->setScale(0.8f);
  menu5->addChild(NPButton);
  menu5->addChild(labelMenuItem5);

  menu5->alignItemsHorizontally();

   auto toggleOffSprite6 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite6 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff6 = CCMenuItemSprite::create(toggleOffSprite6, toggleOffSprite6, nullptr, nullptr);
auto itemOn6 = CCMenuItemSprite::create(toggleOnSprite6,  toggleOnSprite6,  nullptr, nullptr);

  auto NDEButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn6, itemOff6, noDeathEffect),
    getMenuToggleSprite(itemOff6, itemOn6, noDeathEffect),  
    this, 
    menu_selector(ToggleHack::nde) 
);

  auto menu6 = CCMenu::create();
   auto counterLabel6 = CCLabelBMFont::create(
            CCString::createWithFormat("No Death Effect")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel6->setScale(0.5f);
        auto labelMenuItem6 = CCMenuItemLabel::create(
    counterLabel6, 
    this, 
    menu_selector(ExtraLayer::showNDE) 
);
  NDEButton->setScale(0.8f);
  menu6->addChild(NDEButton);
  menu6->addChild(labelMenuItem6);

  menu6->alignItemsHorizontally();

auto toggleOffSprite7 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite7 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff7 = CCMenuItemSprite::create(toggleOffSprite7, toggleOffSprite7, nullptr, nullptr);
auto itemOn7 = CCMenuItemSprite::create(toggleOnSprite7,  toggleOnSprite7,  nullptr, nullptr);

  auto NOCLIPButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn7, itemOff7, noclip),
    getMenuToggleSprite(itemOff7, itemOn7, noclip),  
    this, 
    menu_selector(ToggleHack::toggleNoclip)
  );
  auto menu7 = CCMenu::create();
  auto counterLabel7 = CCLabelBMFont::create(
            CCString::createWithFormat("No-Clip")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel7->setScale(0.5f);
        auto labelMenuItem7 = CCMenuItemLabel::create(
    counterLabel7, 
    this, 
    menu_selector(ExtraLayer::showNOCLIP) 
);

auto noclipBtnSprite = ButtonSprite::create(
    "+", 25, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto noclipButtonMore = CCMenuItemSpriteExtra::create(
    noclipBtnSprite,
    noclipBtnSprite,
    this,
    menu_selector(ExtraLayer::onNoclipOptions)
  );
  NOCLIPButton->setScale(0.8f);
  noclipButtonMore->setPosition(-100, 0);
  menu7->addChild(NOCLIPButton);
  menu7->addChild(labelMenuItem7);
  menu7->addChild(noclipButtonMore);
  menu7->alignItemsHorizontally();

  auto toggleOffSprite8 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite8 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  
  auto itemOff8 = CCMenuItemSprite::create(toggleOffSprite8, toggleOffSprite8, nullptr, nullptr);
auto itemOn8 = CCMenuItemSprite::create(toggleOnSprite8,  toggleOnSprite8,  nullptr, nullptr);

  auto PEEButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn8, itemOff8, speedhack),
    getMenuToggleSprite(itemOff8, itemOn8, speedhack),  
    this, 
    menu_selector(ToggleHack::speedhackT)
  );
  auto menu8 = CCMenu::create();
  auto counterLabel8 = CCLabelBMFont::create(
            CCString::createWithFormat("Speedhack")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel8->setScale(0.5f);
        auto labelMenuItem8 = CCMenuItemLabel::create(
    counterLabel8, 
    this, 
    menu_selector(ExtraLayer::showPEE) 
);

auto PEEBtnSprite = ButtonSprite::create(
    "+", 25, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto menu9 = CCMenu::create();
  auto PEEButtonMore = CCMenuItemSpriteExtra::create(
    PEEBtnSprite,
    PEEBtnSprite,
    this,
    menu_selector(ExtraLayer::onPEEOptions)
  );
  menu9->addChild(PEEButtonMore);

  PEEButton->setScale(0.8f);
  menu8->addChild(PEEButton);
  menu8->addChild(labelMenuItem8);
  menu8->alignItemsHorizontally();


  /* auto menu7 = CCMenu::create();
  auto noclipBtnSprite = ButtonSprite::create(
    "Practice UI", 100, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto noclipButton = CCMenuItemSpriteExtra::create(
    noclipBtnSprite,
    noclipBtnSprite,
    this,
    menu_selector(ToggleHack::toggleNoclip)
  );

  noclipButton->setScale(0.8f);
  menu7->addChild(noclipButton); */

  float paddingBetweenMenus = 20.0f;
  float horizontalPadding = 100.0f;
  float xPos = 5.0f;

  menu1->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu2->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu3->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu4->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu5->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu6->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu7->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  menu8->setAnchorPoint(CCPointMake(0.0f, 0.5f));

menu1->setPosition(ccp(xPos, paddingBetweenMenus));

menu2->setPosition(ccp(xPos + 19, -(paddingBetweenMenus)));

menu3->setPosition(ccp(xPos + 57, -(paddingBetweenMenus) * 3));

menu4->setPosition(ccp(xPos - 40, -(paddingBetweenMenus) * 5));

menu5->setPosition(ccp(xPos + 25, -(paddingBetweenMenus) * 7));

menu6->setPosition(ccp(xPos + 300, paddingBetweenMenus));

menu7->setPosition(ccp(xPos + 238, -(paddingBetweenMenus)));

menu8->setPosition(ccp(xPos + 248, -(paddingBetweenMenus) * 3));

labelMenuItem->setPosition(12.5, 7);
labelMenuItem2->setPosition(12.5, 7);
labelMenuItem3->setPosition(12.5, 7);
labelMenuItem4->setPosition(12.5, 7);
labelMenuItem5->setPosition(12.5, 7);
labelMenuItem6->setPosition(12.5, 7);
labelMenuItem7->setPosition(0.5, 7);
labelMenuItem8->setPosition(12.5, 7);

mainLayoutLayer->setPosition(ccp(win_size.width / 3.0f, win_size.height / 2.0f + 30));
menu9->setPosition( 350/* 325 */, -(paddingBetweenMenus) * 3);

  mainLayoutLayer->addChild(menu1);
  mainLayoutLayer->addChild(menu2);
  mainLayoutLayer->addChild(menu3);
  mainLayoutLayer->addChild(menu4);
  mainLayoutLayer->addChild(menu5);
  mainLayoutLayer->addChild(menu6);
  mainLayoutLayer->addChild(menu7);
  mainLayoutLayer->addChild(menu9);
  mainLayoutLayer->addChild(menu8);
  mainLayoutLayer->reorderChild(menu9, 1000);
  mainLayoutLayer->reorderChild(menu8, 999);
  this->addChild(mainLayoutLayer);

  auto infoLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Click on the labels for more info")->getCString(), 
            "chatFont.fnt"
        );
        infoLabel->setScale(0.5f);
        infoLabel->setAnchorPoint(CCPointMake(0.5f, 0.5f));
        infoLabel->setColor(ccc3(129, 68, 37));
        infoLabel->setPosition(win_size.width / 2, 25);

        this->addChild(infoLabel);

  
  this->setKeypadEnabled(true);
  return true;
  }

  void ExtraLayer::onClose(CCObject* sender) {
    if (sender) this->retain();
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
    elc = false;
    this->saveSettingsToFile();
  }

  void ExtraLayer::keyBackClicked() {
    onClose(nullptr);
  }

  void ExtraLayer::showInfo(ExtraLayerInfo::InfoType type) {
    switch(type) {
      case ExtraLayerInfo::InfoType::PMH: 
      displayText = "Replaces practice music.\n<cr>WARNING:</c> It might not work as intended!";
      titleText = "Practice Music Hack";
      break;
      case ExtraLayerInfo::InfoType::HA: 
      displayText = "Hides the Attempt Label while playing.";
      titleText = "Hide Attempts";
      break;
      case ExtraLayerInfo::InfoType::SEI: 
      displayText = "Shows Attempts and Jumps on your current level session.";
      titleText = "Show Extra Info";
      break;
      case ExtraLayerInfo::InfoType::SF:
      displayText = "Enables Select Filter.";
      titleText = "Select Filter";
      break;
      case ExtraLayerInfo::InfoType::FPS:
      displayText = "Displays Framerate. \n <cy>If you're using an android emulator on PC, you don't need this option.</c>";
      titleText = "Show FPS";
      break;
      case ExtraLayerInfo::InfoType::NP:
      displayText = "Displays percentage as a number.";
      titleText = "Show Percentage";
      break;
      case ExtraLayerInfo::InfoType::NDE:
      displayText = "Removes all effects when dying.";
      titleText = "No Death Effect";
      break;
      case ExtraLayerInfo::InfoType::NOCLIP:
      displayText = "Disables death on collision. <cr>Turns on Safe Mode automatically</c>";
      titleText = "No-Clip";
      break;
      case ExtraLayerInfo::InfoType::DEATHS:
      displayText = "Shows deaths when turning noclip on.";
      titleText = "NoClip Deaths";
      break;
      case ExtraLayerInfo::InfoType::FLASH:
      displayText = "Shows a red flash over the screen on death.";
      titleText = "Show Flash";
      break;
      case ExtraLayerInfo::InfoType::PEE:
      displayText = "Turns on speedhack. <cr>Turns on Safe Mode automatically</c>";
      titleText = "Speedhack";
      break;
      default:
      displayText = "<cr>unknown error</c>";
      titleText = "ERROR";
    }
    FLAlertLayer::create(
            nullptr,
            CCString::createWithFormat("%s", titleText)->getCString(),
            CCString::createWithFormat("%s", displayText)->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
  }

  void ExtraLayer::showHA() {
    showInfo(ExtraLayerInfo::InfoType::HA);
  }

  void ExtraLayer::showEI() {
    showInfo(ExtraLayerInfo::InfoType::SEI);
  }

  void ExtraLayer::showPMH() {
    showInfo(ExtraLayerInfo::InfoType::PMH);
  }

  void ExtraLayer::showSF() {
  showInfo(ExtraLayerInfo::InfoType::SF);
  }

  void ExtraLayer::showFPS() {
    showInfo(ExtraLayerInfo::InfoType::FPS);
  }

   void ExtraLayer::showNP() {
    showInfo(ExtraLayerInfo::InfoType::NP);
  }

  void ExtraLayer::showNDE() {
    showInfo(ExtraLayerInfo::InfoType::NDE);
  }

  void ExtraLayer::showNOCLIP() {
    showInfo(ExtraLayerInfo::InfoType::NOCLIP);
  }

  void ExtraLayer::showFLASH() {
    showInfo(ExtraLayerInfo::InfoType::FLASH);
  }

  void ExtraLayer::showPEE() {
    showInfo(ExtraLayerInfo::InfoType::PEE);
  }

ExtraLayer* extra = nullptr;
  void PauseLayer::onOpenMenu() {
    if(!elc) { //HaxOverlay::create(this);
    this->setTouchEnabled(false);
    this->setKeypadEnabled(false);
    extra = ExtraLayer::create(this);
    this->addChild(extra, 1000);
    elc = true;
    extra->_setZOrder(10000000);
    } else {
      menu_selector(ExtraLayer::onClose);
    }
}

std::map<GJGameLevel*, int> magic;

GJGameLevel* (*GJGameLevel_create)(CCDictionary* dict);
GJGameLevel* GJGameLevel_create_H(CCDictionary* dict) {
  auto lvl = GJGameLevel_create(dict);
  if (dict->valueForKey("42")->intValue() > 0) magic[lvl] = 1;
  else magic[lvl] = 0;
  return lvl;
}

void (*GJGameLevel_encodeWithCoder)(GJGameLevel* self, DS_Dictionary* dsd);
void GJGameLevel_encodeWithCoder_H(GJGameLevel* self, DS_Dictionary* dsd) {
  GJGameLevel_encodeWithCoder(self, dsd);
  dsd->setIntegerForKey("k42", magic[self]);
}

GJGameLevel* (*GJGameLevel_createWithCoder)(DS_Dictionary* dsd);
GJGameLevel* GJGameLevel_createWithCoder_H(DS_Dictionary* dsd) {
  auto lvl = GJGameLevel_createWithCoder(dsd);
  magic[lvl] = dsd->getIntegerForKey("k42");
  return lvl;
}

void (*GJGameLevel_destructor)(GJGameLevel*);
void GJGameLevel_destructor_H(GJGameLevel* self) {
  auto val = magic.find(self);
  if (val != magic.end()) magic.erase(val);
  GJGameLevel_destructor(self);
}

 int GJGameLevel::getFeatured(GJGameLevel* self) { return 0; }
void (*LevelCell_loadCustomLevelCell)(LevelCell*);
void LevelCell_loadCustomLevelCell_H(LevelCell* self) {
  LevelCell_loadCustomLevelCell(self);
  GJGameLevel* gamelevel;
  auto lvl = from<GJGameLevel*>(self, 0x180);
  int epic = MEMBER_BY_OFFSET(int, lvl, 0x184);
  if (epic < 2) return;
  CCSprite* frame = CCSprite::create("GJ_epicCoin_001.png");
   frame->retain();
    frame->setPosition({26.f, 40.f});
    frame->_setZOrder(-1);
    self->addChild(frame);
     frame->release();
}

bool (*LevelInfoLayer_init)(CCLayer* self, GJGameLevel* lvl);
bool LevelInfoLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
  auto win_size = CCDirector::sharedDirector()->getWinSize();
  LevelInfoLayer_init(self, lvl);
  auto epic = MEMBER_BY_OFFSET(int, lvl, 0x184);
  int audioTrack = MEMBER_BY_OFFSET(int, lvl, 0x148);
    CCNode* containerNode = CCNode::create();
    auto trackLabel = CCLabelBMFont::create(
            CCString::createWithFormat("%s", LevelTools_getAudioTitle_H(audioTrack))->getCString(), 
            "bigFont.fnt"
        );
  std::string audiotitlestd = LevelTools_getAudioTitle_H(audioTrack);
  trackLabel->setScale(0.5f);
  CCSprite* note = CCSprite::createWithSpriteFrameName("GJ_noteIcon_001.png");
  note->setPosition({win_size.width / 2 + 60, win_size.height / 2 + 30});
  trackLabel->setAnchorPoint(CCPointMake(0.0f, 0.5f));
  trackLabel->setPosition({note->getPosition().x + 20, note->getPosition().y});
  containerNode->addChild(trackLabel);
  containerNode->addChild(note);
  self->addChild(containerNode);

/* auto button2 = CCSprite::create("GJ_duplicateBtn_001.png");
CCTexture2D *texture = button2->getTexture();
    texture->setAntiAliasTexParameters(); 
CCMenuItemSpriteExtra* menuBtn2 = CCMenuItemSpriteExtra::create(button2, button2, self, menu_selector(LevelInfoLayer::onClone));
menuBtn2->setScale(1.f);
menuBtn2->_setZOrder(100);
menuBtn2->setPosition({10, win_size.height / 2});
auto buttonMenu2 = CCMenu::create(menuBtn2, NULL);
self->addChild(buttonMenu2); */
  if (epic < 2) return true;
  CCSprite* frame = CCSprite::create("GJ_epicCoin_001.png");
   frame->retain();
    frame->setPosition({(win_size.width / 2) - 120, (win_size.height / 2) + 35});
    frame->_setZOrder(-1);
    self->addChild(frame);
     frame->release();
  gjlvl = lvl;
  return true;
}

char * (*LevelTools_getAudioFilename)(int);
char * LevelTools_getAudioFilename_H(int songID) {
  switch (songID) {
    case 0: return "StereoMadness.mp3";
    case 2: return "Polargeist.mp3";
    case 3: return "DryOut.mp3";
    case 4: return "BaseAfterBase.mp3";
    case 5: return "CantLetGo.mp3";
    case 6: return "Jumper.mp3";
    case 7: return "TimeMachine.mp3";
    case 8: return "Cycles.mp3";
    case 9: return "xStep.mp3";
    case 10: return "Clutterfunk.mp3";
    case 11: return "TheoryOfEverything.mp3";
    case 12: return "Electroman.mp3";
    case 13: return "Clubstep.mp3";
    case 14: return "Active.mp3";
    case 15: return "Electrodynamix.mp3";
    case 16: return "HexagonForce.mp3";
    case 17: return "BlastProcessing.mp3";
    case 18: return "TheoryOfEverything2.mp3";
    case 19: return "CosmicDreamer.mp3";
    case 20: return "SkyFortress.ogg";
    case 21: return "SoundOfInfinity.mp3";
    case 22: return "Rupture.mp3";
    case 23: return "Stalemate.mp3";
    case 24: return "GloriousMorning.ogg";
    default: return "BackOnTrack.mp3";
  }
}

/*
const char* (*LevelTools_getAudioString)(int);
const char* LevelTools_getAudioString_H(int ID) {
  switch (ID) {
    case 0: return "0.19;0.80;0.38;0.80;0.75;0.80;1.12;0.80;1.50;0.80;1.88;0.80;2.25;0.80;2.62;0.80;3.00;0.80;3.38;0.80;3.75;0.80;4.12;0.80;4.50;0.80;4.88;0.80;5.25;0.80;5.62;0.80;6.00;0.80;6.38;0.80;6.75;0.80;7.12;0.80;7.50;0.80;7.88;0.80;8.25;0.80;8.62;0.80;9.00;0.80;9.38;0.80;9.75;0.80;10.12;0.80;10.50;0.80;10.88;0.80;11.25;0.80;11.62;0.80;12.00;1.00;12.00;0.90;12.38;0.90;12.75;0.90;13.12;0.90;13.50;0.90;13.88;0.90;14.25;0.90;14.62;0.90;15.00;0.90;15.38;0.90;15.75;0.90;16.12;0.90;16.50;0.90;16.88;0.90;17.25;0.90;17.62;0.90;18.00;0.90;18.38;0.90;18.75;0.90;19.12;0.90;19.50;0.90;19.88;0.90;20.25;0.90;20.62;0.90;21.00;0.90;21.38;0.90;21.75;0.90;22.12;0.90;22.50;0.90;22.88;0.90;23.25;0.90;23.62;0.90;25.50;1.00;25.50;0.90;25.88;0.90;26.25;0.90;26.62;0.90;27.00;1.00;27.00;0.90;27.38;0.90;27.75;0.90;28.12;0.90;28.50;1.00;28.50;0.90;28.88;0.90;29.25;0.90;29.62;0.90;30.00;1.00;30.00;0.90;30.38;0.90;30.75;0.90;31.12;0.90;31.50;1.00;31.50;0.90;31.88;0.90;32.25;0.90;32.62;0.90;33.00;1.00;33.00;0.90;33.38;0.90;33.75;0.90;34.12;0.90;34.50;1.00;34.50;0.90;34.88;0.90;35.25;0.90;35.62;0.90;36.00;1.00;36.00;0.90;36.38;0.90;36.75;0.90;37.12;0.90;37.50;1.00;37.50;0.90;37.88;0.90;38.25;0.90;38.62;0.90;39.00;1.00;39.00;0.90;39.38;0.90;39.75;0.90;40.12;0.90;40.31;1.00;40.50;0.90;40.88;0.90;41.25;0.90;41.62;0.90;42.00;1.00;42.00;0.90;42.38;0.90;42.75;0.90;43.12;0.90;43.50;1.00;43.50;0.90;43.88;0.90;44.25;0.90;44.62;0.90;45.00;1.00;45.00;0.90;45.38;0.90;45.75;0.90;46.12;0.90;46.50;1.00;46.50;0.90;46.88;0.90;47.25;0.90;47.62;0.90;48.00;1.00;48.00;0.90;48.38;0.90;48.75;0.90;49.12;0.90;49.50;0.80;49.88;0.80;50.25;0.80;50.62;0.80;51.00;0.80;51.38;0.80;51.75;0.80;52.12;0.80;52.50;0.80;52.88;0.80;53.25;0.80;53.62;0.80;54.00;0.80;54.38;0.80;54.75;0.80;55.12;0.80;55.50;0.80;55.88;0.80;56.25;0.80;56.62;0.80;57.00;0.80;57.38;0.80;57.75;0.80;58.12;0.80;58.50;0.80;58.88;0.80;59.25;0.80;59.62;0.80;60.00;0.80;60.38;0.80;60.75;0.80;61.12;0.80;61.50;0.80;61.88;0.80;62.25;0.80;62.62;0.80;63.00;0.80;63.38;0.80;63.75;0.80;64.12;0.80;64.50;0.80;64.88;0.80;65.25;0.80;65.62;0.80;66.00;0.80;66.38;0.80;66.75;0.80;67.12;0.80;67.50;0.80;67.88;0.80;68.25;0.80;68.62;0.80;69.00;0.80;69.38;0.80;69.75;0.80;70.12;0.80;70.50;0.80;70.88;0.80;71.25;0.80;71.62;0.80;72.00;0.80;72.38;0.80;72.75;0.80;73.12;0.80;73.50;1.00;73.50;0.80;73.88;0.80;74.06;0.80;74.44;0.80;74.62;0.80;75.00;0.80;75.19;0.80;75.56;0.80;75.75;0.80;76.12;0.80;76.31;0.80;76.50;0.80;76.88;0.80;77.06;0.80;77.44;0.80;77.62;0.80;78.00;0.80;78.19;0.80;78.38;0.80;78.75;0.80;78.94;0.80;79.31;0.80;79.50;0.80;79.88;0.80;80.06;0.80;80.25;0.80;80.62;0.80;80.81;0.80;81.19;0.80;81.38;0.80;81.75;0.80;81.94;0.80;82.31;0.80;82.50;0.80;82.88;0.80;83.06;0.80;83.25;0.80;83.62;0.80;83.81;0.80;84.19;0.80;84.38;0.80;84.75;0.80;84.94;0.80;85.31;0.80;85.50;0.80;85.69;1.00;85.69;0.90;85.88;0.80;85.88;0.90;86.25;0.90";
    case 1: return "0.21;0.80;0.42;0.80;1.27;0.80;1.69;0.80;2.11;0.80;2.54;0.80;2.96;0.80;3.17;0.80;3.38;0.80;3.80;0.80;4.65;0.80;5.07;0.80;5.49;0.80;5.70;0.80;5.92;0.80;6.34;0.80;6.55;0.80;6.76;0.80;7.18;0.80;8.03;0.80;8.45;0.80;8.87;0.80;9.30;0.80;9.72;0.80;9.93;0.80;10.14;0.80;10.56;0.80;11.41;0.80;11.83;0.80;12.25;0.80;12.46;0.80;12.68;0.80;13.10;0.80;13.31;0.80;13.52;0.80;13.52;0.90;13.94;0.90;13.94;1.00;14.58;0.90;14.79;1.00;14.79;0.90;15.21;0.90;15.63;1.00;15.63;0.90;16.06;0.90;16.48;0.90;16.48;1.00;16.69;0.90;16.90;0.90;17.32;1.00;17.32;0.90;17.96;0.90;18.17;1.00;18.17;0.90;18.59;0.90;19.01;1.00;19.01;0.90;19.44;0.90;19.86;0.90;19.86;1.00;20.07;0.90;20.28;0.90;20.70;0.90;20.70;1.00;21.34;0.90;21.55;1.00;21.55;0.90;21.97;0.90;22.39;0.90;22.39;1.00;22.82;0.90;23.24;0.90;23.24;1.00;23.45;0.90;23.66;0.90;24.08;0.90;24.08;1.00;24.72;0.90;24.93;1.00;24.93;0.90;25.35;0.90;25.77;1.00;25.77;0.90;26.20;0.90;26.62;1.00;26.62;0.90;26.83;0.90;27.04;0.90;27.46;0.90;27.46;1.00;28.10;0.90;28.31;0.90;28.31;1.00;28.73;0.90;29.15;1.00;29.15;0.90;29.58;0.90;30.00;1.00;30.00;0.90;30.21;0.90;30.42;0.90;30.85;1.00;30.85;0.90;31.48;0.90;31.69;1.00;31.69;0.90;32.11;0.90;32.54;1.00;32.54;0.90;32.96;0.90;33.38;1.00;33.38;0.90;33.59;0.90;33.80;0.90;34.23;0.90;34.23;1.00;34.86;0.90;35.07;1.00;35.07;0.90;35.49;0.90;35.92;1.00;35.92;0.90;36.34;0.90;36.76;1.00;36.76;0.90;36.97;0.90;37.18;0.90;37.61;1.00;37.61;0.90;38.24;0.90;38.45;1.00;38.45;0.90;38.87;0.90;39.30;1.00;39.30;0.90;39.72;0.90;40.14;1.00;40.14;0.90;40.35;0.90;40.56;0.90;40.99;0.90;41.62;0.80;41.83;0.80;42.04;0.80;42.25;0.80;42.68;0.80;42.89;0.80;43.10;0.80;43.52;0.80;43.73;0.80;43.94;0.80;44.37;0.80;45.00;0.80;45.21;0.80;45.42;0.80;45.63;0.80;46.06;0.80;46.27;0.80;46.48;0.80;46.90;0.80;47.11;0.80;47.32;0.80;47.75;0.80;48.38;0.80;48.59;0.80;48.80;0.80;49.01;0.80;49.44;0.80;49.65;0.80;49.86;0.80;50.28;0.80;50.49;0.80;50.70;0.80;51.13;0.80;51.76;0.80;51.97;0.80;52.18;0.80;52.39;0.80;52.82;0.80;53.03;0.80;53.24;0.80;53.66;0.80;53.87;0.80;54.08;0.90;54.51;1.00;54.51;0.90;55.14;0.90;55.35;0.90;55.35;1.00;55.56;0.90;55.77;0.90;56.20;1.00;56.20;0.90;56.41;0.90;56.62;0.90;57.04;1.00;57.04;0.90;57.25;0.90;57.46;0.90;57.89;1.00;57.89;0.90;58.52;0.90;58.73;1.00;58.73;0.90;58.94;0.90;59.15;0.90;59.58;1.00;59.58;0.90;59.79;0.90;60.00;0.90;60.42;1.00;60.42;0.90;60.63;0.90;60.85;0.90;61.27;0.90;61.27;1.00;61.90;0.90;62.11;1.00;62.11;0.90;62.32;0.90;62.54;0.90;62.96;1.00;62.96;0.90;63.17;0.90;63.38;0.90;63.80;1.00;63.80;0.90;64.01;0.90;64.23;0.90;64.65;0.90;64.65;1.00;65.28;0.90;65.49;0.90;65.49;1.00;65.70;0.90;65.92;0.90;66.34;1.00;66.34;0.90;66.55;0.90;66.76;0.90;67.18;1.00;67.18;0.90;67.39;0.90;67.61;0.90;67.61;0.80;68.03;0.90;68.03;0.80;68.45;0.90;68.45;0.80;68.66;0.80;68.87;0.80;69.08;0.80;69.30;0.80;69.72;0.80;70.14;0.80;70.35;0.80;70.56;0.80;70.77;0.80;70.99;0.80;71.41;0.80;71.83;0.80;72.04;0.80;72.25;0.80;72.46;0.80;72.68;0.80;73.10;0.80;73.52;0.80;74.37;0.80;74.79;0.80;75.21;0.80;75.42;0.80;75.63;1.00;75.63;0.80;75.85;0.80;76.06;0.80;76.48;0.80;76.48;1.00;76.90;0.80;77.11;0.80;77.32;1.00;77.32;0.80;77.54;0.80;77.75;0.80;78.17;1.00;78.17;0.80;78.59;0.80;78.80;0.80;79.01;1.00;79.01;0.80;79.23;0.80;79.44;0.80;79.86;1.00;79.86;0.80;80.70;1.00;81.13;0.90;81.55;1.00;81.55;0.90;81.97;0.90";
    case 2: return "1.66;1.00;2.02;0.80;2.21;0.80;2.58;0.80;3.50;0.80;3.68;0.80;4.05;0.80;4.97;0.80;5.15;0.80;5.52;0.80;6.44;0.80;6.63;0.80;6.99;0.80;7.36;1.00;7.91;0.80;8.10;0.80;8.47;0.80;9.39;0.80;9.57;0.80;9.94;0.80;10.86;0.80;11.04;0.80;11.41;0.80;12.33;0.80;12.52;0.80;12.88;0.80;13.80;0.80;13.99;0.80;14.17;0.80;15.28;0.80;15.46;0.80;15.83;0.80;16.75;0.80;16.93;0.80;17.30;0.80;18.22;0.80;18.40;0.80;18.77;0.80;19.14;1.00;19.14;0.90;19.69;0.90;19.88;0.90;20.25;0.90;20.43;0.90;20.80;0.90;21.17;0.90;21.35;0.90;21.72;0.90;22.09;0.90;22.64;0.90;22.82;0.90;23.19;0.90;23.37;0.90;23.74;0.90;24.11;0.90;24.29;0.90;24.66;0.90;25.03;0.90;25.58;0.90;25.77;0.90;26.13;0.90;26.32;0.90;26.69;0.90;27.06;0.90;27.24;0.90;27.61;0.90;27.98;0.90;28.16;0.90;28.53;0.90;28.71;0.90;29.08;0.90;29.26;0.90;29.63;0.90;29.82;0.90;30.18;0.90;30.55;0.90;31.10;0.80;31.29;0.80;31.47;0.80;31.84;0.80;32.02;0.80;32.39;0.80;32.76;0.80;33.13;0.80;33.50;0.80;34.42;0.80;34.79;0.80;35.15;0.80;35.52;0.80;35.89;0.80;36.07;0.80;36.44;0.80;36.81;0.80;37.36;0.80;37.73;0.80;38.10;0.80;38.28;0.80;38.65;0.80;39.02;0.80;39.39;0.80;40.31;0.80;40.67;0.80;41.04;0.80;41.41;0.80;41.78;0.80;41.96;0.80;42.33;0.80;43.07;1.00;43.07;0.90;43.44;0.90;43.62;0.90;43.99;0.90;44.36;0.90;44.72;0.90;45.09;0.90;45.46;0.90;45.83;1.00;45.83;0.80;46.01;0.80;46.20;0.80;46.38;0.80;46.56;0.80;46.75;0.80;46.93;0.80;47.12;0.80;47.30;0.80;47.48;0.80;48.96;0.90;49.33;0.90;49.51;0.90;49.88;0.90;50.25;0.90;50.61;0.90;50.98;0.90;51.35;0.90;51.72;0.80;51.90;0.80;52.09;0.80;52.27;0.80;52.45;0.80;52.64;0.80;52.82;0.80;53.01;0.80;53.19;0.80;53.37;0.80;54.85;0.90;55.21;0.90;57.79;0.90;58.16;0.90;58.34;0.80;58.53;0.80;58.71;0.80;58.90;0.80;59.08;0.80;59.26;0.80;60.74;0.90;61.10;0.90;61.29;0.90;61.66;0.90;62.02;0.90;62.39;0.90;62.76;0.90;63.13;0.90;63.50;0.80;63.68;0.80;63.87;0.80;64.05;0.80;64.23;0.80;64.42;0.80;64.60;0.80;64.79;0.80;64.97;0.80;65.15;0.80;66.26;1.00;68.28;0.80;68.47;0.80;68.83;0.80;69.75;0.80;69.94;0.80;70.31;0.80;71.23;0.80;71.41;0.80;71.78;0.80;72.70;0.80;72.88;0.80;73.25;0.80;74.17;0.80;74.36;0.80;74.72;0.80;75.64;0.80;75.83;0.80;76.20;0.80;76.38;0.80;76.75;0.80;77.12;0.80;77.30;0.80;77.67;0.80;78.04;1.00;78.04;0.90;78.59;0.90;78.77;0.90;79.14;0.90;79.33;0.90;79.69;0.90;80.06;0.90;80.25;0.90;80.61;0.90;80.98;0.90;81.53;0.90;81.72;0.90;82.09;0.90;82.27;0.90;82.64;0.90;83.01;0.90;83.19;0.90;83.56;0.90;84.11;0.90;84.48;0.90;84.66;0.90;85.03;0.90;85.21;0.90;85.58;0.90;85.95;0.90;86.13;0.90;86.50;0.90;86.87;0.90;87.06;0.90;87.42;0.90;87.61;0.90;87.98;0.90;88.16;0.90;88.53;0.90;88.71;0.90;89.08;0.90;89.45;0.90;89.82;1.00";
    case 3: return "0.21;0.80;0.41;0.80;0.52;0.80;0.83;0.80;1.14;0.80;1.34;0.80;1.55;0.80;1.76;0.80;2.07;0.80;2.38;0.80;2.90;0.80;3.10;0.80;3.31;0.80;3.52;0.80;3.62;0.80;3.93;0.80;4.03;0.80;4.55;0.80;4.76;0.80;4.97;0.80;5.17;0.80;5.38;0.80;5.69;0.80;6.10;0.80;6.31;0.80;6.52;0.80;6.83;0.80;7.03;0.80;7.14;0.80;7.45;0.80;7.86;0.80;8.07;0.80;8.28;0.80;8.48;0.80;8.79;0.80;9.10;0.80;9.52;0.80;9.72;0.80;9.93;0.80;10.03;0.80;10.34;0.80;10.55;0.80;10.76;0.80;11.07;0.80;11.28;0.80;11.59;0.80;11.79;0.80;12.00;0.80;12.31;0.80;12.72;0.80;12.93;0.80;13.24;0.80;13.24;1.00;13.24;0.90;13.45;0.80;13.66;0.90;13.66;0.80;13.86;0.80;14.07;0.80;14.07;0.90;14.38;0.80;14.48;0.90;14.69;0.80;14.90;0.80;14.90;0.90;15.00;0.80;15.31;0.90;15.41;0.80;15.72;0.90;15.72;0.80;16.14;0.90;16.14;0.80;16.34;0.80;16.55;0.90;16.55;0.80;16.76;0.80;16.97;0.80;16.97;0.90;17.07;0.80;17.38;0.80;17.38;0.90;17.79;0.80;17.79;0.90;18.00;0.80;18.10;0.90;18.21;0.80;18.41;0.80;18.62;0.90;18.72;0.80;18.93;0.90;19.03;0.80;19.34;0.90;19.45;0.80;19.66;0.80;19.76;0.90;19.86;0.80;20.07;0.80;20.17;0.90;20.28;0.80;20.48;0.80;20.59;0.90;20.69;0.80;21.00;0.90;21.00;0.80;21.31;0.80;21.52;0.90;21.52;0.80;21.72;0.80;21.83;0.90;22.03;0.80;22.24;0.90;22.24;0.80;22.66;0.90;22.76;0.80;22.97;0.80;23.07;0.90;23.17;0.80;23.28;0.80;23.48;0.90;23.48;0.80;23.69;0.80;24.00;0.90;24.00;0.80;24.31;0.90;24.31;0.80;24.52;0.80;24.72;0.80;24.83;0.90;25.03;0.80;25.14;0.90;25.34;0.80;25.55;0.90;25.55;0.80;25.97;0.90;26.07;0.80;26.17;0.80;26.38;0.90;26.38;0.80;26.69;0.80;28.14;0.90;28.14;1.00;28.24;0.80;28.55;0.90;28.86;0.90;29.17;0.90;29.59;0.90;29.79;0.90;29.79;1.00;30.21;0.90;30.52;0.90;30.83;0.90;31.24;0.90;31.45;1.00;31.45;0.90;31.86;0.90;32.28;0.90;32.48;0.90;32.90;0.90;33.10;1.00;33.10;0.90;33.52;0.90;33.93;0.90;34.34;0.90;34.55;0.90;34.66;0.90;34.76;1.00;35.17;0.90;35.48;0.90;35.79;0.90;36.21;0.90;36.31;1.00;36.41;0.90;36.83;0.90;37.24;0.90;37.55;0.90;37.86;0.90;37.97;1.00;38.07;0.90;38.48;0.90;38.90;0.90;39.10;0.90;39.52;0.90;39.72;0.90;39.72;1.00;40.03;0.90;40.55;0.90;40.97;0.90;41.17;0.90;41.38;1.00;41.38;0.90;41.79;0.80;41.79;0.90;42.00;0.80;42.10;0.80;42.21;0.90;42.41;0.90;42.83;0.90;43.03;1.00;43.03;0.90;43.24;0.80;43.45;0.90;43.55;0.80;43.76;0.80;43.76;0.90;44.07;0.90;44.48;0.90;44.69;0.90;44.69;1.00;44.79;0.80;45.00;0.80;45.10;0.90;45.31;0.80;45.52;0.80;45.52;0.90;45.72;0.90;46.14;0.90;46.34;1.00;46.34;0.90;46.55;0.80;46.76;0.90;46.76;0.80;47.07;0.80;47.17;0.90;47.59;0.90;47.79;0.90;48.00;1.00;48.00;0.90;48.10;0.80;48.41;0.80;48.41;0.90;48.62;0.80;48.72;0.80;48.83;0.90;49.03;0.90;49.45;0.90;49.66;1.00;49.66;0.90;49.76;0.80;50.07;0.90;50.17;0.80;50.38;0.80;50.48;0.90;50.69;0.90;51.10;0.90;51.31;1.00;51.31;0.90;51.41;0.80;51.62;0.80;51.72;0.90;51.93;0.80;52.03;0.80;52.14;0.90;52.34;0.90;52.76;0.90;52.97;0.90;52.97;1.00;53.07;0.80;53.38;0.80;53.38;0.90;53.69;0.80;53.79;0.90;54.21;0.90;54.41;0.90;54.62;1.00;54.62;0.90;55.34;0.90;55.55;0.90;55.86;0.90;56.07;0.90;56.28;0.90;56.79;0.90;57.31;0.90;57.52;0.90;57.72;0.90;57.93;0.90;58.55;0.90;58.76;0.90;58.97;0.90;59.17;0.90;59.38;0.90;59.59;0.90;60.10;0.90;60.31;0.90;60.83;0.90;61.03;0.90;61.24;0.90;61.86;0.90;62.07;0.90;62.28;0.90;62.48;0.90;62.69;0.90;62.90;0.90;63.41;0.90;63.93;0.90;64.03;0.90;64.34;0.90;64.55;0.90;65.17;0.90;65.38;0.90;65.59;0.90;65.79;0.90;66.00;0.90;66.21;0.90;66.83;0.90;66.93;0.90;67.45;0.90;67.66;0.90;67.86;0.90;68.07;0.90;68.48;0.80;68.79;0.80;69.00;0.80;69.21;0.80;69.52;0.80;69.72;0.80;70.24;0.80;70.66;0.80;70.97;0.80;71.07;0.80;71.28;0.80;72.00;0.80;72.21;0.80;72.41;0.80;72.52;0.80;72.83;0.80;72.93;0.80;73.55;0.80;73.86;0.80;74.17;0.80;74.48;0.80;74.69;0.80;75.21;0.80;75.41;0.80;75.62;0.80;75.93;0.80;76.03;0.80;76.34;0.80;76.86;0.80;77.28;0.80;77.59;0.80;77.79;0.80;78.00;0.80;78.52;0.80;78.72;0.80;78.93;0.80;79.24;0.80;79.45;0.80;79.66;0.80;80.17;0.80;80.38;0.80;80.79;0.80;81.00;0.80;81.10;1.00;81.10;0.90;81.21;0.80;81.83;0.80;81.93;0.90;82.03;0.80;82.14;0.90;82.34;0.80;82.34;0.90;82.55;0.80;82.55;0.90;82.76;0.90;82.76;0.80;82.97;0.80;83.28;0.90;83.48;0.80;83.79;0.90;83.90;0.80;84.00;0.90;84.10;0.80;84.21;0.90;84.31;0.80;84.41;0.90;84.62;0.80";
    case 4: return "0.32;0.80;0.53;0.80;0.85;0.80;1.06;0.80;1.27;0.80;1.48;0.80;1.69;0.80;2.43;0.80;2.54;0.80;2.85;0.80;3.06;0.80;3.17;0.80;3.38;0.80;3.91;0.80;4.01;0.80;4.23;0.80;4.44;0.80;4.65;0.80;4.86;0.80;4.96;0.80;5.28;0.80;5.39;0.80;5.49;0.80;5.81;0.80;6.02;0.80;6.13;0.80;6.44;0.80;6.65;0.80;6.76;0.80;6.87;0.90;7.61;0.80;7.82;0.80;8.03;0.80;8.35;0.80;8.45;0.80;9.08;0.80;9.40;0.80;9.61;0.80;9.82;0.80;10.04;0.80;10.25;0.80;10.67;0.80;10.88;0.80;11.09;0.80;11.30;0.80;11.51;0.80;11.73;0.80;11.83;0.80;12.15;0.80;12.25;0.80;12.36;0.80;12.57;0.80;12.78;0.80;12.89;0.80;12.89;0.80;13.20;0.80;13.52;0.80;13.63;1.00;13.63;0.90;13.73;0.80;14.15;0.80;14.47;0.80;14.89;0.80;15.11;0.80;15.32;0.80;15.32;0.90;15.53;0.80;15.74;0.80;15.95;0.80;16.16;0.80;16.37;0.80;16.58;0.80;16.80;0.80;17.01;0.80;17.01;0.90;17.43;0.80;17.64;0.80;17.85;0.80;18.06;0.80;18.27;0.80;18.49;0.80;18.70;0.80;18.70;0.90;18.91;0.80;19.01;0.80;19.12;0.80;19.33;0.80;19.65;0.80;19.65;0.80;19.75;0.80;19.96;0.80;20.18;0.80;20.39;0.80;20.49;0.90;20.92;0.80;21.13;0.80;21.34;0.80;21.55;0.80;21.65;0.80;21.87;0.80;22.08;0.90;22.08;0.80;22.61;0.80;22.82;0.80;23.03;0.80;23.24;0.80;23.45;0.80;23.66;0.80;23.77;0.90;23.87;0.80;24.30;0.80;24.51;0.80;24.72;0.80;24.82;0.80;25.14;0.80;25.35;0.80;25.46;0.90;25.46;0.80;25.77;0.80;25.88;0.80;26.20;0.80;26.30;0.90;26.41;0.80;26.51;0.80;26.62;0.80;26.73;0.80;26.83;0.80;26.94;0.80;27.15;0.80;27.25;0.80;27.25;0.90;27.36;0.80;27.46;0.80;27.46;0.80;27.57;0.80;27.68;0.80;27.78;0.80;27.78;0.80;27.99;0.80;27.99;0.80;28.10;0.80;28.20;0.80;28.31;0.80;28.42;0.80;28.52;0.80;28.63;0.80;28.94;0.80;28.94;1.00;30.53;0.90;30.63;1.00;30.63;0.80;31.06;0.90;31.27;0.90;31.48;0.90;31.69;0.90;31.80;0.90;32.01;0.90;32.22;0.90;32.32;0.90;32.54;0.90;32.64;0.90;32.75;0.90;33.06;0.90;33.27;0.90;33.49;0.90;33.59;0.90;33.80;0.90;34.12;0.90;34.54;0.90;34.75;0.90;34.96;0.90;35.18;0.90;35.28;0.90;35.60;0.90;35.81;0.90;35.92;0.90;36.02;0.90;36.13;0.90;36.44;0.90;36.65;0.90;36.76;0.90;36.87;0.90;37.08;0.90;37.29;0.90;37.50;0.90;37.92;0.90;38.35;0.90;38.77;0.90;39.19;0.90;39.30;0.90;39.40;0.90;39.51;0.90;39.72;0.90;39.82;0.90;40.04;0.90;40.25;0.90;40.35;0.90;40.46;0.90;40.67;0.90;40.88;0.90;41.30;0.90;41.73;0.90;42.15;0.90;42.57;0.90;42.99;0.90;43.42;0.90;43.84;0.90;44.26;0.90;44.26;1.00;44.68;0.90;44.89;0.90;45.11;0.90;45.21;0.90;45.53;0.90;45.74;0.90;45.95;0.90;46.16;0.90;46.27;0.90;46.37;0.90;46.58;0.90;46.90;0.90;47.01;0.90;47.22;0.90;47.43;0.90;47.64;0.90;48.06;0.90;48.38;0.90;48.49;0.90;48.70;0.90;49.01;0.90;49.23;0.90;49.33;0.90;49.54;0.90;49.65;0.90;49.75;0.90;50.07;0.90;50.28;0.90;50.39;0.90;50.39;0.90;50.70;0.90;50.92;0.90;51.13;0.90;51.55;0.90;51.76;0.90;51.97;0.90;52.18;0.90;52.39;0.90;52.50;0.90;52.82;0.90;53.03;0.90;53.45;0.90;53.66;0.90;53.87;0.90;54.08;0.90;54.30;0.90;54.51;0.90;54.93;0.90;55.14;0.90;55.56;0.90;55.99;0.90;56.41;0.90;56.62;0.90;57.04;0.90;57.25;0.90;57.78;0.90;57.89;1.00;58.10;0.80;58.31;1.00;58.31;0.80;58.42;0.90;58.52;0.90;58.73;1.00;58.73;0.90;58.94;0.90;59.15;1.00;59.15;0.90;59.37;0.90;59.58;1.00;59.58;0.90;60.00;1.00;60.21;0.90;60.42;1.00;60.74;0.90;60.85;1.00;61.06;0.90;61.27;1.00;61.69;1.00;61.80;0.90;61.90;0.90;62.22;1.00;62.22;0.90;62.43;0.90;62.54;1.00;62.54;0.90;62.85;0.90;62.96;1.00;63.06;0.90;63.17;0.90;63.38;0.90;63.38;1.00;63.80;1.00;63.91;0.90;64.12;0.90;64.23;1.00;64.54;0.90;64.65;1.00;65.07;1.00;65.18;0.90;65.39;0.90;65.49;1.00;65.81;0.90;66.02;1.00;66.23;0.90;66.34;1.00;66.65;0.90;66.76;1.00;66.87;0.90;67.18;1.00;67.29;0.90;67.61;0.90;67.71;1.00;67.92;0.90;68.03;1.00;68.56;1.00;68.56;0.90;68.98;1.00;68.98;0.90;69.40;1.00;69.40;0.90;69.82;1.00;69.82;0.90;70.14;0.90;70.25;1.00;70.46;0.90;70.67;1.00;70.77;0.90;71.09;1.00;71.09;0.90;71.51;0.90;71.62;0.80;71.83;0.80;71.94;0.90;72.15;0.80;72.15;0.90;72.57;0.80;72.68;0.80;72.99;0.80;72.99;0.90;73.10;0.80;73.20;0.90;73.52;0.80;73.63;0.90;73.84;0.80;73.84;0.90;74.15;0.80;74.26;0.90;74.47;0.80;74.47;0.90;74.58;0.80;74.68;0.90;74.79;0.80;74.89;0.90;75.32;0.90;75.74;0.90;75.85;0.80;76.16;0.80;76.16;0.90;76.27;0.80;76.48;0.80;76.58;0.90;77.01;0.90;77.43;0.90;77.54;0.80;77.75;0.80;77.85;0.90;78.27;0.80;78.27;0.90;78.70;0.80;78.70;0.90;78.91;0.80;79.12;0.90;79.33;0.80;79.44;0.80;79.75;0.80;79.96;0.80;79.96;0.90;80.39;0.80;80.60;0.80;80.92;0.90;81.02;0.80;81.23;0.80;81.44;0.80;81.65;0.80;82.18;0.90;82.61;0.90;82.71;0.80;82.92;0.80;82.92;0.90;83.03;0.80;83.35;0.80";
    case 5: return "0.35;0.80;0.53;0.80;0.88;0.80;1.24;0.80;1.59;0.80;1.76;0.80;2.12;0.80;2.47;0.80;2.65;0.80;2.82;0.80;3.00;0.80;3.35;0.80;3.71;0.80;4.06;0.80;4.24;0.80;4.59;0.80;4.94;0.80;5.29;0.80;5.47;0.80;5.65;0.80;5.82;0.80;6.18;0.80;6.53;0.80;6.88;0.80;7.24;0.80;7.41;0.80;7.76;0.80;8.12;0.80;8.29;0.80;8.47;0.80;8.65;0.80;9.00;0.80;9.35;0.80;9.71;0.80;9.88;0.80;10.24;0.80;10.59;0.80;10.94;0.80;11.12;0.80;11.29;0.80;11.29;0.90;11.47;0.80;11.65;0.90;11.82;0.80;12.00;0.90;12.18;0.80;12.35;0.90;12.53;0.80;12.71;0.90;12.88;0.80;13.06;0.80;13.06;0.90;13.41;0.90;13.41;0.80;13.76;0.80;13.76;0.90;13.94;0.80;14.12;0.90;14.12;0.80;14.29;0.80;14.47;0.90;14.65;0.80;14.82;0.90;15.00;0.80;15.18;0.90;15.35;0.80;15.53;0.80;15.53;0.90;15.88;0.80;15.88;0.90;16.24;0.80;16.24;0.90;16.59;0.80;16.59;0.90;16.76;0.80;16.94;0.80;16.94;0.90;17.12;0.80;17.29;0.90;17.47;0.80;17.65;0.90;17.82;0.80;18.00;0.90;18.18;0.80;18.35;0.90;18.53;0.80;18.71;0.80;18.71;0.90;19.06;0.80;19.06;0.90;19.41;0.80;19.41;0.90;19.59;0.80;19.76;0.80;19.76;0.90;19.94;0.80;20.12;0.90;20.29;0.80;20.47;0.90;20.65;0.80;20.82;0.90;21.00;0.80;21.18;0.80;21.18;0.90;21.53;0.80;21.53;0.90;21.88;0.80;21.88;0.90;22.24;0.90;22.24;0.80;22.41;0.80;22.59;0.90;22.59;0.80;22.76;0.80;24.00;0.90;24.00;1.00;24.18;0.80;24.35;1.00;24.35;0.90;24.71;1.00;24.71;0.90;25.06;1.00;25.06;0.90;25.41;1.00;25.41;0.90;25.76;0.90;25.94;1.00;26.12;0.90;26.47;1.00;26.47;0.90;26.82;0.90;26.82;1.00;27.18;1.00;27.18;0.90;27.53;1.00;27.53;0.90;27.88;1.00;27.88;0.90;28.24;1.00;28.24;0.90;28.59;0.90;28.76;1.00;28.94;0.90;29.29;1.00;29.29;0.90;29.65;0.90;29.65;1.00;30.00;1.00;30.00;0.90;30.35;1.00;30.35;0.90;30.71;1.00;30.71;0.90;31.06;1.00;31.06;0.90;31.41;0.90;31.59;1.00;31.76;0.90;32.12;1.00;32.12;0.90;32.47;0.90;32.47;1.00;32.82;1.00;32.82;0.90;33.18;1.00;33.18;0.90;33.53;1.00;33.53;0.90;33.88;1.00;33.88;0.90;34.24;0.90;34.41;1.00;34.59;0.90;34.94;1.00;34.94;0.90;35.29;0.90;35.29;1.00;35.65;1.00;35.65;0.90;36.00;1.00;36.00;0.90;36.35;1.00;36.35;0.90;36.71;1.00;36.71;0.90;37.06;0.90;37.24;1.00;37.41;0.90;37.76;1.00;37.76;0.90;38.12;0.90;38.12;1.00;38.47;0.90;38.47;1.00;38.82;1.00;38.82;0.90;39.18;1.00;39.18;0.90;39.53;1.00;39.53;0.90;39.88;0.90;40.06;1.00;40.24;0.90;40.59;1.00;40.59;0.90;40.94;0.90;40.94;1.00;41.29;1.00;41.29;0.90;41.65;1.00;41.65;0.90;42.00;1.00;42.00;0.90;42.35;1.00;42.35;0.90;42.71;0.90;42.88;1.00;43.06;0.90;43.41;1.00;43.41;0.90;43.76;0.90;43.76;1.00;44.12;1.00;44.12;0.90;44.47;1.00;44.47;0.90;44.82;1.00;44.82;0.90;45.18;1.00;45.18;0.90;45.53;0.90;45.71;1.00;45.88;0.90;46.24;1.00;46.24;0.90;46.59;0.90;46.59;1.00;46.94;1.00;46.94;0.90;47.29;1.00;47.29;0.90;47.65;1.00;47.65;0.90;48.00;1.00;48.00;0.90;48.35;0.90;48.53;1.00;48.71;0.90;49.06;1.00;49.06;0.90;49.41;0.90;49.41;1.00;49.76;1.00;49.76;0.90;50.12;1.00;50.12;0.90;50.47;1.00;50.47;0.90;50.82;1.00;50.82;0.90;51.18;0.90;51.35;1.00;51.53;0.90;51.88;1.00;51.88;0.90;52.24;0.90;52.24;1.00;52.59;0.90;52.59;1.00;52.94;1.00;52.94;0.90;53.29;1.00;53.29;0.90;53.65;1.00;53.65;0.90;54.00;0.90;54.18;1.00;54.35;0.90;54.71;1.00;54.71;0.90;55.06;1.00;55.06;0.90;55.41;1.00;55.41;0.90;55.76;1.00;55.76;0.90;56.12;1.00;56.12;0.90;56.47;1.00;56.47;0.90;56.82;0.90;57.00;1.00;57.18;0.90;57.53;1.00;57.53;0.90;57.88;1.00;57.88;0.90;58.24;1.00;58.24;0.90;58.59;1.00;58.59;0.90;58.94;1.00;58.94;0.90;59.29;1.00;59.29;0.90;59.65;0.90;59.82;1.00;60.00;0.90;60.35;1.00;60.35;0.90;60.71;1.00;60.71;0.90;61.06;1.00;61.06;0.90;61.41;1.00;61.41;0.90;61.76;1.00;61.76;0.90;62.12;1.00;62.12;0.90;62.47;0.90;62.65;1.00;62.82;0.90;63.18;1.00;63.18;0.90;63.53;1.00;63.53;0.90;63.88;0.90;63.88;1.00;64.24;0.90;64.24;1.00;64.59;1.00;64.59;0.90;64.94;1.00;64.94;0.90;65.29;0.90;65.47;1.00;65.65;0.90;66.00;1.00;66.00;0.90;66.35;0.90;66.35;1.00;66.71;1.00;66.71;0.90;67.06;1.00;67.06;0.90;67.41;1.00;67.41;0.90;67.76;1.00;67.76;0.90;68.12;0.80;68.12;0.90;68.29;1.00;68.47;0.90;68.47;0.80;68.65;1.00;68.82;0.80;69.00;0.80;69.18;0.80;69.35;0.80;69.71;0.80;70.06;0.80;70.41;0.80;70.76;0.80;70.94;0.80;71.29;0.80;71.65;0.80;71.82;0.80;72.00;0.80;72.18;0.80;72.53;0.80;72.88;0.80;73.24;0.80;73.41;0.80;73.76;0.80;74.12;0.80;74.29;0.80;74.65;0.80;74.82;0.80;75.00;0.80;75.35;0.80;75.53;0.80;75.71;0.80;75.88;0.80;76.41;0.80;76.59;0.80;76.94;0.80;77.29;0.80;77.47;0.80;77.65;0.80;77.82;0.80;78.18;0.80;78.35;0.80;78.53;0.80;78.88;0.80;79.06;0.80;79.24;0.80;79.41;0.80;79.76;0.80;80.12;0.80;80.29;0.80;80.47;0.80;80.65;0.80";
    case 6: return "0.00;0.90;0.34;0.90;0.67;0.90;1.01;0.90;1.35;0.90;1.69;0.90;2.02;0.90;2.36;0.90;2.70;0.90;3.03;0.90;3.37;0.90;3.71;0.90;4.04;0.90;4.38;0.90;4.72;0.90;5.39;0.90;5.73;0.90;6.07;0.90;6.40;0.90;6.74;0.90;7.08;0.90;7.42;0.90;7.75;0.90;8.09;0.90;8.43;0.90;8.76;0.90;9.10;0.90;9.44;0.90;9.78;0.90;10.11;0.90;10.45;0.90;10.79;1.00;10.79;0.90;11.12;0.90;11.46;0.90;11.80;0.90;12.13;0.90;12.47;0.90;12.81;0.90;13.15;0.90;13.48;0.90;13.82;0.90;14.16;0.90;14.49;0.90;14.83;0.90;15.17;0.90;15.51;0.90;15.84;0.90;16.18;0.90;16.52;0.90;16.85;0.90;17.19;0.90;17.53;0.90;17.87;0.90;18.20;0.90;18.54;0.90;18.88;0.90;19.21;0.90;19.55;0.90;19.89;0.90;20.22;0.90;20.56;0.90;20.90;0.90;21.24;0.90;21.57;1.00;21.57;0.90;21.91;0.90;22.25;0.90;22.58;0.90;22.92;0.90;23.26;0.90;23.60;0.90;23.93;0.90;24.27;0.90;24.61;0.90;24.94;0.90;25.28;0.90;25.62;0.90;25.96;0.90;26.29;0.90;26.63;0.90;26.97;0.90;27.30;0.90;27.64;0.90;27.98;0.90;28.31;0.90;28.65;0.90;28.99;0.90;29.33;0.90;29.66;0.90;30.00;0.90;30.34;0.90;30.67;0.90;31.01;0.90;31.35;0.90;31.69;0.90;32.02;0.90;32.36;1.00;32.36;0.90;32.70;0.90;33.03;0.90;33.71;0.90;34.04;0.90;34.38;0.90;34.72;0.90;35.06;0.90;35.39;0.90;35.73;0.90;36.40;0.90;36.74;0.90;37.08;0.90;37.42;0.90;37.75;0.90;38.09;0.90;38.43;0.90;39.10;0.90;39.44;0.90;39.78;0.90;40.11;0.90;40.45;0.90;40.79;0.90;41.12;0.90;41.80;0.90;42.13;0.90;42.47;0.90;42.81;0.90;43.15;1.00;43.15;0.90;43.48;0.90;43.82;0.90;44.49;0.90;44.83;0.90;45.17;0.90;45.51;0.90;45.84;0.90;46.18;0.90;46.52;0.90;47.19;0.90;47.53;0.90;47.87;0.90;48.20;0.90;48.54;0.90;48.88;0.90;49.21;0.90;49.89;0.90;50.22;0.90;50.56;0.90;50.90;0.90;51.24;0.90;51.57;0.90;51.91;0.90;52.58;0.90;52.92;0.90;53.26;0.90;53.60;0.90;53.93;1.00;53.93;0.90;54.27;0.90;54.61;0.90;54.94;0.90;55.28;0.90;55.62;0.90;55.96;0.90;56.29;0.90;56.63;0.90;56.97;0.90;57.30;0.90;57.64;0.90;57.98;0.90;58.31;0.90;58.65;0.90;59.33;0.90;59.66;0.90;60.00;0.90;60.34;0.90;60.67;0.90;61.01;0.90;61.35;0.90;61.69;0.90;62.02;0.90;62.36;0.90;62.70;0.90;63.03;0.90;63.37;0.90;63.37;0.90;63.71;0.90;64.04;0.90;64.38;0.90;64.72;1.00;64.72;0.90;65.39;0.90;65.73;0.90;66.07;0.90;66.40;0.90;66.74;0.90;67.08;0.90;67.42;0.90;68.09;0.90;68.43;0.90;68.76;0.90;69.10;0.90;69.44;0.90;69.78;0.90;70.11;0.90;70.79;0.90;71.12;0.90;71.46;0.90;71.80;0.90;72.13;0.90;72.47;0.90;72.81;0.90;73.48;0.90;73.82;0.90;74.16;0.90;74.49;0.90;74.83;0.90;75.17;0.90;75.51;1.00;75.51;0.90;75.84;0.90;76.18;0.90;76.52;0.90;76.85;0.90;77.19;0.90;77.53;0.90;77.87;0.90;78.20;0.90;78.54;0.90;78.88;0.90;79.21;0.90;79.55;0.90;79.89;0.90;80.22;0.90;80.56;0.90;80.90;0.90;81.24;0.90;81.57;0.90;81.91;0.90;82.25;0.90;82.58;0.90;82.92;0.90;83.26;0.90;83.60;0.90;83.93;0.90;84.27;0.90;84.61;0.90;84.94;0.90;85.28;0.90;85.62;0.90;85.96;0.90;86.29;1.00;86.29;0.90;86.63;0.90;86.97;0.90;87.30;0.90;87.64;0.90";
    case 7: return "1.78;0.90;2.62;0.90;3.41;0.90;3.72;0.90;4.04;0.90;4.25;0.90;5.14;0.90;5.93;0.90;6.77;0.90;7.08;0.90;7.40;0.90;7.60;0.90;8.50;0.90;9.28;0.90;10.12;0.90;10.44;0.90;10.75;0.90;11.01;0.90;11.38;0.90;11.80;0.90;12.64;0.90;13.48;0.90;13.90;0.90;14.32;0.90;15.10;1.00;15.21;0.90;16.00;0.90;16.84;0.90;17.15;0.90;17.47;0.90;17.67;0.90;18.62;0.90;19.41;0.90;20.24;0.90;20.56;0.90;20.87;0.90;21.08;0.90;21.92;0.90;22.71;0.90;23.55;0.90;23.92;0.90;24.23;0.90;24.44;0.90;24.81;0.90;25.23;0.90;26.01;0.90;26.91;0.90;27.33;0.90;27.74;0.90;28.53;1.00;28.64;0.90;29.06;0.90;29.32;1.00;29.48;0.90;29.90;0.90;30.16;1.00;30.31;0.90;30.42;1.00;30.79;1.00;30.79;0.90;31.05;1.00;31.15;0.90;31.63;0.90;31.89;1.00;31.99;0.90;32.41;0.90;32.67;1.00;32.83;0.90;33.30;0.90;33.51;1.00;33.72;0.90;33.83;1.00;34.14;1.00;34.14;0.90;34.35;1.00;34.51;0.90;34.93;0.90;35.24;1.00;35.35;0.90;35.77;0.90;36.03;1.00;36.19;0.90;36.66;0.90;36.87;1.00;37.03;0.90;37.19;1.00;37.50;0.90;37.50;1.00;37.76;1.00;37.87;0.90;38.13;1.00;38.29;0.90;38.65;1.00;38.71;0.90;39.13;0.90;39.39;1.00;39.55;0.90;39.97;0.90;40.23;1.00;40.44;0.90;40.65;1.00;40.80;0.90;41.07;1.00;41.28;0.90;41.70;0.90;41.91;1.00;42.12;0.90;42.53;0.90;42.74;1.00;42.90;0.90;43.32;0.90;43.58;1.00;43.79;0.90;43.85;1.00;44.16;1.00;44.16;0.90;44.42;1.00;44.58;0.90;45.00;0.90;45.31;1.00;45.47;0.90;45.89;0.90;46.10;1.00;46.31;0.90;46.73;0.90;46.94;1.00;47.15;0.90;47.26;1.00;47.57;1.00;47.57;0.90;47.83;1.00;47.99;0.90;48.36;0.90;48.62;1.00;48.78;0.90;49.20;0.90;49.46;1.00;49.67;0.90;50.09;0.90;50.35;1.00;50.51;0.90;50.66;1.00;50.93;1.00;50.93;0.90;51.19;1.00;51.35;0.90;51.56;1.00;51.71;0.90;51.98;1.00;52.13;0.90;52.60;0.90;52.76;1.00;52.97;0.90;53.39;0.90;53.65;1.00;53.81;0.90;54.07;1.00;54.28;0.90;54.49;1.00;54.65;0.90;55.38;1.00;55.49;0.90;55.91;0.90;56.38;0.90;56.75;0.90;57.17;0.90;57.59;0.90;58.01;0.90;58.43;0.90;58.85;0.90;59.27;0.90;59.63;0.90;60.10;0.90;60.47;0.90;60.89;0.90;61.31;0.90;62.20;0.90;62.62;0.90;63.04;0.90;63.46;0.90;63.83;0.90;64.30;0.90;64.72;0.90;65.14;0.90;65.56;0.90;65.93;0.90;66.40;0.90;66.77;0.90;67.19;0.90;67.60;0.90;68.02;0.90;68.81;1.00;68.92;0.90;69.34;0.90;69.76;0.90;70.17;0.90;70.59;0.90;71.01;0.90;71.43;0.90;71.85;0.90;72.27;0.90;72.69;0.90;73.11;0.90;73.58;0.90;74.00;0.90;74.37;0.90;74.84;0.90;75.63;0.90;76.05;0.90;76.52;0.90;76.89;0.90;77.31;0.90;77.73;0.90;78.20;0.90;78.62;0.90;78.99;0.90;79.41;0.90;79.83;0.90;80.24;0.90;80.66;0.90;81.14;0.90;81.50;0.90;81.92;0.90;82.19;1.00;82.40;0.90;82.60;0.90;82.97;0.90;83.02;1.00;83.18;0.90;83.39;0.90;83.86;0.90;83.86;1.00;84.02;0.90;84.23;0.90;84.65;0.90;84.76;1.00;84.86;0.90;85.07;0.90;85.28;0.90;85.59;1.00;85.59;0.90;85.80;0.90;85.96;0.90;86.38;1.00;86.38;0.90;86.59;0.90;86.75;0.90;87.17;0.90;87.27;1.00;87.38;0.90;87.59;0.90;88.01;0.90;88.06;1.00;88.22;0.90;88.43;0.90;88.69;0.90;88.85;0.90;88.90;1.00;89.06;0.90;89.32;0.90;89.74;0.90;89.74;1.00;89.90;0.90;90.10;0.90;90.52;0.90;90.58;1.00;90.73;0.90;90.94;0.90;91.36;0.90;91.42;1.00;91.57;0.90;91.78;0.90;91.99;0.90;92.26;1.00;92.41;0.90;92.62;0.90;93.09;1.00;93.09;0.90;93.25;0.90;93.46;0.90;93.93;0.90;93.93;1.00;94.09;0.90;94.35;0.90;94.62;0.90;94.77;1.00;94.83;0.90;94.98;0.90;95.35;0.90;95.56;0.90;95.61;1.00;95.77;0.90";
    case 8: return "0.21;0.80;0.43;0.80;0.75;0.80;1.07;0.80;1.61;0.80;1.82;0.80;2.14;0.80;2.36;0.80;2.79;0.80;3.32;0.80;3.43;0.80;3.96;0.80;4.07;0.80;4.50;0.80;4.93;0.80;5.14;0.80;5.57;0.80;5.79;0.80;6.21;0.80;6.64;0.80;6.86;0.80;7.29;0.80;7.50;0.80;7.93;0.80;8.36;0.80;8.57;0.80;9.00;0.80;9.21;0.80;9.64;0.80;10.07;0.80;10.29;0.80;10.71;0.80;10.93;0.80;11.36;0.80;11.79;0.80;12.00;0.80;12.43;0.80;12.64;0.80;13.07;0.80;13.50;0.80;13.61;0.80;13.71;0.90;14.04;0.70;14.14;0.80;14.36;0.80;14.79;0.80;15.11;0.70;15.21;0.80;15.43;0.80;15.86;0.80;15.86;0.70;15.96;0.80;16.50;0.80;16.82;0.70;16.82;0.80;17.04;0.80;17.57;0.80;17.57;0.70;17.68;0.80;18.11;0.80;18.54;0.80;18.75;0.80;19.29;0.80;19.39;0.80;19.93;0.80;20.36;0.80;20.46;0.80;20.68;0.70;21.00;0.80;21.00;0.70;21.11;0.80;21.43;0.70;21.54;0.80;21.86;0.70;22.07;0.80;22.18;0.80;22.29;0.70;22.61;0.80;22.82;0.70;22.82;0.80;23.14;0.70;23.36;0.80;23.57;0.70;23.79;0.80;24.00;0.80;24.00;0.70;24.43;0.80;24.43;0.70;24.54;0.80;24.54;0.70;24.75;0.70;25.07;0.80;25.07;0.70;25.29;0.70;25.29;0.70;25.50;0.70;25.50;0.80;25.61;0.70;25.61;0.80;25.71;0.70;25.82;0.70;25.93;0.70;25.93;0.70;26.04;0.70;26.04;0.80;26.14;0.70;26.25;0.70;26.25;0.80;26.36;0.70;26.46;0.70;26.57;0.70;26.68;0.70;26.68;0.80;26.68;0.70;26.79;0.70;26.89;0.70;27.00;0.70;27.11;0.80;27.11;0.70;27.21;0.70;27.32;0.70;27.43;0.70;29.14;1.00;29.25;0.90;29.57;0.90;29.79;0.90;29.89;0.90;29.89;0.70;30.00;0.70;30.21;0.90;30.43;0.70;30.43;0.70;30.64;0.90;30.75;0.70;30.86;0.90;30.86;0.70;31.29;0.90;31.29;0.70;31.29;0.70;31.50;0.90;31.71;0.70;31.71;0.70;31.93;0.90;32.14;0.70;32.14;0.70;32.36;0.90;32.57;0.70;32.57;0.90;32.57;0.70;32.89;0.70;33.00;0.70;33.00;0.90;33.21;0.90;33.32;0.70;33.43;0.70;33.75;0.90;33.86;0.70;33.86;0.70;34.07;0.90;34.18;0.70;34.29;0.90;34.29;0.70;34.61;0.70;34.71;0.70;34.82;0.90;34.93;0.90;35.04;0.70;35.14;0.70;35.36;0.90;35.57;0.70;35.57;0.70;35.79;0.90;35.89;0.70;36.00;0.90;36.00;0.70;36.43;0.70;36.43;0.90;36.43;0.70;36.64;0.90;36.75;0.70;36.86;0.70;37.07;0.90;37.07;0.70;37.18;0.70;37.61;0.90;37.61;0.70;37.61;0.70;37.71;0.90;38.04;0.70;38.14;0.70;38.14;0.90;38.46;0.90;38.57;0.70;38.57;0.70;38.79;0.90;39.00;0.70;39.00;0.70;39.32;0.90;39.32;0.70;39.43;0.70;39.43;0.90;39.75;0.70;39.86;0.70;39.86;0.90;40.07;0.90;40.18;0.70;40.18;0.70;40.50;0.90;40.61;0.70;40.61;0.70;40.93;0.90;41.04;0.70;41.14;0.70;41.14;0.90;41.57;0.70;41.57;0.70;41.68;0.90;41.79;0.90;42.00;0.70;42.00;0.70;42.32;0.90;42.32;0.70;42.43;0.70;42.75;0.90;42.75;0.70;42.75;0.70;42.86;0.90;43.18;0.70;43.29;0.70;43.29;1.00;43.39;0.90;43.50;0.90;43.71;0.70;43.71;0.70;44.04;0.90;44.14;0.70;44.14;1.00;44.14;0.70;44.36;0.90;44.57;0.70;44.57;0.70;44.57;0.90;44.89;0.70;45.00;0.70;45.00;1.00;45.00;0.90;45.21;0.90;45.32;0.70;45.43;0.70;45.64;0.90;45.75;0.70;45.86;0.70;45.86;1.00;46.18;0.90;46.29;0.70;46.29;0.70;46.29;0.90;46.61;0.70;46.71;0.70;46.71;1.00;46.71;0.90;46.93;0.90;47.14;0.70;47.14;0.70;47.46;0.90;47.57;0.70;47.57;1.00;47.57;0.70;47.89;0.90;48.00;0.70;48.00;0.70;48.00;0.90;48.32;0.70;48.43;0.70;48.43;1.00;48.43;0.90;48.64;0.90;48.75;0.70;48.86;0.70;48.96;0.70;49.07;0.90;49.18;0.70;49.18;1.00;49.29;0.70;49.50;0.90;49.50;0.70;49.61;0.70;49.71;0.70;49.71;0.90;49.71;0.70;49.93;0.70;50.04;0.70;50.14;1.00;50.14;0.90;50.14;0.70;50.36;0.90;50.46;0.70;50.57;0.70;50.89;0.90;50.89;0.70;51.00;1.00;51.00;0.70;51.21;0.90;51.32;0.70;51.43;0.70;51.43;0.90;51.75;0.70;51.86;1.00;51.86;0.70;51.86;0.90;52.07;0.90;52.18;0.70;52.29;0.70;52.50;0.90;52.71;0.70;52.71;1.00;52.71;0.70;52.93;0.90;53.14;0.70;53.14;0.70;53.14;0.90;53.46;0.70;53.57;1.00;53.57;0.70;53.57;0.90;53.79;0.90;54.00;0.70;54.00;0.70;54.21;0.90;54.43;0.70;54.43;0.70;54.43;1.00;54.64;0.90;54.75;0.70;54.86;0.70;54.86;0.90;55.18;0.70;55.29;0.70;55.29;1.00;55.29;0.90;55.50;0.90;55.71;0.70;55.71;0.70;56.04;0.90;56.04;0.70;56.14;0.70;56.14;1.00;56.36;0.90;58.29;0.90;58.29;1.00;58.71;0.70;58.71;0.70;59.14;0.70;59.25;0.70;59.57;0.70;59.57;0.70;60.00;0.70;60.00;0.70;60.43;0.70;60.43;0.70;60.43;0.90;60.75;0.70;60.86;0.70;61.29;0.70;61.29;0.70;61.29;0.90;61.71;0.70;61.71;0.70;62.14;0.70;62.14;0.70;62.14;0.90;62.57;0.70;62.57;0.70;63.00;0.70;63.00;0.70;63.00;0.90;63.32;0.70;63.43;0.70;63.75;0.70;63.75;0.70;63.86;0.90;64.07;0.70;64.18;0.70;64.50;0.70;64.50;0.70;64.71;0.90;64.93;0.70;64.93;0.70;65.36;0.70;65.36;0.70;65.57;0.90;65.79;0.70;65.89;0.70;66.21;0.70;66.32;0.70;66.43;0.90;66.64;0.70;66.75;0.70;67.07;0.70;67.18;0.70;67.29;0.90;67.50;0.70;67.50;0.70;67.93;0.70;68.04;0.70;68.14;0.90;68.36;0.70;68.36;0.70;68.79;0.70;68.79;0.70;69.00;0.90;69.21;0.70;69.21;0.70;69.64;0.70;69.64;0.70;69.86;0.90;70.07;0.70;70.07;0.70;70.50;0.70;70.71;0.90;70.93;0.70;70.93;0.70;71.36;0.70;71.46;0.70;71.57;0.90;71.79;0.70;71.79;0.70;72.21;0.70;72.32;0.70;72.43;0.90;72.75;0.70;73.07;0.70;73.18;0.70;73.29;0.90;73.50;0.70;73.50;0.70;73.93;0.70;73.93;0.70;74.14;0.90;74.36;0.70;74.36;0.70;74.79;0.70;74.79;0.70;75.00;0.90;75.21;0.70;75.32;0.70;75.75;0.70;75.75;0.70;75.86;0.90;76.07;0.70;76.18;0.70;76.50;0.70;76.61;0.70;76.71;0.90;76.93;0.70;77.04;0.70;77.36;0.70;77.46;0.70;77.57;0.90;77.79;0.70;77.79;0.70;78.21;0.70;78.32;0.70;78.43;0.90;78.64;0.70;78.75;0.70;79.07;0.70;79.18;0.70;79.29;0.90;79.50;0.70;79.50;0.70;79.93;0.70;80.04;0.70;80.04;0.90;80.36;0.70;80.46;0.70;80.89;0.70;80.89;0.70;81.00;0.90;81.32;0.70;81.32;0.70;81.75;0.70;81.86;0.90;82.07;0.70;82.07;0.70;82.50;0.70;82.50;0.70;82.71;0.90;82.93;0.70;82.93;0.70;83.36;0.70;83.36;0.70;83.57;0.90;83.79;0.70;83.79;0.70;84.21;0.70;84.32;0.70;84.43;0.90;84.64;0.70;84.75;0.70;85.07;0.70;85.07;0.70;85.29;0.90;85.61;0.70;86.04;0.70;86.14;0.70;87.00;0.70;87.54;0.70";
    case 9: return "0.23;0.80;0.35;0.80;0.69;0.80;0.92;0.80;1.15;0.80;1.38;0.80;1.62;0.80;1.85;0.80;2.19;0.80;2.54;0.80;2.88;0.80;3.23;0.80;3.46;0.80;3.69;0.80;4.04;0.80;4.38;0.80;4.62;0.80;4.85;0.80;5.08;0.80;5.31;0.80;5.54;0.80;5.88;0.80;6.23;0.80;6.46;0.80;6.81;0.80;7.04;0.80;7.38;0.80;7.38;0.90;7.73;0.80;8.08;0.80;8.42;0.80;8.77;0.80;9.00;0.80;9.23;0.80;9.58;0.80;9.92;0.80;10.27;0.80;10.62;0.80;10.85;0.80;11.08;0.80;11.42;0.80;11.77;0.80;12.00;0.80;12.23;0.80;12.46;0.80;12.69;0.80;12.92;0.80;13.27;0.80;13.62;0.80;13.85;0.80;14.19;0.80;14.54;0.80;14.77;0.90;14.77;0.80;15.12;0.80;15.46;0.80;15.81;0.80;16.15;0.80;16.38;0.80;16.62;0.80;16.96;0.80;17.31;0.80;17.65;0.80;18.00;0.80;18.23;0.80;18.46;0.80;18.81;0.80;19.04;0.80;19.38;0.80;19.62;0.80;19.85;0.80;20.08;0.80;20.31;0.80;20.65;0.80;21.00;0.80;21.23;0.80;21.58;0.80;21.81;0.80;22.15;0.80;22.15;0.90;22.50;0.80;22.85;0.80;23.08;0.80;23.42;0.80;23.77;0.80;24.00;0.80;24.35;0.80;24.69;0.80;24.92;0.80;25.27;0.80;25.50;0.80;25.85;0.80;26.08;0.80;26.31;0.80;26.54;0.80;26.77;0.80;27.00;0.80;27.23;0.80;27.46;0.80;27.69;0.80;27.92;0.80;28.15;0.80;28.27;0.80;28.38;0.80;28.50;0.80;28.62;0.80;28.73;0.80;28.73;0.80;28.85;0.80;28.96;0.80;28.96;0.80;29.08;0.80;29.19;0.80;29.31;0.80;29.54;0.90;29.65;0.70;29.65;0.70;29.77;0.70;30.00;0.90;30.00;0.70;30.12;0.70;30.23;0.70;30.35;0.70;30.35;0.70;30.46;0.90;30.58;0.70;30.69;0.70;31.38;0.70;31.38;0.90;31.38;0.70;31.50;0.70;31.62;0.70;31.73;0.70;31.85;0.70;31.96;0.70;32.08;0.70;32.31;0.90;32.31;0.70;32.42;0.70;32.54;0.70;32.65;0.70;32.65;0.70;32.77;0.70;33.23;0.90;33.23;0.70;33.23;0.70;33.35;0.70;33.46;0.70;33.58;0.70;33.58;0.70;33.69;0.90;33.81;0.70;33.81;0.70;33.92;0.70;34.04;0.70;34.15;0.90;34.15;0.70;34.27;0.70;34.38;0.70;34.38;0.70;34.50;0.70;35.08;0.90;35.08;0.70;35.19;0.70;35.31;0.70;35.42;0.70;35.42;0.70;35.54;0.70;36.00;0.90;36.00;0.70;36.12;0.70;36.23;0.70;36.23;0.70;36.35;0.70;36.58;0.90;36.58;0.70;36.58;0.70;36.69;0.70;36.92;0.70;36.92;0.90;37.04;0.70;37.15;0.70;37.27;0.70;37.38;0.70;37.38;0.90;37.38;0.70;37.62;0.70;37.62;0.70;37.85;0.70;37.85;0.70;37.85;0.90;37.96;0.70;38.77;0.70;38.77;0.90;38.88;0.70;38.88;0.70;38.88;0.70;39.00;0.70;39.12;0.70;39.23;0.70;39.35;0.70;39.35;0.70;39.69;0.90;39.69;0.70;39.81;0.70;39.92;0.70;40.04;0.70;40.04;0.70;40.15;0.70;40.27;0.70;40.62;0.90;40.62;0.70;40.73;0.70;40.73;0.70;40.85;0.70;40.96;0.70;41.08;0.90;41.19;0.70;41.19;0.70;41.31;0.70;41.42;0.70;41.54;0.90;41.54;0.70;41.65;0.70;41.65;0.70;41.77;0.70;42.46;0.70;42.46;0.90;42.58;0.70;42.58;0.70;42.81;0.70;42.92;0.70;43.04;0.70;43.38;0.90;43.38;0.70;43.50;0.70;43.62;0.70;43.62;0.70;43.85;0.70;43.85;0.90;43.96;0.70;43.96;0.70;44.08;0.70;44.31;0.70;44.31;0.90;44.42;0.70;44.54;0.70;44.54;0.70;44.77;0.70;44.77;0.90;44.88;0.70;45.00;0.70;45.23;0.70;45.23;0.90;45.23;0.70;45.35;0.70;46.15;0.90;46.15;0.70;46.27;0.70;46.38;0.70;46.50;0.70;46.62;0.70;46.73;0.70;46.73;0.70;47.08;0.90;47.19;0.70;47.31;0.70;47.42;0.70;47.54;0.70;48.00;0.70;48.00;0.70;48.00;0.90;48.12;0.70;48.12;0.70;48.46;0.70;48.46;0.90;48.46;0.70;48.81;0.70;48.92;0.70;48.92;0.90;48.92;0.70;49.15;0.70;49.85;0.70;49.96;0.90;49.96;0.70;50.08;0.70;50.19;0.70;50.31;0.70;50.42;0.70;50.77;0.90;50.77;0.70;50.88;0.70;51.00;0.70;51.00;0.70;51.12;0.70;51.23;0.70;51.23;0.90;51.46;0.70;51.69;0.70;51.69;0.90;51.92;0.70;52.04;0.70;52.15;0.70;52.15;0.90;52.27;0.70;52.38;0.70;52.38;0.70;52.62;0.70;52.62;0.90;52.62;0.70;52.73;0.70;53.54;0.70;53.54;0.90;53.65;0.70;53.65;0.70;53.77;0.70;53.88;0.70;53.88;0.70;54.00;0.70;54.46;0.90;54.46;0.70;54.58;0.70;54.58;0.70;54.81;0.70;54.81;0.70;55.38;0.70;55.38;0.90;55.38;0.70;55.50;0.70;55.62;0.70;55.73;0.70;55.85;0.70;55.85;0.90;55.96;0.70;56.08;0.70;56.19;0.70;56.31;0.70;56.31;0.90;57.23;0.90;57.23;0.70;57.35;0.70;57.35;0.70;57.46;0.70;57.58;0.70;57.69;0.70;57.81;0.70;58.15;0.70;58.15;0.90;58.15;0.70;58.38;0.70;58.38;0.70;58.62;0.70;58.62;0.90;58.62;0.70;59.08;1.00;59.08;0.70;59.19;0.70;59.19;0.70;59.31;0.70;59.42;0.70;59.54;0.70;59.54;1.00;59.54;0.70;59.65;0.70;59.88;0.70;59.88;0.70;60.00;0.70;60.00;1.00;60.12;0.70;60.92;0.70;60.92;1.00;60.92;0.70;61.04;0.70;61.15;0.70;61.15;0.70;61.27;0.70;61.38;0.70;61.50;0.70;61.85;1.00;61.85;0.70;61.96;0.70;61.96;0.70;62.08;0.70;62.08;0.70;62.31;0.70;62.31;0.70;62.77;1.00;62.77;0.70;62.88;0.70;62.88;0.70;63.00;0.70;63.12;0.70;63.23;1.00;63.35;0.70;63.46;0.70;63.46;0.70;63.58;0.70;63.69;1.00;63.81;0.70;63.92;0.70;63.92;0.70;64.62;0.70;64.62;1.00;64.62;0.70;64.73;0.70;64.85;0.70;64.96;0.70;65.54;1.00;65.54;0.70;65.65;0.70;65.65;0.70;66.00;0.70;66.00;1.00;66.12;0.70;66.12;0.70;66.35;0.70;66.46;1.00;66.58;0.70;66.58;0.70;66.69;0.70;66.81;0.70;66.92;1.00;67.04;0.70;67.04;0.70;67.15;0.70;67.27;0.70;67.27;0.70;67.38;1.00;67.38;0.70;68.31;1.00;68.31;0.70;68.42;0.70;68.42;0.70;68.65;0.70;68.65;0.70;68.77;0.70;68.77;0.70;69.23;0.70;69.23;1.00;69.35;0.70;69.46;0.70;69.58;0.70;69.58;0.70;70.15;1.00;70.15;0.70;70.15;0.70;70.27;0.70;70.38;0.70;70.62;0.70;70.62;1.00;70.62;0.70;70.73;0.70;70.96;0.70;71.08;1.00;71.08;0.70;71.19;0.70;72.00;0.70;72.00;1.00;72.12;0.70;72.23;0.70;72.23;0.70;72.46;0.70;72.81;0.70;72.92;1.00;72.92;0.70;73.04;0.70;73.27;0.70;73.38;1.00;73.38;0.70;73.50;0.70;73.85;0.70;73.85;0.90;73.96;0.70;74.08;0.70;74.31;0.70;74.42;0.90;74.42;0.70;74.42;0.70;74.65;0.70;74.77;0.90;74.77;0.70;75.46;0.70;75.58;0.70;75.69;0.90;75.69;0.70;76.04;0.70;76.50;0.70;76.62;0.70;76.62;0.90;76.73;0.70;77.08;0.70;77.54;0.70;77.54;0.90;77.65;0.70;77.77;0.70;78.00;0.70;78.12;0.90;78.12;0.70;78.46;0.90;78.46;0.70;78.58;0.70;79.38;0.90;79.38;0.70;79.50;0.70;79.62;0.70;79.73;0.70;80.31;0.90;80.31;0.70;80.42;0.70;80.54;0.70;80.77;0.90;81.23;0.70;81.23;0.90;81.23;0.70;81.35;0.70;81.35;0.70;81.69;0.90;81.81;0.70;81.92;0.70;82.15;0.90;82.27;0.70;83.08;0.90;83.08;0.70;83.19;0.70;83.19;0.70;83.31;0.70;84.00;0.70;84.00;0.90;84.12;0.70;84.12;0.70;84.23;0.70;84.92;0.90;84.92;0.70;84.92;0.70;85.04;0.70;85.04;0.70;85.38;0.70;85.38;0.90;85.50;0.70;85.50;0.70;85.85;0.70;85.85;0.70;85.85;0.90;85.96;0.70;86.88;0.90;86.88;0.70;86.88;0.70;87.12;0.70;87.69;0.70;87.69;0.90;88.04;0.70;88.15;0.70;88.15;0.90;88.27;0.70;88.27;0.70;88.73;1.00";
    case 10: return "0.42;1.00;0.53;0.90;0.96;0.90;1.39;0.90;1.81;0.90;2.24;0.90;3.96;0.90;4.39;0.90;4.81;0.90;5.24;0.90;5.67;0.90;5.99;0.90;6.10;0.90;6.31;0.90;6.53;0.90;7.39;0.90;7.92;0.90;8.24;0.90;8.67;0.90;9.10;0.90;10.81;0.90;11.24;0.90;11.67;0.90;12.10;0.90;12.53;0.90;12.85;0.90;12.96;0.90;13.17;0.90;13.39;0.90;14.14;1.00;14.24;0.80;14.24;0.90;14.67;0.90;14.67;0.80;15.10;0.80;15.10;0.90;15.53;0.80;15.53;0.90;15.96;0.80;15.96;0.90;16.39;0.80;16.81;0.80;17.24;0.80;17.67;0.90;17.67;0.80;18.10;0.90;18.10;0.80;18.53;0.90;18.53;0.80;18.96;0.90;18.96;0.80;19.39;0.90;19.39;0.80;19.71;0.90;19.81;0.80;19.81;0.90;20.03;0.90;20.24;0.90;20.24;0.80;20.67;0.80;21.10;0.80;21.21;0.90;21.53;0.90;21.53;0.80;21.96;0.90;21.96;0.80;22.39;0.90;22.39;0.80;22.81;0.90;22.81;0.80;23.24;0.80;23.67;0.80;24.10;0.80;24.53;0.90;24.53;0.80;24.96;0.90;24.96;0.80;25.39;0.90;25.39;0.80;25.81;0.90;25.81;0.80;26.24;0.90;26.24;0.80;26.56;0.90;26.67;0.90;26.67;0.80;26.89;0.90;27.10;0.90;27.10;0.80;27.53;0.80;27.85;1.00;27.96;0.90;28.06;0.80;28.92;0.80;29.56;0.70;29.67;0.80;29.99;0.70;30.42;0.70;30.64;0.80;30.85;0.70;31.28;0.70;31.60;0.70;32.03;0.70;32.56;0.70;32.99;0.70;33.31;0.70;33.42;0.70;33.64;0.70;33.85;0.70;33.96;0.70;34.06;0.70;34.28;0.70;34.39;0.70;34.49;0.70;34.71;0.70;34.71;1.00;34.71;0.70;34.81;0.90;34.92;0.80;34.92;0.70;35.03;0.70;35.03;0.80;35.03;0.70;35.35;0.80;35.46;0.80;35.46;0.70;35.56;0.70;35.56;1.00;35.56;0.80;35.67;0.70;35.67;0.90;35.78;0.70;35.78;0.80;35.89;1.00;35.99;0.90;35.99;0.80;36.21;1.00;36.31;0.90;36.31;0.80;36.31;0.70;36.42;0.70;36.53;0.70;36.53;0.90;36.53;0.80;36.64;0.70;36.74;0.70;36.85;0.70;37.17;0.80;37.28;0.90;37.49;0.70;37.60;0.70;37.71;0.70;37.71;0.80;37.81;0.70;37.81;0.90;37.92;0.80;38.03;0.80;38.14;0.70;38.14;1.00;38.14;0.70;38.24;0.80;38.24;0.70;38.24;0.90;38.35;0.70;38.56;0.80;38.89;0.70;38.99;0.80;38.99;0.70;39.10;0.80;39.10;0.90;39.31;0.70;39.42;0.80;39.53;0.90;39.64;0.80;39.74;0.90;39.96;0.80;39.96;0.90;40.06;0.70;40.17;0.70;40.28;0.70;40.39;0.70;40.49;0.70;40.49;0.70;40.60;0.80;40.60;0.70;40.60;0.90;40.71;0.70;40.81;0.70;41.03;0.70;41.14;0.70;41.24;0.70;41.24;0.80;41.35;0.70;41.35;0.90;41.56;1.00;41.67;0.70;41.67;0.80;41.78;0.90;41.89;0.70;41.99;0.80;42.10;0.90;42.21;0.70;42.31;0.80;42.42;0.70;42.53;0.90;42.53;0.70;42.53;0.80;42.96;0.80;43.28;0.70;43.39;0.90;43.49;0.70;43.60;0.80;43.81;0.80;44.03;0.80;44.24;0.90;44.24;0.80;44.46;0.70;44.46;0.80;44.56;0.70;44.67;0.80;44.67;0.90;44.99;0.70;44.99;1.00;45.10;0.90;45.10;0.80;45.21;0.70;45.42;0.80;45.42;0.90;45.74;0.90;45.74;0.80;45.74;0.70;45.85;0.80;45.85;0.90;45.96;0.70;45.96;0.80;46.06;0.90;46.28;0.80;46.28;0.90;46.49;0.80;46.60;0.90;46.71;0.70;46.81;0.80;46.81;0.70;46.92;0.70;47.03;0.70;47.03;0.90;47.03;0.80;47.14;0.70;47.24;0.70;47.24;0.70;47.35;0.70;47.46;0.70;47.46;0.70;47.56;0.80;47.56;0.70;47.67;0.70;47.67;0.90;47.78;0.70;47.89;0.70;47.89;0.70;48.10;0.70;48.10;0.70;48.42;0.70;48.53;0.80;48.53;0.90;48.96;0.80;48.96;0.90;49.17;0.70;49.81;0.80;50.24;0.80;50.35;0.90;50.67;0.90;50.67;0.80;50.99;1.00;51.10;0.90;51.10;0.80;51.53;0.80;51.53;0.90;51.85;0.80;51.96;0.90;52.17;0.80;52.39;0.80;52.49;0.80;52.81;0.90;52.81;0.80;53.24;0.90;53.24;0.80;53.67;0.80;53.67;0.90;53.78;0.80;53.99;0.80;54.10;0.90;54.10;0.80;54.53;0.90;54.53;0.80;54.85;0.80;54.96;0.90;55.39;0.80;55.39;0.90;55.49;0.80;55.71;0.80;55.81;0.80;55.81;0.90;56.24;0.90;56.24;0.80;56.67;0.80;56.67;0.90;57.10;0.90;57.10;0.80;57.31;0.80;57.53;0.90;57.53;0.80;57.74;0.80;57.96;0.90;57.96;0.80;58.39;0.80;58.39;0.90;58.81;0.80;58.81;0.90;58.92;0.80;59.03;0.80;59.14;0.80;59.24;0.90;59.67;0.80;59.67;0.90;60.10;0.80;60.10;0.90;60.53;0.80;60.53;0.90;60.64;0.80;60.96;0.90;61.17;0.80;61.39;0.90;61.39;0.80;61.81;0.90;61.81;0.80;62.24;0.80;62.24;0.90;62.35;0.80;62.67;0.90;63.10;0.80;63.10;0.90;63.53;0.90;63.53;0.80;63.96;0.90;64.39;0.90;64.71;1.00;64.81;0.90;64.81;0.80;65.14;0.90;65.35;0.80;65.46;0.90;65.67;0.80;65.89;0.90;66.10;0.80;66.53;0.90;66.53;0.80;66.85;0.90;66.96;0.80;67.17;0.90;67.39;0.90;67.39;0.80;67.81;0.90;67.81;0.80;68.24;0.90;68.24;0.80;68.67;0.90;68.67;0.80;68.89;0.90;69.10;0.80;69.53;0.90;69.53;0.80;69.96;0.80;69.96;0.90;70.28;0.90;70.39;0.80;70.60;0.90;70.81;0.80;70.81;0.90;71.24;0.90;71.24;0.80;71.67;0.90;71.67;0.80;72.10;0.80;72.10;0.90;72.42;0.90;72.53;0.80;72.96;0.90;72.96;0.80;73.39;0.90;73.39;0.80;73.71;0.90;73.81;0.80;74.03;0.90;74.24;0.80;74.24;0.90;74.67;0.80;74.67;0.90;75.10;0.90;75.10;0.80;75.53;0.90;75.53;0.80;75.74;0.90;75.96;0.80;76.28;0.90;76.39;0.80;76.81;0.90;76.92;0.80;77.24;0.80;77.67;0.90;77.67;0.80;78.10;0.80;78.42;1.00;78.53;0.80;78.53;0.90;78.96;0.80;79.17;0.90;79.39;0.90;79.39;0.80;79.60;0.90;79.81;0.90;79.81;0.80;80.03;0.90;80.14;0.90;80.24;0.80;80.46;0.90;80.67;0.90;80.67;0.80;80.78;0.90;81.10;0.90;81.10;0.80;81.31;0.90;81.53;0.90;81.53;0.80;81.64;0.90;81.96;0.90;81.96;0.80;82.39;0.80;82.60;0.90;82.81;0.90;82.81;0.80;83.03;0.90;83.24;0.90;83.35;0.80;83.46;0.90;83.56;0.90;83.67;0.80;83.89;0.90;83.99;0.90;84.10;0.80;84.21;0.90;84.42;0.90;84.53;0.80;84.74;0.90;84.96;0.80;84.96;0.90;85.39;0.90;85.39;0.80;85.81;0.80;86.03;0.90;86.24;0.90;86.24;0.80;86.46;0.90;86.67;0.90;86.78;0.80;86.89;0.90;86.99;0.90;87.10;0.80;87.31;0.90;87.53;0.90;87.53;0.80;87.85;0.90;88.06;0.80;88.17;0.90;88.39;0.90;88.49;0.80;88.81;0.90;89.46;0.90;89.67;0.90;89.89;0.90;90.10;0.90;90.31;0.90;90.42;0.90;90.74;0.90;90.85;0.90;91.28;0.90;91.60;0.90;91.71;0.90;91.92;0.90;92.14;1.00;92.24;0.90;92.56;0.90;92.67;0.90;92.89;0.90;93.10;0.90;93.31;0.90;93.53;0.90;93.85;0.90;93.96;0.90;94.28;0.90;94.39;0.90;94.60;0.90;94.81;0.90;95.03;0.90;95.24;0.90;95.46;0.90;95.67;0.90";
    case 11: return "0.43;0.90;0.78;0.90;0.93;0.90;1.21;0.90;1.55;0.90;1.71;0.90;1.85;0.90;2.31;0.90;2.61;0.90;2.80;0.90;3.06;0.90;3.36;0.90;3.53;0.90;3.68;0.90;4.15;0.90;4.46;0.90;4.61;0.90;4.91;0.90;5.18;0.90;5.38;0.90;5.51;0.90;5.94;0.90;6.24;0.90;6.41;0.90;6.74;0.90;7.01;0.90;7.34;0.90;7.36;1.00;7.79;0.90;8.13;0.90;8.28;0.90;8.59;0.90;8.86;0.90;9.01;0.90;9.19;0.90;9.66;0.90;9.96;0.90;10.13;0.90;10.41;0.90;10.69;0.90;10.86;0.90;11.00;0.90;11.49;0.90;11.78;0.90;11.94;0.90;12.19;0.90;12.56;0.90;12.71;0.90;12.86;0.90;13.33;0.90;13.63;0.90;13.78;0.90;14.13;0.90;14.39;0.90;14.71;0.90;14.77;1.00;15.19;1.00;15.51;1.00;15.66;1.00;15.96;1.00;16.27;1.00;16.46;1.00;16.59;1.00;17.04;1.00;17.35;1.00;17.51;1.00;17.81;1.00;18.14;1.00;18.27;1.00;18.44;1.00;18.91;1.00;19.21;1.00;19.37;1.00;19.53;0.70;19.67;1.00;19.99;1.00;20.00;0.70;20.12;1.00;20.25;0.70;20.27;1.00;20.40;0.70;20.56;0.70;20.71;0.70;20.74;1.00;20.86;0.70;21.02;0.70;21.12;1.00;21.15;0.70;21.29;1.00;21.31;0.70;21.48;0.70;21.56;1.00;21.58;0.70;21.70;0.70;21.76;0.70;21.82;1.00;21.83;0.70;21.88;0.70;21.91;0.70;22.06;0.70;22.09;1.00;22.23;0.90;22.56;0.90;22.75;0.80;22.87;0.80;23.04;0.90;23.48;0.90;23.67;0.80;23.78;0.80;23.93;0.90;24.13;0.80;24.27;0.80;24.39;0.90;24.86;0.90;25.05;0.80;25.15;0.80;25.33;0.90;25.81;0.90;25.81;0.70;25.90;0.70;25.97;0.80;26.03;0.70;26.08;0.80;26.26;0.90;26.71;0.70;26.74;0.90;26.83;0.70;26.88;0.80;27.02;0.80;27.21;0.90;27.66;0.90;27.85;0.80;27.98;0.80;28.13;0.90;28.56;0.90;28.57;1.00;28.99;1.00;29.03;0.90;29.51;0.90;29.67;0.80;29.80;0.80;29.99;0.90;30.44;0.90;30.60;0.80;30.75;0.80;30.88;0.90;31.36;0.90;31.53;0.80;31.67;0.80;31.83;0.90;32.28;0.90;32.43;0.80;32.57;0.80;32.74;0.90;33.19;0.90;33.38;0.80;33.53;0.80;33.68;0.90;34.11;0.90;34.28;0.80;34.40;0.80;34.61;0.90;35.06;0.90;35.23;0.80;35.38;0.80;35.51;0.90;35.96;1.00;35.96;0.90;36.15;0.80;36.30;0.80;36.37;1.00;36.41;0.90;36.89;0.90;36.94;1.00;37.07;0.80;37.34;0.90;37.67;1.00;37.81;0.90;37.82;1.00;38.13;1.00;38.24;0.90;38.42;1.00;38.59;1.00;38.74;1.00;38.74;0.90;39.21;0.90;39.61;1.00;39.69;0.90;39.97;1.00;40.13;0.90;40.29;1.00;40.47;1.00;40.59;0.90;40.62;1.00;41.04;0.90;41.44;1.00;41.51;0.90;41.79;1.00;41.96;0.90;42.11;1.00;42.27;1.00;42.44;1.00;42.44;0.90;42.89;0.90;43.33;1.00;43.34;0.90;43.64;1.00;43.84;0.90;43.92;1.00;44.19;1.00;44.29;0.90;44.74;0.90;45.12;1.00;45.21;0.90;45.46;1.00;45.69;0.90;45.81;1.00;45.97;1.00;46.11;0.90;46.12;1.00;46.54;0.90;47.02;1.00;47.04;0.90;47.34;1.00;47.51;0.90;47.66;1.00;47.81;1.00;47.97;1.00;47.99;0.90;48.43;0.90;48.86;1.00;48.91;0.90;49.06;1.00;49.36;0.90;49.47;1.00;49.64;1.00;49.79;1.00;49.83;0.90;50.26;0.90;50.61;1.00;50.73;0.90;50.77;1.00;51.07;1.00;51.11;0.70;51.24;0.90;51.25;0.70;51.34;1.00;51.46;0.70;51.62;1.00;51.65;0.70;51.66;0.90;51.81;0.70;51.93;0.70;52.08;0.70;52.22;0.70;52.28;0.70;52.46;0.70;52.60;0.70;52.70;0.70;52.81;0.70;52.93;0.70;53.03;0.70;53.15;0.70;53.28;0.70;53.40;0.70;53.49;0.90;53.51;1.00;53.79;1.00;53.96;0.90;53.97;1.00;54.26;1.00;54.41;1.00;54.44;0.90;54.54;1.00;54.71;1.00;54.89;1.00;54.93;0.90;55.21;1.00;55.37;1.00;55.38;0.90;55.84;1.00;55.84;0.90;56.29;0.90;56.57;1.00;56.72;1.00;56.74;0.90;56.89;1.00;57.06;1.00;57.21;0.90;57.22;1.00;57.51;1.00;57.66;1.00;57.69;0.90;57.97;1.00;58.11;0.90;58.12;1.00;58.31;1.00;58.44;1.00;58.57;1.00;58.60;0.90;58.91;1.00;59.07;1.00;59.08;0.90;59.51;1.00;59.53;0.90;59.99;0.90;60.41;1.00;60.46;0.90;60.88;1.00;60.93;0.90;61.22;1.00;61.36;1.00;61.39;0.90;61.67;1.00;61.82;1.00;61.84;0.90;61.99;1.00;62.14;1.00;62.26;0.90;62.29;1.00;62.61;1.00;62.76;0.90;62.77;1.00;63.19;1.00;63.23;0.90;63.71;0.90;63.94;1.00;64.11;1.00;64.13;0.90;64.26;1.00;64.42;1.00;64.59;1.00;64.59;0.90;64.91;1.00;65.06;1.00;65.06;0.90;65.36;1.00;65.51;0.90;65.52;1.00;65.69;1.00;65.84;1.00;65.97;1.00;65.98;0.90;66.29;1.00;66.44;0.90;66.46;1.00;66.77;1.00;66.86;0.90;66.92;1.00;67.07;1.00;67.22;1.00;67.33;0.90;67.41;1.00;67.81;1.00;68.28;0.90;68.29;1.00;68.62;1.00;68.76;0.90;68.79;1.00;69.04;1.00;69.19;1.00;69.21;0.90;69.36;1.00;69.51;1.00;69.66;0.90;69.67;1.00;69.96;1.00;70.12;1.00;70.13;0.90;70.57;1.00;70.61;0.90;71.09;0.90;71.29;1.00;71.42;1.00;71.54;0.90;71.61;1.00;71.77;1.00;71.96;1.00;71.98;0.90;72.29;1.00;72.44;1.00;72.44;0.90;72.76;1.00;72.91;1.00;72.91;0.90;73.06;1.00;73.21;1.00;73.36;1.00;73.36;0.90;73.71;1.00;73.83;0.90;73.84;1.00;74.26;1.00;74.29;0.90;74.74;0.90;75.21;1.00;75.21;0.70;75.23;0.90;75.31;0.70;75.62;0.70;75.65;1.00;75.66;0.90;76.01;1.00;76.13;0.90;76.14;1.00;76.46;1.00;76.59;0.90;76.60;1.00;76.77;1.00;76.92;1.00;77.06;0.90;77.07;1.00;77.36;1.00;77.51;1.00;77.53;0.90;77.94;1.00;77.99;0.90;78.44;0.90;78.71;1.00;78.87;1.00;78.91;0.90;79.04;1.00;79.21;1.00;79.36;1.00;79.38;0.90;79.71;1.00;79.81;0.90;79.84;1.00;80.12;1.00;80.27;1.00;80.29;0.90;80.44;1.00;80.57;1.00;80.63;0.70;80.74;1.00;80.74;0.90;80.80;0.70;80.95;0.70;81.06;1.00;81.08;0.70;81.22;1.00;81.23;0.70;81.36;0.70;81.48;0.70;81.52;1.00;81.60;0.70;81.66;1.00;81.73;0.70;81.82;1.00;81.83;0.70;81.95;0.70;81.97;1.00;82.06;0.70;82.13;0.90;82.16;0.70;82.28;0.70;82.29;1.00;82.40;0.70;82.48;0.70;82.56;1.00;82.58;0.90;82.63;0.70;82.64;1.00;82.75;0.70;82.81;0.70;83.04;1.00;83.04;0.90;84.24;0.90;84.41;0.90;84.74;0.90;84.93;0.90;85.38;0.90;85.81;0.90;86.29;0.90;86.74;0.90;87.94;0.90;88.11;0.90;88.28;0.90;88.46;0.90;88.66;0.90;89.08;0.90;89.51;0.90;90.01;0.90;90.44;0.90;91.64;0.90;91.81;0.90;92.14;0.90;92.29;0.90;92.74;0.90;93.19;0.90;93.66;0.90;93.81;0.90;93.98;0.90;94.16;0.90;95.03;0.90";
    case 12: return "0.65;0.90;0.99;0.90;1.18;0.90;1.56;0.90;1.71;0.90;2.08;0.90;2.38;0.90;2.74;0.90;3.13;0.90;3.44;0.90;3.83;0.90;3.98;0.90;4.38;0.90;4.53;0.90;4.88;0.90;5.23;0.90;5.59;0.90;5.93;0.90;6.31;0.90;6.66;0.90;6.78;0.90;7.18;0.90;7.33;0.90;7.69;0.90;8.01;0.90;8.38;0.90;8.71;0.90;9.06;0.90;9.43;0.90;9.58;0.90;9.99;0.90;10.13;0.90;10.48;0.90;10.83;0.90;11.16;0.90;11.26;1.00;11.74;0.90;11.94;0.90;12.08;0.90;12.24;0.90;12.41;0.90;12.96;0.90;13.33;0.90;13.68;0.90;14.01;0.90;14.61;0.90;14.74;0.90;14.89;0.90;15.06;0.90;15.21;0.90;15.76;0.90;15.91;0.90;16.11;0.90;16.27;0.90;16.39;0.90;16.73;0.90;16.87;0.90;17.44;0.90;17.57;0.90;17.76;0.90;17.92;0.90;18.09;0.90;18.52;0.90;18.71;0.90;18.89;0.90;19.07;0.90;19.21;0.90;19.39;0.90;19.69;0.90;20.14;0.90;20.33;0.90;20.49;0.90;20.67;0.90;20.89;0.90;21.37;0.90;21.51;0.90;21.71;0.90;21.86;0.90;22.19;0.90;22.47;0.90;22.51;1.00;22.86;0.90;23.02;0.90;23.19;0.90;23.36;0.90;23.51;0.90;23.67;0.90;24.07;0.90;24.23;0.90;24.41;0.90;24.57;0.90;25.22;0.90;25.27;0.90;25.39;0.90;25.57;0.90;25.61;0.90;25.71;0.90;25.91;0.90;26.02;0.90;26.07;0.90;26.26;0.90;26.27;0.90;26.41;0.90;26.59;0.90;26.67;0.90;26.74;0.90;26.79;0.90;26.92;0.90;27.01;0.90;27.09;0.90;27.29;0.90;27.36;0.90;27.47;0.90;27.67;0.90;27.71;0.90;27.83;0.90;28.02;0.90;28.19;0.90;28.54;0.90;28.71;0.90;28.87;0.90;29.04;0.90;29.24;0.90;29.41;0.90;29.46;1.00;29.94;0.90;30.12;0.90;30.32;0.90;30.51;0.90;30.61;0.90;30.81;1.00;30.81;0.90;31.32;0.90;31.51;0.90;31.69;0.90;31.84;0.90;32.02;0.90;32.22;0.90;32.26;1.00;32.59;0.90;32.76;0.90;32.92;0.90;33.11;0.90;33.31;0.90;33.49;0.90;33.82;0.90;34.16;0.90;34.34;0.90;34.54;0.90;34.71;0.90;34.84;0.90;35.02;1.00;35.02;0.90;35.56;0.90;35.72;0.90;35.96;0.90;36.14;0.90;36.29;0.90;36.46;1.00;36.97;0.90;37.16;0.90;37.32;0.90;37.51;0.90;37.71;0.90;37.87;0.90;37.89;1.00;38.19;0.90;38.36;0.90;38.54;0.90;38.72;0.90;38.92;0.90;39.09;0.90;39.46;0.90;39.49;1.00;39.84;1.00;40.04;1.00;40.21;1.00;40.41;1.00;40.54;0.80;40.56;1.00;40.74;1.00;41.25;0.80;41.28;1.00;41.46;1.00;41.59;0.80;41.64;1.00;41.81;1.00;41.95;0.80;41.98;1.00;42.14;1.00;42.32;0.80;42.62;1.00;42.64;0.80;42.84;1.00;43.01;1.00;43.01;0.80;43.19;1.00;43.32;0.80;43.37;1.00;43.54;1.00;43.69;0.80;43.87;1.00;44.02;0.80;44.07;1.00;44.24;1.00;44.40;0.80;44.43;1.00;44.59;1.00;44.74;1.00;44.75;0.80;45.11;1.00;45.14;0.80;45.45;0.80;45.49;1.00;45.67;1.00;45.82;1.00;45.82;0.80;46.02;1.00;46.16;0.80;46.17;1.00;46.36;1.00;46.52;0.80;46.84;0.80;46.86;1.00;47.06;1.00;47.21;1.00;47.24;0.80;47.39;1.00;47.54;1.00;47.57;0.80;47.74;1.00;47.92;0.80;48.24;1.00;48.27;0.80;48.44;1.00;48.59;1.00;48.64;0.80;48.77;1.00;48.97;1.00;49.00;0.80;49.14;1.00;49.32;0.80;49.46;1.00;49.65;0.80;49.66;1.00;49.82;1.00;49.97;1.00;50.06;0.80;50.14;1.00;50.31;1.00;50.39;0.80;50.47;1.00;50.75;0.80;50.77;1.00;51.09;0.80;51.12;1.00;51.32;1.00;51.45;0.80;51.49;1.00;51.66;1.00;51.81;1.00;51.82;0.80;51.99;1.00;52.16;0.80;52.49;1.00;52.52;0.80;52.67;1.00;52.84;1.00;52.86;0.80;53.02;1.00;53.21;1.00;53.24;0.80;53.39;1.00;53.59;0.80;53.89;1.00;53.92;0.80;54.06;1.00;54.24;1.00;54.27;0.80;54.41;1.00;54.57;1.00;54.67;0.80;54.77;1.00;55.00;0.80;55.12;1.00;55.31;1.00;55.34;0.80;55.51;1.00;55.66;1.00;55.72;0.80;55.81;1.00;55.99;1.00;56.06;0.80;56.39;1.00;56.41;0.80;56.72;1.00;56.77;0.80;56.94;1.00;57.07;1.00;57.14;0.80;57.26;1.00;57.42;1.00;57.51;0.80;57.61;1.00;57.84;0.80;58.12;1.00;58.19;0.80;58.31;1.00;58.47;1.00;58.54;0.80;58.64;1.00;58.82;1.00;58.89;0.80;58.99;1.00;59.24;0.80;59.52;1.00;59.62;0.80;59.72;1.00;59.90;1.00;59.94;0.80;60.07;1.00;60.24;1.00;60.30;0.80;60.44;1.00;60.62;0.80;60.76;1.00;60.94;1.00;60.99;0.80;61.11;1.00;61.31;1.00;61.35;0.80;61.44;1.00;61.62;1.00;61.69;0.80;62.04;1.00;62.08;0.90;62.09;0.80;62.41;0.90;62.45;0.80;62.47;1.00;62.74;1.00;62.78;0.90;62.82;0.80;63.11;0.90;63.12;1.00;63.14;0.80;63.54;1.00;63.82;1.00;64.19;1.00;64.69;1.00;64.89;0.90;64.92;1.00;65.24;1.00;65.24;0.90;65.61;1.00;65.64;0.90;65.96;1.00;65.96;0.90;66.13;0.90;66.31;1.00;66.53;0.90;66.62;1.00;66.68;0.90;66.99;1.00;67.01;0.90;67.35;0.90;67.37;1.00;67.72;1.00;67.76;0.90;68.06;0.90;68.07;1.00;68.41;0.90;68.44;1.00;68.74;0.90;68.79;1.00;69.12;1.00;69.44;1.00;69.81;1.00;70.16;1.00;70.52;1.00;70.54;0.90;70.86;1.00;70.86;0.90;71.22;1.00;71.25;0.90;71.57;1.00;71.60;0.90;71.76;0.90;71.92;1.00;72.11;0.90;72.27;1.00;72.29;0.90;72.64;0.90;72.64;0.80;72.66;1.00;72.87;0.80;72.99;1.00;73.01;0.90;73.02;0.80;73.36;0.80;73.37;1.00;73.54;0.80;73.71;1.00;73.72;0.80;73.89;0.80;74.06;1.00;74.07;0.80;74.24;0.80;74.40;0.80;74.41;1.00;74.59;0.80;74.76;1.00;75.05;0.80;75.12;1.00;75.24;0.80;75.42;0.80;75.46;1.00;75.60;0.80;75.84;1.00;76.14;1.00;76.14;0.80;76.51;1.00;76.82;0.80;76.87;1.00;77.04;0.80;77.22;1.00;77.24;0.80;77.42;0.80;77.57;1.00;77.91;1.00;77.92;0.80;78.14;0.80;78.27;1.00;78.32;0.80;78.66;1.00;78.90;0.80;79.01;1.00;79.12;0.80;79.35;0.80;79.36;1.00;79.69;1.00;79.70;0.80;79.85;0.80;80.07;1.00;80.15;0.80;80.42;1.00;80.65;0.80;80.77;1.00;80.84;0.80;81.02;0.80;81.12;1.00;81.19;0.80;81.35;0.80;81.49;1.00;81.52;0.80;81.72;0.80;81.82;1.00;81.89;0.80;82.17;1.00;82.32;0.80;82.51;0.80;82.56;1.00;82.87;1.00;82.99;0.80;83.14;0.80;83.26;1.00;83.59;1.00;83.67;0.80;83.82;0.80;83.94;1.00;83.99;0.80;84.15;0.80;84.29;1.00;84.64;0.80;84.67;1.00;84.68;0.90;85.18;0.90;85.69;0.80;86.05;0.80;86.08;0.90;86.39;0.80;86.59;0.90;86.74;0.80;87.11;0.80;87.44;0.80;87.46;0.90;87.80;0.80;87.99;0.90;88.14;0.80;88.49;0.80;88.84;0.80;88.88;0.90;89.19;0.80;89.59;0.90;89.93;0.90;90.29;0.90;90.84;0.90;91.71;0.90;92.23;0.90;93.09;0.90;93.64;0.90;94.53;0.90";
    case 13: return "0.23;0.80;0.43;0.80;0.63;0.80;0.82;0.80;0.98;0.80;1.17;0.80;1.37;0.80;1.60;0.80;1.77;0.80;1.88;0.80;2.02;0.80;2.20;0.80;2.40;0.80;2.67;0.80;2.83;0.80;3.02;0.80;3.23;0.80;3.48;0.80;3.63;0.80;3.80;0.80;3.92;0.80;4.13;0.80;4.35;0.80;4.58;0.80;4.70;0.80;4.90;0.80;5.10;0.80;5.37;0.80;5.48;0.80;5.63;0.80;5.77;0.80;5.95;0.80;6.20;0.80;6.42;0.80;6.55;0.80;6.75;0.80;7.00;0.80;7.23;0.80;7.37;0.80;7.50;0.80;7.65;0.80;7.88;0.80;8.07;0.80;8.33;0.80;8.45;0.80;8.68;0.80;8.93;0.80;9.12;0.80;9.27;0.80;9.42;0.80;9.53;0.80;9.75;0.80;9.97;0.80;10.22;0.80;10.33;0.80;10.55;0.80;10.80;0.80;11.02;0.80;11.15;0.80;11.32;0.80;11.43;0.80;11.67;0.80;11.87;0.80;12.10;0.80;12.23;0.80;12.43;0.80;12.68;0.80;12.88;0.80;13.05;0.80;13.22;0.80;13.43;0.80;13.63;0.80;13.82;0.80;13.98;0.80;15.15;0.80;15.36;0.80;15.56;0.80;15.89;0.80;16.49;0.80;16.73;0.80;17.06;0.80;17.83;0.80;18.36;0.80;18.88;0.80;20.62;0.70;20.77;0.70;20.93;0.70;21.07;0.70;21.18;0.70;21.33;0.70;21.48;0.70;21.60;0.70;21.65;0.80;21.75;0.70;21.80;0.80;21.92;0.70;21.93;0.80;22.25;0.80;22.43;0.80;22.46;0.90;22.73;0.90;22.93;0.90;23.21;0.90;23.34;0.90;23.53;0.90;23.79;0.90;23.99;0.90;24.14;0.90;24.29;0.90;24.39;0.90;24.59;0.90;24.83;0.90;25.08;0.90;25.21;0.90;25.44;0.90;25.66;0.90;25.93;0.90;26.04;0.90;26.19;0.90;26.46;0.90;26.69;0.90;26.93;0.90;27.08;0.90;27.33;0.90;27.54;0.90;27.73;0.90;27.88;0.90;28.01;0.90;28.16;0.90;28.34;0.90;28.60;0.90;28.81;0.90;28.94;0.90;29.14;0.90;29.39;0.90;29.51;0.90;29.66;0.90;29.81;0.90;29.94;0.90;30.09;0.90;30.28;0.90;30.48;0.90;30.69;0.90;30.84;0.90;31.08;0.90;31.28;0.90;31.49;0.90;31.63;0.90;31.78;0.90;31.88;0.90;32.11;0.90;32.33;0.90;32.56;0.90;32.69;0.90;32.93;0.90;33.15;0.90;33.36;0.90;33.48;0.90;33.63;0.90;33.76;0.90;34.01;0.90;34.23;0.90;34.48;0.90;34.59;0.90;34.78;0.90;35.03;0.90;35.24;0.90;35.38;0.90;35.51;0.90;35.68;0.90;35.86;0.90;36.09;0.90;36.29;0.90;36.46;0.90;36.64;0.90;36.89;0.90;37.03;0.90;37.14;0.90;37.29;0.90;37.52;1.00;37.73;1.00;37.95;1.00;38.18;1.00;38.31;1.00;38.50;1.00;38.78;1.00;39.10;1.00;39.25;1.00;39.38;1.00;39.81;1.00;40.18;1.00;40.36;1.00;40.63;1.00;40.95;1.00;41.10;1.00;41.23;1.00;41.48;1.00;41.70;1.00;41.95;1.00;42.08;1.00;42.28;1.00;42.56;1.00;42.80;1.00;42.95;1.00;43.08;1.00;44.48;1.00;44.66;1.00;44.78;1.00;44.95;1.00;45.18;1.00;45.41;1.00;45.65;1.00;45.80;1.00;46.01;1.00;46.23;1.00;46.55;1.00;46.68;1.00;46.83;1.00;47.30;1.00;47.65;1.00;47.86;1.00;48.13;1.00;48.41;1.00;48.58;1.00;48.71;1.00;48.97;1.00;49.16;1.00;49.43;1.00;49.56;1.00;49.76;1.00;50.03;1.00;50.30;1.00;50.43;1.00;50.61;0.90;50.84;0.90;51.06;0.90;51.33;0.90;51.46;0.90;51.64;0.90;51.88;0.90;52.03;0.90;52.13;0.90;52.28;0.90;52.39;0.90;52.73;0.70;52.92;0.70;53.33;0.70;53.48;0.70;53.73;0.70;53.93;0.70;54.08;0.70;54.28;0.70;54.43;0.70;54.57;0.70;54.72;0.70;54.87;0.70;55.28;0.70;55.47;0.70;55.63;0.70;55.78;0.70;55.93;0.70;56.22;0.70;56.35;1.00;56.78;1.00;57.03;1.00;57.17;1.00;57.40;1.00;57.60;1.00;57.85;1.00;58.00;1.00;58.12;1.00;58.22;1.00;58.63;1.00;58.93;1.00;59.07;1.00;59.27;1.00;59.52;1.00;59.73;1.00;59.87;1.00;60.07;1.00;60.55;1.00;60.77;1.00;60.95;1.00;61.17;1.00;61.32;1.00;61.45;1.00;61.67;1.00;61.93;1.00;62.67;1.00;62.80;1.00;63.02;1.00;63.23;1.00;63.60;1.00;63.73;1.00;63.88;1.00;64.08;1.00;64.32;1.00;64.55;1.00;64.70;1.00;64.90;1.00;65.15;1.00;65.47;1.00;65.59;1.00;66.00;1.00;66.20;1.00;66.40;1.00;66.53;1.00;66.78;1.00;66.98;1.00;67.25;1.00;67.38;1.00;67.52;1.00;67.63;1.00;68.05;1.00;68.25;1.00;68.38;1.00;68.65;1.00;68.77;1.00;68.92;1.00;69.18;1.00;69.43;1.00;69.60;1.00;69.80;1.00;69.98;1.00;70.13;1.00;70.30;1.00;70.52;1.00;70.70;1.00;70.80;1.00;71.13;1.00;71.30;1.00;71.42;1.00;71.60;1.00;71.82;1.00;72.03;1.00;72.20;1.00;72.38;1.00;72.60;1.00;72.85;1.00;72.97;1.00;73.12;1.00;73.25;1.00;73.48;1.00;73.67;1.00;73.90;1.00;74.03;1.00;74.25;1.00;74.50;1.00;74.72;1.00;74.83;1.00;75.02;1.00;75.30;1.00;75.57;1.00;75.77;1.00;75.92;1.00;76.13;1.00;76.28;1.00;76.42;1.00;76.58;1.00;76.73;1.00;76.92;1.00;77.63;1.00;77.78;1.00;78.02;1.00;78.23;1.00;78.60;1.00;78.75;1.00;78.88;1.00;79.08;1.00;79.30;1.00;79.55;1.00;79.68;1.00;79.92;1.00;80.12;1.00;80.45;1.00;80.58;1.00;80.95;1.00;81.15;1.00;81.37;1.00;81.50;1.00;81.63;1.00;81.75;1.00;82.00;1.00;82.23;1.00;82.38;1.00;82.52;1.00;82.78;1.00;83.00;1.00;83.25;1.00;83.38;1.00;83.60;1.00;83.78;1.00;83.90;1.00;84.10;1.00;84.38;1.00;84.53;1.00;84.72;1.00;84.90;1.00;85.10;1.00;85.27;1.00;85.48;1.00;85.75;1.00;86.08;1.00;86.20;1.00";
    case 14: return "0.25;0.90;0.43;0.90;0.88;0.90;1.00;0.90;1.13;0.90;1.38;0.90;2.05;0.90;2.34;0.90;2.72;0.90;2.85;0.90;2.95;0.90;3.50;0.90;3.68;0.90;3.82;0.90;3.98;0.90;4.18;0.90;4.60;0.90;4.72;0.90;4.87;0.90;5.15;0.90;5.90;0.80;6.15;0.80;6.26;0.80;6.38;0.80;6.50;0.80;6.58;0.80;6.68;0.80;6.81;0.80;7.10;0.80;7.60;1.00;7.70;0.90;7.92;0.90;8.48;0.90;8.78;0.90;8.88;0.90;9.00;0.90;9.10;0.90;9.18;0.90;9.57;0.90;9.78;0.90;10.32;0.90;10.42;0.90;10.52;0.90;11.48;0.90;11.51;1.00;11.68;0.90;12.45;0.90;12.58;0.90;12.70;0.90;12.82;0.90;12.90;0.90;13.58;0.90;14.15;0.90;14.27;0.90;15.23;0.90;15.38;0.90;15.57;0.90;15.68;0.90;15.82;0.90;15.92;0.90;16.20;0.90;16.67;0.90;17.13;0.90;17.23;0.90;17.37;0.90;17.48;0.90;17.60;0.90;17.70;0.90;17.85;0.90;18.15;0.90;18.58;0.90;18.93;1.00;19.05;0.90;19.23;0.90;19.80;0.90;20.13;0.90;20.27;0.90;20.37;0.90;20.47;0.90;20.77;0.90;21.10;0.80;21.28;0.80;21.45;0.80;21.51;0.80;21.63;0.80;21.75;0.80;21.86;0.80;22.01;0.80;22.25;0.80;22.70;1.00;22.76;0.70;22.78;0.70;22.83;0.90;22.88;0.70;23.00;0.70;23.10;0.90;23.57;0.90;23.70;0.90;23.83;0.90;24.07;0.90;24.30;0.90;24.50;0.90;24.70;0.90;24.95;0.90;25.53;0.70;25.63;0.90;25.66;0.70;25.71;0.70;25.83;0.70;25.95;0.70;26.00;0.70;26.65;0.90;26.88;0.90;27.10;0.90;27.37;0.90;27.60;0.90;27.85;0.90;28.03;0.90;28.30;0.90;28.50;0.90;28.75;0.90;29.25;0.70;29.31;0.70;29.40;0.70;29.43;0.90;29.51;0.70;29.61;0.70;29.71;0.70;29.76;0.70;29.93;0.70;30.00;0.70;30.38;0.90;30.62;0.90;31.04;0.90;31.15;0.90;31.28;0.90;31.53;0.90;31.80;0.90;32.02;0.90;32.28;0.90;32.48;0.90;33.03;0.70;33.11;0.70;33.20;0.90;33.21;0.70;33.30;0.70;33.40;0.70;33.51;0.70;34.15;0.90;34.35;0.90;34.88;0.90;35.05;0.90;35.13;0.90;35.37;0.90;35.57;0.90;35.85;0.90;36.05;0.90;36.30;0.90;36.53;0.90;36.78;0.90;37.05;0.90;37.27;0.90;37.48;0.90;37.72;0.90;37.83;1.00;37.95;0.90;38.67;0.90;38.77;1.00;38.82;0.90;38.93;0.90;39.15;0.90;39.25;1.00;39.42;0.90;39.60;0.90;39.73;1.00;39.82;0.90;40.03;0.90;40.58;0.90;40.65;1.00;40.70;0.90;40.87;0.90;41.05;0.90;41.08;1.00;41.28;0.90;41.50;0.90;41.60;1.00;41.73;0.90;41.97;0.90;42.43;0.90;42.52;1.00;42.60;0.90;42.72;0.90;42.93;0.90;43.03;1.00;43.17;0.90;43.38;0.90;43.48;1.00;43.63;0.90;43.82;0.90;44.40;0.90;44.45;1.00;44.53;0.90;44.82;0.90;44.93;1.00;45.27;0.90;45.38;1.00;45.50;0.90;45.78;0.90;45.98;0.90;46.32;1.00;46.67;0.90;46.83;1.00;46.92;0.90;47.17;0.90;47.28;1.00;47.37;0.90;47.65;0.90;47.87;0.90;48.13;0.90;48.20;1.00;48.33;0.90;48.57;0.90;48.72;1.00;48.82;0.90;49.07;0.90;49.12;1.00;49.28;0.90;49.52;0.90;49.75;0.90;50.08;1.00;50.43;0.90;50.60;1.00;50.68;0.90;50.92;0.90;51.03;1.00;51.17;0.90;51.42;0.90;51.65;0.90;52.00;1.00;52.35;0.90;52.47;1.00;52.58;0.90;53.03;0.90;53.30;0.90;53.80;0.90;54.05;0.90;54.27;0.90;54.52;0.90;54.95;0.90;55.22;0.90;55.70;0.90;55.78;0.90;55.92;0.90;56.03;0.90;56.15;0.90;56.20;0.70;56.25;0.90;56.26;0.70;56.36;0.70;56.40;0.90;56.48;0.70;56.87;0.90;57.10;0.90;57.62;0.90;57.82;0.90;58.07;0.90;58.25;0.90;58.75;0.90;58.97;0.90;59.43;0.90;59.52;0.90;59.65;0.90;59.78;0.90;59.92;0.90;59.98;0.70;60.01;0.70;60.05;0.90;60.13;0.70;60.18;0.90;60.65;0.90;60.77;0.90;60.92;0.90;61.02;0.90;61.15;0.90;61.23;0.90;61.37;0.90;61.52;0.90;61.98;0.90;62.48;0.90;62.60;0.90;62.75;0.90;62.83;0.90;63.02;0.90;63.13;0.90;63.28;0.90;63.42;0.90;63.90;0.90;64.42;0.90;64.62;0.90;65.10;0.90;65.20;0.90;65.62;0.90;65.65;0.70;65.73;0.70;65.81;0.70;65.85;0.90;66.28;0.90;66.50;0.90;67.10;0.90;67.38;0.90;67.50;0.90;67.63;0.90;67.75;0.90;67.85;0.90;68.07;1.00;68.17;0.90;68.43;0.90;68.82;0.90;68.95;0.90;69.00;1.00;69.07;0.90;69.37;0.90;69.50;1.00;69.65;0.90;69.88;0.90;69.93;1.00;70.10;0.90;70.33;0.90;70.80;0.90;70.88;1.00;70.93;0.90;71.07;0.90;71.28;0.90;71.38;1.00;71.48;0.90;71.72;0.90;71.83;1.00;71.98;0.90;72.22;0.90;72.70;0.90;72.77;0.90;72.78;1.00;72.88;0.90;73.15;0.90;73.26;1.00;73.35;0.90;73.58;0.90;73.73;1.00;73.88;0.90;74.05;0.90;74.55;0.90;74.63;1.00;74.65;0.90;74.77;0.90;75.02;0.90;75.15;1.00;75.53;0.90;75.65;1.00;75.73;0.90;75.97;0.90;76.45;0.90;76.58;0.90;76.73;0.90;76.97;0.90;77.15;0.90;77.40;0.90;77.63;0.90;77.75;0.90;77.90;0.90;77.97;0.90;78.13;0.90;78.22;0.90;78.33;0.90;78.45;0.90;78.57;0.90;79.05;0.90;79.55;0.90;79.73;0.90;80.22;0.90;80.32;0.90;80.45;0.90;80.73;0.90;80.92;0.90;81.18;0.90";
    case 15: return "0.20;0.80;0.50;0.80;1.05;0.80;1.23;0.80;1.58;0.80;1.98;0.80;2.55;0.80;2.72;0.80;3.03;0.80;3.42;0.80;4.02;0.80;4.15;0.80;4.50;0.80;4.90;0.80;5.48;0.80;5.63;0.80;5.98;0.80;6.37;0.80;6.90;0.80;7.05;0.80;7.43;0.80;7.82;0.80;8.38;0.80;8.52;0.80;8.90;0.80;9.27;0.80;9.83;0.80;9.98;0.80;10.39;0.80;10.77;0.80;11.30;0.90;11.38;0.80;11.45;0.90;11.53;0.80;11.61;0.90;11.80;0.90;12.18;0.90;12.56;0.90;12.71;0.90;13.03;0.90;13.07;0.70;13.12;0.70;13.24;0.70;13.37;0.70;13.42;0.70;13.46;0.70;13.56;0.70;13.67;0.70;13.98;0.90;14.71;0.90;15.06;0.90;15.46;0.90;15.65;0.90;16.03;0.90;16.41;0.90;16.60;0.90;16.76;0.90;17.15;0.90;17.33;0.90;17.71;0.90;18.05;0.90;18.45;0.90;18.60;0.90;18.94;0.90;19.86;0.90;19.92;0.70;19.99;0.70;20.06;0.70;20.12;0.70;20.22;0.70;20.29;0.70;20.39;0.70;20.41;0.70;20.63;0.90;20.98;0.90;21.40;0.90;21.55;0.90;21.91;0.90;22.28;0.90;22.46;0.90;22.65;0.90;23.05;0.90;23.21;0.90;23.58;0.90;23.75;0.90;23.93;0.90;24.13;0.90;24.48;0.90;24.86;0.90;24.94;0.70;24.99;0.70;25.06;0.70;25.16;0.70;25.26;0.70;25.41;0.70;25.73;0.90;25.74;0.70;25.87;0.70;25.95;0.70;26.04;0.70;26.12;0.70;26.21;0.70;26.46;0.90;26.85;0.90;27.25;0.90;27.43;0.90;27.78;0.90;28.20;0.90;28.36;0.90;28.56;0.90;28.73;0.90;28.93;0.90;29.10;0.90;29.28;0.90;29.45;1.00;29.45;0.90;29.86;0.90;30.23;0.90;30.40;0.90;30.74;0.70;30.75;0.90;30.85;0.70;30.92;0.70;31.00;0.70;31.07;0.70;31.16;0.70;31.27;0.70;31.36;0.70;31.47;0.70;31.52;0.70;31.61;0.70;31.69;0.70;31.79;0.70;31.82;0.70;31.96;0.90;32.40;0.90;32.56;0.90;32.74;0.90;33.11;0.90;33.53;0.90;33.71;0.90;34.08;0.90;34.26;0.90;34.63;0.90;35.00;0.90;35.36;0.90;35.75;0.90;36.11;0.90;36.30;0.90;36.64;0.90;36.64;0.70;36.71;0.70;36.77;0.70;36.89;0.70;36.92;0.70;37.12;0.70;37.19;0.70;37.32;0.70;37.36;0.70;37.44;0.70;37.47;0.70;37.90;0.90;38.28;0.90;38.45;0.90;38.60;0.90;39.03;0.90;39.38;0.90;39.55;0.90;39.96;0.90;40.13;0.90;40.54;0.90;40.90;0.90;41.22;1.00;41.25;0.90;41.60;0.90;41.96;0.90;42.15;0.90;42.53;0.90;42.54;0.70;42.62;0.70;42.77;0.70;42.84;0.70;43.04;0.70;43.07;0.70;43.16;0.70;43.21;0.70;43.35;0.70;43.80;0.90;44.20;0.90;44.36;0.90;44.55;0.90;44.95;0.90;45.31;0.90;45.50;0.90;45.85;0.90;46.00;0.90;46.40;0.90;46.73;0.90;47.13;0.90;47.50;0.90;47.91;0.90;48.05;0.90;48.41;0.90;48.51;0.70;48.61;0.70;48.69;0.70;48.77;0.70;48.92;0.70;49.01;0.70;49.07;0.70;49.18;0.70;49.70;0.90;50.08;0.90;50.23;0.90;50.41;0.90;50.83;0.90;51.20;0.90;51.38;0.90;51.76;0.90;51.91;0.90;52.30;0.90;52.68;0.90;52.98;1.00;53.03;0.90;53.20;0.90;53.38;0.90;53.56;0.90;53.76;0.90;53.93;0.90;54.13;0.90;54.31;0.90;54.66;0.90;54.86;0.90;55.06;0.90;55.43;0.90;55.61;0.90;55.96;0.90;55.97;0.70;56.04;0.70;56.14;0.70;56.19;0.70;56.30;0.70;56.42;0.70;56.50;0.70;56.57;0.70;56.68;0.90;56.86;0.90;57.05;0.90;57.23;0.90;57.44;0.90;57.63;0.90;57.81;0.90;58.01;0.90;58.35;0.90;58.55;0.90;58.91;0.90;58.95;0.70;59.05;0.70;59.17;0.70;59.26;0.70;59.34;0.70;59.61;0.90;59.80;0.90;60.00;0.90;60.20;0.90;60.56;0.90;60.71;0.90;60.98;0.90;61.32;0.90;61.46;0.90;61.84;0.70;61.85;0.90;61.89;0.70;61.99;0.70;62.06;0.70;62.14;0.70;62.22;0.70;62.31;0.70;62.52;0.70;62.59;0.70;62.81;0.90;62.99;0.90;63.33;0.90;63.51;0.90;63.70;0.90;63.88;0.90;64.20;0.90;64.40;0.90;64.81;0.90;64.82;1.00;64.96;0.90;65.16;0.90;65.35;0.90;65.73;0.90;65.90;0.90;66.11;0.90;66.46;0.90;66.63;0.90;66.83;0.90;67.33;0.90;67.51;0.90;67.73;0.90;67.96;0.90;68.15;0.90;68.31;0.90;68.66;0.90;69.03;0.90;69.06;0.70;69.14;0.70;69.22;0.70;69.26;0.70;69.45;0.70;69.59;0.70;69.66;0.70;69.74;0.70;69.82;0.70;69.86;0.70;69.92;0.70;70.68;0.90;70.88;0.90;71.06;0.90;71.26;0.90;71.45;0.90;71.63;0.90;71.81;0.90;72.00;0.90;72.18;0.90;72.37;0.90;72.53;0.90;72.73;0.90;72.90;0.90;73.63;0.90;73.83;0.90;74.01;0.90;74.20;0.90;74.53;0.90;74.93;0.90;75.26;0.90;75.65;0.90;75.83;0.90;76.21;0.90;76.53;1.00;76.58;0.90;76.96;0.90;77.35;0.90;77.53;0.90;77.90;0.90;77.97;0.70;78.09;0.70;78.16;0.70;78.27;0.70;78.34;0.70;78.42;0.70;78.51;0.70;78.67;0.70;79.15;0.90;79.55;0.90;79.71;0.90;79.90;0.90;80.28;0.90;80.66;0.90;80.84;0.90;81.21;0.90;81.40;0.90;81.78;0.90;82.15;0.90;82.55;0.90;82.89;0.90;83.23;0.90;83.40;0.90;83.73;0.90;83.76;0.70;83.87;0.70;83.94;0.70;84.01;0.70;84.09;0.70;84.26;0.70;84.32;0.70;84.42;0.70;84.49;0.70;85.03;0.90;85.41;0.90;85.58;0.90;85.76;0.90;86.11;0.90;86.51;0.90;86.70;0.90;87.05;0.90;87.23;0.90;87.60;0.90;88.00;0.90;88.27;0.70;88.32;1.00;88.34;0.70;88.36;0.90;88.42;0.70;88.52;0.70;88.61;0.70;88.69;0.70;88.79;0.70;88.94;0.70;89.22;0.70;89.34;0.70";
    case 16: return "0.20;0.90;0.63;0.70;0.83;0.70;1.06;0.70;1.27;0.90;1.28;0.70;1.51;0.70;1.75;0.70;1.88;0.90;1.93;0.70;2.16;0.70;2.38;0.70;2.63;0.70;2.83;0.70;3.00;0.90;3.08;0.70;3.31;0.70;3.55;0.70;3.65;0.90;3.73;0.70;3.93;0.90;3.96;0.70;4.16;0.70;4.27;0.90;4.40;0.70;4.61;0.70;4.77;0.90;4.81;0.70;4.98;0.90;5.06;0.70;5.30;0.70;5.42;0.90;5.53;0.70;5.76;0.70;5.77;0.90;5.96;0.70;6.08;0.90;6.20;0.70;6.41;0.70;6.55;0.90;6.68;0.70;6.75;0.90;6.88;0.70;7.13;0.70;7.20;0.90;7.33;0.70;7.55;0.70;7.71;0.70;7.93;0.70;8.18;0.70;8.30;0.90;8.40;0.70;8.63;0.70;8.86;0.70;8.97;0.90;9.06;0.70;9.31;0.70;9.55;0.70;9.75;0.70;9.95;0.70;10.12;0.90;10.16;0.70;10.41;0.70;10.63;0.70;10.77;0.90;10.81;0.70;11.06;0.70;11.08;0.90;11.30;0.70;11.40;0.90;11.55;0.70;11.75;0.70;11.93;0.90;11.95;0.70;12.13;0.90;12.16;0.70;12.40;0.70;12.57;0.90;12.61;0.70;12.86;0.70;12.90;0.90;13.08;0.70;13.15;0.90;13.28;0.70;13.53;0.70;13.67;0.90;13.75;0.70;13.85;0.90;13.96;0.70;14.25;0.70;14.28;1.00;14.33;0.90;14.68;0.90;14.95;0.70;15.00;0.90;15.10;0.70;15.17;0.90;15.20;0.70;15.32;0.90;15.36;0.70;15.50;0.90;15.53;0.70;15.70;0.90;15.77;0.90;16.15;0.90;16.43;0.90;16.56;0.70;16.70;0.70;16.77;0.90;16.80;0.70;16.92;0.90;16.95;0.70;17.02;0.90;17.11;0.70;17.25;0.90;17.28;0.70;17.43;0.90;17.57;0.90;17.75;0.70;17.83;1.00;17.88;0.90;18.05;0.70;18.18;0.90;18.35;0.70;18.48;0.70;18.55;0.90;18.61;0.70;18.70;0.90;18.84;0.90;18.85;0.70;19.03;0.70;19.05;0.90;19.20;0.70;19.25;0.90;19.53;0.70;19.72;0.90;19.85;0.70;20.02;0.90;20.11;0.70;20.26;0.70;20.32;0.90;20.40;0.70;20.43;0.90;20.57;0.90;20.65;0.70;20.78;0.90;20.83;0.70;21.00;0.90;21.28;0.70;21.42;1.00;21.45;0.90;21.63;0.70;21.77;0.90;21.83;0.70;21.96;0.70;22.03;0.90;22.08;0.70;22.13;0.90;22.21;0.70;22.25;0.90;22.37;0.90;22.45;0.70;22.58;0.90;22.61;0.70;22.76;0.70;22.77;0.90;22.88;0.90;23.06;0.70;23.23;0.90;23.38;0.70;23.55;0.90;23.77;0.90;23.80;0.70;23.92;0.90;23.93;0.70;24.03;0.90;24.03;0.70;24.15;0.90;24.23;0.70;24.38;0.90;24.40;0.70;24.57;0.90;24.85;0.70;24.93;1.00;24.98;0.90;25.16;0.70;25.33;0.90;25.43;0.70;25.62;0.90;25.65;0.70;25.72;0.90;25.73;0.70;25.83;0.90;25.96;0.70;26.11;0.70;26.12;0.90;26.30;0.90;26.63;0.70;26.80;0.90;26.91;0.70;27.10;0.90;27.43;0.70;27.47;0.90;27.55;0.70;27.62;0.90;27.66;0.70;27.87;0.90;28.05;0.90;28.25;0.90;28.43;0.90;28.48;1.00;28.63;0.90;28.98;0.90;29.06;0.70;29.22;0.90;29.23;0.70;29.48;0.90;29.48;0.70;29.63;0.70;29.78;0.90;30.10;0.90;30.32;0.90;30.53;0.70;30.63;0.70;30.75;0.90;30.78;0.70;30.90;0.70;30.98;0.90;31.20;0.90;31.55;0.90;31.85;0.90;32.08;0.90;32.25;0.70;32.38;0.70;32.48;0.70;32.52;0.90;32.61;0.70;32.73;0.90;32.98;0.90;33.03;0.70;33.33;0.90;33.65;0.90;33.90;0.90;34.05;0.70;34.18;0.70;34.31;0.70;34.32;0.90;34.45;0.70;34.53;0.90;34.75;0.90;35.10;0.90;35.23;0.90;35.45;0.90;35.67;0.90;35.80;0.70;35.93;0.70;36.06;0.70;36.12;0.90;36.20;0.70;36.35;0.90;36.57;0.90;36.90;0.90;37.22;0.90;37.44;0.90;37.58;0.70;37.70;0.70;37.81;0.70;37.87;0.90;37.95;0.70;38.08;0.90;38.30;0.90;38.67;0.90;38.93;0.90;39.17;0.90;39.35;0.70;39.48;0.70;39.61;0.70;39.65;0.90;39.78;0.70;39.88;0.90;40.10;0.90;40.13;0.70;40.42;0.90;40.73;0.90;40.95;0.90;41.21;0.70;41.31;0.70;41.42;0.90;41.43;0.70;41.55;0.70;41.65;0.90;41.85;0.90;42.32;0.90;42.77;1.00;42.80;0.90;43.10;0.90;43.25;0.70;43.38;0.70;43.45;0.90;43.53;0.70;43.57;0.90;43.65;0.70;43.70;0.90;43.76;0.70;43.93;0.90;43.96;0.70;44.12;0.90;44.58;0.90;44.88;0.90;45.15;0.90;45.28;0.90;45.38;0.90;45.65;0.90;45.87;0.90;46.30;1.00;46.33;0.90;46.63;0.90;46.93;0.90;47.05;0.90;47.17;0.90;47.42;0.90;47.68;0.90;48.01;0.70;48.12;0.90;48.20;0.70;48.42;0.90;48.43;0.70;48.68;0.70;48.75;0.90;48.88;0.70;49.11;0.70;49.12;0.90;49.35;0.70;49.38;1.00;49.42;0.90;49.53;0.70;49.76;0.70;49.85;1.00;49.88;0.90;49.96;0.70;50.02;1.00;50.08;0.90;50.20;0.70;50.43;0.70;50.52;0.90;50.65;0.70;50.84;0.90;50.88;0.70;50.95;0.90;51.07;0.90;51.11;0.70;51.29;0.90;51.31;0.70;51.55;0.70;51.70;0.90;51.76;0.70;51.96;0.70;51.97;0.90;52.18;0.70;52.27;0.90;52.38;0.70;52.47;0.90;52.59;0.90;52.66;0.70;52.88;0.70;52.95;1.00;53.02;0.90;53.13;0.70;53.33;0.70;53.37;1.00;53.42;0.90;53.51;0.70;53.58;1.00;53.60;0.90;53.75;0.70;53.95;0.70;54.07;0.90;54.08;1.00;54.18;0.70;54.43;0.70;54.63;0.70;54.70;0.90;54.85;0.70;54.94;0.90;55.10;0.70;55.17;0.90;55.28;0.70;55.45;0.90;55.50;0.70;55.69;0.90;55.70;0.70;55.82;0.90;55.93;0.70;55.97;0.90;56.18;0.70;56.40;0.70;56.63;0.70;56.85;0.70;56.94;0.90;56.98;1.00;57.24;0.90;57.49;0.90;57.64;0.90;57.79;0.90;58.34;0.90;58.61;0.70;58.77;1.00;58.82;0.90;58.85;0.70;58.99;0.90;59.11;0.70;59.27;0.90;59.40;0.90;59.50;0.70;59.55;0.90;59.73;0.70;59.98;0.70;60.25;0.70;60.30;0.90;60.45;0.70;60.55;0.90;60.65;0.70;60.77;0.90;60.88;0.70;61.00;0.90;61.10;0.70;61.17;0.90;61.29;0.90;61.31;0.70;61.53;0.70;61.75;0.70;61.98;0.70;62.10;0.90;62.21;0.70;62.35;0.90;62.41;0.70;62.59;0.90;62.66;0.70;62.80;0.90;62.88;0.70;62.92;0.90;63.09;0.90;63.10;0.70;63.33;0.70;63.58;0.70;63.64;0.90;63.78;0.70;63.80;0.90;64.01;0.70;64.14;0.90;64.23;0.70;64.29;0.90;64.43;0.70;64.47;0.90;64.66;0.70;64.70;0.90;64.84;0.90;64.86;0.70;65.01;0.90;65.06;0.70;65.31;0.70;65.53;0.70;65.69;0.90;65.75;0.70;65.92;0.90;65.95;0.70;66.14;0.90;66.18;0.70;66.37;0.90;66.41;0.70;66.50;0.90;66.61;0.70;66.65;0.90;66.86;0.70;67.06;0.70;67.33;0.70;67.42;0.90;67.55;0.70;67.67;0.90;67.76;0.70;67.89;0.90;67.98;0.70;68.12;0.90;68.20;0.70;68.29;0.90;68.42;0.90;68.43;0.70;68.52;0.90;68.66;0.70;68.86;0.70;69.13;0.70;69.22;0.90;69.33;0.70;69.45;0.90;69.53;0.70;69.62;0.90;69.78;0.70;69.92;0.90;69.98;0.70;70.07;0.90;70.19;0.90;70.20;0.70;70.45;0.70;70.67;0.70;70.77;0.90;70.88;0.70;70.99;0.90;71.08;0.70;71.27;0.90;71.33;0.70;71.39;0.90;71.55;0.70;71.57;0.90;71.76;0.70;71.82;0.90;71.94;0.90;71.98;0.70;72.17;0.90;72.64;0.90;72.75;0.90;72.95;1.00;73.09;0.90;73.42;0.90;73.50;0.70;73.57;0.90;73.76;0.70;73.79;0.90;73.95;0.70;74.20;0.70;74.27;0.90;74.41;0.70;74.42;0.90;74.61;0.70;74.79;0.90;74.86;0.70;75.00;0.90;75.03;0.70;75.22;0.90;75.33;0.70;75.34;0.90;75.47;0.90;75.53;0.70;75.57;0.90;75.73;0.70;75.96;0.70;76.09;0.90;76.19;0.90;76.20;0.70;76.34;0.90;76.45;0.70;76.53;1.00;76.59;0.90;76.63;0.70;76.87;0.70;77.00;0.90;77.05;0.70;77.17;0.90;77.30;0.70;77.39;0.90;77.50;0.70;77.72;0.70;77.89;0.90;77.98;0.70;78.04;0.90;78.15;0.90;78.20;0.70;78.40;0.90;78.43;0.70;78.60;0.90;78.61;0.70;78.84;0.90;78.90;0.70;78.99;0.90;79.12;0.90;79.13;0.70;79.30;0.70;79.56;0.70;79.67;0.90;79.76;0.70;79.79;0.90;79.94;0.90;80.01;0.70;80.20;0.90;80.20;0.70;80.40;0.70;80.55;0.90;80.65;0.70;80.85;0.90;80.85;0.70;81.00;0.90;81.05;0.70;81.26;0.70;81.48;0.70;81.55;0.90;81.69;0.90;81.71;0.70;81.82;0.90;81.96;0.70;82.18;0.70;82.22;0.90;82.34;0.90;82.41;0.70;82.63;0.70;82.65;0.90;82.80;0.90;82.86;0.70;83.02;0.90;83.06;0.70;83.28;0.70;83.40;0.90;83.55;0.90;83.55;0.70;83.69;0.90;83.75;0.70;83.98;0.70;84.20;0.70;84.29;0.90;84.44;0.90;84.45;0.70;84.65;0.70;84.67;0.90;84.93;0.70;85.10;0.70;85.14;0.90;85.26;0.70;85.27;0.90;85.40;0.90;85.48;0.70;85.67;0.90;85.92;0.90;86.14;0.90;86.27;0.90;86.37;0.90;86.67;0.90;87.06;0.70;87.18;1.00;87.22;0.90;87.23;0.70;87.47;0.90;87.83;0.70;88.00;0.90;88.25;0.70;88.46;0.70;88.65;0.70;88.86;0.70;89.08;0.70;89.26;0.70;89.51;0.70;89.75;0.70;89.98;0.70;90.18;0.70;90.19;0.90;90.38;0.70;90.42;0.90;90.55;0.90;90.63;0.70;90.82;0.90;90.85;0.70;90.99;0.90;91.10;0.70;91.33;0.70;91.56;0.70;91.76;0.70;92.01;0.70;92.21;0.70;92.43;0.70;92.66;0.70;92.86;0.70;93.08;0.70;93.31;0.70;93.51;0.70;93.75;0.70;93.96;0.70;94.14;0.90;94.20;0.70;94.29;0.90;94.43;0.70;94.44;0.90;94.66;0.70;94.86;0.70;94.87;0.90;95.11;0.70;95.27;0.90;95.31;0.70;95.53;0.70;95.70;0.90;95.76;0.70;95.98;0.70;96.15;0.90;96.21;0.70;96.41;0.70;96.57;0.90;96.68;0.70;96.88;0.70;97.02;0.90;97.13;0.70;97.33;0.70;97.44;0.90;97.53;0.70;97.75;0.70;97.89;0.90;97.98;0.70;98.33;0.70;98.50;0.70;98.75;0.70;98.98;0.70;99.13;0.70;99.17;0.90;99.37;0.70;99.58;0.70;99.67;0.90";
    case 17: return "0.21;0.80;0.41;0.80;0.86;0.80;1.14;0.70;1.15;0.80;1.23;0.70;1.31;0.80;1.75;0.80;1.93;0.80;2.00;0.70;2.16;0.70;2.23;0.80;2.31;0.70;2.45;0.70;2.56;0.70;2.63;0.80;3.00;0.80;3.16;0.80;3.58;0.80;3.73;0.80;4.03;0.80;4.48;0.80;4.80;0.80;4.96;0.80;5.40;0.80;5.50;0.70;5.55;0.80;5.73;0.70;5.85;0.80;5.90;0.70;6.03;0.70;6.26;0.80;6.61;0.80;6.78;0.80;7.20;0.80;7.36;0.80;7.37;0.70;7.66;0.80;7.68;0.70;7.87;0.70;8.07;0.70;8.08;0.80;8.22;0.70;8.40;0.70;8.43;0.80;8.58;0.70;8.60;0.80;9.00;0.80;9.03;0.70;9.18;0.80;9.35;0.70;9.48;0.70;9.50;0.80;9.80;0.70;9.88;0.80;9.94;0.70;10.08;0.70;10.24;0.70;10.35;0.70;10.50;0.70;10.63;0.70;10.80;0.70;10.90;1.00;11.30;0.90;11.46;0.90;11.58;0.70;11.75;0.90;11.79;0.70;11.91;0.90;12.08;0.90;12.21;0.90;12.36;0.90;12.68;0.90;12.85;0.90;13.10;0.90;13.50;0.90;13.65;0.90;13.70;0.90;13.85;0.90;13.95;0.90;14.13;0.90;14.45;0.70;14.46;0.90;14.62;0.70;14.63;0.90;14.96;0.90;15.11;0.90;15.56;0.90;15.73;0.90;15.85;0.70;15.90;0.90;16.06;0.90;16.07;0.70;16.31;0.90;17.21;1.00;17.38;0.90;17.66;1.00;17.83;0.90;18.09;0.70;18.25;0.70;18.28;0.90;18.41;0.90;18.42;0.70;18.57;0.70;18.58;0.90;18.76;0.90;18.90;0.90;18.90;0.70;19.05;0.70;19.08;0.90;19.21;0.90;19.22;0.70;19.37;0.70;19.38;0.90;19.53;0.90;19.68;0.90;20.00;0.90;20.11;0.90;20.38;0.90;20.49;0.70;20.60;0.70;20.73;0.90;20.73;0.70;20.80;0.90;20.88;0.70;20.90;0.90;21.06;0.90;21.10;0.70;21.17;0.90;21.30;0.90;21.48;0.90;21.81;0.90;21.98;0.90;22.26;0.90;22.40;0.90;22.73;0.90;22.75;0.70;22.86;0.90;22.97;0.70;23.01;0.90;23.17;0.90;23.30;0.90;23.76;0.90;23.93;0.90;24.08;0.90;24.25;0.90;24.40;0.90;24.58;0.90;24.73;0.90;24.86;0.90;25.01;0.90;25.16;0.90;25.37;0.70;25.41;1.00;25.50;0.70;25.53;0.90;25.65;0.70;25.70;0.90;25.84;0.70;25.88;0.90;26.01;0.90;26.17;0.70;26.35;0.90;26.37;0.70;26.51;0.90;26.53;0.70;26.68;0.90;26.91;0.90;27.26;0.90;27.43;0.90;27.68;0.90;27.80;0.70;27.93;0.70;27.96;0.90;28.06;0.90;28.07;0.70;28.16;0.90;28.22;0.70;28.26;0.90;28.35;0.90;28.41;0.90;28.53;0.90;28.73;0.90;29.02;0.70;29.05;0.90;29.06;1.00;29.15;0.70;29.23;0.90;29.30;0.70;29.47;0.70;29.53;0.90;29.68;0.90;29.82;0.70;29.95;0.90;29.95;0.70;30.10;0.90;30.14;0.70;30.28;0.90;30.30;0.70;30.43;0.90;30.45;0.70;30.56;0.90;30.74;0.70;30.84;1.00;30.86;0.90;30.88;0.70;31.10;0.90;32.61;1.00;32.72;0.70;32.81;0.90;32.85;0.70;33.00;0.70;33.17;0.70;33.40;0.90;33.76;0.90;34.00;0.70;34.12;0.70;34.26;0.90;34.70;0.90;34.98;0.90;35.13;0.90;35.37;0.70;35.39;1.00;35.52;0.70;35.58;0.90;35.91;0.90;36.06;0.90;36.51;0.90;36.93;0.90;37.38;0.90;37.86;0.90;38.33;0.90;38.78;0.90;39.20;0.90;39.33;0.90;39.51;0.90;39.65;0.90;40.13;0.90;40.55;0.90;40.83;0.70;40.85;1.00;41.00;0.70;41.01;0.90;41.15;0.70;41.51;0.90;41.96;0.90;42.25;0.90;42.38;0.90;42.63;1.00;42.67;0.70;42.82;0.70;42.85;0.90;42.98;0.70;43.05;0.70;43.20;0.90;43.35;0.90;43.76;0.90;44.21;0.90;44.55;0.70;44.70;0.90;44.80;0.70;45.00;0.70;45.13;0.90;45.35;0.70;45.48;0.70;45.65;0.70;45.82;0.70;45.95;0.70;46.10;0.70;46.23;0.70;46.35;0.70;46.47;0.70;46.60;0.70;46.70;0.70;46.82;0.70;46.88;0.70;47.05;0.70;47.13;0.70;47.20;0.70;47.21;1.00;48.19;0.70;48.23;0.90;48.30;0.70;48.71;0.90;48.92;0.70;49.08;0.70;49.21;0.90;49.65;0.90;49.93;1.00;50.11;0.90;50.56;0.90;51.12;0.70;51.23;0.70;51.32;0.90;51.48;0.90;51.78;0.90;51.78;0.70;51.97;0.90;52.27;0.90;52.42;0.90;52.68;0.90;52.68;0.70;52.78;0.70;52.83;0.90;52.88;0.70;53.02;0.70;53.17;0.90;53.17;0.70;53.23;0.70;53.32;0.90;53.38;0.70;53.52;0.70;53.63;0.90;53.63;0.70;53.80;0.90;54.08;0.90;54.25;0.90;54.52;0.90;54.55;0.70;54.72;0.70;54.83;0.70;54.88;1.00;54.95;0.70;55.18;0.70;55.70;0.90;55.82;0.70;55.91;1.00;55.95;0.70;56.07;0.70;56.17;0.70;56.18;1.00;56.32;0.70;56.34;1.00;56.37;0.90;56.52;0.90;56.66;1.00;56.77;0.90;56.77;0.70;56.84;1.00;56.92;0.90;57.00;1.00;57.09;0.90;57.22;0.90;57.34;1.00;57.42;0.90;57.63;1.00;57.72;0.90;57.89;1.00;58.03;0.90;58.16;1.00;58.34;0.90;58.97;1.00;58.97;0.70;59.10;0.70;59.12;0.90;59.25;0.70;60.22;0.70;60.32;0.70;60.45;0.70;60.53;0.70;60.88;0.70;61.02;0.90;61.02;0.70;61.17;0.70;61.30;0.70;61.43;0.70;61.82;0.70;61.98;0.70;62.17;0.70;62.35;0.70;62.74;0.70;63.05;0.70;63.77;0.80;64.01;0.70;64.02;0.80;64.18;0.80;64.52;0.80;64.69;0.80;64.98;0.80;65.15;0.80;65.30;0.80;65.47;0.80;65.59;0.80;65.93;0.80;66.08;0.80;66.34;0.80;66.50;0.80;66.80;0.80;66.95;0.80;67.12;0.80;67.25;0.80;67.40;0.80;67.72;0.80;67.90;0.80;68.15;0.80;68.32;0.80;68.59;0.80;68.73;0.80;68.90;0.80;69.03;0.70;69.05;0.80;69.20;0.70;69.35;0.70;69.52;0.70;69.65;0.70;69.78;0.70;69.93;0.70;70.12;0.70;70.23;0.70;70.40;0.70;70.54;0.70;70.70;0.70;70.84;0.70;70.98;0.70;71.00;0.90;71.15;0.70;71.28;0.70;71.35;0.70;71.37;0.90;71.48;0.70;71.53;0.90;71.58;0.70;71.72;0.70;71.93;0.90;72.32;0.90;72.63;0.70;72.65;1.00;72.80;0.90;72.80;0.70;72.93;0.70;73.07;0.70;73.15;0.90;73.17;0.70;73.30;0.70;73.32;0.90;73.37;0.70;73.49;0.70;73.60;0.70;73.62;0.90;73.77;0.90;73.80;0.70;73.90;0.70;74.02;0.70;74.05;0.90;74.12;0.70;74.18;0.90;74.20;0.70;74.32;0.70;74.33;0.90;74.43;0.70;74.52;1.00;74.63;0.90;74.98;0.90;75.15;0.90;75.30;0.90;75.48;0.90;75.92;0.90;76.32;0.90;76.48;0.90;76.78;0.90;76.98;0.70;77.10;0.90;77.10;0.70;77.22;0.90;77.27;0.70;77.38;0.90;77.48;0.90;77.60;0.90;77.75;0.90;77.92;0.90;78.11;1.00;78.15;0.70;78.18;0.90;78.27;0.70;78.33;0.90;78.52;0.70;78.60;0.90;78.68;0.70;78.75;0.90;78.85;0.70;78.88;0.90;79.03;0.90;79.42;0.90;79.45;0.70;79.55;0.70;79.57;0.90;79.72;0.90;80.02;0.90;80.78;0.70;80.89;1.00;80.90;0.70;80.98;0.90;81.36;1.00;81.48;0.90;81.62;0.70;81.75;0.70;81.90;0.70;81.92;0.90;82.07;0.90;82.07;0.70;82.22;0.90;82.42;0.90;82.55;0.90;82.72;0.90;82.87;0.90;83.04;0.90;83.17;0.90;83.30;0.90;83.56;1.00;83.63;0.90;83.89;0.90;84.03;0.90;84.22;0.70;84.31;0.90;84.33;0.70;84.40;0.90;84.50;0.70;84.53;0.90;84.58;0.90;84.66;0.90;84.83;0.90;85.01;0.90;85.13;0.90;85.41;1.00;85.43;0.90;85.58;0.90;85.75;0.70;85.85;0.90;85.85;0.70;85.98;0.90;86.30;0.70;86.33;0.90;86.45;0.70;86.48;0.90;86.55;0.70;86.61;0.90;86.70;0.70;86.76;0.90;86.91;0.90;87.20;0.70;87.21;0.90;87.23;1.00;87.32;0.70;87.41;0.90;87.47;0.70;87.62;0.70;87.73;0.70;87.82;0.70;87.93;0.70;88.07;0.70;88.15;0.70;88.25;0.70;88.37;0.70;88.48;0.70;88.55;0.70;88.65;0.70;88.75;0.70;88.89;0.70;89.01;1.00;89.25;0.80;89.68;0.80;90.13;0.80;90.61;0.80;91.05;0.80;91.37;0.80;91.52;0.80;91.93;0.80;92.23;0.80;92.40;0.80";
    default: return "0.00;0.00";
  }
}
  */

int (*LevelTools_artistForAudio)(int);
int LevelTools_artistForAudio_H(int ID) {
  int aToS[36] = {2, 3, 0, 4, 0, 0, 0, 1, 1, 0, 0, 1, 5, 1, 5, 5, 1};
  if (ID == -1) return 2;
  if (ID > 36) return 0;
  return aToS[ID];
}

char * (*LevelTools_nameForArtist)(int);
char * LevelTools_nameForArtist_H(int ID) {
  switch (ID) {
    case 0: return "DJVI";
    case 1: return "Waterflame";
    case 2: return "OcularNebula";
    case 3: return "ForeverBound";
    case 4: return "Step";
    case 5: return "DJ-Nate";
    case 6: return "F-777";
    case 7: return "TMM43";
    case 8: return "Kid2Will";
    case 9: return "ParagonX9";
    case 10: return "MadhouseDUDE";
    case 11: return "EnV";
    case 12: return "KzX";
    case 13: return "Bossfight";
    case 14: return "Dimrain47";
    case 15: return "Helix";
    case 16: return "Solkrieg";
    case 17: return "Jumper";
    default: return "Unknown";
  }
}
cocos2d::CCArray* getPlayLayerHazards(PlayLayer* playLayer) {
    return MEMBER_BY_OFFSET(cocos2d::CCArray*, playLayer, 0x192);
}

void (*PlayLayer_destroyPlayer)(PlayLayer*);
void PlayLayer_destroyPlayer_H(PlayLayer* self) {
  if (!noclip) {
    PlayLayer_destroyPlayer(self);
    auto audioEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
    if(MEMBER_BY_OFFSET(int, self, 0x29d) && pmh) audioEngine->pauseBackgroundMusic(); // check for practice mode
  }
  else if(noclip) {
    if (lastDeadFrame < frameCount - 1) {
            deaths++;
            if(ExtraLayer::m_flash) self->triggerRedPulse(0.5f);
            self->pulseLabelRed(deathsLabel, 0.5f);
            deathsLabel->setString(
                CCString::createWithFormat("%i deaths", deaths)->getCString()
            );
        }
        lastDeadFrame = frameCount;
    MEMBER_BY_OFFSET(cocos2d::CCArray*, self, 0x18c)->removeAllObjects();
  } 
  return;
}


void (*MenuLayer_init)(MenuLayer*);
void MenuLayer_init_H(MenuLayer* self) {
MenuLayer_init(self);
auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(MenuLayer::keyBackClicked));
menuBtn->setPosition({-25,-25});
buttonMenu->setPosition({win_size.width, win_size.height});
buttonMenu->addChild(menuBtn);
self->addChild(buttonMenu);
ExtraLayer::onLoadSettings();
iconHack = CCUserDefault::sharedUserDefault()->getBoolForKey("iconHack", false);
}

void (*PauseLayer_customSetup)(CCLayer* self);
void PauseLayer_customSetup_H(CCLayer* self) {
PauseLayer_customSetup(self);
CCDirector::sharedDirector()->getScheduler()->setTimeScale(1.0f);
ExtraLayer::saveSettingsToFile();
auto win_size = CCDirector::sharedDirector()->getWinSize();
  auto buttonMenu = CCMenu::create();
auto infobutton = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(infobutton, infobutton, self, menu_selector(ToggleHack::showAtts));
menuBtn->setPosition({-20,-20});
menuBtn->setScale(1.f);
menuBtn->_setZOrder(100);
buttonMenu->setPosition({60, win_size.height - 30});
if(extrainfo) {
buttonMenu->addChild(menuBtn);
self->addChild(buttonMenu);
}

 auto buttonMenu2 = CCMenu::create();
auto button2 = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
CCMenuItemSpriteExtra* menuBtn2 = CCMenuItemSpriteExtra::create(button2, button2, self, menu_selector(PauseLayer::onOpenMenu));
menuBtn2->setPosition({-20,-20});
menuBtn2->setScale(1.f);
menuBtn2->_setZOrder(100);
buttonMenu2->setPosition({win_size.width - 60, win_size.height - 30});
buttonMenu2->addChild(menuBtn2);
self->addChild(buttonMenu2);
}

void (*setBackgroundMusicTimeJNI)(float time);
void setBackgroundMusicTimeJNI_H(float time) {
seekBackgroundMusicTo(time * 1000);
}

void (*PlayLayer_levelComplete)(CCLayer*);
void PlayLayer_levelComplete_H(CCLayer* self) {
  if (safeMode) callFunctionFromSymbol<void (*)(CCLayer*)>("_ZN9PlayLayer6onQuitEv")(self);
  else PlayLayer_levelComplete(self);
}

bool getPlayLayerPractice(PlayLayer* playLayer);
bool getPlayLayerPractice();
void setPlayLayerPractice(PlayLayer* playLayer, bool isPractice);
void setPlayLayerPractice(bool isPractice);
cocos2d::CCArray* getPlayLayerCheckpoints(PlayLayer* playLayer);
cocos2d::CCArray* getPlayLayerCheckpoints();
cocos2d::CCPoint getCheckpointPosition(CCNode* checkpoint);
cocos2d::CCPoint getStartPos(PlayLayer* playLayer);
cocos2d::CCPoint getStartPos();

cocos2d::CCPoint getStartPos(PlayLayer* playLayer) {
    return from<CCPoint>(playLayer, 0x268);
}

cocos2d::CCPoint getCheckpointPosition(CCNode* checkpoint) {
    return from<CCPoint>(checkpoint, 0x12c);
}


void (*PlayLayer_resetLevel)(PlayLayer*);
void PlayLayer_resetLevel_H(PlayLayer* self) {
  PlayLayer_resetLevel(self);
  deaths = 0;
  deathsLabel->setString("0 deaths");
  if(noclip || speedhack) safeMode = true;
  else safeMode = false;
}
/* cocos2d::CCLabelBMFont* percentage;
  percentage = nullptr;
void UILayer::createPercentageLabel() {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();
    auto percentageLabel = CCLabelBMFont::create("0%", "bigFont.fnt");
    percentageLabel->setPosition(ccp(winSize.width / 2, winSize.height - 10));
    percentageLabel->setScale(0.5f);
    percentage = percentageLabel;
    addChild(percentageLabel, 10000);
} */

void (*PlayLayer_resume)(CCLayer*);
void PlayLayer_resume_H(CCLayer* self) {
  PlayLayer_resume(self);
  if(noclip || speedhack) safeMode = true;
  extraLayerCreated = false;
elc = false;
    }

cocos2d::CCLabelBMFont* noclipLabel;
cocos2d::CCLabelBMFont* FPSLabel;
cocos2d::CCLabelBMFont* percentageLabel;
cocos2d::CCLabelBMFont* goldenPercentageLabel;
void (*UILayer_init)(CCLayer*);
void UILayer_init_H(CCLayer* self) {
  UILayer_init(self);
  noclipLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Safe Mode")->getCString(), 
            "chatFont.fnt"
        );
        noclipLabel->setScale(0.5f);
        noclipLabel->setAnchorPoint({0.f, 0.5f});
        noclipLabel->setPosition(10,10);
        noclipLabel->setColor(ccc3(255, 0, 0));
        self->addChild(noclipLabel, 10000);

        auto win_size = CCDirector::sharedDirector()->getWinSize();
        FPSLabel = CCLabelBMFont::create(
            CCString::createWithFormat("FPS: 0")->getCString(), 
            "chatFont.fnt"
        );
        FPSLabel->setScale(1.f);
        FPSLabel->setAnchorPoint({0.f, 0.5f});
        FPSLabel->setPosition(10,win_size.height - 10);
        self->addChild(FPSLabel, 10000);

        percentageLabel = CCLabelBMFont::create(
            CCString::createWithFormat("0%")->getCString(), 
            "bigFont.fnt"
        );
        percentageLabel->setScale(0.5f);
        percentageLabel->setAnchorPoint({0.f, 0.5f});
        self->addChild(percentageLabel, 10000);

        goldenPercentageLabel = CCLabelBMFont::create(
            CCString::createWithFormat("0%")->getCString(), 
            "goldFont.fnt"
        );
        goldenPercentageLabel->setScale(0.5f);
        goldenPercentageLabel->setAnchorPoint({0.f, 0.5f});
        self->addChild(goldenPercentageLabel, 10000);

        deathsLabel = CCLabelBMFont::create(
            CCString::createWithFormat("0 deaths")->getCString(), 
            "chatFont.fnt"
        );
        deathsLabel->setScale(1.f);
        deathsLabel->setAnchorPoint({0.f, 0.5f});
        self->addChild(deathsLabel, 10000);
}


void (*SupportLayer_onEmail)(void*);
  void SupportLayer_onEmail_H(void*) {
    FLAlertLayer::create(
            nullptr,
            "Credits",
            CCString::createWithFormat("<cy>AntiMatter (Unsimply)</c>: For making update 11, 12 and 13 possible\n<cg>Gastiblast</c>: Creating the Pokemon Series \n<cl>Nikolyas</c>: Massive help with update 10 \n<cp>elektrick</c>: Making the \"nano\" logo \n<cr>Misty</c>: Patching the particles for the purple coins\n\nYou: For playing!")->getCString(),
            "OK",
            nullptr,
            600.f
        )->show();
        return;
  }

bool (*GameManager_isColorUnlocked)(GameManager*, int, bool);
bool GameManager_isColorUnlocked_H(GameManager* self, int id, bool idk) {
  return true;
}

bool (*GameManager_isIconUnlocked)(GameManager*, int, int);
bool GameManager_isIconUnlocked_H(GameManager* self, int id, int id2) {
  return true;
}


void PlayLayer::draw() {
  if(hitbox) {
    CCLayer::draw();
  CCArray* children = this->getChildren();
for (unsigned int i = 0; i < children->count(); ++i) {
    CCSprite* sprite = dynamic_cast<CCSprite*>(children->objectAtIndex(i));
if(sprite != nullptr) {
CCRect rect = sprite->boundingBox();
CCLayer::draw();
CCPoint lowerLeft = rect.origin;
CCPoint lowerRight = ccp(rect.origin.x + rect.size.width, rect.origin.y);
CCPoint upperRight = ccp(rect.origin.x + rect.size.width, rect.origin.y + rect.size.height);
CCPoint upperLeft = ccp(rect.origin.x, rect.origin.y + rect.size.height);

ccDrawColor4F(1.0f, 0.0f, 0.0f, 1.0f);

ccDrawRect(lowerLeft, upperRight);
}
}
  }
}

FPSCounter* FPSCounter::sharedCounter()
{
    if (!s_sharedCounter)
    {
        s_sharedCounter = new FPSCounter();
    }
    return s_sharedCounter;
}

void FPSCounter::update(float dt) {
        return;
      }

    int frames = 0;
  float accumulatedTime = 0.f;
  float currentFPS = 0.f;

void toggleGoldenPercentage(bool activate) {
  if(!activate) {
    percentageLabel->setVisible(true);
    goldenPercentageLabel->setVisible(false);
  } else {
    percentageLabel->setVisible(false);
    goldenPercentageLabel->setVisible(true);
  }
}
void (*PlayLayer_onUpdate)(PlayLayer* self, float dt);
void PlayLayer_onUpdate_H(PlayLayer* self, float dt) { // typo
PlayLayer_onUpdate(self, dt);
auto normalPercentage = (int)MEMBER_BY_OFFSET(float, gjlvl, 0x170);
CCDirector* director = CCDirector::sharedDirector();
CCScheduler* scheduler = director->getScheduler();
if(speedhack) scheduler->setTimeScale(ExtraLayer::m_speedhack);
else scheduler->setTimeScale(1.0f);
auto win_size = CCDirector::sharedDirector()->getWinSize();
frameCount++;
auto deaths = ExtraLayer::m_deaths;
if(deaths) {
  deathsLabel->setVisible(true);
} else {
  deathsLabel->setVisible(false);
}
cocos2d::CCUserDefault* ccdefault = cocos2d::CCUserDefault::sharedUserDefault();
if(noclipLabel != nullptr) {
    if(noclip) noclipLabel->setVisible(true);
    else noclipLabel->setVisible(false);
  }
  atts = MEMBER_BY_OFFSET(int, self, 0x2d8);
  jumps = MEMBER_BY_OFFSET(int, self, 0x2dc);
  if(hideatts) MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(false);
  else MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(true);
  extraLayerCreated = false;
elc = false;
if(fps) {
  deathsLabel->setPosition(10,win_size.height - 30);
  FPSLabel->setVisible(true);
  auto sharedCounter = FPSCounter::sharedCounter();
 accumulatedTime += dt;
       frames++;
        if (accumulatedTime >= 0.25f)
        {
            currentFPS = (float)frames / accumulatedTime;
            FPSLabel->setString(cocos2d::CCString::createWithFormat("FPS: %i", (int)currentFPS)->getCString());
            frames = 0;
            accumulatedTime = 0.0f;
        }
}
else {
  deathsLabel->setPosition(10,win_size.height - 10);
  FPSLabel->setVisible(false);
}
if(ccdefault->getBoolForKey("noParticles", false) || noParticles) {
  if(!percentageLabel->getParent() && percentageLabel != nullptr) self->addChild(percentageLabel, 10000);
  percentageLabel->setVisible(true);
  PlayerObject* player = MEMBER_BY_OFFSET(PlayerObject*, self, 0x274);
    float percent = (player->getPosition().x / MEMBER_BY_OFFSET(float, self, 0x1dc)) * 100.0; // destroyplayer
    if(percent > 100) percent = 100;
    if(percent < 0) percent = 0;
    percent = floorf(percent);
  percentageLabel->setString(CCString::createWithFormat("%i%%", (int)percent)->getCString());
  goldenPercentageLabel->setString(CCString::createWithFormat("%i%%", (int)percent)->getCString());
  if((int)percent > normalPercentage && MEMBER_BY_OFFSET(int, self, 0x29d) == 0) toggleGoldenPercentage(true);
  else toggleGoldenPercentage(false);
  GameManager* state = GameManager::sharedState();
        if(MEMBER_BY_OFFSET(int, state, 0x1a5)) {
          percentageLabel->setPosition(win_size.width - 190, win_size.height - 10); // progress bar
          goldenPercentageLabel->setPosition(win_size.width - 190, win_size.height - 10); 
        }
        else {
          percentageLabel->setPosition(win_size.width / 2, win_size.height - 10);
          goldenPercentageLabel->setPosition(win_size.width / 2, win_size.height - 10);
        }
} else {
  percentageLabel->setVisible(false);
  goldenPercentageLabel->setVisible(false);
}
}

void removeSpritesWithPNG(CCNode* parentNode, const std::string& pngFileName) {
    // 1. Get the target Texture2D object from the cache (v2.x method)
    CCTexture2D* targetTexture = CCTextureCache::sharedTextureCache()->addImage(pngFileName.c_str());

    // 2. Prepare a C++ vector to collect pointers to sprites we want to remove
    std::vector<CCSprite*> spritesToRemove;

    // 3. Iterate over the children of the parent node using v2.x CCArray methods
    CCArray* children = parentNode->getChildren();
    
    // Get the number of children
    unsigned int numChildren = children->count();

    // Iterate using a standard for loop or the specific FOREACH_OBJECT macro
    for (unsigned int i = 0; i < numChildren; i++) 
    {
        // Get the object at the index and cast it to a CCNode*
        CCNode* node = (CCNode*)children->objectAtIndex(i);

        // Check if the node is actually a CCSprite
        CCSprite* sprite = dynamic_cast<CCSprite*>(node);
        if (sprite) {
            // 4. Check if the sprite's current texture matches the target texture
            if (sprite->getTexture() == targetTexture) {
                spritesToRemove.push_back(sprite);
            }
        }
    }

    // 5. Remove all the collected sprites from their parent
    for (CCSprite* sprite : spritesToRemove) {
        // Calling removeFromParentAndCleanup(true) removes the sprite
        parentNode->removeChild(sprite, true);
    }


}

void (*LevelSettingsLayer_init)(CCLayer*, LevelSettingsObject*);
void LevelSettingsLayer_init_H(CCLayer* self, LevelSettingsObject* obj) {
  LevelSettingsLayer_init(self, obj);
/*
  auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(MenuLayer::onOptions));
menuBtn->setPosition({-20,-20});
menuBtn->setScale(1.f);
buttonMenu->setPosition({win_size.width, win_size.height});
buttonMenu->addChild(menuBtn);
self->addChild(buttonMenu); */
}

void (*CreatorLayer_init)(CCLayer*);
void CreatorLayer_init_H(CCLayer* self) {
  CreatorLayer_init(self);
  CCArray* children = self->getChildren();

    std::vector<CCNode*> labelsToRemove;
    CCObject* child;

    CCARRAY_FOREACH(children, child)
    {
        CCNode* node = static_cast<CCNode*>(child);
        CCSprite* label = dynamic_cast<CCSprite*>(node);
        if (label != nullptr)
        {
            labelsToRemove.push_back(node);
        }
    }
    for (CCNode* label : labelsToRemove)
    {
        self->removeChild(label, true);
    }
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    CCSprite* backgroundSprite = CCSprite::create("GJ_gradientBG.png");
    backgroundSprite->setPosition(ccp(winSize.width / 2.0f, winSize.height / 2.0f));
    self->addChild(backgroundSprite, -2);
    float scaleX = (winSize.width + 10.0f) / backgroundSprite->getContentSize().width;
    float scaleY = (winSize.height + 10.0f) / backgroundSprite->getContentSize().height;
    backgroundSprite->setScaleX(scaleX);
    backgroundSprite->setScaleY(scaleY);
    backgroundSprite->setOpacity(255);
    backgroundSprite->setColor(ccc3(87, 87, 255));


  auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
bool closed = CoderLayer::checkIfTheDoorInCreateLayerInitIsClosedOrNot();
if(!closed) { auto button = CCSprite::create("secretDoor_open.png");
  CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(CreatorLayer::onSecret));
menuBtn->setPosition({0,0});
menuBtn->setScale(0.5f);
menuBtn->_setZOrder(100);
int maxZ = -9999;
    CCArray* children = self->getChildren();
    for (unsigned int i = 0; i < children->count(); i++) {
        CCNode* node = (CCNode*)children->objectAtIndex(i);
        if (node->getZOrder() > maxZ) {
            maxZ = node->getZOrder();
        }
    }

    self->reorderChild(menuBtn, maxZ + 1);
buttonMenu->setPosition({win_size.width - 20, 20});
buttonMenu->addChild(menuBtn, 99999);
self->addChild(buttonMenu);
}
else {
  auto button = CCSprite::create("secretDoor_closed.png");
  CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(CreatorLayer::onSecret));
menuBtn->setPosition({0,0});
menuBtn->setScale(0.5f);
menuBtn->_setZOrder(100);
buttonMenu->setPosition({win_size.width - 20, 20});
buttonMenu->addChild(menuBtn, 99999);
self->addChild(buttonMenu);

int maxZ = -9999;
    CCArray* children = self->getChildren();
    for (unsigned int i = 0; i < children->count(); i++) {
        CCNode* node = (CCNode*)children->objectAtIndex(i);
        if (node->getZOrder() > maxZ) {
            maxZ = node->getZOrder();
        }
    }

    self->reorderChild(menuBtn, maxZ + 1);

}
}

void (*PlayLayer_onQuit)(CCLayer*);
void PlayLayer_onQuit_H(CCLayer* self) {
PlayLayer_onQuit(self);
added = false;
extraLayerCreated = false;
elc = false;
deaths = 0;
deathsLabel->setString("0 deaths");
CCDirector::sharedDirector()->setDisplayStats(false);
}

void (*CommentCell_loadFromComment)(CCLayer*, GJComment*);
void CommentCell_loadFromComment_H(CCLayer* self, GJComment* comment) {
  CommentCell_loadFromComment(self, comment);
  auto com = from<GJComment*>(self, 0x400);
  int commentID = MEMBER_BY_OFFSET(int, com, 0x130);
  CCLabelBMFont* idLabel = CCLabelBMFont::create( CCString::createWithFormat("ID: %i", commentID)->getCString(), 
            "chatFont.fnt"
        );
        self->addChild(idLabel);
}

char* (*LoadingLayer_getLoadingString)(void*);
char* LoadingLayer_getLoadingString_H(void*) {

      static const char* randArr[] = {
      "i loove gd cologne",
      "Made by our idiotic developer, nano",
      "Previously known as PokemonGDPS.",
      "are you ready to die?",
      "Loading everything we can...",
      "reading this gives bad luck...",
      "1 star low quality gdps",
      "Made on April 30th 2025.",
      "The Master Pack makes you question reality.",
      "fun fact: i use this menu loop because of evogdps",
      "PACKED with new songs!",
      "\"heavily modded gdps\" -System32",
      "I hope nobody reads this...",
      "If you ever feel like you're at the bottom of the sea, well, think about SeaGDPS.",
      "I'm seeing a bag, somewhere...",
      "Shenanigans are happening behind the screen.",
      "you AREN'T supposed to see this message.",
      "There's a vaultkeeper that gives you rewards for solving codes, i do not know what that means.",
      "i love my life",
      "You MUST be bored to play this GDPS.",
      "hey guys welcome to object fool",
      "Loading the",
      "59 crashes and 12 abandoned projects, welcome to VioletPS.",
      "Loading until i find the offsets...",
      "Waiting for planets to align",
      "Loading until the next solar eclipse",
      "Type \"funny codes\" on the vaultkeeper and you will get a reward!",
      "Don't you hear construction sounds?",
      "Invading 4 undiscovered civilizations",
      "Getting 82\% on allegiance",
      "Getting GDPS of the Month...",
      "Sorry guys, i dropped my purple paint onto the coins, hope it doesn't bother... .. .",
      "I need the fire extinguisher for the chicken!"
};
    return const_cast<char*>(randArr[random_array_index(randArr)]);
}

void selectSong(CCLayer* self, int currentSong) {
  return;
}

void (*LevelSettingsLayer_audioNext)(CCLayer*);
void (*LevelSettingsLayer_audioPrevious)(CCLayer*);

void LevelSettingsLayer_audioNext_H(CCLayer* self) {
  auto song = MEMBER_BY_OFFSET(CCLayer*, self, 0x1dc);
  int currentSong = MEMBER_BY_OFFSET(int, song, 0x138);
  if(currentSong < 18) selectSong(self, currentSong + 1);
return;
}

void LevelSettingsLayer_audioPrevious_H(CCLayer* self) {
  auto song = MEMBER_BY_OFFSET(CCLayer*, self, 0x1dc);
  int currentSong = MEMBER_BY_OFFSET(int, song, 0x138);
  if(currentSong > -1) selectSong(self, currentSong - 1);
return;
}

int (*PlayLayer_init)(CCLayer*, GJGameLevel*);
int PlayLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
  PlayLayer_init(self, lvl);
  gjlvl = lvl;
  return 1;
}

void (*PlayLayer_togglePracticeMode)(CCLayer*, bool idk);
void PlayLayer_togglePracticeMode_H(CCLayer* self, bool idk) {
  if((gjlvl == nullptr && idk) || (!pmh && idk)) {
        PlayLayer_togglePracticeMode(self, true);
        return;
    }
  if(!idk) {
        PlayLayer_togglePracticeMode(self, false);
        return;
    }
PlayLayer_togglePracticeMode(self, true);
SimpleAudioEngine* audioengine;
int audioTrack = MEMBER_BY_OFFSET(int, gjlvl, 0x148);
char* audiotitlestd = LevelTools_getAudioFilename_H(audioTrack);
CocosDenshion::SimpleAudioEngine* sharedEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
sharedEngine->playBackgroundMusic(audiotitlestd,true); 
return;
}

void (*EditLevelLayer_init)(CCLayer* self, GJGameLevel* lvl);
void EditLevelLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
EditLevelLayer_init(self, lvl);
/* auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(ToggleHack::showLevelInfo));
menuBtn->setPosition({-20,-20});
menuBtn->setScale(1.f);
menuBtn->_setZOrder(100);
buttonMenu->setPosition({50, win_size.height / 2});
buttonMenu->addChild(menuBtn);
self->addChild(buttonMenu); */
}

void (*OptionsLayer_onRate)(void);
void OptionsLayer_onRate_H(void) {
FLAlertLayer::create(
            nullptr,
            "Info",
            CCString::createWithFormat("Nothing here..\n\n :)")->getCString(),
            "OK?",
            nullptr,
            300.f
        )->show();
}

void (*InfoLayer_init)(CCLayer* self, GJGameLevel* lvl);
void InfoLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
  InfoLayer_init(self, lvl);
  auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(ToggleHack::showMoreLevelInfo));
menuBtn->setPosition({-20,-20});
menuBtn->setScale(1.f);
menuBtn->_setZOrder(100);
buttonMenu->setPosition({win_size.width - 10, win_size.height - 10});
buttonMenu->addChild(menuBtn);
self->addChild(buttonMenu);

}

void EditorUI::deleteAll() {
    CCArray* selected = MEMBER_BY_OFFSET(CCArray*, this, 0x180);
    LevelEditorLayer* editLayer = MEMBER_BY_OFFSET(LevelEditorLayer*, this, 0x1d0);
    CCArray* sections = MEMBER_BY_OFFSET(CCArray*, editLayer, 0x144);
    
    if(editLayer == nullptr || sections == nullptr || selected == nullptr) {
        return;
    }

    GameObject* definingObject = nullptr;
    if (selected->count() > 0) {
        definingObject = static_cast<GameObject*>(selected->objectAtIndex(0));
    } else {
       definingObject = MEMBER_BY_OFFSET(GameObject*, this, 0x254);
    }
  
    if (definingObject == nullptr) {
        return;
    }
    
    auto* selectedObj1Member = MEMBER_BY_OFFSET(void*, definingObject, 0x32c);
    
    if (selectedObj1Member == nullptr) {
        return;
    }

    std::vector<GameObject*> objectsToDelete;

    for (int i = 0; i < sections->count(); i++) {
        auto section = static_cast<CCArray*>(sections->objectAtIndex(i));
        if (section == nullptr) continue; 
        
        for (int j = 0; j < section->count(); j++) {
            auto currObj = static_cast<GameObject*>(section->objectAtIndex(j));
            
            if (currObj == nullptr) continue; 
            auto* currObjMember = MEMBER_BY_OFFSET(void*, currObj, 0x32c);
            if (currObjMember == selectedObj1Member) {
                objectsToDelete.push_back(currObj);
            }
        }
    }
    
    for (GameObject* obj : objectsToDelete) {
        editLayer->removeObject(obj);
    }
    
    // this->deselectAll(); 
    this->toggleDuplicateButton();
}


CCMenuItemSpriteExtra* menuDeleteAll = nullptr;
void (*EditorUI_setupDeleteMenu)(CCLayer* self);
void EditorUI_setupDeleteMenu_H(CCLayer* self) {
  EditorUI_setupDeleteMenu(self);
  auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::create("deleteAll_001.png");

if(!button || button == nullptr) return;
menuDeleteAll = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(EditorUI::deleteAll));
menuDeleteAll->setPosition({0,0});
menuDeleteAll->setScale(1.f);
CCLabelBMFont* label = CCLabelBMFont::create( CCString::createWithFormat("Delete All")->getCString(), 
            "bigFont.fnt"
        );
CCPoint point = CCPointMake(win_size.width - 100, win_size.height / 5);
buttonMenu->setPosition(point);
label->setPosition(point);
buttonMenu->addChild(menuDeleteAll);
// self->addChild(label);
if(filterOption) {
auto toggleOffSprite2 = CCSprite::create("GJ_filterBtn_002.png");
  auto toggleOnSprite2 = CCSprite::create("GJ_filterBtn_001.png");
  auto itemOff2 = CCMenuItemSprite::create(toggleOffSprite2, toggleOffSprite2, nullptr, nullptr);
auto itemOn2  = CCMenuItemSprite::create(toggleOnSprite2,  toggleOnSprite2,  nullptr, nullptr);

  auto menuFilter = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn2, itemOff2, filter),
    getMenuToggleSprite(itemOff2, itemOn2, filter),  
    self,
    menu_selector(EditorUI::setFilterType) 
);
menuFilter->setPosition({0,0});
menuFilter->setScale(1.f);
buttonMenu->setPosition({win_size.width - 27, win_size.height / 2 - 25});
buttonMenu->addChild(menuFilter);
// self->addChild(label);
self->addChild(buttonMenu, 100);
}
}

void (*EditorUI_toggleMode)(CCLayer* self, CCNode* param);
void EditorUI_toggleMode_H(CCLayer* self, CCNode* param) {
    EditorUI_toggleMode(self, param);
  auto type = MEMBER_BY_OFFSET(int, self, 0x1cc);
  if(type != 1 && menuDeleteAll != nullptr) {
    menuDeleteAll->removeFromParentAndCleanup(true);
    menuDeleteAll = nullptr;
  } else if(type == 1 && menuDeleteAll == nullptr) {
    auto buttonMenu = CCMenu::create();
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto button = CCSprite::create("deleteAll_001.png");
menuDeleteAll = CCMenuItemSpriteExtra::create(button, button, self, menu_selector(EditorUI::deleteAll));
menuDeleteAll->setPosition(175, -100);
menuDeleteAll->setScale(1.f);
buttonMenu->addChild(menuDeleteAll);
self->addChild(buttonMenu, 100);
  }
}


void* G_TARGET_FILTER_TYPE_ID = nullptr; 

void EditorUI::setFilterType() {
  CCArray* selected = MEMBER_BY_OFFSET(CCArray*, this, 0x180);

  if(filter) {
    filter = false;
    G_TARGET_FILTER_TYPE_ID = nullptr;
    return;
  } else filter = true;
    
    if(selected == nullptr) {
        return;
    }

    GameObject* definingObject = nullptr;
    if (selected->count() > 0) {
        return;
    } else {
       definingObject = MEMBER_BY_OFFSET(GameObject*, this, 0x254);
    }
    if (definingObject) {
        G_TARGET_FILTER_TYPE_ID = MEMBER_BY_OFFSET(void*, definingObject, 0x32c);
    } else {
        G_TARGET_FILTER_TYPE_ID = nullptr;
    }
}


CCArray* (*LevelEditorLayer_objectsInRect)(float, float, int, CCRect*);
CCArray* LevelEditorLayer_objectsInRect_H(float param_1, float param_2, int param_3, CCRect* param_4) {
    
    // Call the original function to get the base list of all objects within the rectangle
    CCArray* allObjectsInRect = LevelEditorLayer_objectsInRect(param_1, param_2, param_3, param_4);

    // If no filter is set, just return the original list.
    if (G_TARGET_FILTER_TYPE_ID == nullptr) {
        return allObjectsInRect;
    }

    // --- Apply the filter ---

    // Create a new CCArray to hold only the filtered objects
    CCArray* filteredArray = CCArray::create(); 
    // Cocos2d-x arrays returned by create() are usually autoreleased, which is fine here.

    for (int i = 0; i < allObjectsInRect->count(); i++) {
        GameObject* currObj = static_cast<GameObject*>(allObjectsInRect->objectAtIndex(i));
        
        if (currObj == nullptr) continue;

        // Get the "type" identifier for the current object (offset 0x32c)
        auto* currObjMember = MEMBER_BY_OFFSET(void*, currObj, 0x32c);

        // Check if the object's type matches our global filter ID
        if (currObjMember == G_TARGET_FILTER_TYPE_ID) {
            // It matches the type, add it to the filtered list
            filteredArray->addObject(currObj);
        }
    }
    
    // Return the filtered list instead of the original list.
    return filteredArray;
}
int (*EditorPauseLayer_init)(CCLayer* self, CCLayer* editor);
int EditorPauseLayer_init_H(CCLayer* self, CCLayer* editor) {
  EditorPauseLayer_init(self, editor);
  CCArray* children = self->getChildren();

    std::vector<CCNode*> labelsToRemove;
    CCObject* child;

    CCARRAY_FOREACH(children, child)
    {
        CCNode* node = static_cast<CCNode*>(child);
        CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(node);
        if (label != nullptr)
        {
            labelsToRemove.push_back(node);
        }
    }
    for (CCNode* label : labelsToRemove)
    {
        self->removeChild(label, true);
    }
  auto win_size = CCDirector::sharedDirector()->getWinSize();
  int objectCount = MEMBER_BY_OFFSET(int, editor, 0x150);
  auto objText = CCString::createWithFormat("%i/%i objects", objectCount, 16000)->getCString();
  auto objLabel = CCLabelBMFont::create(objText, "goldFont.fnt");
  objLabel->setAnchorPoint({0, 1});
  objLabel->setPosition({10, win_size.height - 5});
  objLabel->setScale(0.5f);
  self->addChild(objLabel);

  int editorDistance = floorf((MEMBER_BY_OFFSET(int, editor, 0x164) / 311.0) + 1);
  int minutes = editorDistance / 60;
  int seconds = editorDistance % 60;
  CCString* distText = nullptr;
  if(minutes == 1 && seconds != 1) distText = CCString::createWithFormat("%i minute %i seconds", 1, seconds);
  else if(minutes == 1 && seconds == 1) distText = CCString::createWithFormat("%i minute %i second", 1, 1);
  else if(minutes > 1 && seconds != 1) distText = CCString::createWithFormat("%i minutes %i seconds", minutes, seconds);
  else if(minutes > 1 && seconds == 1) distText = CCString::createWithFormat("%i minutes %i second", minutes, 1);
  else if(minutes == 0 && seconds != 1) distText = CCString::createWithFormat("%i seconds", seconds);
  else if(minutes == 0 && seconds == 1) distText = CCString::createWithFormat("%i second", 1);
  else if(minutes > 1 && seconds == 0) distText = CCString::createWithFormat("%i minutes", minutes);
  else if(minutes == 1 && seconds == 0) distText = CCString::createWithFormat("%i minute", 1);
  auto distLabel = CCLabelBMFont::create(distText->getCString(), "goldFont.fnt");
  distLabel->setAnchorPoint({0, 1});
  distLabel->setPosition({10, win_size.height - 20});
  distLabel->setScale(0.5f);
  self->addChild(distLabel);
  auto toggleOffSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite3 = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto itemOff3 = CCMenuItemSprite::create(toggleOffSprite3, toggleOffSprite3, nullptr, nullptr);
auto itemOn3  = CCMenuItemSprite::create(toggleOnSprite3,  toggleOnSprite3,  nullptr, nullptr);

  auto PMHButton = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn3, itemOff3, filterOption),
    getMenuToggleSprite(itemOff3, itemOn3, filterOption),  
    self, 
    menu_selector(ToggleHack::sf) 
);

  auto menu3 = CCMenu::create();
   auto counterLabel3 = CCLabelBMFont::create(
            CCString::createWithFormat("Select Filter")->getCString(), 
            "bigFont.fnt"
        );
        counterLabel3->setScale(0.5f);
        auto labelMenuItem3 = CCMenuItemLabel::create(
    counterLabel3, 
    self, 
    menu_selector(ExtraLayer::showSF) 
);
  PMHButton->setScale(0.8f);
  menu3->addChild(PMHButton);
  menu3->addChild(labelMenuItem3);
  menu3->setPosition({150, 25});

  menu3->alignItemsHorizontally();
  self->addChild(menu3);
  cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    
    def->setBoolForKey("filterOption", filterOption);

    def->flush();
  return 1;
}

 CCParticleSystemQuad* (*PlayLayer_createParticle)(void* self, int a1, const char* a2, int a3, tCCPositionType type);
 CCParticleSystemQuad* PlayLayer_createParticle_H(void* self, int a1, const char* a2, int a3, tCCPositionType type) {
     CCParticleSystemQuad* particles = PlayLayer_createParticle(self, a1, a2, a3, type);
     if (noParticles) particles->setVisible(false);
     return particles;
 }

void (*PauseLayer_onProgressBar)();
void PauseLayer_onProgressBar_H() {
  PauseLayer_onProgressBar();
  if (noParticles) {
  if(!percentageLabel) {
    percentageLabel = CCLabelBMFont::create(
            CCString::createWithFormat("0%")->getCString(), 
            "bigFont.fnt"
        );
        percentageLabel->setScale(0.5f);
        percentageLabel->setAnchorPoint({0.f, 0.5f});
  }
  GameManager* state = GameManager::sharedState();
  auto win_size = CCDirector::sharedDirector()->getWinSize();
  if(MEMBER_BY_OFFSET(int, state, 0x1a5)) percentageLabel->setPosition(win_size.width - 190, win_size.height - 10);
  else percentageLabel->setPosition(win_size.width / 2, win_size.height - 10);
  }
}

void (*PlayerObject_playerDestroyed)(PlayerObject*);
void PlayerObject_playerDestroyed_H(PlayerObject* self) {
  if(!noDeathEffect) PlayerObject_playerDestroyed(self);
}

void (*EditorUI_showMaxError)();
void EditorUI_showMaxError_H() {
  FLAlertLayer::create(
            nullptr,
            "Max Objects",
            CCString::createWithFormat("You cannot create more than <cy>%i</c> <cg>objects</c> in a single level.", 16000)->getCString(),
            "OK",
            nullptr,
            300.f
  )->show();
}

void (*OptionsLayer_customSetup)(OptionsLayer* self);
void OptionsLayer_customSetup_H(OptionsLayer* self) {
OptionsLayer_customSetup(self);
auto win_size = CCDirector::sharedDirector()->getWinSize();
  auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto button = CCMenuItemToggler::create(
    getToggleSprite(toggleOn, toggleOff, iconHack),
    getToggleSprite(toggleOff, toggleOn, iconHack),
    self,
    menu_selector(ToggleHack::iconHackF)
  );
  auto menu = CCMenu::create();
  menu->addChild(button);
  menu->setPosition(20, win_size.height / 2);
  menu->setScale(0.85f);
  button->setPosition(0,0);
  
  self->addChild(menu);
}

EditorUI* EditorUI::create() {
  auto ret = new EditorUI();
  return ret;
}

CCArray* selObjects = nullptr;
GameObject* selObject = nullptr;
void (*EditorUI_ccTouchMoved)(CCTouch* touch, CCEvent* event);
void EditorUI_ccTouchMoved_H(CCTouch* touch, CCEvent* event) {
EditorUI_ccTouchMoved(touch, event);
auto self = EditorUI::create();
CCArray* selected = MEMBER_BY_OFFSET(CCArray*, self, 0x180);
    if (selected->count() > 0) {
        selObjects = selected;
    } else {
       selObject = MEMBER_BY_OFFSET(GameObject*, self, 0x254);
    }
CCPoint rawViewLocation = touch->locationInView();
CCPoint touchLocation = CCDirector::sharedDirector()->convertToGL(rawViewLocation);
if(selObjects != nullptr) {
  CCObject* object = NULL;
  std::vector<cocos2d::CCPoint> initialOffsets;
  CCARRAY_FOREACH(selObjects, object) 
        {
            if (object)
            {
                GameObject* obj = dynamic_cast<GameObject*>(object);
                CCPoint offset = ccpSub(obj->getPosition(), touchLocation);
                initialOffsets.push_back(offset);
            }
} 

unsigned int index = 0;
    CCARRAY_FOREACH(selObjects, object) 
    {
        if (object && index < initialOffsets.size())
        {
            GameObject* obj = dynamic_cast<GameObject*>(object);
            CCPoint newPosition = ccpAdd(touchLocation, initialOffsets[index]);
            obj->setPosition(newPosition);
            index++;
        }
    }
}
}


extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
  // HOOK("_ZN8EditorUI12ccTouchBeganEPN7cocos2d7CCTouchEPNS0_7CCEventE", EditorUI_ccTouchMoved_H, EditorUI_ccTouchMoved);
  HOOK("_ZN8EditorUI12showMaxErrorEv", EditorUI_showMaxError_H, EditorUI_showMaxError);
  HOOK("_ZN12PlayerObject15playerDestroyedEv", PlayerObject_playerDestroyed_H, PlayerObject_playerDestroyed);
  HOOK("_ZN10PauseLayer13onProgressBarEv", PauseLayer_onProgressBar_H, PauseLayer_onProgressBar);
  // HOOK("_ZN8EditorUI19transformObjectCallEPN7cocos2d6CCNodeE", EditorUI_transformObjectCall_H, EditorUI_transformObjectCall);
  // HOOK("_ZN9PlayLayer14createParticleEiPKciN7cocos2d15tCCPositionTypeE", PlayLayer_createParticle_H, PlayLayer_createParticle);
 // HOOK("_ZN16GameStatsManager7getStatEPKc", GameStatsManager_getStat_H, GameStatsManager_getStat);
  HOOK("_ZN16GameStatsManager13incrementStatEPKci", GameStatsManager_incrementStat_H, GameStatsManager_incrementStat);
  HOOK("_ZN16EditorPauseLayer4initEP16LevelEditorLayer", EditorPauseLayer_init_H, EditorPauseLayer_init);
  HOOK("_ZN16LevelEditorLayer13objectsInRectEN7cocos2d6CCRectE", LevelEditorLayer_objectsInRect_H, LevelEditorLayer_objectsInRect);
  HOOK("_ZN8EditorUI10toggleModeEPN7cocos2d6CCNodeE", EditorUI_toggleMode_H, EditorUI_toggleMode);
  HOOK("_ZN8EditorUI15setupDeleteMenuEv", EditorUI_setupDeleteMenu_H, EditorUI_setupDeleteMenu);
  HOOK("_ZN12OptionsLayer6onRateEv", OptionsLayer_onRate_H, OptionsLayer_onRate);
  HOOK("_ZN14EditLevelLayer4initEP11GJGameLevel", EditLevelLayer_init_H, EditLevelLayer_init);

  HOOK("_ZN9PlayLayer4initEP11GJGameLevel", PlayLayer_init_H, PlayLayer_init);
  HOOK("_ZN9PlayLayer18togglePracticeModeEb", PlayLayer_togglePracticeMode_H, PlayLayer_togglePracticeMode);
  // HOOK("_ZN18LevelSettingsLayer9audioNextEv", LevelSettingsLayer_audioNext_H, LevelSettingsLayer_audioNext);
  // HOOK("_ZN18LevelSettingsLayer13audioPreviousEv", LevelSettingsLayer_audioPrevious_H, LevelSettingsLayer_audioPrevious);
  HOOK("_ZN12LoadingLayer16getLoadingStringEv", LoadingLayer_getLoadingString_H, LoadingLayer_getLoadingString);
  // HOOK("_ZN11CommentCell15loadFromCommentEP9GJComment", CommentCell_loadFromComment_H, CommentCell_loadFromComment);
  HOOK("_ZN9PlayLayer6onQuitEv", PlayLayer_onQuit_H, PlayLayer_onQuit);

  HOOK("_ZN12CreatorLayer4initEv", CreatorLayer_init_H, CreatorLayer_init);

  HOOK("_ZN18LevelSettingsLayer4initEP19LevelSettingsObject", LevelSettingsLayer_init_H, LevelSettingsLayer_init);
  // HOOK("_ZN11GameManager14isIconUnlockedEi8IconType", GameManager_isIconUnlocked_H, GameManager_isIconUnlocked);
  // HOOK("_ZN11GameManager15isColorUnlockedEib", GameManager_isColorUnlocked_H, GameManager_isColorUnlocked);

  HOOK("_ZN12SupportLayer7onEmailEv", SupportLayer_onEmail_H, SupportLayer_onEmail);

// Disabled: handler type mismatch
  HOOK("_ZN9PlayLayer13destroyPlayerEv", PlayLayer_destroyPlayer_H, PlayLayer_destroyPlayer);
  HOOK("_ZN9PlayLayer13levelCompleteEv", PlayLayer_levelComplete_H, PlayLayer_levelComplete);
  HOOK("_ZN7UILayer4initEv", UILayer_init_H, UILayer_init);
  HOOK("_ZN9PlayLayer10resetLevelEv", PlayLayer_resetLevel_H, PlayLayer_resetLevel);
  HOOK("_ZN9PlayLayer6resumeEv", PlayLayer_resume_H, PlayLayer_resume);

 HOOK("_ZN13CocosDenshion17SimpleAudioEngine22setBackgroundMusicTimeEf", setBackgroundMusicTimeJNI_H, setBackgroundMusicTimeJNI);

  HOOK("_ZN10PauseLayer11customSetupEv", PauseLayer_customSetup_H, PauseLayer_customSetup);

  HOOK("_ZN10LevelTools13getAudioTitleEi", LevelTools_getAudioTitle_H, LevelTools_getAudioTitle);
  HOOK("_ZN10LevelTools16getAudioFileNameEi", LevelTools_getAudioFilename_H, LevelTools_getAudioFilename);

   HOOK("_ZN9MenuLayer4initEv", MenuLayer_init_H, MenuLayer_init);

  HOOK("_ZN9PlayLayer6updateEf", PlayLayer_onUpdate_H, PlayLayer_onUpdate);
  
  /* HOOK("_ZN10LevelTools14getAudioStringEi", LevelTools_getAudioString_H, LevelTools_getAudioString);
  HOOK("_ZN10LevelTools14artistForAudioEi", LevelTools_artistForAudio_H, LevelTools_artistForAudio);
  HOOK("_ZN10LevelTools13nameForArtistEi", LevelTools_nameForArtist_H, LevelTools_nameForArtist); */
  
  HOOK("_ZN9LevelCell19loadCustomLevelCellEv", LevelCell_loadCustomLevelCell_H, LevelCell_loadCustomLevelCell);
  HOOK("_ZN11GJGameLevel15encodeWithCoderEP13DS_Dictionary", GJGameLevel_encodeWithCoder_H, GJGameLevel_encodeWithCoder);
  HOOK("_ZN11GJGameLevel15createWithCoderEP13DS_Dictionary", GJGameLevel_createWithCoder_H, GJGameLevel_createWithCoder);
  HOOK("_ZN14LevelInfoLayer4initEP11GJGameLevel", LevelInfoLayer_init_H, LevelInfoLayer_init);
  HOOK("_ZN11GJGameLevelD1Ev", GJGameLevel_destructor_H, GJGameLevel_destructor); 
  return JNI_VERSION_1_6;
}