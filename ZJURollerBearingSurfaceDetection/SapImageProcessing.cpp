//	SapImageProcessing.cpp : implementation file
//
#include "stdafx.h"
#include "SapImageProcessing.h"
#include "ZJURollerBearingSurfaceDetection.h"


//
// Constructor/Destructor
//
SapImageProcessing::SapImageProcessing(SapBuffer *pBuffers, SapProCallback pCallback, void *pContext,Mat *pSrc,Mat *pDst,int Thread)
   : SapProcessing(pBuffers, pCallback, pContext)
{
	m_ProcessBuffers = pBuffers;
    // m_ColorConv = pColorConv;
	 m_Src = pSrc ;
	 m_Dst = pDst;
	 thresh = Thread;
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

	
	Mat src_gray(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1, (void*)pData);
	//Mat src(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC3, (void*)pData);
	/*m_Src->create(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1);
	m_Src->data = pData;*/
	
	/*Mat src;*/
	//cvtColor(src_gray, src_gray, COLOR_BGRA2GRAY);
	//m_Src->create(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC3);
	//cvtColor(src_gray, *m_Src, COLOR_GRAY2BGR);

	*m_Src = src_gray.clone();
	cvtColor(*m_Src, *m_Src, COLOR_GRAY2BGR);

	
	//Mat dst;
	blur(src_gray, src_gray, Size(3, 3));

	thresh_callback(0, 0, m_Src,&src_gray, m_Dst);
	// Mat(int rows, int cols, int type, void* data, size_t step=AUTO_STEP);
	//DIP with pBuffers;
   /*if (m_ColorConv->IsSoftwareEnabled())
   {
      m_ColorConv->Convert( GetIndex());
   }*/

   return TRUE;
}

void SapImageProcessing::thresh_callback(int, void*,Mat *src, Mat *src_gray, Mat *dst)
{
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	threshold(*src_gray, threshold_output, thresh, 255, THRESH_BINARY);
	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//��ȥ̫������̫�̵�����  
	int cmin = 23;  //23  200
	int cmax = 200;  //200 700
					 //vector<Vec4i>::iterator itc = contours.begin();
	vector<vector<Point>>::iterator itc = contours.begin();
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
	while (itc != contours.end())
	{
		//Mat humomentMat(7, 1, CV_32FC1);
		//�������еľ�  
		CvMoments mom = moments(Mat(*itc++));
		//CvMoments moment;
		//cvMoments(src, &moment, 2);   //���������ص��0�������е�0���ص㱻����0����0���ص㱻����1  
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
		LOG(TRACE) << vecNum;
		vecNum.clear();
		//���㲢��������  
		circle(*dst, Point(mom.m10 / mom.m00, mom.m01 / mom.m00), 2, Scalar(2), 2);
	}
	LOG(TRACE) << "*****************************************************************************";
	vector<RotatedRect> minRect(contours.size());
	vector<RotatedRect> minEllipse(contours.size());

	for (size_t i = 0; i < contours.size(); i++)
	{
		minRect[i] = minAreaRect(Mat(contours[i]));
		if (contours[i].size() > 5)
		{
			minEllipse[i] = fitEllipse(Mat(contours[i]));
		}
	}

	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	dst->create(threshold_output.size(), CV_8UC3);
	drawing.copyTo(*dst);
	for (size_t i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		// contour
		drawContours(*dst, contours, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point());
		// ellipse
		ellipse(*dst, minEllipse[i], color, 2, 8);
		// rotated rectangle
		Point2f rect_points[4]; minRect[i].points(rect_points);
		for (int j = 0; j < 4; j++)
			line(*dst, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
	}
	alpha = 0.4;
	beta = (1.0 - alpha);
	addWeighted(*src, alpha, *dst, beta, 0.0, *src);
}