#include "../vendor/cocos/cocos2dx/include/cocos2d.h"
#include "../vendor/cocos/CocosDenshion/include/SimpleAudioEngine.h"
#include <iostream>
#include <string>
#include "../vendor/other/hooking.h"
#include "../vendor/cocos/cocos2dx/platform/CCPlatformMacros.h"
#include "../vendor/robtop/GJGameLevel.hpp"
#include "../vendor/robtop/EditButtonBar.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include "../vendor/robtop/FPSCounter.hpp"
#include "../vendor/robtop/OptionsLayer.hpp"
#include "../vendor/robtop/FLAlertLayer.hpp"
#include "../vendor/robtop/GameManager.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/other/levels.h"
#include "../vendor/robtop/ButtonSprite.hpp"
#include "../vendor/robtop/AppDelegate.hpp"
#include "../vendor/robtop/VersionRequest.hpp"
#include "../vendor/robtop/HitboxLayer.hpp"
#include "../vendor/robtop/EditLevelLayer.hpp"
#include "../vendor/robtop/MenuLayer.hpp"
#include "platform/android/jni/JniHelper.h"
#include "cocos2dExt.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpClient.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpRequest.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpResponse.h"
#include <typeinfo>
#include <atomic>
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
// #include "../vendor/robtop/GJGameLevel.hpp"
#include "../vendor/robtop/EditorUI.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/GJComment.hpp"
#include "../vendor/robtop/PauseLayer.hpp"
#include "../vendor/cocos/cocos2dx/cocoa/CCDictionary.h"
#include "../vendor/cocos/cocos2dx/cocoa/CCDictionary.cpp"
#include "../vendor/cocos/cocos2dx/cocoa/CCString.h"
#include "../vendor/cocos/cocos2dx/cocoa/CCString.cpp"
#include "MemoryPatch.h"
#include <thread>
#include "../vendor/robtop/HidePauseLayer.hpp"

#define PAGE_1 743276
#define PAGE_2 743277
#define CURRENT_VERSION 14
using namespace cocos2d;
using namespace cocos2d::extension;

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
bool noRotation = false;
bool noRespawn = false;
bool noRespawnReset = false;
bool pauseLayerVisible = true;
bool hidePauseMenu = false;
bool deleteAll = false;
bool levelLength = false;
float musicVolume = 1.0f;
CCLayer* menulayer = nullptr;
bool moreEditorButtons = false;
cocos2d::CCLabelBMFont* deathsLabel;
CCLabelBMFont* updateLabel;
bool warnedOutdated = false;
bool showSong = false;
bool showEpic = false;

static JavaVM* g_vm = nullptr;

bool ExtraLayer::m_deaths = false;
bool ExtraLayer::m_flash = false;
float ExtraLayer::m_speedhack = 1.0f;
bool ExtraLayer::m_speedhackEnabled = false;
int ExtraLayer::m_currentPage = 1;
CCLayer* ExtraLayer::m_page1 = nullptr;
CCLayer* ExtraLayer::m_page2 = nullptr;
CCMenu* ExtraLayer::m_right = nullptr;
CCMenu* ExtraLayer::m_left = nullptr;
CCLayer* HidePauseLayer::m_pauseLayer = nullptr;
MenuLayer* MenuLayer::sharedLayer = nullptr;
CCLayer* ExtraLayer::m_self = nullptr;
int datedVersion = 0;
float lastSpeed = -1.0f;

bool checked = false;
std::atomic<int> MenuLayer::s_newVersion;
std::atomic<bool> MenuLayer::s_hasVersionData;

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


void openURL(const char* url) {
    JniMethodInfo t;
    if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/lib/Cocos2dxActivity", "openWebURL", "(Ljava/lang/String;)V")) {
        jstring stringArg = t.env->NewStringUTF(url);
        t.env->CallStaticVoidMethod(t.classID, t.methodID, stringArg);
        t.env->DeleteLocalRef(stringArg);
        t.env->DeleteLocalRef(t.classID);
    }
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

using namespace std;

class PatchManager {
private:
    vector<MemoryPatch> patches;
public:
    void addPatch(const char *libraryName, uintptr_t address,std::string hex){
        patches.push_back(MemoryPatch::createWithHex(libraryName,address,hex));
    }

    void Modify(){
        for(int k = 0; k < patches.size(); k++){
            patches[k].Modify();
        }
    }

    void Restore(){
        for(int k = 0; k < patches.size(); k++){
            patches[k].Restore();
        }
    }

};

#define cpatch(addr, val) addPatch("libgame.so", addr, val)

void PlayLayer::triggerRedPulse(float duration)
{
    CCSize win_size = CCDirector::sharedDirector()->getWinSize();
    CCLayerColor* redPulseLayer = CCLayerColor::create(ccc4(255, 0, 0, 80), win_size.width, win_size.height);
    redPulseLayer->setOpacity(80);
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
    CCTintTo* tintToRed = CCTintTo::create(0.f, pulseColor.r, pulseColor.g, pulseColor.b);
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
    case 25: return "Chaoz Fantasy";
    case 26: return "Phazd";
    default: return "Unknown";
  }
}

void (*SupportLayer_onEmail)(void*);
  void SupportLayer_onEmail_H(void*) {
    FLAlertLayer::create(
            nullptr,
            "Credits",
            CCString::createWithFormat("<cy>AntiMatter (Unsimply)</c>: For making updates 11-14 possible\n<cg>Gastiblast</c>: Creating the Pokemon Series \n<cl>Nikolyas</c>: Massive help with update 10 \n<cp>elektrick</c>: Making the \"nano\" logo \n<cr>Misty</c>: Patching the particles for the purple coins\n\nYou: For playing!")->getCString(),
            "OK",
            nullptr,
            600.f
        )->show();
        return;
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
    auto scene = cocos2d::CCDirector::sharedDirector()->getRunningScene();
    cocos2d::CCArray* children = scene->getChildren();
    cocos2d::CCObject* obj;

    PauseLayer* pauseLayer = nullptr;
    CCARRAY_FOREACH(children, obj) {
        if (dynamic_cast<PauseLayer*>(obj)) {
            pauseLayer = (PauseLayer*)obj;
            break;
        }
    }
      if(!pauseLayer) {
        FLAlertLayer::create(
            nullptr,
            "Error",
            CCString::createWithFormat("You're <cr>not</c> currently playing a level!")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
        return;
      }
      FLAlertLayer::create(
            nullptr,
            "Info",
            CCString::createWithFormat("<cy>Attempts:</c> %i\n<cg>Jumps:</c> %i", atts, jumps)->getCString(),
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

    void showCredits() {
      FLAlertLayer::create(
            nullptr,
            "Credits",
            CCString::createWithFormat("<cy>AntiMatter (Unsimply)</c>: For making update 11, 12 and 13 possible\n<cg>Gastiblast</c>: Creating the Pokemon Series \n<cl>Nikolyas</c>: Massive help with update 10 \n<cp>elektrick</c>: Making the \"nano\" logo \n<cr>Misty</c>: Patching the particles for the purple coins\n\nYou: For playing!")->getCString(),
            "OK",
            nullptr,
            600.f
        )->show();
    }

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
    def->setBoolForKey("noRotation", noRotation);
    def->setBoolForKey("noRespawn", noRespawn);
    def->setBoolForKey("hidePauseMenu", hidePauseMenu);
    def->setBoolForKey("deleteAll", deleteAll);
    def->setBoolForKey("filterOption", filterOption);
    def->setBoolForKey("levelLength", levelLength);
    def->setFloatForKey("musicVolume", musicVolume);
    def->setBoolForKey("moreEditorButtons", moreEditorButtons);
    def->setBoolForKey("showEpic", showEpic);
    def->setBoolForKey("showSong", showSong);
    def->flush();
    return true;
  }

  void ExtraLayer::onLoadSettings() {
    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
  noclip = def->getBoolForKey("noclip", false);
  hideatts = def->getBoolForKey("hideatts", false);
  extrainfo = def->getBoolForKey("extrainfo", false);
  extrainfo = false;
  doorClosed = def->getBoolForKey("doorClosed", false);
  filterOption = def->getBoolForKey("filterOption", false);
  fps = def->getBoolForKey("fps", false);
  noParticles = def->getBoolForKey("noParticles", false);
  noDeathEffect = def->getBoolForKey("noDeathEffect", false);
  speedhack = def->getBoolForKey("speedhack", false);
  ExtraLayer::m_deaths = def->getBoolForKey("noclipdeaths", false);
  ExtraLayer::m_flash = def->getBoolForKey("noclipflash", false);
  ExtraLayer::m_speedhack = def->getFloatForKey("speedhackInt", 1.0f);
  noRotation = def->getBoolForKey("noRotation", false);
  noRespawn = def->getBoolForKey("noRespawn", false);
  hidePauseMenu = def->getBoolForKey("hidePauseMenu", false);
  deleteAll = def->getBoolForKey("deleteAll", false);
  levelLength = def->getBoolForKey("levelLength", false);
  musicVolume = def->getFloatForKey("musicVolume", 1.0f);
  moreEditorButtons = def->getBoolForKey("moreEditorButtons", false);
  showEpic = def->getBoolForKey("showEpic", false);
  showSong = def->getBoolForKey("showSong", false);
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

void ExtraLayer::switchPages() {
  CCObject* pObj = NULL;
  if(ExtraLayer::m_currentPage == 1) {
    ExtraLayer::m_currentPage = 2;
    m_page2->setVisible(true);
    m_page2->setTouchEnabled(true);

    m_left->setVisible(true);
    m_left->setEnabled(true);

    m_right->setVisible(false);
    m_right->setEnabled(false);

    CCArray* childrenpage2 = m_page2->getChildren(); 

CCARRAY_FOREACH(childrenpage2, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(true); 
    }
  }

    m_page1->setVisible(false);
    m_page1->setTouchEnabled(false);
    CCArray* childrenpage1 = m_page1->getChildren(); 

CCARRAY_FOREACH(childrenpage1, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(false); 
    }
  }
  } else {
    ExtraLayer::m_currentPage = 1;
    m_page2->setVisible(false);
    m_page2->setTouchEnabled(false);
    CCArray* childrenpage2 = m_page2->getChildren(); 

    m_left->setVisible(false);
    m_left->setEnabled(false);

    m_right->setVisible(true);
    m_right->setEnabled(true);

CCARRAY_FOREACH(childrenpage2, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(false); 
    }
  }

    m_page1->setVisible(true);
    m_page1->setTouchEnabled(true);
    CCArray* childrenpage1 = m_page1->getChildren(); 

CCARRAY_FOREACH(childrenpage1, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(true); 
    }
  }
}
}

void ExtraLayer::toggle(CCObject* sender) {
    auto btn = static_cast<CCMenuItemToggler*>(sender);
    bool* toggleVar = static_cast<bool*>(btn->getUserData());
    if(toggleVar) *toggleVar = !*toggleVar;
}

void ExtraLayer::dummy(CCObject*) {
  return;
}

void ExtraLayer::volumeSliderCallback(CCObject* pSender, CCControlEvent controlEvent) {
    CCControlSlider* pSlider = (CCControlSlider*)pSender;
    float newVolume = pSlider->getValue();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(newVolume);
    musicVolume = newVolume;
}

void PauseLayer::toggleVisibility() {
    CCObject* child;
    CCARRAY_FOREACH(this->getChildren(), child) {
        auto node = dynamic_cast<CCNode*>(child);
        if (node) node->setVisible(false);
        auto menu = dynamic_cast<CCMenu*>(node);
                if (menu) {
                    menu->setTouchEnabled(false);
                }
    }
    auto HPL = HidePauseLayer::create(this);
    this->addChild(HPL, 1000);
}

CCNode* ExtraLayer::optionToggler(const char* display, bool* toggleVar, bool addinfo, ExtraLayerInfo::InfoType infoType) {
  auto toggleOffSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
  auto toggleOnSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
  auto itemOff = CCMenuItemSprite::create(toggleOffSprite, toggleOffSprite, nullptr, nullptr);
  auto itemOn  = CCMenuItemSprite::create(toggleOnSprite,  toggleOnSprite,  nullptr, nullptr);

auto btn = CCMenuItemToggler::create(
    getMenuToggleSprite(itemOn, itemOff, *toggleVar), 
    getMenuToggleSprite(itemOff, itemOn, *toggleVar),  
    this, 
    menu_selector(ExtraLayer::toggle) 
);

btn->setUserData(toggleVar);
btn->setAnchorPoint(ccp(0, 0.5f));
btn->setScale(0.8f);

   auto counterLabel = CCLabelBMFont::create(
            CCString::createWithFormat("%s", display)->getCString(), 
            "bigFont.fnt"
        );
  counterLabel->setScale(0.5f);
  counterLabel->setAnchorPoint(ccp(0, 0.5f));
  btn->addChild(counterLabel);

  if(addinfo) {
    auto infobutton = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(infobutton, infobutton, this, menu_selector(ExtraLayer::showInfo));
    menuBtn->setUserData((void*)infoType);
    auto btnMenu = CCMenu::create(menuBtn, NULL);
    btnMenu->ignoreAnchorPointForPosition(false);
    btnMenu->setContentSize(CCSizeZero);
    btnMenu->setPosition(ccp(0, 0));
    menuBtn->setPosition(ccp(-6.0f, (btn->getContentSize().height * 2) + 6));
    btnMenu->setScale(0.5f);
    btn->addChild(btnMenu, 10);
  }
  auto menu = CCMenu::create(btn, NULL);
  menu->setPosition(CCPointZero);

  float padding = 5.0f;
  counterLabel->setPosition(ccp(btn->getContentSize().width + padding, btn->getContentSize().height / 2));
  float totalWidth = btn->getContentSize().width + padding + (counterLabel->getContentSize().width * counterLabel->getScaleX());
  float totalHeight = MAX(btn->getContentSize().height, counterLabel->getContentSize().height);

  auto container = CCNode::create();
  float totalWidth2 = padding + counterLabel->getContentSize().width;
  container->setContentSize(CCSizeMake(totalWidth2, btn->getContentSize().height));
  container->addChild(menu);

  return container;
}

CCMenu* ExtraLayer::createOptionsMenu(const char* display, int length, SEL_MenuHandler func) {
  auto sprite = ButtonSprite::create(
    CCString::createWithFormat("%s", display)->getCString(), length, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto menu = CCMenu::create();
  auto btn = CCMenuItemSpriteExtra::create(
    sprite,
    sprite,
    this,
    func
  );

  menu->addChild(btn);
  menu->setPosition(CCPointZero);
  return menu;
}

  bool ExtraLayer::init(CCLayer* self) {

auto win_size = CCDirector::sharedDirector()->getWinSize();

    CCNode* pivot = CCNode::create();
    pivot->setPosition(ccp(win_size.width/2, win_size.height/2));
    this->addChild(pivot);
    CCNode* contentHolder = CCNode::create();
    contentHolder->setPosition(ccp(-win_size.width/2, -win_size.height/2));
    pivot->addChild(contentHolder);
    pivot->setScale(0.1f);
    CCScaleTo* scaleUp = CCScaleTo::create(0.5f, 1.0f);
    CCEaseElasticOut* ease = CCEaseElasticOut::create(scaleUp, 0.5f);
    pivot->runAction(ease);

    CCNode* leftParent = CCNode::create();
     CCLayerColor *overlay = CCLayerColor::layerWithColor(ccc4(0, 0, 0, 127),
                                                win_size.width,
                                                 win_size.height);
    overlay->setPosition(0,0);
    contentHolder->addChild(overlay);
    contentHolder->addChild(leftParent);
    leftParent->setPosition(win_size.width / 2, win_size.height / 2);


    CCRect rect = CCRectMake(0, 0, 80, 80);
    cocos2d::extension::CCScale9Sprite* panel = cocos2d::extension::CCScale9Sprite::create("GJ_square01-hd.png", rect);
    panel->setContentSize(CCSizeMake(win_size.width - 75, win_size.height - 25));
    leftParent->addChild(panel);
    panel->setPosition({0.f, 0.f});

    CCLayer* mainLayoutLayer = CCLayer::create();
    mainLayoutLayer->setAnchorPoint(ccp(0.0f, 0.0f));


auto buttonMenu = CCMenu::create();
auto button = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, this, menu_selector(ExtraLayer::keyBackClicked));
menuBtn->setPosition({-50,-25});
buttonMenu->setPosition({win_size.width, win_size.height});
buttonMenu->addChild(menuBtn);
contentHolder->addChild(buttonMenu);

auto rightButtonMenu = CCMenu::create();
auto rightButton = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
rightButton->setFlipX(true);
CCMenuItemSpriteExtra* rightMenuBtn = CCMenuItemSpriteExtra::create(rightButton, rightButton, this, menu_selector(ExtraLayer::switchPages));
rightMenuBtn->setPosition({0,0});
rightButtonMenu->setPosition({win_size.width - 20, win_size.height / 2});
rightButtonMenu->addChild(rightMenuBtn);
m_right = rightButtonMenu;
contentHolder->addChild(rightButtonMenu);

auto leftButtonMenu = CCMenu::create();
auto leftButton = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
CCMenuItemSpriteExtra* leftMenuBtn = CCMenuItemSpriteExtra::create(leftButton, leftButton, this, menu_selector(ExtraLayer::switchPages));
leftMenuBtn->setPosition({0,0});
leftButtonMenu->setPosition({20, win_size.height / 2});
leftButtonMenu->addChild(leftMenuBtn);
m_left = leftButtonMenu;
contentHolder->addChild(leftButtonMenu);

auto titleLabel = CCLabelBMFont::create(
            CCString::createWithFormat("VioletMod")->getCString(), 
            "goldFont-hd.fnt"
        );

  titleLabel->setPosition({win_size.width / 2, win_size.height - 35});
  titleLabel->setScale(1.0f);
  contentHolder->addChild(titleLabel);

  int padding = 35;
  int startX = 100;
  int endX = startX * 3 + 10;

  auto noclipBtn = this->optionToggler("No-Clip", &noclip);
  noclipBtn->setPosition(startX, win_size.height - 100);
  auto noclipBtnMore = this->createOptionsMenu("+", 50, menu_selector(ExtraLayer::onNoclipOptions));
  noclipBtnMore->setPosition(ccp(noclipBtn->getPosition().x + noclipBtn->getContentSize().width, noclipBtn->getPosition().y));

  auto speedhackBtn = this->optionToggler("Speedhack", &speedhack, true, ExtraLayerInfo::InfoType::PEE);
  speedhackBtn->setPosition(startX, win_size.height - 100 - padding);
  auto speedhackBtnMore = this->createOptionsMenu("+", 50, menu_selector(ExtraLayer::onPEEOptions));
  speedhackBtnMore->setPosition(ccp(noclipBtnMore->getPosition().x, speedhackBtn->getPosition().y));

  auto percentageBtn = this->optionToggler("Display Percentage", &noParticles);
  percentageBtn->setPosition(startX, win_size.height - 100 - (padding * 2));

  auto noDeathEffectBtn = this->optionToggler("No Death Effect", &noDeathEffect);
  noDeathEffectBtn->setPosition(startX, win_size.height - 100 - (padding * 3));

  auto hideAttemptsBtn = this->optionToggler("Hide Attempts", &hideatts);
  hideAttemptsBtn->setPosition(startX, win_size.height - 100 - (padding * 4));

  auto playSessionInfoBtn = this->optionToggler("Hide Pause Menu", &hidePauseMenu, true, ExtraLayerInfo::InfoType::HPM);
  playSessionInfoBtn->setPosition(endX, win_size.height - 100);
  
  auto displayFPSBtn = this->optionToggler("Display FPS", &fps, true, ExtraLayerInfo::InfoType::FPS);
  displayFPSBtn->setPosition(endX, win_size.height - 100 - (padding));

  auto noRotationBtn = this->optionToggler("No Rotation", &noRotation);
  noRotationBtn->setPosition(endX, win_size.height - 100 - (padding * 2));

  auto noRespawnBtn = this->optionToggler("No Respawn Time", &noRespawn, true, ExtraLayerInfo::InfoType::NRT);
  noRespawnBtn->setPosition(endX, win_size.height - 100 - (padding * 3));

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

auto creditsBtnSprite = ButtonSprite::create(
    "Credits", 100, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto creditsBtn = CCMenuItemSpriteExtra::create(
    creditsBtnSprite,
    creditsBtnSprite,
    this,
    menu_selector(ToggleHack::showCredits)
  );

auto creditsMenu = CCMenu::create();
creditsMenu->addChild(creditsBtn);
creditsMenu->setPosition(ccp(100, 35));

auto sessionBtnSprite = ButtonSprite::create(
    "Session Info", 100, 0, 1, false, "goldFont.fnt", "GJ_button_01-hd.png"
  );
  
  auto sessionBtn = CCMenuItemSpriteExtra::create(
    sessionBtnSprite,
    sessionBtnSprite,
    this,
    menu_selector(ToggleHack::showAtts)
  );

auto sessionMenu = CCMenu::create();
sessionMenu->addChild(sessionBtn);
sessionMenu->setPosition(ccp(100, 35 + sessionBtnSprite->getContentSize().height + 10));

auto filterBtn = this->optionToggler("Select Filter", &filterOption, true, ExtraLayerInfo::InfoType::SF);
filterBtn->setPosition(startX, win_size.height - 100);

auto deleteAllBtn = this->optionToggler("Delete All", &deleteAll, true, ExtraLayerInfo::InfoType::DA);
deleteAllBtn->setPosition(startX, win_size.height - 100 - padding);

auto levelLengthBtn = this->optionToggler("Show Level Length", &levelLength, true, ExtraLayerInfo::InfoType::LL);
levelLengthBtn->setPosition(startX, win_size.height - 100 - (padding * 2));

auto MEBBtn = this->optionToggler("More Editor Buttons", &moreEditorButtons, true, ExtraLayerInfo::InfoType::MEB);
MEBBtn->setPosition(startX, win_size.height - 100 - (padding * 3));

auto epicBtn = this->optionToggler("Display Epic Icons", &showEpic);
epicBtn->setPosition(endX, win_size.height - 100);

auto songBtn = this->optionToggler("Display Audio Track", &showSong, true, ExtraLayerInfo::InfoType::DAT);
songBtn->setPosition(endX, win_size.height - 100 - padding);


CCControlSlider* pSlider = CCControlSlider::create(
        "slidergroove.png", 
        "00_transparent.png", 
        "sliderthumb.png"
    );
    pSlider->setMinimumValue(0.0f);
    pSlider->setMaximumValue(1.0f);
    float currentVol = CocosDenshion::SimpleAudioEngine::sharedEngine()->getBackgroundMusicVolume();
    pSlider->setValue(currentVol);
    pSlider->setPosition(ccp(win_size.width / 2, 30));
    pSlider->addTargetWithActionForControlEvents(
        this, 
        cccontrol_selector(ExtraLayer::volumeSliderCallback), 
        CCControlEventValueChanged
    );

    auto volumeLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Music Volume")->getCString(), 
            "bigFont.fnt"
        );
  volumeLabel->setScale(0.5f);
  volumeLabel->setPosition(ccp(pSlider->getPosition().x, 50));

m_page1 = CCLayer::create();
m_page2 = CCLayer::create();
m_page1->addChild(noclipBtn);
m_page1->addChild(noclipBtnMore);
m_page1->addChild(speedhackBtn);
m_page1->addChild(speedhackBtnMore);
m_page1->addChild(percentageBtn);
m_page1->addChild(noDeathEffectBtn);
m_page1->addChild(hideAttemptsBtn);
m_page1->addChild(playSessionInfoBtn);
m_page1->addChild(displayFPSBtn);
m_page1->addChild(noRotationBtn);
m_page1->addChild(noRespawnBtn);

  m_page2->addChild(creditsMenu);
  m_page2->addChild(sessionMenu);
  m_page2->addChild(filterBtn);
  m_page2->addChild(deleteAllBtn);
  m_page2->addChild(levelLengthBtn);
  m_page2->addChild(MEBBtn);
  m_page2->addChild(epicBtn);
  m_page2->addChild(songBtn);
  m_page2->addChild(volumeLabel);
  m_page2->addChild(pSlider);
  pSlider->setValue(musicVolume);
  m_page2->setVisible(false);
  m_page2->setTouchEnabled(false);
  CCObject* pObj = NULL;
  CCArray* childrenpage2 = m_page2->getChildren(); 

CCARRAY_FOREACH(childrenpage2, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(false); 
    }
  }

  CCArray* childrenpage1 = m_page1->getChildren(); 

CCARRAY_FOREACH(childrenpage1, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(true); 
    }
  }

CCArray* childrenself = self->getChildren(); 

CCARRAY_FOREACH(childrenself, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(false); 
    }
  }

  m_self = self;

  m_left->setVisible(false);
    m_left->setEnabled(false);

    m_right->setVisible(true);
    m_right->setEnabled(true);

  contentHolder->addChild(m_page1);
  contentHolder->addChild(m_page2);
  contentHolder->addChild(mainLayoutLayer);

  auto infoLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Version: %i", CURRENT_VERSION)->getCString(), 
            "chatFont.fnt"
        );
        infoLabel->setScale(0.5f);
        infoLabel->setAnchorPoint(CCPointMake(0.5f, 0.5f));
        infoLabel->setColor(ccc3(129, 68, 37));
        infoLabel->setPosition(win_size.width - 75, 25);

        contentHolder->addChild(infoLabel);

  
  this->setKeypadEnabled(true);
  return true;
  }

  void ExtraLayer::onClose(CCObject* sender) {
    if (sender) this->retain();
    this->setKeypadEnabled(false);
    this->setTouchEnabled(false);
    this->removeFromParentAndCleanup(true);
    elc = false;
    ExtraLayer::m_currentPage = 1;
    CCArray* childrenself = m_self->getChildren();
    CCObject* pObj = NULL; 

CCARRAY_FOREACH(childrenself, pObj) {
    CCMenu* menu = dynamic_cast<CCMenu*>(pObj);
    if (menu) {
        menu->setEnabled(true); 
    }
  }
    this->saveSettingsToFile();
  }

  void ExtraLayer::keyBackClicked() {
    onClose(nullptr);
  }

  void ExtraLayer::showInfo(CCObject* sender) {
    ExtraLayerInfo::InfoType type = static_cast<ExtraLayerInfo::InfoType>(reinterpret_cast<intptr_t>(static_cast<CCMenu*>(sender)->getUserData()));
    const char* displayText;
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
      case ExtraLayerInfo::InfoType::NRT:
      displayText = "Removes wait time for respawn, respawn position might be unconsistent.";
      titleText = "No Death Effect";
      break;
      case ExtraLayerInfo::InfoType::HPM:
      displayText = "Allows you to hide the pause menu, you can zoom and move around the screen.";
      titleText = "Hide Pause Menu";
      break;
      case ExtraLayerInfo::InfoType::DA:
      displayText = "<cg>Deletes all objects of the same type</c> your selected object has. If multiple objects selected, the first one will be chosen.";
      titleText = "Delete All";
      break;
      case ExtraLayerInfo::InfoType::LL:
      displayText = "Displays the length of the level in the editor pause menu.";
      titleText = "Show Level Length";
      break;
      case ExtraLayerInfo::InfoType::MEB:
      displayText = "Adds 1/2 grid move buttons and 1/30 grid move buttons.";
      titleText = "More Editor Buttons";
      break;
      case ExtraLayerInfo::InfoType::DAT:
      displayText = "<cg>Displays the song of the level</c> in the main page.";
      titleText = "Display Audio Track";
      break;
      default:
      displayText = "<cr>unknown error</c>";
      titleText = "ERROR";
    }
    FLAlertLayer::create(
            nullptr,
            CCString::createWithFormat("Info")->getCString(),
            CCString::createWithFormat("%s", displayText)->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
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
  if(!showEpic) return;
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
  if(showSong) {
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
      }
  if(!showEpic) return 1;

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

char* audioFiles[] = {
    "StereoMadness.mp3",        // 0
    "BackOnTrack.mp3",          // 1 default
    "Polargeist.mp3",           // 2
    "DryOut.mp3",               // 3
    "BaseAfterBase.mp3",        // 4
    "CantLetGo.mp3",            // 5
    "Jumper.mp3",               // 6
    "TimeMachine.mp3",          // 7
    "Cycles.mp3",               // 8
    "xStep.mp3",                // 9
    "Clutterfunk.mp3",          // 10
    "TheoryOfEverything.mp3",   // 11
    "Electroman.mp3",           // 12
    "Clubstep.mp3",             // 13
    "Active.mp3",               // 14
    "Electrodynamix.mp3",       // 15
    "HexagonForce.mp3",         // 16
    "BlastProcessing.mp3",      // 17
    "TheoryOfEverything2.mp3",  // 18
    "CosmicDreamer.mp3",        // 19
    "SkyFortress.ogg",          // 20
    "SoundOfInfinity.mp3",      // 21
    "Rupture.mp3",              // 22
    "Stalemate.mp3",            // 23
    "GloriousMorning.ogg",       // 24
    "ChaozFantasy.ogg",
    "Phazd.ogg"
};

#define SONGS sizeof(audioFiles) / sizeof(audioFiles[0]) - 1 // made this as easy to replace as possible


char * (*LevelTools_getAudioFilename)(int);
char * LevelTools_getAudioFilename_H(int songID) {
  if (songID < 0 || songID > SONGS + 1) return "BackOnTrack.mp3";
  return audioFiles[songID];
}

class LevelTools {

};

LevelTools* (*LevelTools_getAudioString)(LevelTools*, int);
LevelTools* LevelTools_getAudioString_H(LevelTools* self, int ID) {
  return LevelTools_getAudioString(self, ID);
  }
  

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
    CCDirector* director = CCDirector::sharedDirector();
    CCScheduler* scheduler = director->getScheduler();
    if(noRespawn) scheduler->setTimeScale(99.0f);
    if(noRespawn) noRespawnReset = true;
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

int verval = 0;

VersionRequest* VersionRequest::sharedRequest() {
  VersionRequest* request = new VersionRequest();
  if(request) return request;
}

void MenuLayer::onVersionReceived(CCObject* obj) {
    CCNotificationCenter::sharedNotificationCenter()->removeObserver(this, "VERSION_CHECK_FINISHED");

    if (!this || !this->isRunning()) return;

    CCInteger* val = static_cast<CCInteger*>(obj);
    if(val && val->getValue() > CURRENT_VERSION) {
        verval = val->getValue();
        this->showUpdateAlert(0); 
    }
}

void MenuLayer::checkVersionIsOutdated() {
    MenuLayer::s_hasVersionData = false;
    VersionRequest::sharedRequest()->fetchVersion();
    this->schedule(schedule_selector(MenuLayer::versionCheckPoll), 0.1f);
}

void MenuLayer::versionCheckPoll(float dt) {
    if (MenuLayer::s_hasVersionData) {
        this->unschedule(schedule_selector(MenuLayer::versionCheckPoll));
        
        int fetchedVersion = MenuLayer::s_newVersion.load();
        if (fetchedVersion > CURRENT_VERSION) {
            verval = fetchedVersion;
            this->showUpdateAlert(0);
        }
        MenuLayer::s_hasVersionData = false;
    }
CCFadeOut* fadeout = CCFadeOut::create(2.f);
CCCallFunc* callback = CCCallFunc::create(updateLabel, callfunc_selector(CCNode::removeFromParentAndCleanup));
auto sequence = CCSequence::create(fadeout, callback, NULL);

updateLabel->runAction(sequence);
}


void MenuLayer::showUpdateAlert(float dt) {
  if (!this->isRunning()) return;

    auto alert = FLAlertLayer::create(
        this,
        "New update found!",
        CCString::createWithFormat("Your version of VioletPS is <cr>outdated</c>, download the new version to obtain the new features! \n\n<cy>Your version:</c> %i\n<cp>New version:</c> %i", CURRENT_VERSION, verval)->getCString(),
        "OK", "Download", 300.f
    );
    alert->show();
}

void (*MenuLayer_init)(MenuLayer*);
void MenuLayer_init_H(MenuLayer* self) {
MenuLayer_init(self);
musicVolume = CCUserDefault::sharedUserDefault()->getFloatForKey("musicVolume", 1.0f);
CocosDenshion::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(musicVolume);
auto win_size = CCDirector::sharedDirector()->getWinSize();
if(!checked) {
updateLabel = CCLabelBMFont::create(
            CCString::createWithFormat("Checking for updates")->getCString(), 
            "goldFont-hd.fnt"
        );

  updateLabel->setPosition({win_size.width / 2, 100});
  updateLabel->setScale(0.5f);
  self->addChild(updateLabel);
if(!warnedOutdated) self->checkVersionIsOutdated();
}
checked = true;
MenuLayer::sharedLayer = self;
auto buttonMenu = CCMenu::create();
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
HidePauseLayer::m_pauseLayer = self;
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

auto buttonMenuu = CCMenu::create();
auto buttoon = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
CCMenuItemSpriteExtra* menuu = CCMenuItemSpriteExtra::create(buttoon, buttoon, self, menu_selector(PauseLayer::toggleVisibility));
menuu->setPosition({-20,-20});
menuu->setScale(1.f);
menuu->_setZOrder(100);
buttonMenuu->setPosition({60, win_size.height - 30});
buttonMenuu->addChild(menuu);
if(hidePauseMenu) self->addChild(buttonMenuu);

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
  CCDirector* director = CCDirector::sharedDirector();
  CCScheduler* scheduler = director->getScheduler();
  scheduler->setTimeScale(1.f);
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
  musicVolume = CCUserDefault::sharedUserDefault()->getFloatForKey("musicVolume", 1.0f);
CocosDenshion::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(musicVolume);
CCDirector* director = CCDirector::sharedDirector();
CCScheduler* scheduler = director->getScheduler();
  if(noRespawnReset) {
  scheduler->setTimeScale(1.0f);
  noRespawnReset = false;
}
  PlayLayer_resetLevel(self);
  deaths = 0;
  deathsLabel->setString("0 deaths");
  if(noclip || speedhack) safeMode = true;
  else safeMode = false;
  return;
  HitboxLayer* debug = HitboxLayer::create();
if (debug) {
    debug->l_targetLayer = MEMBER_BY_OFFSET(CCSpriteBatchNode*, self, 0x2a0); 
    cocos2d::CCScene* currentScene = cocos2d::CCDirector::sharedDirector()->getRunningScene();
    if (currentScene) {
        currentScene->addChild(debug, 9999);
    }
}
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
        if(fps) {
  deathsLabel->setPosition(10,win_size.height - 30);
        } else {
  deathsLabel->setPosition(10,win_size.height - 10);
}
        self->addChild(deathsLabel, 10000);
        if(ExtraLayer::m_deaths) deathsLabel->setVisible(true);
        else deathsLabel->setVisible(false);

        if(!safeMode) noclipLabel->setVisible(false);
        else noclipLabel->setVisible(true);

        if(!fps) FPSLabel->setVisible(false);
        else FPSLabel->setVisible(true);

        if(noParticles) {
          percentageLabel->setVisible(true);
          percentageLabel->setString("0%");
          GameManager* state = GameManager::sharedState();
  if(MEMBER_BY_OFFSET(int, state, 0x1a5)) percentageLabel->setPosition(win_size.width - 190, win_size.height - 10);
  else percentageLabel->setPosition(win_size.width / 2, win_size.height - 10);
          goldenPercentageLabel->setVisible(false);
        } else {
          percentageLabel->setVisible(false);
          goldenPercentageLabel->setVisible(false);
        }
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
if(!gjlvl || MEMBER_BY_OFFSET(float, self, 0x1dc) == 0 || !MEMBER_BY_OFFSET(PlayerObject*, self, 0x274)) return;
auto normalPercentage = MEMBER_BY_OFFSET(int, gjlvl, 0x170);
CCDirector* director = CCDirector::sharedDirector();
CCScheduler* scheduler = director->getScheduler();
if(speedhack) scheduler->setTimeScale(ExtraLayer::m_speedhack);
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
    if(safeMode) noclipLabel->setVisible(true);
    else noclipLabel->setVisible(false);
  }
  atts = MEMBER_BY_OFFSET(int, self, 0x2d8);
  jumps = MEMBER_BY_OFFSET(int, self, 0x2dc);
  if(MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)) {
  if(hideatts) MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(false);
  else MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(true);
  }
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
  PlayerObject* player = MEMBER_BY_OFFSET(PlayerObject*, self, 0x274);
    float percent = (player->getPosition().x / MEMBER_BY_OFFSET(float, self, 0x1dc)) * 100.0; // destroyplayer
    if(percent > 100) percent = 100;
    if(percent < 0) percent = 0;
    percent = floorf(percent);
  std::string str = std::to_string((int)percent) + "%";
if(percentageLabel) percentageLabel->setString(str.c_str());
  if(goldenPercentageLabel) goldenPercentageLabel->setString(str.c_str());
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


CCMenu* createSpriteButtonWithFunction(CCSprite* sprite, CCPoint position, cocos2d::SEL_MenuHandler selector, CCLayer* layer) {
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(sprite, sprite, layer, selector);
auto buttonMenu = CCMenu::create(menuBtn, NULL);
buttonMenu->setAnchorPoint(CCPointZero);
buttonMenu->setPosition(CCPointZero);
buttonMenu->setPosition(position);
return buttonMenu;
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
if(!closed) { 
  auto button = CCSprite::create("secretDoor_open.png");
  button->setPosition(CCPointMake(win_size.width - 20, 20));
  auto buttonMenu = createSpriteButtonWithFunction(button, button->getPosition(), menu_selector(CreatorLayer::onSecret), self);
  buttonMenu->setScale(0.5f);
self->addChild(buttonMenu);
}
else {
  auto button = CCSprite::create("secretDoor_closed.png");
  button->setPosition(CCPointMake(win_size.width - 20, 20));
  auto buttonMenu = createSpriteButtonWithFunction(button, button->getPosition(), menu_selector(CreatorLayer::onSecret), self);
  buttonMenu->setScale(0.5f);
self->addChild(buttonMenu);
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
      "I need the fire extinguisher for the chicken!",
      "A Fantasy of Chaoz!",
      "Life is bad if you only focus on the deaths you have."
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
  HidePauseLayer::m_playLayer = self;
  if(hideatts) MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(false);
  else MEMBER_BY_OFFSET(CCLabelBMFont*, self, 0x1e4)->setVisible(true);
  gjlvl = lvl;
  if(!percentageLabel->getParent() && percentageLabel != nullptr) MEMBER_BY_OFFSET(CCLayer*, self, 0x270)->addChild(percentageLabel, 10000);
  if(!goldenPercentageLabel->getParent() && goldenPercentageLabel != nullptr) MEMBER_BY_OFFSET(CCLayer*, self, 0x270)->addChild(goldenPercentageLabel, 10000);
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

void EditLevelLayer::onOpenMenu() {
  this->setTouchEnabled(false);
    extra = ExtraLayer::create(this);
    this->addChild(extra, 1000);
}

void (*EditLevelLayer_init)(CCLayer* self, GJGameLevel* lvl);
void EditLevelLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
EditLevelLayer_init(self, lvl);
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto buttonMenu2 = CCMenu::create();
auto button2 = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
CCMenuItemSpriteExtra* menuBtn2 = CCMenuItemSpriteExtra::create(button2, button2, self, menu_selector(EditLevelLayer::onOpenMenu));
buttonMenu2->setPosition(CCPointZero);
menuBtn2->setPosition({-60, win_size.height + 90});
buttonMenu2->addChild(menuBtn2);
buttonMenu2->setScale(0.5f);
buttonMenu2->setPosition(-55, 12.5);
self->addChild(buttonMenu2, 100);
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
        obj->stopAllActions();
        obj->unscheduleAllSelectors();
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
if(deleteAll) {
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
      }
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
    if(!deleteAll) return;
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
    CCArray* allObjectsInRect = LevelEditorLayer_objectsInRect(param_1, param_2, param_3, param_4);
    if (G_TARGET_FILTER_TYPE_ID == nullptr) {
        return allObjectsInRect;
    }
    CCArray* filteredArray = CCArray::create(); 
    for (int i = 0; i < allObjectsInRect->count(); i++) {
        GameObject* currObj = static_cast<GameObject*>(allObjectsInRect->objectAtIndex(i));
        if (currObj == nullptr) continue;
        auto* currObjMember = MEMBER_BY_OFFSET(void*, currObj, 0x32c);
        if (currObjMember == G_TARGET_FILTER_TYPE_ID) {
            filteredArray->addObject(currObj);
        }
    }
    return filteredArray;
}
int (*EditorPauseLayer_init)(CCLayer* self, CCLayer* editor);
int EditorPauseLayer_init_H(CCLayer* self, CCLayer* editor) {
  EditorPauseLayer_init(self, editor);
  CCArray* children = self->getChildren();
    std::vector<CCNode*> labelsToRemove;
    CCObject* child;
    CCARRAY_FOREACH(children, child) {
        CCNode* node = static_cast<CCNode*>(child);
        CCLabelBMFont* label = dynamic_cast<CCLabelBMFont*>(node);
        if (label != nullptr) labelsToRemove.push_back(node);
    }
    for (CCNode* label : labelsToRemove) {
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
  if(!levelLength) return 1;

  int editorDistance = floorf((MEMBER_BY_OFFSET(int, editor, 0x164) / 311.0) + 3);
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
return;
auto win_size = CCDirector::sharedDirector()->getWinSize();
auto buttonMenu2 = CCMenu::create();
auto button2 = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
CCMenuItemSpriteExtra* menuBtn2 = CCMenuItemSpriteExtra::create(button2, button2, self, menu_selector(OptionsLayer::onOpenMenu));
buttonMenu2->setPosition(CCPointZero);
menuBtn2->setPosition({win_size.width, win_size.height});
buttonMenu2->addChild(menuBtn2);
buttonMenu2->setScale(0.5f);
buttonMenu2->setPosition(25, 10);
self->addChild(buttonMenu2, 100);
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

struct LegacyString {
    struct Metadata {
        int capacity;
        int length;
        int refCount;
    };
    char* data;
};

// helper to create a string in the format the game expects
void* to_fake_str(const char* text) {
    size_t len = strlen(text);
    auto* totalBuffer = (uint8_t*)malloc(12 + len + 1);

    int* meta = (int*)totalBuffer;
    meta[0] = len;
    meta[1] = len;
    meta[2] = -1;
    
    char* dataPtr = (char*)(totalBuffer + 12);
    strcpy(dataPtr, text);
    
    return dataPtr; // the game expects a pointer to the TEXT, not the header
}

GJGameLevel* (*levelTools_getLevel)(int level);
GJGameLevel* levelTools_getLevel_H(int level) {
  if(level != 15) return levelTools_getLevel(level);
  auto lvl = levelTools_getLevel(6);
  /*
  std::string* levelString = new std::string("kS1,40,kS2,125,kS3,255,kS4,0,kS5,102,kS6,255,kA1,10;");
  MEMBER_BY_OFFSET(std::string*, lvl, 0x134) = levelString;

   std::string* levelName = new std::string("test");
  MEMBER_BY_OFFSET(std::string, lvl, 0x12c) = CCString::create("test")->m_sString; */

/* auto setLevelName = reinterpret_cast<void(__thiscall*)(GJGameLevel*, std::string)>(
    *(uintptr_t*)(*(uintptr_t*)lvl + 0x134)
  );
  setLevelName(lvl, "test"); 

  lvl->setLevelName("test");
  lvl->setLevelString("S1,40,kS2,125,kS3,255,kS4,0,kS5,102,kS6,255,kA1,10;");
  */
    uintptr_t vtable = *reinterpret_cast<uintptr_t*>(lvl);
    using setLevelName_t = void(*)(void*, std::string);
    setLevelName_t setLevelName = *reinterpret_cast<setLevelName_t*>(vtable + 0x134);
    using setLevelString_t = void(*)(void*, std::string);
    setLevelString_t setLevelString = *reinterpret_cast<setLevelString_t*>(vtable + 0x144);
    void* fakeStringData = to_fake_str(LEVELNAME_1);
    void* fakeStringData2 = to_fake_str(LEVELSTRING_1);
    *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(lvl) + 0x12c) = fakeStringData;
    *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(lvl) + 0x134) = fakeStringData2;



  MEMBER_BY_OFFSET(int, lvl, 0x148) = 25;
  lvl->setDifficulty(lvl, 4);
  MEMBER_BY_OFFSET(int, lvl, 0x18c) = 12;
  MEMBER_BY_OFFSET(int, lvl, 0x128) = 15;
  MEMBER_BY_OFFSET(int, lvl, 0x1e0) = 1;
  MEMBER_BY_OFFSET(int, lvl, 0x194) = 3;
  MEMBER_BY_OFFSET(int, lvl, 0x1b0) = 0;
  return lvl;
}

std::vector<uint8_t> uintptrToBytes(uintptr_t value) {
    std::vector<uint8_t> bytes(sizeof(uintptr_t));

    for (size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
    }

    return bytes;
}

void LevelEditorLayer_update(LevelEditorLayer* self, float dt) {
    CCRect visible_rect;
    
    auto vr_origin = MEMBER_BY_OFFSET(cocos2d::CCLayer*, self, 0x158)->convertToNodeSpace(CCPoint(0,0));
    auto vr_dest = CCDirector::sharedDirector()->getWinSize();
    
    visible_rect.origin = vr_origin;
    visible_rect.size = vr_dest;
    
    // handle zooming
    visible_rect.size.width *= 1 / MEMBER_BY_OFFSET(cocos2d::CCLayer*, self, 0x158)->getScale();
    visible_rect.size.height *= 1 / MEMBER_BY_OFFSET(cocos2d::CCLayer*, self, 0x158)->getScale();

    // fix blocks disappearing where it shouldn't by extending the visible rect by a few blocks
    visible_rect.origin.x -= 75;
    visible_rect.origin.y -= 75;
    
    visible_rect.size.width += 150;
    visible_rect.size.height += 150;

    auto bn = MEMBER_BY_OFFSET(CCSpriteBatchNode*, self, 0x140);

    for (uint32_t section_id = 0; section_id < MEMBER_BY_OFFSET(CCArray*, self, 0x144)->count(); section_id++) {
        CCArray* section_objects = static_cast<CCArray*>(MEMBER_BY_OFFSET(CCArray*, self, 0x144)->objectAtIndex(section_id));

        for (uint32_t index = 0; index < section_objects->count(); index++) {
            GameObject* object = static_cast<GameObject*>(section_objects->objectAtIndex(index));
            CCPoint object_pos = object->getPosition();

            if (CCRect::CCRectContainsPoint(visible_rect, object_pos)) {
                if (!object->getParent()) {
                    OrderingData* s = static_cast<OrderingData*>(object->getUserData());

                    bn->addChild(object, s->z_order);
                    //object->setOrderOfArrival(s->order_of_arrival);

                    bn->sortAllChildren();
                }
            } else {
                if (object->getParent()) {
                    if (object->getUserData() == nullptr) {
                        OrderingData s = OrderingData {object->getOrderOfArrival(), object->getZOrder()};
                        object->setUserData((void*)&s);
                    }

                    bn->removeChild(object, false);
                }
            }

        }
    }
}

bool (*levelEditorLayer_init)(CCLayer* self, GJGameLevel* lvl);
bool levelEditorLayer_init_H(CCLayer* self, GJGameLevel* lvl) {
  levelEditorLayer_init(self, lvl);
   void** vtable = *(void***)self; // ty omnimenu
   void (LevelEditorLayer::* ptr)(float) = &LevelEditorLayer::update;
   void* offset = *(void**)&ptr;
   DobbyCodePatch(&vtable[((uintptr_t)offset)/sizeof(void*)], uintptrToBytes((uintptr_t)&LevelEditorLayer_update).data(), 4);
   return true;
}

void (*PlayerObject_updateShipRotation)(PlayerObject* self, float dt);
void PlayerObject_updateShipRotation_H(PlayerObject* self, float dt) {
    if (noRotation) return;
    PlayerObject_updateShipRotation(self, dt);
}

void (*PlayerObject_runRotateAction)(PlayerObject* self);
void PlayerObject_runRotateAction_H(PlayerObject* self) {
    if (noRotation) return;
    PlayerObject_runRotateAction(self);
}
void (*PlayerObject_runBallRotation2)(PlayerObject* self);
void PlayerObject_runBallRotation2_H(PlayerObject* self) {
    if (noRotation) return;
    PlayerObject_runBallRotation2(self);
}

void setOriginalScale(CCMenuItemSpriteExtra* btn, float scale) {
    MEMBER_BY_OFFSET(float, btn, 0x150) = scale;
}

CCMenuItemSpriteExtra* EditorUI::getSpriteButton2(const char* name, SEL_MenuHandler callback, CCMenu* menu, float scale)
{
    auto spr = CCSprite::create(name);
    auto btnSpr = ButtonSprite::create(spr, 32, 0, 32, 1.0, true, "GJ_button_01.png");
    auto btn = CCMenuItemSpriteExtra::create(btnSpr, 0, this, callback);
    btn->setScale(scale);
    setOriginalScale(btn, scale);

    if (menu) menu->addChild(btn);

    return btn;
}
CCMenuItemSpriteExtra* EditorUI::getSpriteButton3(const char* name, SEL_MenuHandler callback, CCMenu* menu, float scale, float sprScale)
{
    auto spr = CCSprite::createWithSpriteFrameName(name);
    spr->setScale(sprScale);
    auto btnSpr = ButtonSprite::create(spr, 32, 0, 32, 1.0, true, "GJ_button_01.png");
    auto btn = CCMenuItemSpriteExtra::create(btnSpr, 0, this, callback);
    btn->setScale(scale);
    setOriginalScale(btn, scale);

    if (menu) menu->addChild(btn);

    return btn;
}

void (*EditorUI_createMoveMenu)(EditorUI* self);
void EditorUI_createMoveMenu_H(EditorUI* self) {
  if(moreEditorButtons) {
        CCArray* buttons = CCArray::create();

        CCMenuItemSpriteExtra* btn;

        btn = self->getSpriteButton("edit_upBtn_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(3);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_downBtn_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(4);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_leftBtn_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(1);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_rightBtn_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(2);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_upBtn2_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(7);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_downBtn2_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(8);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_leftBtn2_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(5);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_rightBtn2_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(6);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_upBtn3_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(11);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_downBtn3_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(12);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_leftBtn3_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(9);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_rightBtn3_001.png", menu_selector(EditorUI::moveObjectCall), nullptr, 0.9);
        btn->setTag(10);
        buttons->addObject(btn);

                btn = self->getSpriteButton3("edit_upBtn_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9, 0.8);
        btn->setTag(1001);
        buttons->addObject(btn);
        btn = self->getSpriteButton3("edit_downBtn_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9, 0.8);
        btn->setTag(1002);
        buttons->addObject(btn);
        btn = self->getSpriteButton3("edit_leftBtn_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9, 0.8);
        btn->setTag(1003);
        buttons->addObject(btn);
        btn = self->getSpriteButton3("edit_rightBtn_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9, 0.8);
        btn->setTag(1004);
        buttons->addObject(btn);
        btn = self->getSpriteButton2("edit_upBtn5_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9);
        btn->setTag(1005);
        buttons->addObject(btn);
        btn = self->getSpriteButton2("edit_downBtn5_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9);
        btn->setTag(1006);
        buttons->addObject(btn);
        btn = self->getSpriteButton2("edit_leftBtn5_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9);
        btn->setTag(1007);
        buttons->addObject(btn);
        btn = self->getSpriteButton2("edit_rightBtn5_001.png", menu_selector(EditorUI::moveObjectCall2), nullptr, 0.9);
        btn->setTag(1008);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_flipXBtn_001.png", menu_selector(EditorUI::transformObjectCall), nullptr, 0.9);
        btn->setTag(17);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_flipYBtn_001.png", menu_selector(EditorUI::transformObjectCall), nullptr, 0.9);
        btn->setTag(18);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_cwBtn_001.png", menu_selector(EditorUI::transformObjectCall), nullptr, 0.9);
        btn->setTag(19);
        buttons->addObject(btn);
        btn = self->getSpriteButton("edit_ccwBtn_001.png", menu_selector(EditorUI::transformObjectCall), nullptr, 0.9);
        btn->setTag(20);
        buttons->addObject(btn);
        CCDirector* director = CCDirector::sharedDirector();
        auto win_size = director->getWinSize();
        EditButtonBar* newBar = EditButtonBar::create(buttons, ccp(win_size.width * 0.5 - 5, MEMBER_BY_OFFSET(float, director->getOpenGLView(), 0xe4) + MEMBER_BY_OFFSET(float, self, 0x158) - 6.f), false);
        MEMBER_BY_OFFSET(EditButtonBar*, self, 0x148) = newBar;
        self->addChild(newBar, 11);
  } else EditorUI_createMoveMenu(self);
    }

    void EditorUI::moveObjectCall2(CCNode* sender) {
    if (!MEMBER_BY_OFFSET(GameObject*, this, 0x254) && MEMBER_BY_OFFSET(CCArray*, this, 0x180)->count() <= 0) return;

    auto transform = ccp(0, 0);

    switch (sender->getTag()) {

        case 1001:
            transform.y = 1;
            break;
        case 1002:
            transform.y = -1;
            break;
        case 1003:
            transform.x = -1;
            break;
        case 1004:
            transform.x = 1;
            break;
        case 1005:
            transform.y = 15;
            break;
        case 1006:
            transform.y = -15;
            break;
        case 1007:
            transform.x = -15;
            break;
        case 1008:
            transform.x = 15;
            break;

        default:
            break;
    }
    if (MEMBER_BY_OFFSET(CCArray*, this, 0x180)->count() > 0) {
        for (int i = 0; i < MEMBER_BY_OFFSET(CCArray*, this, 0x180)->count(); i++) {
            this->moveObject(static_cast<GameObject*>(MEMBER_BY_OFFSET(CCArray*, this, 0x180)->objectAtIndex(i)), transform);
        }
    } else {
        this->moveObject(MEMBER_BY_OFFSET(GameObject*, this, 0x254), transform);
    }
}

void OptionsLayer::onOpenMenu() {
  this->setTouchEnabled(false);
  extra = ExtraLayer::create(this);
  this->addChild(extra, 1000);
  extra->_setZOrder(10000000);
}



void (*CocosDenshion_SimpleAudioEngine_playBackgroundMusic)(CocosDenshion::SimpleAudioEngine*, char const*, bool);
void CocosDenshion_SimpleAudioEngine_playBackgroundMusic_H(CocosDenshion::SimpleAudioEngine* engine, const char* song, bool idk) {
CocosDenshion_SimpleAudioEngine_playBackgroundMusic(engine, song, idk);
musicVolume = CCUserDefault::sharedUserDefault()->getFloatForKey("musicVolume", 1.0f);
CocosDenshion::SimpleAudioEngine::sharedEngine()->setBackgroundMusicVolume(musicVolume);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void VersionRequest::fetchVersion() {
  std::thread t(&VersionRequest::networkThread, this);
  t.detach();
}

void VersionRequest::networkThread() {
    void* handle = dlopen("libcurl.so", RTLD_LAZY);
    if (!handle) handle = dlopen("libgame.so", RTLD_LAZY);
    if (!handle) return;

    typedef void* (*curl_init_t)();
    typedef int (*curl_setopt_t)(void*, int, ...);
    typedef int (*curl_perform_t)(void*);
    typedef void (*curl_cleanup_t)(void*);
    typedef const char* (*curl_error_t)(int);

    auto curl_init = (curl_init_t)dlsym(handle, "curl_easy_init");
    auto curl_setopt = (curl_setopt_t)dlsym(handle, "curl_easy_setopt");
    auto curl_perform = (curl_perform_t)dlsym(handle, "curl_easy_perform");
    auto curl_cleanup = (curl_cleanup_t)dlsym(handle, "curl_easy_cleanup");

    void* curl = curl_init();
    if (curl) {
        std::string responseString;
        curl_setopt(curl, 10002, "pok.ps.fhgdps.com/ver.php");
        curl_setopt(curl, 20011, WriteCallback);
        curl_setopt(curl, 10001, &responseString);
        curl_setopt(curl, 64, 0L);

        int res = curl_perform(curl);

        if (res == 0) { // CURLE_OK
            int versionInt = atoi(responseString.c_str());
            MenuLayer::s_newVersion = versionInt;
            MenuLayer::s_hasVersionData = true;
        }

        curl_cleanup(curl);
    }
    dlclose(handle);
}

typedef void (*MenuLayer_Clicked_T)(MenuLayer*, FLAlertLayer*, bool);
MenuLayer_Clicked_T MenuLayer_FLAlert_Clicked_Orig = nullptr;
void MenuLayer_FLAlert_Clicked_H(MenuLayer* self, FLAlertLayer* alert, bool btn2) {
  if(!alert || !btn2) return;
    switch (alert->getTag()) { 
      case 1001:
      cocos2d::CCApplication::sharedApplication().openURL("http://pok.ps.fhgdps.com/downloads");
      break;
      case 2:
      cocos2d::CCApplication::sharedApplication().openURL("http://www.youtube.com/@nano56gd");
      break;
      case 0:
      auto delegate = (AppDelegate *)AppDelegate::get();
      AppDelegate::trySaveGame();
      self->endGame();
    } 
}

void (*GJGameLevel_savePercentage)(GJGameLevel*, int, bool);
void GJGameLevel_savePercentage_H(GJGameLevel* self, int percentage, bool asfcsdvfvnvhf) {
if(safeMode) return;
}

void (*PlayLayer_showNewBest)(CCLayer*);
void PlayLayer_showNewBest_H(CCLayer* self) {
  if(!safeMode) PlayLayer_showNewBest(self);
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
  // HOOK("_ZN9PlayLayer11showNewBestEv", PlayLayer_showNewBest_H, PlayLayer_showNewBest);
  // HOOK("_ZN11GJGameLevel14savePercentageEib", GJGameLevel_savePercentage_H, GJGameLevel_savePercentage);
  HOOK("_ZN9MenuLayer15FLAlert_ClickedEP12FLAlertLayerb", MenuLayer_FLAlert_Clicked_H, MenuLayer_FLAlert_Clicked_Orig);
  HOOK("_ZN12OptionsLayer11customSetupEv", OptionsLayer_customSetup_H, OptionsLayer_customSetup);
  // HOOK("_ZN13CocosDenshion17SimpleAudioEngine19playBackgroundMusicEPKcb", CocosDenshion_SimpleAudioEngine_playBackgroundMusic_H, CocosDenshion_SimpleAudioEngine_playBackgroundMusic);
  HOOK("_ZN8EditorUI14createMoveMenuEv", EditorUI_createMoveMenu_H, EditorUI_createMoveMenu);
  HOOK("_ZN12PlayerObject18updateShipRotationEf", PlayerObject_updateShipRotation_H, PlayerObject_updateShipRotation);
  HOOK("_ZN12PlayerObject15runRotateActionEv", PlayerObject_runRotateAction_H, PlayerObject_runRotateAction);
  HOOK("_ZN12PlayerObject16runBallRotation2Ev", PlayerObject_runBallRotation2_H, PlayerObject_runBallRotation2);
  HOOK("_ZN10LevelTools8getLevelEi", levelTools_getLevel_H, levelTools_getLevel);
  HOOK("_ZN16LevelEditorLayer4initEP11GJGameLevel", levelEditorLayer_init_H, levelEditorLayer_init);
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
  HOOK("_ZN11GameManager14isIconUnlockedEi8IconType", GameManager_isIconUnlocked_H, GameManager_isIconUnlocked);
  HOOK("_ZN11GameManager15isColorUnlockedEib", GameManager_isColorUnlocked_H, GameManager_isColorUnlocked);

  HOOK("_ZN12SupportLayer7onEmailEv", SupportLayer_onEmail_H, SupportLayer_onEmail);

  HOOK("_ZN9PlayLayer13destroyPlayerEv", PlayLayer_destroyPlayer_H, PlayLayer_destroyPlayer);
  HOOK("_ZN9PlayLayer13levelCompleteEv", PlayLayer_levelComplete_H, PlayLayer_levelComplete);
  HOOK("_ZN7UILayer4initEv", UILayer_init_H, UILayer_init);
  HOOK("_ZN9PlayLayer10resetLevelEv", PlayLayer_resetLevel_H, PlayLayer_resetLevel);
  HOOK("_ZN9PlayLayer6resumeEv", PlayLayer_resume_H, PlayLayer_resume);

  // HOOK("_ZN13CocosDenshion17SimpleAudioEngine22setBackgroundMusicTimeEf", setBackgroundMusicTimeJNI_H, setBackgroundMusicTimeJNI);

  HOOK("_ZN10PauseLayer11customSetupEv", PauseLayer_customSetup_H, PauseLayer_customSetup);

  HOOK("_ZN10LevelTools13getAudioTitleEi", LevelTools_getAudioTitle_H, LevelTools_getAudioTitle);
  HOOK("_ZN10LevelTools16getAudioFileNameEi", LevelTools_getAudioFilename_H, LevelTools_getAudioFilename);

   HOOK("_ZN9MenuLayer4initEv", MenuLayer_init_H, MenuLayer_init);

  HOOK("_ZN9PlayLayer6updateEf", PlayLayer_onUpdate_H, PlayLayer_onUpdate);
  
  HOOK("_ZN10LevelTools14getAudioStringEi", LevelTools_getAudioString_H, LevelTools_getAudioString);
  // HOOK("_ZN10LevelTools14artistForAudioEi", LevelTools_artistForAudio_H, LevelTools_artistForAudio);
  // HOOK("_ZN10LevelTools13nameForArtistEi", LevelTools_nameForArtist_H, LevelTools_nameForArtist); */
  
  HOOK("_ZN9LevelCell19loadCustomLevelCellEv", LevelCell_loadCustomLevelCell_H, LevelCell_loadCustomLevelCell);
  HOOK("_ZN11GJGameLevel15encodeWithCoderEP13DS_Dictionary", GJGameLevel_encodeWithCoder_H, GJGameLevel_encodeWithCoder);
  HOOK("_ZN11GJGameLevel15createWithCoderEP13DS_Dictionary", GJGameLevel_createWithCoder_H, GJGameLevel_createWithCoder);
  HOOK("_ZN14LevelInfoLayer4initEP11GJGameLevel", LevelInfoLayer_init_H, LevelInfoLayer_init);
  HOOK("_ZN11GJGameLevelD1Ev", GJGameLevel_destructor_H, GJGameLevel_destructor); 

  PatchManager pm;
  pm.cpatch(0x14d8d4, CCString::createWithFormat("%02X 29", SONGS + 1)->getCString()); // LevelSettingsLayer::selectSong
  pm.cpatch(0x14d8d8, CCString::createWithFormat("%02X 21", SONGS)->getCString()); // LevelSettingsLayer::selectSong
  pm.cpatch(0x14d920, CCString::createWithFormat("%02X 28", SONGS + 1)->getCString()); // LevelSettingsLayer::audioNext

  pm.cpatch(0x16bbf4, "10 2d"); // LevelSelectLayer::init

  pm.cpatch(0x16aa8c, "04 21"); // LevelPage::init

  pm.cpatch(0x14fff4, "00 00 00 00"); // EditorUI::onCreateObject
  pm.cpatch(0x14ee10, "00 00 00 00"); // EditorUI::moveObject

  pm.Modify();
  return JNI_VERSION_1_6;
}