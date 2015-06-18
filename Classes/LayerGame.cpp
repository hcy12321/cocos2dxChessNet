#include "LayerGame.h"
#include "Net.h"

float LayerGame::_scaleValue = 0;
bool LayerGame::_bServer = false;
SOCKET LayerGame::m_connect = 0;
CCString * LayerGame::m_ipAddress = NULL;
bool LayerGame::init()
{
	if (!CCLayer::init())
	{
		return false;
	}

	// 摆放棋盘
	createPlate();

	// 棋子
	SpriteStone::_scaleValue = _scaleValue * 2.08f;
	SpriteStone::_d = CCDirector::sharedDirector()->getWinSize().height / 10;
	SpriteStone::_offx = SpriteStone::_d;
	SpriteStone::_offy = SpriteStone::_d / 2;
	for (int i = 0; i < 32; i++)
	{
		_s[i] = SpriteStone::create();
		_s[i]->initStone(i);
		addChild(_s[i]);
	}

	_selectSprite = CCSprite::create("selected.png");
	_selectSprite->setScale(SpriteStone::_scaleValue);
	_selectSprite->setVisible(false);
	addChild(_selectSprite);

	// 使用触摸
	setTouchEnabled(true);
	setTouchMode(kCCTouchesOneByOne);
	_selectID = -1;
	_isRedRun = true;
	// 走棋轨迹
	_steps.clear();

	// 增加控制面板
	addCtrlPanel();
	return true;
}

void LayerGame::addCtrlPanel()
{
	CCMenu * menu = CCMenu::create();
	addChild(menu);

	CCMenuItemImage * regretItem = CCMenuItemImage::create("regret.jpg", "regret.jpg", this, menu_selector(LayerGame::Regret));
	menu->addChild(regretItem);
	moveNode(regretItem, ccp(160, 60));
	if (_bServer)
	{
		startServer(NULL);
	}
	else
	{
		startClient(NULL);
	}
	/*
	// 启动服务按钮
	CCMenuItem * startServerItem = CCMenuItemFont::create("Server", this, menu_selector(LayerGame::startServer));
	menu->addChild(startServerItem);
	moveNode(startServerItem, ccp(160, 0));
	// 客户端启动按钮
	CCMenuItem * startClientItem = CCMenuItemFont::create("Client", this, menu_selector(LayerGame::startClient));
	menu->addChild(startClientItem);
	moveNode(startClientItem, ccp(160, -60));
	*/
}

void LayerGame::startClient(CCObject *)
{
	_bRedSide = false;
	// 游戏开始
	CCLog("client Start Game");

	// 把棋子倒过来
	for (int i = 0; i < 32; i++)
	{
		SpriteStone * s = _s[i];
		s->_row = 9 - s->_row;
		s->_col = 8 - s->_col;
		s->setPosition(s->fromPlate());
	}

	// 开始接收
	Net::RecvStart();
	schedule(schedule_selector(LayerGame::CheckRecv));

}

void LayerGame::CheckRecv(float)
{
	if (Net::isRecvComplete())
	{
		unschedule(schedule_selector(LayerGame::CheckRecv));
		int len;
		char * data =Net::RecvData(len);
		// 根据数据解析协议，根据协议做具体
		if (data[0] == 3)
		{
			doRegret2();
			// 继续接收
			Net::RecvStart();
			schedule(schedule_selector(LayerGame::CheckRecv));
		}
		else if (data[0] == 1)
		{
			// 选中
			_selectID = data[1];
			_selectSprite->setPosition(_s[_selectID]->fromPlate());
			_selectSprite->setVisible(true);
			// 继续接收
			Net::RecvStart();
			schedule(schedule_selector(LayerGame::CheckRecv));
		}
		else if (data[0] == 2)
		{
			// 移动棋子
			SpriteStone * s = _s[data[1]];
			SpriteStone * goalStone = NULL;
			int row = 9 - data[2];
			int col = 8 - data[3];
			int killID = getStoneFromRowCol(row, col);
			if (killID != -1)
			{
				goalStone = _s[killID];
			}
			// 记录移动信息
			recordStep(_selectID, killID, s->_row, _s[_selectID]->_col, row, col);
			// 移动棋子
			s->_row = row;
			s->_col = col;
			CCMoveTo * to = CCMoveTo::create(0.1f, plate2Screen(row, col));
			CCCallFuncO * actionDown = CCCallFuncO::create(this, callfuncO_selector(LayerGame::moveDone), goalStone);
			s->runAction(CCSequence::create(to, actionDown, NULL));
		}
	}
}

void LayerGame::startServer(CCObject *)
{
	_bRedSide = true;
	Net::Listen();
	schedule(schedule_selector(LayerGame::checkListen));
}
void LayerGame::checkListen(float)
{
	// 连接成功
	if (Net::isConnected())
	{
		unschedule(schedule_selector(LayerGame::checkListen));
		// 游戏开始，做一些初始化工作
		CCLog("server start game...");
	}
}
void LayerGame::doRegret()
{
	// 游戏还没开始或刚开始不能悔棋
	if (_steps.size() <= 1)
		return;
	// 恢复最后一步
	Step * step = *_steps.rbegin();
	_steps.pop_back();
	// 具体恢复工作
	SpriteStone * s = _s[step->moveID];
	s->_row = step->rowFrom;
	s->_col = step->colFrom;
	SpriteStone * kill = NULL;
	CCMoveTo *to = CCMoveTo::create(0.2f, s->fromPlate());
	s->runAction(to);
	if (step->killID != -1)
	{
		kill = _s[step->killID];
		kill->_remove = false;
		kill->setVisible(true);
	}
	_isRedRun = !_isRedRun;
	delete step;

	// 避免在选中棋子的情况下悔棋
	_selectID = -1;
	_selectSprite->setVisible(false);
}
void LayerGame::doRegret2()
{
	doRegret();
	doRegret();
}
void LayerGame::Regret(CCObject *)
{
	// 轮到谁走，谁有权点
	if (_bRedSide != _isRedRun)
		return;
	doRegret2();
	// 发送信息
	char buf = 3;
	Net::Send(&buf, 1);
}
void LayerGame::moveNode(CCMenuItem * node, CCPoint pt)
{
	node->setPosition(node->getPosition() + pt);
}
void LayerGame::onExit()
{
	CCLayer::onExit();
}
void LayerGame::createPlate()
{
	CCSprite * plate = CCSprite::create("background.png");
	addChild(plate);
	plate->setPosition(ccp(0, 0));
	plate->setAnchorPoint(ccp(0, 0));
	_scaleValue = CCDirector::sharedDirector()->getWinSize().height / plate->getContentSize().height;
	plate->setScale(_scaleValue);
	
}

bool LayerGame::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
	return true;
}
// 一次选择，一次移动
void LayerGame::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{
	// 移动象棋
	if (_selectID == -1)
	{
		// 选中棋子
		selectStone(pTouch);
	}
	else
	{
		// 移动棋子
		moveStone(pTouch);
	}
}

void LayerGame::selectStone(CCTouch * touch)
{
	CCPoint ptClick = touch->getLocation();
	int row, col;
	bool bClick = screen2Plate(ptClick, row, col);
	// 如果点击在棋盘外，返回
	if (!bClick)
	{
		return;
	}
	int clickID = getStoneFromRowCol(row, col);
	// 点击位置没有象棋，点击无效
	if (clickID == -1)
	{
		return;
	}
	// 如果被点中的棋子和_isRedRun不一致返回，网络版还要满足黑方红方
	if (_s[clickID]->_red != _isRedRun || _s[clickID]->_red != _bRedSide)
	{
		return;
	}

	_selectID = clickID;
	// 显示改棋子被选中的效果
	_selectSprite->setPosition(plate2Screen(row, col));
	_selectSprite->setVisible(true);
	// 发送选择棋子信息给对方
	char buf[2] = { 0 };
	buf[0] = 1;
	buf[1] = _selectID;
	Net::Send(buf, 2);
}

void LayerGame::moveStone(CCTouch * touch)
{
	CCPoint ptClick = touch->getLocation();
	int row, col;
	bool bClick = screen2Plate(ptClick, row, col);
	// 如果点击在棋盘外，返回
	if (!bClick)
	{
		return;
	}
	// 检测目标地点是否有棋子
	int clickID = getStoneFromRowCol(row, col);
	SpriteStone * goalStone = NULL;
	if (clickID != -1)
	{
		if (isSameColor(clickID, _selectID))
		{
			_selectID = clickID;
			_selectSprite->setPosition(plate2Screen(row, col));
			_selectSprite->setVisible(true);
			return;
		}
		goalStone = _s[clickID];
	}
	// 是否可以移动
	if (!canMove(_selectID, row, col, clickID))
	{
		return;
	}
	// 可以移动
	// 记录移动信息
	recordStep(_selectID, clickID, _s[_selectID]->_row, _s[_selectID]->_col, row, col);
	_s[_selectID]->_row = row;
	_s[_selectID]->_col = col;
	//_s[_selectID]->setPosition(plate2Screen(row, col));
	CCMoveTo * to = CCMoveTo::create(0.1f, plate2Screen(row, col));
	CCCallFuncO * actionDown = CCCallFuncO::create(this, callfuncO_selector(LayerGame::moveDone), goalStone);
	_s[_selectID]->runAction(CCSequence::create(to, actionDown, NULL));
	// 发送移动棋子报文
	// 发送移动棋子报文
	Step * s = *_steps.rbegin();
	char buf[4];
	buf[0] = 2;
	buf[1] = s->moveID;
	buf[2] = s->rowTo;
	buf[3] = s->colTo;
	Net::Send(buf, 4);
	// 接收信息
	Net::RecvStart();
	schedule(schedule_selector(LayerGame::CheckRecv));
}
void LayerGame::moveDone(CCObject * stone)
{
	// 不为空则移除
	if (stone != NULL)
	{
		SpriteStone * spriteStone = (SpriteStone *)stone;
		spriteStone->setVisible(false);
		spriteStone->_remove = true;
	}
	_selectID = -1;
	_selectSprite->setVisible(false);
	_isRedRun = !_isRedRun;
}
void LayerGame::recordStep(int moveID, int killID, int rowFrom, int colFrom, int rowTo, int colTo)
{
	Step * s = new Step();
	s->colFrom = colFrom;
	s->colTo = colTo;
	s->killID = killID;
	s->moveID = moveID;
	s->rowFrom = rowFrom;
	s->rowTo = rowTo;
	_steps.push_back(s);
}
bool LayerGame::canMove(int moveid, int row, int col, int killid)
{
	SpriteStone * s = _s[moveid];
	switch (s->_type)
	{
	case SpriteStone::CHE:
		return canMoveChe(moveid, row, col);
		break;
	case SpriteStone::MA:
		return canMoveMa(moveid, row, col);
		break;
	case SpriteStone::PAO:
		return canMovePao(moveid, row, col, killid);
		break;
	case SpriteStone::BING:
		return canMoveBing(moveid, row, col);
		break;
	case SpriteStone::JIANG:
		return canMoveJiang(moveid, row, col, killid);
		break;
	case SpriteStone::SHI:
		return canMoveShi(moveid, row, col);
		break;
	case SpriteStone::XIANG:
		return canMoveXiang(moveid, row, col);
		break;
	}
	return true;
}
bool LayerGame::canMoveChe(int moveid, int row, int col)
{
	// 一条线中间不能有棋子
	SpriteStone * s = _s[moveid];
	return getStoneCount(s->_row, s->_col, row, col) == 0;
}
bool LayerGame::canMoveMa(int moveid, int row, int col)
{
	SpriteStone * s = _s[moveid];
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	if (d == 12 || d == 21)
	{
		// 蹩脚位置
		int cRow, cCol;
		if (d == 12)
		{
			cCol = (col + s->_col) / 2;
			cRow = s->_row;
		}
		else
		{
			cCol = s->_col;
			cRow = (s->_row + row) /2;
		}
		if (getStoneFromRowCol(cRow, cCol) == -1)
			return true;
	}
	return false;
}
bool LayerGame::canMovePao(int moveid, int row, int col, int killid)
{
	SpriteStone * s = _s[moveid];
	if (killid == -1)
		return getStoneCount(s->_row, s->_col, row, col) == 0;
	// 隔子打
	return getStoneCount(s->_row, s->_col, row, col) == 1;
}
bool LayerGame::canMoveBing(int moveid, int row, int col)
{
	SpriteStone * s = _s[moveid];
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	// 只能走一格
	if (d != 1 && d != 10)
		return false;
	/*
	if (s->_red)
	{
	*/
		// 不能后退
		if (row < s->_row) return false;
		// 过河前不能平移
		if (s->_row <= 4 && s->_row == row) return false;
	/*
	}
	else
	{
		if (row > s->_row) return false;
		if (s->_row >= 5 && s->_row == row) return false;
	}
	*/
	return true;
}

bool LayerGame::canMoveJiang(int moveid, int row, int col, int killid)
{
	SpriteStone * s = _s[moveid];
	// 将照面
	if (killid != -1)
	{
		SpriteStone * kill = _s[killid];
		if (kill->_type == SpriteStone::JIANG)
		{
			return canMoveChe(moveid, row, col);
		}
	}
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	// 只能走一格
	if (d != 1 && d != 10)
		return false;
	// 不能出九宫
	if (col < 3 || col > 5) return false;
	/*
	if (s->_red)
	{
	*/
		if (row < 0 || row > 2) return false;
	/*
	}
	else
	{
		if (row < 7 || row > 9) return false;
	}
	*/
	return true;
}
bool LayerGame::canMoveShi(int moveid, int row, int col)
{
	SpriteStone * s = _s[moveid];
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	if (d != 11) return false;

	/*
	if (s->_red)
	{
	*/
	if (row < 0 || row > 2) return false;
	/*
	}
	else
	{
	if (row < 7 || row > 9) return false;
	}
	*/
	return true;
}
bool LayerGame::canMoveXiang(int moveid, int row, int col)
{
	SpriteStone * s = _s[moveid];
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	if (d != 22) return false;
	// 象眼
	int cRow, cCol;
	cRow = (row + s->_row) / 2;
	cCol = (col + s->_col) / 2;
	if (getStoneFromRowCol(cRow, cCol) != -1)
		return false;
	// 象不能过河
	/*
	if (s->_red)
	{
	*/
		if (row > 4) return false;
	/*
	}
	else
	{
		if (row < 5) return false;
	}
	*/
	return true;
}
int LayerGame::getStoneCount(int row1, int col1, int row2, int col2)
{
	int ret = 0;
	if (row1 != row2 && col1 != col2) return -1;
	if (row1 == row2 && col1 == col2) return -1;

	// row1 == row2 或者col1 == col2
	if (row1 == row2)
	{
		int min, max;
		min = col1 < col2 ? col1 : col2;
		max = col1 > col2 ? col1 : col2;
		for (int col = min + 1; col < max; ++col)
		{
			int id = getStoneFromRowCol(row1, col);
			if (id != -1) ++ret;
		}
	}
	else
	{
		int min, max;
		min = row1 < row2 ? row1 : row2;
		max = row1 > row2 ? row1 : row2;
		for (int row = min + 1; row < max; ++row)
		{
			int id = getStoneFromRowCol(row, col1);
			if (id != -1) ++ret;
		}
	}
	return ret;
}

CCPoint LayerGame::plate2Screen(int row, int col)
{
	int x = col * SpriteStone::_d + SpriteStone::_offx;
	int y = row * SpriteStone::_d + SpriteStone::_offy;
	return ccp(x, y);
}
bool LayerGame::screen2Plate(const CCPoint & ptScreen, int & row, int& col)
{

	// 遍历所有棋子，计算棋盘格子坐标点与选中点距离
	int distance = SpriteStone::_d * SpriteStone::_d / 4;
	for (row = 0; row <= 9; ++row)
		for (col = 0; col <= 8; ++col)
		{
			CCPoint ptCenter = plate2Screen(row, col);
			if (ptCenter.getDistanceSq(ptScreen) < distance)
			{
				return true;
			}
		}
	return false;
}

int LayerGame::getStoneFromRowCol(int row, int col)
{
	for (int i = 0; i < 32; ++i)
	{
		if (_s[i]->_row == row && _s[i]->_col == col && !_s[i]->_remove)
			return i;
	}
	return -1;
}
bool LayerGame::isSameColor(int id1, int id2)
{
	return _s[id1]->_red == _s[id2]->_red;
}