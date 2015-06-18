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

	// �ڷ�����
	createPlate();

	// ����
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

	// ʹ�ô���
	setTouchEnabled(true);
	setTouchMode(kCCTouchesOneByOne);
	_selectID = -1;
	_isRedRun = true;
	// ����켣
	_steps.clear();

	// ���ӿ������
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
	// ��������ť
	CCMenuItem * startServerItem = CCMenuItemFont::create("Server", this, menu_selector(LayerGame::startServer));
	menu->addChild(startServerItem);
	moveNode(startServerItem, ccp(160, 0));
	// �ͻ���������ť
	CCMenuItem * startClientItem = CCMenuItemFont::create("Client", this, menu_selector(LayerGame::startClient));
	menu->addChild(startClientItem);
	moveNode(startClientItem, ccp(160, -60));
	*/
}

void LayerGame::startClient(CCObject *)
{
	_bRedSide = false;
	// ��Ϸ��ʼ
	CCLog("client Start Game");

	// �����ӵ�����
	for (int i = 0; i < 32; i++)
	{
		SpriteStone * s = _s[i];
		s->_row = 9 - s->_row;
		s->_col = 8 - s->_col;
		s->setPosition(s->fromPlate());
	}

	// ��ʼ����
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
		// �������ݽ���Э�飬����Э��������
		if (data[0] == 3)
		{
			doRegret2();
			// ��������
			Net::RecvStart();
			schedule(schedule_selector(LayerGame::CheckRecv));
		}
		else if (data[0] == 1)
		{
			// ѡ��
			_selectID = data[1];
			_selectSprite->setPosition(_s[_selectID]->fromPlate());
			_selectSprite->setVisible(true);
			// ��������
			Net::RecvStart();
			schedule(schedule_selector(LayerGame::CheckRecv));
		}
		else if (data[0] == 2)
		{
			// �ƶ�����
			SpriteStone * s = _s[data[1]];
			SpriteStone * goalStone = NULL;
			int row = 9 - data[2];
			int col = 8 - data[3];
			int killID = getStoneFromRowCol(row, col);
			if (killID != -1)
			{
				goalStone = _s[killID];
			}
			// ��¼�ƶ���Ϣ
			recordStep(_selectID, killID, s->_row, _s[_selectID]->_col, row, col);
			// �ƶ�����
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
	// ���ӳɹ�
	if (Net::isConnected())
	{
		unschedule(schedule_selector(LayerGame::checkListen));
		// ��Ϸ��ʼ����һЩ��ʼ������
		CCLog("server start game...");
	}
}
void LayerGame::doRegret()
{
	// ��Ϸ��û��ʼ��տ�ʼ���ܻ���
	if (_steps.size() <= 1)
		return;
	// �ָ����һ��
	Step * step = *_steps.rbegin();
	_steps.pop_back();
	// ����ָ�����
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

	// ������ѡ�����ӵ�����»���
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
	// �ֵ�˭�ߣ�˭��Ȩ��
	if (_bRedSide != _isRedRun)
		return;
	doRegret2();
	// ������Ϣ
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
// һ��ѡ��һ���ƶ�
void LayerGame::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{
	// �ƶ�����
	if (_selectID == -1)
	{
		// ѡ������
		selectStone(pTouch);
	}
	else
	{
		// �ƶ�����
		moveStone(pTouch);
	}
}

void LayerGame::selectStone(CCTouch * touch)
{
	CCPoint ptClick = touch->getLocation();
	int row, col;
	bool bClick = screen2Plate(ptClick, row, col);
	// �������������⣬����
	if (!bClick)
	{
		return;
	}
	int clickID = getStoneFromRowCol(row, col);
	// ���λ��û�����壬�����Ч
	if (clickID == -1)
	{
		return;
	}
	// ��������е����Ӻ�_isRedRun��һ�·��أ�����滹Ҫ����ڷ��췽
	if (_s[clickID]->_red != _isRedRun || _s[clickID]->_red != _bRedSide)
	{
		return;
	}

	_selectID = clickID;
	// ��ʾ�����ӱ�ѡ�е�Ч��
	_selectSprite->setPosition(plate2Screen(row, col));
	_selectSprite->setVisible(true);
	// ����ѡ��������Ϣ���Է�
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
	// �������������⣬����
	if (!bClick)
	{
		return;
	}
	// ���Ŀ��ص��Ƿ�������
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
	// �Ƿ�����ƶ�
	if (!canMove(_selectID, row, col, clickID))
	{
		return;
	}
	// �����ƶ�
	// ��¼�ƶ���Ϣ
	recordStep(_selectID, clickID, _s[_selectID]->_row, _s[_selectID]->_col, row, col);
	_s[_selectID]->_row = row;
	_s[_selectID]->_col = col;
	//_s[_selectID]->setPosition(plate2Screen(row, col));
	CCMoveTo * to = CCMoveTo::create(0.1f, plate2Screen(row, col));
	CCCallFuncO * actionDown = CCCallFuncO::create(this, callfuncO_selector(LayerGame::moveDone), goalStone);
	_s[_selectID]->runAction(CCSequence::create(to, actionDown, NULL));
	// �����ƶ����ӱ���
	// �����ƶ����ӱ���
	Step * s = *_steps.rbegin();
	char buf[4];
	buf[0] = 2;
	buf[1] = s->moveID;
	buf[2] = s->rowTo;
	buf[3] = s->colTo;
	Net::Send(buf, 4);
	// ������Ϣ
	Net::RecvStart();
	schedule(schedule_selector(LayerGame::CheckRecv));
}
void LayerGame::moveDone(CCObject * stone)
{
	// ��Ϊ�����Ƴ�
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
	// һ�����м䲻��������
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
		// ����λ��
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
	// ���Ӵ�
	return getStoneCount(s->_row, s->_col, row, col) == 1;
}
bool LayerGame::canMoveBing(int moveid, int row, int col)
{
	SpriteStone * s = _s[moveid];
	int dRow = abs(s->_row - row);
	int dCol = abs(s->_col - col);
	int d = dRow * 10 + dCol;
	// ֻ����һ��
	if (d != 1 && d != 10)
		return false;
	/*
	if (s->_red)
	{
	*/
		// ���ܺ���
		if (row < s->_row) return false;
		// ����ǰ����ƽ��
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
	// ������
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
	// ֻ����һ��
	if (d != 1 && d != 10)
		return false;
	// ���ܳ��Ź�
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
	// ����
	int cRow, cCol;
	cRow = (row + s->_row) / 2;
	cCol = (col + s->_col) / 2;
	if (getStoneFromRowCol(cRow, cCol) != -1)
		return false;
	// ���ܹ���
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

	// row1 == row2 ����col1 == col2
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

	// �����������ӣ��������̸����������ѡ�е����
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