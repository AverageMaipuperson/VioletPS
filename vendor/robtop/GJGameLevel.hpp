#pragma once

#include <cocos2d.h>
#include "PlayerObject.hpp"
#include "DS_Dictionary.hpp"

typedef enum {
    Local = 1,
    Editor = 2,
    Saved = 3 
} GJLevelType;

class GJGameLevel : public cocos2d::CCNode {
public:
    int m_nLevelID;
    std::string m_sLevelName;
    std::string m_sLevelDesc;
    std::string m_sLevelString;
    std::string m_sUserName;
    std::string m_sRecordString;
    int m_nUserID;
    int m_nDifficulty;
    int m_nAudioTrack;
    int m_nRatings;
    int m_nRatingsSum;
    int m_nDownloads;
    int m_nCompletes;
    bool m_bIsEditable;
    bool m_bIsVerified;
    bool m_bIsUploaded;
    int m_nLevelVersion;
    int m_nGameVersion;
    int m_nAttempts;
    int m_nJumps;
    int m_nNormalPercent;
    int m_nPracticePercent;
    int m_nLikes;
    int m_nDislikes;
    int m_nLevelLength;
    int m_nFeatureScore;
    bool m_bIsDemon;
    int m_nStars;
    int m_bAutoLevel;
    int m_nCoins;
    int m_nStarRatings;
    int m_nStarRatingsSum;
    int m_nMaxStarRatings;
    int m_nMinStarRatings;
    int m_nDemonVotes;
    bool m_bDontSave;
    bool m_bIsHidden;
    int m_nRequiredCoins;
    bool m_bIsUnlocked;
    cocos2d::CCPoint m_obLastCameraPos;
    float m_fLastEditorZoom;
    int m_nLastBuildPage;
    GJLevelType m_eLevelType;
    int m_nM_ID;

    static GJGameLevel* create();
    static GJGameLevel* create(cocos2d::CCDictionary dict);
    static GJGameLevel* createWithCoder(DS_Dictionary* dict);
	static int getFeatured(GJGameLevel* self);
	static int getLengthKey(GJGameLevel* self, int idk);
	virtual int getLevelLength();
    virtual bool init();
    virtual void setIsVerified(bool isVerified);
	virtual void setLevelName(std::string levelName);
	virtual void setLevelString(std::string levelString);
	virtual void setDifficulty(GJGameLevel* lvl, int difficulty);
    int getAverageDifficulty();
};
