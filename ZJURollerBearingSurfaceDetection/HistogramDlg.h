#pragma once

#include <opencv2\opencv.hpp>
#include "CvvImage.h"
#include "afxwin.h"

using namespace cv;

using namespace std;
// CHistogramDlg 对话框

class CHistogramDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistogramDlg)

public:
	CHistogramDlg(Mat *showImage,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHistogramDlg();
	void DrawPicToHDC(Mat *img, CWnd* pWnd);

	Mat *m_Histogram;
	CWnd * HistImageBox;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HISTGRAM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ImageHistogram;
};
