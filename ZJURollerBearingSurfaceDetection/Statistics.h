/*!
 * \class CStatistics
 *
 * \brief 统计软件每次运行检测的滚子数据
 *
 * \author WuJing
 * \date 六月 2016
 */
#ifndef _STATISTICS_H
#define _STATISTICS_H
#include <vector>

//滚子是否有缺陷
enum IsBearingRollerDefected {
	Defectiveness,   
	NoDefects
};

//需要统计的滚子相关信息
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
	//检测本次运行程序所检测滚子的缺陷率
	double CalculateRateOfDefect();
	//void CalculateRateOfDefect();

private:
	//每一个滚子的信息存入到vector里面
	std::vector<BearingRollerDetectionResult> m_List_Info_DetectedBearingRoller;
};

#endif 
