#include "cocos2d.h"
#include "MusicFader.hpp"

class CreatorLayer : public CCLayer, public MusicFader {
	private:
    MusicFader* m_musicFader = nullptr;
    public:
    void onSecret(CCLayer* refereer);
	static CCScene* scene();
	void setMusicFader(MusicFader* fader) {
        m_musicFader = fader;
    }
};