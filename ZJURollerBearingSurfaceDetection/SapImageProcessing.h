#ifndef _SAPImagePROCESSING_H_
#define _SAPImagePROCESSING_H_

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
   int thresh = 40;
   int max_thresh = 255;
   RNG rng;
   double alpha = 0.5; double beta;
   void thresh_callback(int, void*, Mat *src, Mat *src_gray,Mat *dst);
   Mat *m_Src; Mat *m_Dst;

protected:
   virtual BOOL Run();

protected:
    SapBuffer * m_ProcessBuffers;
   //SapColorConversion*	m_ColorConv;
}; 

#endif	// _SAPImagePROCESSING_H_

