//	SapImageProcessing.cpp : implementation file
//
#include "stdafx.h"
#include <stdio.h>
#include "SapImageProcessing.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "CommonMethod.h"

//
// Constructor/Destructor
//
SapImageProcessing::SapImageProcessing(SapBuffer *pBuffers, SapProCallback pCallback, void *pContext, cv::Mat *pSrc, cv::Mat *pDst, int Thread)
	: SapProcessing(pBuffers, pCallback, pContext)
{
	m_ProcessBuffers = pBuffers;
	// m_ColorConv = pColorConv;
	m_pImageProcessResult = pSrc;
	m_pImageProcessComposite = pDst;
	m_nthresh = Thread;
}

SapImageProcessing::~SapImageProcessing()
{
	if (m_bInitOK)
		Destroy();
}


//
// Processing Control
//
BOOL SapImageProcessing::Run()
{
	BYTE *pData;
	m_ProcessBuffers->GetAddress((void **)&pData);
	//int pitch=m_ProcessBuffers->GetPitch();
	//// Access the buffer data
	//for (int lineNum = 0; lineNum < m_ProcessBuffers->GetHeight(); lineNum++)
	//{
	//	BYTE *pLine = (BYTE *)(pData + lineNum * pitch);
	//	for (int pixelNum = 0; pixelNum < m_ProcessBuffers ->GetWidth(); pixelNum++)
	//	{
	//		// Process the current line
	//		*(pLine + pixelNum)=255- *(pLine + pixelNum);
	//	}
	//}


	cv::Mat src_gray(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1, (void*)pData);
	//Mat src(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC3, (void*)pData);
	/*m_Src->create(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1);
	m_Src->data = pData;*/

	/*Mat src;*/
	//cvtColor(src_gray, src_gray, COLOR_BGRA2GRAY);
	//m_Src->create(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC3);
	//cvtColor(src_gray, *m_Src, COLOR_GRAY2BGR);

	*m_pImageProcessResult = src_gray.clone();
	cvtColor(*m_pImageProcessResult, *m_pImageProcessResult, cv::COLOR_GRAY2BGR);


	//Mat dst; GaussianBlur
	blur(src_gray, src_gray, cv::Size(3, 3));
	//GaussianBlur(src_gray, src_gray, Size(3, 3),0,0);
	//medianBlur(src_gray, src_gray, 3);
	//bilateralFilter(src_gray, src_gray, 5, 10, 2);

	thresh_callback(0, 0, m_pImageProcessResult, &src_gray, m_pImageProcessComposite);
	// Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP);
	//DIP with pBuffers;
   /*if (m_ColorConv->IsSoftwareEnabled())
   {
	  m_ColorConv->Convert( GetIndex());
   }*/

	return TRUE;
}

void SapImageProcessing::thresh_callback(int, void*, cv::Mat *src, cv::Mat *src_gray, cv::Mat *dst)
{
	cv::Mat threshold_output;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;

	threshold(*src_gray, threshold_output, m_nthresh, 255, cv::THRESH_BINARY);
	findContours(threshold_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));



	//除去太长或者太短的轮廓  
	int cmin = 23;  //23  200
	int cmax = 200;  //200 700
					 //vector<Vec4i>::iterator itc = contours.begin();
	std::vector<std::vector<cv::Point>>::iterator itc = contours.begin();
	while (itc != contours.end())
	{

		if (itc->size() < cmin || itc->size() > cmax)
			itc = contours.erase(itc);
		else
		{
			++itc;
		}

	}

	std::vector<double> vecNum;
	itc = contours.begin();
	LOG(TRACE) << "***************Bearing Roller Defect characteristic moment[hu]***************";
	int idefectsCounts = 0;
	int  RustCount = 0;   //锈蚀
	int  EtchCount = 0;   //刻蚀
	while (itc != contours.end())
	{
		++idefectsCounts;
		//Mat humomentMat(7, 1, CV_32FC1);
		//计算所有的距  
		CvMoments mom = cv::moments(cv::Mat(*itc++));
		//CvMoments moment;
		//cvMoments(src, &moment, 2);   //第三个像素点非0，则所有的0像素点被当做0，非0像素点被当做1  
		CvHuMoments humoment;
		cvGetHuMoments(&mom, &humoment);
		vecNum.push_back(humoment.hu1);
		vecNum.push_back(humoment.hu2);
		vecNum.push_back(humoment.hu3);
		vecNum.push_back(humoment.hu4);
		vecNum.push_back(humoment.hu5);
		vecNum.push_back(humoment.hu6);
		vecNum.push_back(humoment.hu7);
		el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
		if (humoment.hu1 > 0.6)
		{
			LOG(TRACE) << vecNum;
			++RustCount;
			LOG(TRACE) << L"*******************锈蚀************************";
		}
		else
		{
			LOG(TRACE) << vecNum;
			++EtchCount;
			LOG(TRACE) << L"*******************刻蚀************************";
		}

		vecNum.clear();
		//计算并画出质心  
		circle(*dst, cv::Point(mom.m10 / mom.m00, mom.m01 / mom.m00), 2, cv::Scalar(2), 2);
	}
	LOG(TRACE) << "*****************************************************************************";
	std::vector<cv::RotatedRect> minRect(contours.size());
	std::vector<cv::RotatedRect> minEllipse(contours.size());

	for (size_t i = 0; i < contours.size(); ++i)
	{
		minRect[i] = cv::minAreaRect(cv::Mat(contours[i]));
		if (contours[i].size() > 5)
		{
			minEllipse[i] = cv::fitEllipse(cv::Mat(contours[i]));
		}
	}

	cv::Mat drawing = cv::Mat::zeros(threshold_output.size(), CV_8UC3);
	dst->create(threshold_output.size(), CV_8UC3);
	drawing.copyTo(*dst);
	//Displaying_Random_Text(dst, "Test Text", 1000, 200);
	for (size_t i = 0; i < contours.size(); ++i)
	{
		cv::Scalar color = cv::Scalar(m_rng.uniform(0, 255), m_rng.uniform(0, 255), m_rng.uniform(0, 255));
		// contour
		drawContours(*dst, contours, static_cast<int>(i), color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
		// ellipse
		ellipse(*dst, minEllipse[i], color, 2, 8);
		// rotated rectangle
		cv::Point2f rect_points[4]; minRect[i].points(rect_points);
		for (int j = 0; j < 4; ++j)
			line(*dst, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
	}
	m_alpha = 0.4;
	m_beta = (1.0 - m_alpha);
	addWeighted(*src, m_alpha, *dst, m_beta, 0.0, *src);
	int infoConut = 0;
	char czInfo[100];
	sprintf(czInfo, "Thread Value: %d", m_nthresh);
	Displaying_Random_Text(dst, czInfo, 500, (++infoConut) * 45);

	sprintf(czInfo, "Defects Counts: %d", idefectsCounts);
	Displaying_Random_Text(dst, czInfo, 500, (++infoConut) * 45);

	if (RustCount)
	{
		sprintf(czInfo, "Rust Counts: %d", RustCount);
		Displaying_Random_Text(dst, czInfo, 500, (++infoConut) * 45);
	}

	if (EtchCount)
	{
		sprintf(czInfo, "Etch Counts: %d", EtchCount);
		Displaying_Random_Text(dst, czInfo, 500, (++infoConut) * 45);
	}
	CTime m_timeImageSequence = CTime::GetCurrentTime();
	CString szTime = m_timeImageSequence.Format("Detect Time: %Y/%m/%d %H:%M:%S");
	char* pszMultiByte = NULL;
	pszMultiByte = CCommonMethod::WCharToMByte(szTime);
	Displaying_Random_Text(dst, pszMultiByte, 500, (++infoConut) * 45);
	delete pszMultiByte;


	if (idefectsCounts != 0)
	{
		cv::Mat logo = cv::imread("judgelog/defective.bmp");
		cv::Mat mask = cv::imread("judgelog/defective.bmp", 0);
		cv::Mat imageROI;
		imageROI = (*dst)(cv::Rect(10, 10, logo.cols, logo.rows));
		addWeighted(imageROI, 1.0, logo, 2.0, 0, imageROI);
		Beep(0x0fff, 10);
	}
	else
	{
		cv::Mat logo = cv::imread("judgelog/voidofdefects.bmp");
		cv::Mat mask = cv::imread("judgelog/voidofdefects.bmp", 0);
		cv::Mat imageROI;
		imageROI = (*dst)(cv::Rect(10, 10, logo.cols, logo.rows));
		addWeighted(imageROI, 1.0, logo, 2.0, 0, imageROI);

		//cvAddText();
		//Beep(0x0fff, 100);
	}

	/*Beep(0x0fff, 20);
	Sleep(1000);
	Beep(0x0fff, 10);
	Sleep(1000);
	Beep(0x0fff, 50);
	Sleep(1000);*/

}


void SapImageProcessing::Displaying_Random_Text(cv::Mat *image, char* ShowInfo, int x, int y)
{
	//int lineType = 8;
	cv::Point org;
	if (y <= image->rows)
	{
		org.x = x;
		org.y = y;
	}
	else
	{
		org.x = x + 300;
		org.y = y - image->rows;
	}
	//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//putText(*image, ShowInfo, org, rng.uniform(0, 8),
	//	rng.uniform(0, 100)*0.05 + 0.1, color, rng.uniform(1, 10), lineType); //
	putText(*image, ShowInfo, org, cv::FONT_HERSHEY_PLAIN,
		3, CV_RGB(255, 255, 255), 3);

}


cv::Scalar SapImageProcessing::randomColor(cv::RNG& rng)
{
	int icolor = (unsigned)rng;
	return cv::Scalar(icolor & 255, (icolor >> 8) & 255, (icolor >> 16) & 255);
}