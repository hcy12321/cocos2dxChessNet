#include "LayerStart.h"
#include "LayerGame.h"
#include "Net.h"
#define winSize CCDirector::sharedDirector()->getWinSize()

bool LayerStart::init()
{
	CCLayer::init();
	CCScale9Sprite * sprite = CCScale9Sprite::create("switch-mask.png");

	CCEditBox * edit = CCEditBox::create(CCSize(200, 40), sprite);
	edit->setPlaceHolder("server ip");
	edit->setMaxLength(15);
	edit->setFontColor(ccc3(0, 0, 0));
	addChild(edit);
	m_edit = edit;
	edit->setPosition(CCPoint(winSize.width / 2, winSize.height / 2));

	CCMenu * menu = CCMenu::create();
	menu->setPosition(ccp(winSize.width / 2, winSize.height / 3));
	addChild(menu);


	CCMenuItem * itemServer = CCMenuItemFont::create("As server");
	itemServer->setTarget(this, menu_selector(LayerStart::asServer));
	menu->addChild(itemServer);
	CCMenuItem * itemClient = CCMenuItemFont::create("As Client");
	itemClient->setTarget(this, menu_selector(LayerStart::asClient));
	menu->addChild(itemClient);
	menu->alignItemsHorizontallyWithPadding(20);
	return true;
}


void LayerStart::asServer(CCObject *)
{
	LayerGame::_bServer = true;
	CCScene * scene = CCScene::create();
	scene->addChild(LayerGame::create());
	CCDirector::sharedDirector()->replaceScene(scene);
}
void LayerStart::asClient(CCObject *)
{

	LayerGame::_bServer = false;
	
	if (Net::Connect(m_edit->getText()))
	{
		CCScene * scene = CCScene::create();
		scene->addChild(LayerGame::create());
		LayerGame::m_ipAddress = CCString::create(m_edit->getText());
		CCDirector::sharedDirector()->replaceScene(scene);
	}
	else
	{
		CCLabelTTF * label = CCLabelTTF::create("connect failure", "Arial", 20);
		label->setPosition(ccp(winSize.width / 2, winSize.height * 2 / 3));
		addChild(label);
		m_label = label;
		scheduleOnce(schedule_selector(LayerStart::removeLabel), 3);
	}
	
}

void LayerStart::removeLabel(float)
{
	if (m_label != NULL)
	{
		m_label->removeFromParent();
		m_label = NULL;
	}
}