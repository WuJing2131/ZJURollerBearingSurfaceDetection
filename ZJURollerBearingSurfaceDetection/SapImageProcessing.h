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


//
// SapMyProcessing class declaration
//
class SapImageProcessing : public SapProcessing
{
public:
	// Constructor/Destructor
	SapImageProcessing(SapBuffer *pBuffers, SapProCallback pCallback, void *pContext, cv::Mat *pSrc, cv::Mat *pDst, int Thread);
	virtual ~SapImageProcessing();
	void Displaying_Random_Text(cv::Mat* image, char* window_name, int x, int y);
	static cv::Scalar randomColor(cv::RNG& rng);
	void thresh_callback(int, void*, cv::Mat *src, cv::Mat *src_gray, cv::Mat *dst);

	int    m_nthresh = 40;
	int    m_max_thresh = 255;
	cv::RNG    m_rng;
	double m_alpha = 0.5; 
	double m_beta;
	cv::Mat *  m_pImageProcessResult;
	cv::Mat *  m_pImageProcessComposite;

protected:
	virtual BOOL Run();

protected:
	SapBuffer*  m_ProcessBuffers;
	//SapColorConversion*	m_ColorConv;
};

#endif	// _SAPIMAGEPROCESSING_H_

