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
	//////////////////////////////////////////////////////////////////////////ʵ���߼�����
	void selectStone(CCTouch * touch);
	void moveStone(CCTouch * touch);
	// �ɹ��ƶ���ִ��
	void moveDone(CCObject * stone);
	// �ƶ�����
	bool canMove(int moveid, int row, int col, int killid);
	bool canMoveChe(int moveid, int row, int col);
	bool canMovePao(int moveid, int row, int col, int killid);
	bool canMoveMa(int moveid, int row, int col);
	bool canMoveBing(int moveid, int row, int col);
	bool canMoveJiang(int moveid, int row, int col, int killid);
	bool canMoveShi(int moveid, int row, int col);
	bool canMoveXiang(int moveid, int row, int col);
	// ��¼�ƶ���Ϣ
	void recordStep(int moveID, int killID, int rowFrom, int colFrom, int rowTo, int colTo);
	// �������
	void addCtrlPanel();
	// ����
	void Regret(CCObject *);
	void doRegret();
	void doRegret2();
	// ��������
	void startServer(CCObject *);
	void checkListen(float);
	// �����ͻ���
	void startClient(CCObject *);
	// ������
	void CheckRecv(float);
	//////////////////////////////////////////////////////////////////////////��������
	// ��ȡ����������֮������Ӹ���
	int getStoneCount(int row1, int col1, int row2, int col2);
	// ͨ����Ļ���꣬��ȡ�������꣬����ֵ�����false��ʾptScreen��������
	bool screen2Plate(const CCPoint & ptScreen, int & row, int& col);
	// ͨ���������꣬��ȡ�����ϵ�����ID���������-1��ʾ��λ����û�����ӣ��������-1�����Ƕ�Ӧ����ID
	int getStoneFromRowCol(int row, int col);
	// ͨ�����������ȡ��Ļ����
	CCPoint plate2Screen(int row, int col);
	// �ж�����������ɫ�Ƿ���ͬ
	bool isSameColor(int id1, int id2);

	
	//////////////////////////////////////////////////////////////////////////��Ա������������Ϸ�����е�״̬���
	SpriteStone * _s[32];
	// ѡ�б�Ǿ���
	CCSprite * _selectSprite;
	// ��¼�Ѿ�ѡ������ӵ�ID�������-1��ʾ��ѡ��
	int _selectID;
	// ���Ŵ�С
	static float _scaleValue;
	// ˭��
	bool _isRedRun;
	bool _bRedSide;
	// �Ƿ��Ƿ�����
	static bool _bServer;
	// ��¼��������ַ
	static CCString * m_ipAddress;
	// ��¼�ͻ���socket
	static SOCKET m_connect;
	// ��¼����켣��������
	vector<Step*> _steps;
	
};
#endif // !__LAYERGAME_H__



