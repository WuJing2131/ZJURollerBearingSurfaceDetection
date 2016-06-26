#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "CommonMethod.h"

void CZJURollerBearingSurfaceDetectionDlg::showMatImgToWnd(int nID, cv::Mat *mat, CString ImageSavePrefix)
{

	SetImageFileSaveSetting(mat, ImageSavePrefix);
	BITMAPINFO *pBmpInfo;       //��¼ͼ��ϸ��
	BYTE *pBmpData;             //ͼ������
	BITMAPFILEHEADER bmpHeader; //�ļ�ͷ
	BITMAPINFOHEADER bmpInfo;   //��Ϣͷ
	CFile bmpFile;              //��¼���ļ�
	if (!bmpFile.Open(m_szTempImageSavePath, CFile::modeRead | CFile::typeBinary))
		return;
	if (bmpFile.Read(&bmpHeader, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER))
		return;
	if (bmpFile.Read(&bmpInfo, sizeof(BITMAPINFOHEADER)) != sizeof(BITMAPINFOHEADER))
		return;
	pBmpInfo = (BITMAPINFO *)new char[sizeof(BITMAPINFOHEADER)];
	//Ϊͼ����������ռ�
	memcpy(pBmpInfo, &bmpInfo, sizeof(BITMAPINFOHEADER));
	DWORD dataBytes = bmpHeader.bfSize - bmpHeader.bfOffBits;
	pBmpData = (BYTE*)new char[dataBytes];
	bmpFile.Read(pBmpData, dataBytes);
	bmpFile.Close();
	//��ʾͼ��
	CWnd *pWnd = GetDlgItem(nID); //���pictrue�ؼ����ڵľ�� 
	CRect rect;
	pWnd->GetClientRect(&rect); //���pictrue�ؼ����ڵľ�������
	CDC *pDC = pWnd->GetDC(); //���pictrue�ؼ���DC  IDC_IMAGERESULT
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