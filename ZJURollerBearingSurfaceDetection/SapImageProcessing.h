#ifndef _SAPIMAGEPROCESSING_H_
#define _SAPIAMGEPROCESSING_H_

//	SapMyProcessing.h : header file
//
#include "SapClassBasic.h"
#include <opencv2\opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>

using namespace cv;

using namespace std;

//
// SapMyProcessing class declaration
//
class SapImageProcessing : public SapProcessing
{
public:
	// Constructor/Destructor
	SapImageProcessing(SapBuffer *pBuffers, SapProCallback pCallback, void *pContext, Mat *pSrc, Mat *pDst, int Thread);
	virtual ~SapImageProcessing();
	void Displaying_Random_Text(Mat* image, char* window_name, int x, int y);
	static Scalar randomColor(RNG& rng);
	void thresh_callback(int, void*, Mat *src, Mat *src_gray, Mat *dst);

	int    m_thresh = 40;
	int    m_max_thresh = 255;
	RNG    m_rng;
	double m_alpha = 0.5; 
	double m_beta;
	Mat *  m_Src; 
	Mat *  m_Dst;

protected:
	virtual BOOL Run();

protected:
	SapBuffer*  m_ProcessBuffers;
	//SapColorConversion*	m_ColorConv;
};

#endif	// _SAPIMAGEPROCESSING_H_

