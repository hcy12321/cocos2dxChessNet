#ifndef __STONE_H__
#define __STONE_H__

#include "cocos2d.h"
USING_NS_CC;

class SpriteStone : public CCSprite
{
public:
	CREATE_FUNC(SpriteStone);
	bool init();
	void initStone(int id);
	enum TYPE{ CHE, MA, PAO, BING, JIANG, SHI, XIANG };
	int _id;
	bool _red;
	int _row;
	int _col;
	bool _remove;
	TYPE _type;

	static int _r;
	static int _offx;
	static int _offy;
	static int _d;
	CCPoint fromPlate();

	static float _scaleValue;
};


#endif // !__STONE_H__

