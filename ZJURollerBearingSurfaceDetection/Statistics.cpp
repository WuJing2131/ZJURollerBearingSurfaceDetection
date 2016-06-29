#include "Statistics.h"

CStatistics::CStatistics()
{
}

CStatistics::~CStatistics()
{
}

double CStatistics::CalculateRateOfDefect()
{
	int Num_DetectedBearingRoller = m_List_Info_DetectedBearingRoller.size();
	int Num_DetectedBearingRollerWithDefect = 0;
	std::vector<BearingRollerDetectionResult>::iterator iteratorInfo;
	for (iteratorInfo = m_List_Info_DetectedBearingRoller.begin();
		iteratorInfo != m_List_Info_DetectedBearingRoller.end(); ++iteratorInfo)
	{
		if (iteratorInfo->isBearingRollerDefected == IsBearingRollerDefected::Defectiveness)
			Num_DetectedBearingRollerWithDefect++;
	}
	return Num_DetectedBearingRollerWithDefect / (double) Num_DetectedBearingRoller;
}
