#ifndef HISTOGRAMDLG_H
#define HISTOGRAMDLG_H
#include <opencv2\opencv.hpp>
#include "afxwin.h"
#include "CvvImage/CvvImage.h"


// CHistogramDlg 对话框
class CHistogramDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistogramDlg)

public:
	CHistogramDlg(cv::Mat *showImage,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHistogramDlg();
	void DrawPicToHDC(cv::Mat *img, CWnd* pWnd);
	void DrawPicToHDC(IplImage *img, UINT ID);
	
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HISTGRAM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CWnd * HistImageBox;
	cv::Mat *m_Histogram;
	CStatic m_ImageHistogram;
};

#endif