// HistogramDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "HistogramDlg.h"
#include "afxdialogex.h"



// CHistogramDlg 对话框

IMPLEMENT_DYNAMIC(CHistogramDlg, CDialogEx)

CHistogramDlg::CHistogramDlg(Mat *showImage,CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HISTGRAM, pParent)
	, m_Histogram(showImage)
{
	HistImageBox = GetDlgItem(IDD_HISTGRAM);
	DrawPicToHDC(m_Histogram, HistImageBox);
}


CHistogramDlg::~CHistogramDlg()
{
}

void CHistogramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDD_HISTGRAM, m_ImageHistogram);
}


BEGIN_MESSAGE_MAP(CHistogramDlg, CDialogEx)
END_MESSAGE_MAP()


// CHistogramDlg 消息处理程序


void CHistogramDlg::DrawPicToHDC(Mat *img, CWnd* pWnd)
{
	//CWnd* pWnd = GetDlgItem(ID);
	if (img->empty())
		return;
	static BITMAPINFO *bitMapinfo = NULL;
	static bool First = TRUE;
	if (First)
	{
		BYTE *bitBuffer = new BYTE[40 + 4 * 256];
		if (bitBuffer == NULL)
		{
			return;
		}
		First = FALSE;
		memset(bitBuffer, 0, 40 + 4 * 256);
		bitMapinfo = (BITMAPINFO *)bitBuffer;
		bitMapinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitMapinfo->bmiHeader.biPlanes = 1;      // 目标设备的级别，必须为1  
		for (int i = 0; i < 256; i++)
		{   //颜色的取值范围 (0-255)  
			bitMapinfo->bmiColors[i].rgbBlue = bitMapinfo->bmiColors[i].rgbGreen = bitMapinfo->bmiColors[i].rgbRed = (BYTE)i;
		}

	}

	bitMapinfo->bmiHeader.biHeight = -img->rows;   //如果高度为正的，位图的起始位置在左下角。如果高度为负，起始位置在左上角。  
	bitMapinfo->bmiHeader.biWidth = img->cols;
	bitMapinfo->bmiHeader.biBitCount = img->channels() * 8;     // 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一  

	
	CRect drect;
	pWnd->GetClientRect(drect);    //(drect);  (&drect);  两种方式均可，竟然    
	//m_ImageHistogram.GetClientRect(drect);
	CClientDC dc(pWnd);
	
	HDC hDC = dc.GetSafeHdc();
	SetStretchBltMode(hDC, COLORONCOLOR);  //此句不能少哦  
										   //内存中的图像数据拷贝到屏幕上  
	BOOL TF = StretchDIBits(hDC,
		0,
		0,
		500,        //显示窗口宽度  
		300,       //显示窗口高度  
		0,
		0,
		img->cols,          //图像宽度  
		img->rows,          //图像高度  
		img->data,
		bitMapinfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}