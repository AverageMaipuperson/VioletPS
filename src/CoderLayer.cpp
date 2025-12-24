#include "../vendor/robtop/CoderLayer.hpp"
#include "../vendor/robtop/CreatorLayer.hpp"
#include "../vendor/robtop/FLAlertLayer.hpp"
#include "../vendor/robtop/CCMenuItemToggler.hpp"
#include "../vendor/robtop/CCMenuItemSpriteExtra.hpp"
#include "../vendor/robtop/EditorConfigurationsLayer.hpp"
#include "../vendor/robtop/CCTextInputNode.hpp"
#include "../vendor/robtop/VCustomEncrypt.hpp"
#include "../vendor/cocos/CocosDenshion/include/SimpleAudioEngine.h"
#include "../vendor/cocos/CocosDenshion/android/SimpleAudioEngine.cpp"
#include "../vendor/cocos/CocosDenshion/android/jni/SimpleAudioEngineJni.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpClient.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpRequest.h"
#include "../vendor/cocos/cocos2dx/extensions/network/HttpResponse.h"
#include "cocos2dExt.h"
#include "../vendor/cpp-base64-2.rc.08/base64.h"
#include <cmath>
#include <map>
#include <cstdint>
#include <unordered_map>
#include <ctime>
#include <random>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <cctype>

USING_NS_CC_EXT; 
using namespace cocos2d;

CCTextInputNode* globalInput = nullptr;
CCLabelBMFont* mainTitle = nullptr;
bool code1 = false;
bool code2 = false;
bool code3 = false;
bool code4 = false;
bool code5 = false;
bool code6 = false;
bool code7 = false;
bool code8 = false;
bool code9 = false;
bool code10 = false;
int currentDialog = 0;
bool doorClosed1 = false;
const char* ver = "";
#define FADE_DURATION 1.0f
#define FADE_SPEED 1.0f
#define NEW_SONG "CosmicDreamer.mp3"
#define CURRENT_VERSION 13

#define MEMBER_BY_OFFSET(type, var, offset) \
    (*reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(var) + static_cast<uintptr_t>(offset)))

CoderLayer::~CoderLayer() {
    
}
  
void CoderLayer::sendDataToServer() {
    CCHttpRequest* request = new CCHttpRequest();
    request->setRequestType(CCHttpRequest::kHttpGet); 
    std::string url = "https://pok.ps.fhgdps.com/ver.php";
    request->setUrl(url.c_str());
    // request->setResponseCallback(
    // static_cast<cocos2d::CCObject*>(this),
    // static_cast<cocos2d::SEL_CallFuncND>(&CoderLayer::onHttpRequestCompleted)
// );
    request->setTag("GetVer");
    CCHttpClient::getInstance()->send(request);
    request->release();
}

void CoderLayer::onHttpRequestCompleted(CCHttpClient *sender, CCHttpResponse *response) {
    if (!response) {
        CCLOG("No response from server.");
        return;
    }
    if (response->getResponseCode() != 200) {
        CCLOG("HTTP Error: %d", response->getResponseCode());
        return;
    }
    std::vector<char>* buffer = response->getResponseData();
    if (buffer && buffer->size() > 0) {
        std::string responseData(buffer->begin(), buffer->end());
     ver = responseData.c_str();
}
}

void CoderLayer::updateMusicVolume(float dt) {
    CocosDenshion::SimpleAudioEngine* engine = CocosDenshion::SimpleAudioEngine::sharedEngine();
    float currentVolume = engine->getBackgroundMusicVolume();

    float volumeStep = dt * FADE_SPEED; 
    float newVolume = currentVolume - volumeStep;

    engine->setBackgroundMusicVolume(newVolume);

    if (newVolume <= 0.0f) {
        engine->setBackgroundMusicVolume(0.0f);
        engine->stopBackgroundMusic();
        this->unschedule(schedule_selector(CoderLayer::updateMusicVolume));
 
        engine->playBackgroundMusic(NEW_SONG, true);
    }
}

void CoderLayer::fadeOutAndTransitionMusic() {
    this->schedule(schedule_selector(CoderLayer::updateMusicVolume));
}


void GameStatsManager::reward(int type, int amount) {
GameStatsManager* stats = GameStatsManager::sharedState();
switch(type) {
    case 6: ExtraLayer::incrementStat(stats, "6", amount); // stars
    break;
    case 7: ExtraLayer::incrementStat(stats, "7", amount); // coins
    break;
}
}

void CoderLayer::reward(int type, int amount) {
GameStatsManager::reward(type, amount);
}


void CoderLayer::countUpScore(float dt) {
    oldStarAmount += 1; 
    std::string scoreString = std::to_string(oldStarAmount);
    label->setString(scoreString.c_str(), true);

    if (oldStarAmount >= starAmount) {
        this->unschedule(schedule_selector(CoderLayer::countUpScore));
    }
}

void CoderLayer::animationWithSprite(CCSprite* sprite, float duration, CCPoint destination, int type) {
auto moveBy = CCMoveBy::create(duration, destination);
switch(type) {
    case 0:
    {
    auto ease = CCEaseOut::create(moveBy, 3.0f); 
    sprite->runAction(ease);
    break;
    }
    case 1:
    {
    auto ease = CCEaseIn::create(moveBy, 3.0f); 
    sprite->runAction(ease);
    break;
    }
    default:
    break;
}
}

void CoderLayer::animationWithLabel(CCLabelBMFont* sprite, float duration, CCPoint destination, int type) {
auto moveBy = CCMoveBy::create(duration, destination);
switch(type) {
    case 0:
    {
    auto ease = CCEaseOut::create(moveBy, 3.0f); 
    sprite->runAction(ease);
    break;
    }
    case 1:
    {
    auto ease = CCEaseIn::create(moveBy, 3.0f); 
    sprite->runAction(ease);
    break;
    }
    default:
    break;
}
}

void CoderLayer::starAnimationBack() {
this->animationWithSprite(starSprite, 1.f, CCPointMake(150, 0), 1);
this->animationWithLabel(label, 1.f, CCPointMake(150, 0), 1);
}
void CoderLayer::showStarAnimation(int amount) {
    CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(CCFileUtils::sharedFileUtils()->fullPathFromRelativePath("highscoreGet02.ogg"));
    auto win_size = CCDirector::sharedDirector()->getWinSize();
    starSprite = CCSprite::createWithSpriteFrameName("GJ_starsIcon_001.png");
    starSprite->setPosition({win_size.width + 50, win_size.height - 20});
    label = CCLabelBMFont::create(CCString::createWithFormat("+%i", amount)->getCString(), "bigFont.fnt");
    float horizontal_spacing = 10.0f; 
    float labelX = starSprite->getPosition().x + starSprite->getContentSize().width / 2 + label->getContentSize().width / 2 + horizontal_spacing;
    float labelY = starSprite->getPosition().y;
    label->setPosition({labelX, labelY});
    if(label != nullptr) this->addChild(label);
    this->addChild(starSprite);
    this->animationWithSprite(starSprite, 1.f, CCPointMake(-150, 0), 0);
    this->animationWithLabel(label, 1.f, CCPointMake(-150, 0), 0);
    this->schedule(schedule_selector(CoderLayer::starAnimationBack), 4.f, 0, 4.f);
    return;
}

void CoderLayer::showCoinAnimation(int amount) {
    CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(CCFileUtils::sharedFileUtils()->fullPathFromRelativePath("highscoreGet02.ogg"));
    auto win_size = CCDirector::sharedDirector()->getWinSize();
    starSprite = CCSprite::createWithSpriteFrameName("GJ_coinsIcon_001.png");
    starSprite->setPosition({win_size.width + 50, win_size.height - 20});
    label = CCLabelBMFont::create(CCString::createWithFormat("+%i", amount)->getCString(), "bigFont.fnt");
    float horizontal_spacing = 10.0f; 
    float labelX = starSprite->getPosition().x + starSprite->getContentSize().width / 2 + label->getContentSize().width / 2 + horizontal_spacing;
    float labelY = starSprite->getPosition().y;
    label->setPosition({labelX, labelY});
    if(label != nullptr) this->addChild(label);
    this->addChild(starSprite);
    this->animationWithSprite(starSprite, 1.f, CCPointMake(-150, 0), 0);
    this->animationWithLabel(label, 1.f, CCPointMake(-150, 0), 0);
    this->schedule(schedule_selector(CoderLayer::starAnimationBack), 4.f, 0, 4.f);
    return;
}

CCMenu* CoderLayer::createButtonWithFunction(const char* name, CCPoint position, cocos2d::SEL_MenuHandler selector) {
auto buttonMenu = CCMenu::create();
auto button = CCSprite::createWithSpriteFrameName(name);
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(button, button, this, selector);
buttonMenu->setPosition(position);
buttonMenu->addChild(menuBtn);
return buttonMenu;
}

CCMenu* CoderLayer::createSpriteButtonWithFunction(CCSprite* sprite, CCPoint position, cocos2d::SEL_MenuHandler selector) {
auto buttonMenu = CCMenu::create();
CCMenuItemSpriteExtra* menuBtn = CCMenuItemSpriteExtra::create(sprite, sprite, this, selector);
buttonMenu->setPosition(position);
buttonMenu->addChild(menuBtn);
return buttonMenu;
}

CCLabelBMFont* CoderLayer::newFont(const char* body, CCPoint position, const char* font, float scale) {
    auto titleLabel = CCLabelBMFont::create(
            CCString::createWithFormat("%s", body)->getCString(), 
            CCString::createWithFormat("%s", font)->getCString()
        );

  titleLabel->setPosition(position);
  titleLabel->setScale(scale);
  return titleLabel;
}
template <typename T, size_t size>
int random_array_index(const T (&array)[size]) {
    int num = std::rand() % size; 

    return num;
}
char* CoderLayer::randomString() {

      static const char* randArr[] = {
      "What brings you here?",
      "Get out!",
      "I don't like people saying my name.",
      "...",
      "Who are you?",
      "I'd rather be alone.",
      "Leave!",
      "You don't like it here?",
      "Don't say anything.",
      "I'm too tired to talk to you.",
      "I'm not having it.",
      "What are you doing?",
      "I hate giving you prizes."
};
    return const_cast<char*>(randArr[random_array_index(randArr)]);
}

void CoderLayer::updateLabel(CCLabelBMFont* label) {
    
    label->setString(CoderLayer::randomString());
    mainTitle->setColor(ccc3(255, 255, 255));
    return;
}
const std::string ALLOWED_FONT_CHARS = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 .,!?-";

bool isCharNotAllowed(char c) {
    return ALLOWED_FONT_CHARS.find(c) == std::string::npos;
}

std::string filterStringForFont(std::string input) {
    input.erase(std::remove_if(input.begin(), input.end(), isCharNotAllowed), input.end());
    return input;
}

char* cctolower(const char* input) {
    if (input == nullptr) {
        return nullptr;
    }

    // Allocate memory for the new lowercase string
    // +1 for the null terminator
    char* lowercase_str = new char[strlen(input) + 1]; 
    strcpy(lowercase_str, input); // Copy the content

    // Iterate through the copied string and convert characters
    for (int i = 0; lowercase_str[i] != '\0'; ++i) {
        lowercase_str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowercase_str[i])));
    }
    return lowercase_str;
}

void CoderLayer::updateMainLabel() {
    if(!globalInput || globalInput == nullptr) return;
    if(!mainTitle || mainTitle == nullptr) return;
    auto userInputCStr = MEMBER_BY_OFFSET(CCTextFieldTTF*, globalInput, 0x168)->getString(); // getTextField
    /* userInputStr = filterStringForFont(userInputStr);
    const char* userInputCStr = userInputStr.c_str(); */
    if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("GFFIFvFsFEGDG@GC").c_str()) == 0) { // violetps
        switch(currentDialog) {
            case 0:
            mainTitle->setString("I'd prefer if you wouldn't say that name.");
            mainTitle->setColor(ccc3(189, 97, 242));
            break;
            case 1:
            mainTitle->setString("Don't insist.");
            mainTitle->setColor(ccc3(189, 97, 242));
            break;
            case 2:
            mainTitle->setString("...");
            mainTitle->setColor(ccc3(189, 97, 242));
            break;
            case 3:
            doorClosed1 = true;
            auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
            CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("s1065.wav");
            CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("s1065.wav");
            this->scheduleOnce(schedule_selector(CoderLayer::onCloseWithDelay), 1.2f);
            cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
            
            def->setBoolForKey("doorClosed", true);
            this->doorIsClosed = true;
        }
        currentDialog++;
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FCFHFIG@G@GIFCFvFvFrFIFE").c_str()) == 0 && code1 == false) { // chippycookie
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("I'd wish i could eat that.");
        mainTitle->setColor(ccc3(189, 97, 242));
        starAmount = oldStarAmount + 20;
        this->showStarAnimation(20);
        this->reward(6, 20);
        code1 = true;
        def->setBoolForKey("code1", code1);
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("G@GEGBG@FsGI").c_str()) == 0 && code2 == false) { // purply
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("That's not my name.");
        mainTitle->setColor(ccc3(189, 97, 242));
        starAmount = oldStarAmount + 5;
        this->showStarAnimation(5);
        this->reward(6, 5);
        code2 = true;
        def->setBoolForKey("code2", code2);
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FCGIFBFEGBB@FuFAGDFIFvFu").c_str()) == 0 && code3 == false) { // cyber nation
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("Fine, here's two coins.");
        mainTitle->setColor(ccc3(189, 97, 242));
        coinAmount = oldCoinAmount + 2;
        this->showCoinAnimation(2);
        this->reward(7, 2);
        code3 = true;
        def->setBoolForKey("code3", code3);
    } else if(strcmp(cctolower(userInputCStr), "typethisforareward") == 0) { // typethisforareward
        mainTitle->setString(VCustomEncrypt::ce_decode("EGFHFAGDBsB@GIFvGEB@GDFHFvGEFGFHGDB@GIFvGEB@GGFEGBFEB@FGFEGDGDFIFuFGB@FAB@G@GBFIGqFECvB@DFFIFsGDFHGIB@FCFHFEFAGDFEGBBu").c_str());
        mainTitle->setColor(ccc3(189, 97, 242));
    globalInput->setString("");
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FFGEFuFuGIB@FCFvFDFEGC").c_str()) == 0 && code4 == false) { // funny codes
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("You're an observant.");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(10);
        this->reward(6, 10);
        code4 = true;
        def->setBoolForKey("code4", code4);
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FFGEFuFrGIGGFvGBFsFD").c_str()) == 0 && code5 == false) { // funkyworld
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("The 1.4 update, i miss it.");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(30);
        this->reward(6, 30);
        code5 = true;
        def->setBoolForKey("code5", code5);
        } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FGGBFIFsFsFEFDFCFHFIFCFrFEFu").c_str()) == 0 && code6 == false) { // grilledchicken
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("The chicken was on fire.");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(30);
        this->reward(6, 30);
        code6 = true;
        def->setBoolForKey("code6", code6);
        } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("G@GCGFFIFvFsFEGD").c_str()) == 0 && code7 == false) { // psviolet
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("That sounds familiar, i don't know where it comes from, is it edible?");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(15);
        this->reward(6, 15);
        code7 = true;
        def->setBoolForKey("code7", code7);
        } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FDFEFCFEFtFBFEGB").c_str()) == 0 && code8 == false) { // december
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("The month?");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(10);
        this->reward(6, 10);
        code8 = true;
        def->setBoolForKey("code8", code8);
    } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("G@GCGFFIFvFsFEGD").c_str()) == 0 && code9 == false) { // plsstars
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("Sure.");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(10);
        this->reward(6, 10);
        code9 = true;
        def->setBoolForKey("code9", code9);
        } else if(strcmp(cctolower(userInputCStr), VCustomEncrypt::ce_decode("FDFEFCFEFtFBFEGB").c_str()) == 0 && code10 == false) { // coinislife
        cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
        auto sharedengine = CocosDenshion::SimpleAudioEngine::sharedEngine();
        sharedengine->playEffect("highscoreGet02.ogg", false);
        mainTitle->setString("It definitely is, i am one myself, in fact.");
        mainTitle->setColor(ccc3(189, 97, 242));
        this->showStarAnimation(25);
        this->reward(6, 25);
        code10 = true;
        def->setBoolForKey("code10", code10);
   } else CoderLayer::updateLabel(mainTitle);
   globalInput->setString("");
}

bool CoderLayer::onTextFieldReturn(cocos2d::CCTextFieldTTF* pSender) {
    CoderLayer::updateMainLabel();
    return true; 
}

bool CoderLayer::checkIfTheDoorInCreateLayerInitIsClosedOrNot() {
    return doorClosed1;
}

bool CoderLayer::init(CCLayer* self) {
    cocos2d::CCUserDefault *def = cocos2d::CCUserDefault::sharedUserDefault();
    code1 = def->getBoolForKey("code1", false);
    code2 = def->getBoolForKey("code2", false);
    code3 = def->getBoolForKey("code3", false);
    code4 = def->getBoolForKey("code4", false);
    code5 = def->getBoolForKey("code5", false);
    code6 = def->getBoolForKey("code6", false);
    code7 = def->getBoolForKey("code7", false);
    code8 = def->getBoolForKey("code8", false);
    code7 = def->getBoolForKey("code9", false);
    code8 = def->getBoolForKey("code10", false);
    CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("highscoreGet02.ogg");
    auto director = CCDirector::sharedDirector();
    auto win_size = director->getWinSize();
    auto background = CCSprite::create("gradient_002.png");
    CCSize imageSize = background->getContentSize();
    float scaleX = win_size.width / imageSize.width;
    float scaleY = win_size.height / imageSize.height;
    float scale = std::max(scaleX, scaleY); 
    background->setScale(scale);
    background->_setZOrder(-100);
    background->setPosition(ccp(win_size.width / 2, win_size.height / 2));
    this->addChild(background);
    auto backButton = CoderLayer::createButtonWithFunction("GJ_arrow_01_001.png", ccp(20, win_size.height - 20), menu_selector(CoderLayer::onClose));
    this->addChild(backButton);

    auto title = CoderLayer::newFont("Forgotten Vault", ccp(win_size.width / 2, win_size.height - 20), "goldFont-hd.fnt", 1.0f);
    this->addChild(title);

    char* rand = CoderLayer::randomString();
    mainTitle = CoderLayer::newFont(rand, ccp(win_size.width / 2, win_size.height - 50), "bigFont-hd.fnt", 0.5f);
    this->addChild(mainTitle);

    auto bg = extension::CCScale9Sprite::create("square02_001.png", CCRectMake(0,0,80,80));
    bg->setContentSize(CCSizeMake(240, 60));
    bg->_setZOrder(-1);
    bg->setPosition({win_size.width / 2, win_size.height - 100});
    bg->setOpacity(127);
    this->addChild(bg);
    bg->setScale(0.9);

    CCTextInputNode* textInput = CCTextInputNode::create(100.0, 40.0, "...", "Thonburi", 12, "bigFont.fnt");
    globalInput = textInput;
    textInput->setTag(INPUT_TAG);
    MEMBER_BY_OFFSET(cocos2d::CCTextFieldDelegate*, textInput, 0x16c) = this; // setDelegate
    textInput->setPosition(ccp(win_size.width / 2, win_size.height - 100));
    textInput->setMaxLabelScale(0.7);
    textInput->setLabelPlaceholderScale(0.7);
    MEMBER_BY_OFFSET(int, textInput, 0x170) = 32;
    // textInput->setAllowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,+-/!?");
    textInput->setAnchorPoint({0, 0.5});
   CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("vaultAnimation-hd.plist");
CCSprite* animatedSprite = CCSprite::createWithSpriteFrameName("vault001.png");

if (animatedSprite == nullptr) {
    CCLOGERROR("Failed to create animatedSprite. Check plist/png files.");
    return false; 
}
CCArray* animFrames = CCArray::arrayWithCapacity(4);
char str[100] = {0};
for (int i = 1; i <= 4; i++)
{
    sprintf(str, "vault00%d.png", i);
    CCSpriteFrame* frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(str);
    if (frame) animFrames->addObject(frame);
}

CCAnimation* animation = CCAnimation::create(animFrames, 0.1f);
CCAnimate* animate = CCAnimate::create(animation);
CCActionInterval* repeatAction = CCRepeatForever::create(animate);
animatedSprite->runAction(repeatAction);
CCPoint desiredPosition = ccp(win_size.width / 2, win_size.height / 2 - 10);

CCMenuItemSprite* vaultMenuItem = CCMenuItemSprite::create(
    animatedSprite,
    NULL,
    NULL,
    this,
    menu_selector(CoderLayer::updateMainLabel)
);

vaultMenuItem->setPosition(ccp(
    roundf(desiredPosition.x), 
    roundf(desiredPosition.y)
));
vaultMenuItem->setScale(0.15f);
CCMenu* menu = CCMenu::create(vaultMenuItem, NULL);
menu->setPosition(CCPointZero);
this->addChild(menu);

    
    this->setKeypadEnabled(true);

    this->addChild(textInput);
    return true;
}

CCLayer* CoderLayer::create(CCLayer* referrer) {
    auto ret = new CoderLayer();
    if (ret->init(referrer)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* CoderLayer::scene(CCLayer* referrer) {
    CCLayer* instance = CoderLayer::create(referrer);
    CreatorLayer* creatorInstance = dynamic_cast<CreatorLayer*>(referrer);
    if (creatorInstance) {
        creatorInstance->setMusicFader(dynamic_cast<MusicFader*>(instance)); 
    }
    CCScene* scene = CCScene::create();
    
    scene->addChild(instance);

    return scene; 
}

void CreatorLayer::onSecret(CCLayer* referrer) {
    if(doorClosed1) {
        FLAlertLayer::create(
            nullptr,
            "Vault",
            CCString::createWithFormat("The door is closed.")->getCString(),
            "OK",
            nullptr,
            300.f
        )->show();
        return;
    }
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    CCScene* coderscene = CoderLayer::scene(referrer);
    CCTransitionFade* transition = CCTransitionFade::create(0.5f, coderscene);
    CCDirector::sharedDirector()->replaceScene(transition);
    CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("Heliosphere.mp3", true);
}

void CoderLayer::delayMenuLoop() {
    CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("menuLoop.mp3", true);
}
void CoderLayer::onClose(CCObject* sender) {
    if (sender) this->retain();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    CCScene* coderscene = CreatorLayer::scene();
    CCTransitionFade* transition = CCTransitionFade::create(0.5f, coderscene);
    CCDirector::sharedDirector()->replaceScene(transition);
    if(doorClosed1) {
        this->scheduleOnce(schedule_selector(CoderLayer::delayMenuLoop), 1.2f);
        return;
    }
    CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("menuLoop.mp3", true);
  }

  void CoderLayer::onCloseWithDelay(CCObject* sender) {
    if (sender) this->retain();
    CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
    CCScene* coderscene = CreatorLayer::scene();
    CCTransitionFade* transition = CCTransitionFade::create(0.5f, coderscene);
    CCDirector::sharedDirector()->replaceScene(transition);
        this->scheduleOnce(schedule_selector(CoderLayer::delayMenuLoop), 1.2f);
        return;
  }

void CoderLayer::keyBackClicked() {
    CoderLayer::onClose(nullptr);
}