#ifndef __LAYERGAME_H__
#define __LAYERGAME_H__
#include "cocos2d.h"
USING_NS_CC;

#include "SpriteStone.h"
#include <vector>
using namespace std;


#ifdef WIN32
#include <WinSock2.h>
#else
// in linux android
#define SOCKET int
#endif

struct Step
{
	int moveID;
	int killID;
	int rowFrom;
	int colFrom;
	int rowTo;
	int colTo;
};

class LayerGame:public CCLayer
{
public:
	CREATE_FUNC(LayerGame);
	bool init();
	void createPlate();

	bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
	void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);
	void onExit();
	void moveNode(CCMenuItem * item, CCPoint point);
	//////////////////////////////////////////////////////////////////////////实现逻辑函数
	void selectStone(CCTouch * touch);
	void moveStone(CCTouch * touch);
	// 成功移动后执行
	void moveDone(CCObject * stone);
	// 移动规则
	bool canMove(int moveid, int row, int col, int killid);
	bool canMoveChe(int moveid, int row, int col);
	bool canMovePao(int moveid, int row, int col, int killid);
	bool canMoveMa(int moveid, int row, int col);
	bool canMoveBing(int moveid, int row, int col);
	bool canMoveJiang(int moveid, int row, int col, int killid);
	bool canMoveShi(int moveid, int row, int col);
	bool canMoveXiang(int moveid, int row, int col);
	// 记录移动信息
	void recordStep(int moveID, int killID, int rowFrom, int colFrom, int rowTo, int colTo);
	// 控制面板
	void addCtrlPanel();
	// 悔棋
	void Regret(CCObject *);
	void doRegret();
	void doRegret2();
	// 启动服务
	void startServer(CCObject *);
	void checkListen(float);
	// 启动客户端
	void startClient(CCObject *);
	// 检查接收
	void CheckRecv(float);
	//////////////////////////////////////////////////////////////////////////辅助函数
	// 获取两个点坐标之间的棋子个数
	int getStoneCount(int row1, int col1, int row2, int col2);
	// 通过屏幕坐标，获取棋盘坐标，返回值如果是false表示ptScreen在棋盘外
	bool screen2Plate(const CCPoint & ptScreen, int & row, int& col);
	// 通过棋盘坐标，获取坐标上的棋子ID，如果返回-1表示该位置上没有棋子，如果不是-1，就是对应棋子ID
	int getStoneFromRowCol(int row, int col);
	// 通过象棋坐标获取屏幕坐标
	CCPoint plate2Screen(int row, int col);
	// 判断两个棋子颜色是否相同
	bool isSameColor(int id1, int id2);

	
	//////////////////////////////////////////////////////////////////////////成员变量，用于游戏过程中的状态标记
	SpriteStone * _s[32];
	// 选中标记精灵
	CCSprite * _selectSprite;
	// 记录已经选择的棋子的ID，如果是-1表示被选中
	int _selectID;
	// 缩放大小
	static float _scaleValue;
	// 谁走
	bool _isRedRun;
	bool _bRedSide;
	// 是否是服务器
	static bool _bServer;
	// 记录服务器地址
	static CCString * m_ipAddress;
	// 记录客户端socket
	static SOCKET m_connect;
	// 记录走棋轨迹用来悔棋
	vector<Step*> _steps;
	
};
#endif // !__LAYERGAME_H__



