#pragma once

#include <cocos2d.h>
#include "cocos2dExt.h"
#include "../cocos/cocos2dx/extensions/network/HttpClient.h"
#include "../cocos/cocos2dx/extensions/network/HttpRequest.h"
#include "../cocos/cocos2dx/extensions/network/HttpResponse.h"
using namespace cocos2d::extension;

class VersionRequest : public cocos2d::CCObject
{
	public:
	static VersionRequest* sharedRequest();
	void fetchVersion();
	void postUpdateNotification(float dt);
	cocos2d::CCInteger* m_receivedVersion;
	private:
    void networkThread();
};