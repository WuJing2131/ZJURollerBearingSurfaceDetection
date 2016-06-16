#pragma once

#include <opencv2\opencv.hpp>
#include "CvvImage.h"
#include "afxwin.h"

using namespace cv;

using namespace std;
// CHistogramDlg �Ի���

class CHistogramDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHistogramDlg)

public:
	CHistogramDlg(Mat *showImage,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CHistogramDlg();
	void DrawPicToHDC(Mat *img, CWnd* pWnd);

	Mat *m_Histogram;
	CWnd * HistImageBox;
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HISTGRAM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_ImageHistogram;
};
