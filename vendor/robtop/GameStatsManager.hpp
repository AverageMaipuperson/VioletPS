#include "cocos2d.h"

class GameStatsManager : public CCNode {
	public:
	static GameStatsManager* sharedState();
	void incrementStat(GameStatsManager* self, char* type, int amount);
	int getStat(GameStatsManager* self, char* type);
	static void reward(int type, int amount);
};