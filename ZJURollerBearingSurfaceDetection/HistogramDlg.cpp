// HistogramDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "HistogramDlg.h"
#include "afxdialogex.h"



// CHistogramDlg �Ի���

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


// CHistogramDlg ��Ϣ�������


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
		bitMapinfo->bmiHeader.biPlanes = 1;      // Ŀ���豸�ļ��𣬱���Ϊ1  
		for (int i = 0; i < 256; i++)
		{   //��ɫ��ȡֵ��Χ (0-255)  
			bitMapinfo->bmiColors[i].rgbBlue = bitMapinfo->bmiColors[i].rgbGreen = bitMapinfo->bmiColors[i].rgbRed = (BYTE)i;
		}

	}

	bitMapinfo->bmiHeader.biHeight = -img->rows;   //����߶�Ϊ���ģ�λͼ����ʼλ�������½ǡ�����߶�Ϊ������ʼλ�������Ͻǡ�  
	bitMapinfo->bmiHeader.biWidth = img->cols;
	bitMapinfo->bmiHeader.biBitCount = img->channels() * 8;     // ÿ�����������λ����������1(˫ɫ), 4(16ɫ)��8(256ɫ)��24(���ɫ)֮һ  

	
	CRect drect;
	pWnd->GetClientRect(drect);    //(drect);  (&drect);  ���ַ�ʽ���ɣ���Ȼ    
	//m_ImageHistogram.GetClientRect(drect);
	CClientDC dc(pWnd);
	
	HDC hDC = dc.GetSafeHdc();
	SetStretchBltMode(hDC, COLORONCOLOR);  //�˾䲻����Ŷ  
										   //�ڴ��е�ͼ�����ݿ�������Ļ��  
	BOOL TF = StretchDIBits(hDC,
		0,
		0,
		500,        //��ʾ���ڿ��  
		300,       //��ʾ���ڸ߶�  
		0,
		0,
		img->cols,          //ͼ����  
		img->rows,          //ͼ��߶�  
		img->data,
		bitMapinfo,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}