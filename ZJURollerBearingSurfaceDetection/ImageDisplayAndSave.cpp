#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "CommonMethod.h"

void CZJURollerBearingSurfaceDetectionDlg::showMatImgToWnd(int nID, cv::Mat *mat, CString ImageSavePrefix)
{

	SetImageFileSaveSetting(mat, ImageSavePrefix);
	BITMAPINFO *pBmpInfo;       //记录图像细节
	BYTE *pBmpData;             //图像数据
	BITMAPFILEHEADER bmpHeader; //文件头
	BITMAPINFOHEADER bmpInfo;   //信息头
	CFile bmpFile;              //记录打开文件
	if (!bmpFile.Open(m_szTempImageSavePath, CFile::modeRead | CFile::typeBinary))
		return;
	if (bmpFile.Read(&bmpHeader, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
		return;
	if (bmpFile.Read(&bmpInfo, sizeof(BITMAPINFOHEADER)) != sizeof(BITMAPINFOHEADER))
		return;
	pBmpInfo = (BITMAPINFO *)new char[sizeof(BITMAPINFOHEADER)];
	//为图像数据申请空间
	memcpy(pBmpInfo, &bmpInfo, sizeof(BITMAPINFOHEADER));
	DWORD dataBytes = bmpHeader.bfSize - bmpHeader.bfOffBits;
	pBmpData = (BYTE*)new char[dataBytes];
	bmpFile.Read(pBmpData, dataBytes);
	bmpFile.Close();
	//显示图像
	CWnd *pWnd = GetDlgItem(nID); //获得pictrue控件窗口的句柄 
	CRect rect;
	pWnd->GetClientRect(&rect); //获得pictrue控件所在的矩形区域
	CDC *pDC = pWnd->GetDC(); //获得pictrue控件的DC  IDC_IMAGERESULT
	pDC->SetStretchBltMode(COLORONCOLOR);
	GetClientRect(&rect);
	if (nID == IDC_IMAGERESULT)
	{
		StretchDIBits(pDC->GetSafeHdc(), 0, 0, 700, 265, 0, 0,
			bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);
		LOG(TRACE) << " Show Image Processed Result...";
	}
	else
	{
		StretchDIBits(pDC->GetSafeHdc(), 200, 0, 700, 265, 0, 0,
			bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);
		LOG(TRACE) << " Show Image Processed Composite...";
	}
	delete[] pBmpInfo;
	delete[] pBmpData;
}


void CZJURollerBearingSurfaceDetectionDlg::SetImageFileSaveSetting(cv::Mat *pmat, CString HeadString)
{
	m_timeImageSequence = CTime::GetCurrentTime();
	CString szTime = m_timeImageSequence.Format("%Y-%m-%d_%H-%M-%S.bmp");
	m_szTempImageSavePath = m_szSavePath + HeadString + szTime;
	if (pmat != NULL)
	{
		char* pszMultiByte = NULL;

		pszMultiByte = CCommonMethod::WCharToMByte(m_szTempImageSavePath);
		imwrite(pszMultiByte, *pmat);
		LOG(TRACE) << " Image Save:" << pszMultiByte;
		delete pszMultiByte;

	}
	else
	{
		m_ProcessBuffers->Save(CStringA(m_szTempImageSavePath), "-format bmp");
		LOG(TRACE) << " Image Save:  " << m_szTempImageSavePath;
	}

}