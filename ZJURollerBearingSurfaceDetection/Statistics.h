/*!
 * \class CStatistics
 *
 * \brief ͳ�����ÿ�����м��Ĺ�������
 *
 * \author WuJing
 * \date ���� 2016
 */
#ifndef _STATISTICS_H
#define _STATISTICS_H
#include <vector>

//�����Ƿ���ȱ��
enum IsBearingRollerDefected {
	Defectiveness,   
	NoDefects
};

//��Ҫͳ�ƵĹ��������Ϣ
struct BearingRollerDetectionResult
{
	int BearingRollerID;
	IsBearingRollerDefected  isBearingRollerDefected;
	int EtchCount;
	int RustCount;
	int OtherDefect;
};

class CStatistics
{
public:
	CStatistics();
	~CStatistics();
	//��Ȿ�����г����������ӵ�ȱ����
	double CalculateRateOfDefect();
	//void CalculateRateOfDefect();

private:
	//ÿһ�����ӵ���Ϣ���뵽vector����
	std::vector<BearingRollerDetectionResult> m_List_Info_DetectedBearingRoller;
};

#endif 
