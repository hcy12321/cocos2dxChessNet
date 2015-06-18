#ifndef __LAYERSTART_H__
#define __LAYERSTART_H__
#include "cocos2d.h"
#include "cocos-ext.h"
USING_NS_CC;
USING_NS_CC_EXT;



class LayerStart : public CCLayer
{
public:
	CREATE_FUNC(LayerStart);
	bool init();

	void asServer(CCObject *);
	void asClient(CCObject *);

	void removeLabel(float);

	// ±‡º≠øÚ
	CCEditBox * m_edit;
	// ¥ÌŒÛÃ· æ
	CCLabelTTF * m_label;
};



#endif // !__LAYERSTART_H__
