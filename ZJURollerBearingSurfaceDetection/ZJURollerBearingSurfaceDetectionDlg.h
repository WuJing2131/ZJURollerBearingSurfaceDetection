// RollerBearingSurfaceDetectionDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "SapClassBasic.h"
#include "SapClassGui.h"
#include "SapImageProcessing.h"
#include "SerialPort.h"
#include <queue>

//#define FLATFIELD    //平场校正

// CRollerBearingSurfaceDetectionDlg 对话框

#define PKG_SIZE 7

#define SEPARATOR 0
#define STATUSBAR_TX 1
#define STATUSBAR_RX 2
#define STATUSBAR_COMM 3
#define STATUSBAR_SENDKEY 4
#define STATUPIXELPOSTION 5

UINT ReadReportThread(LPVOID lpParam);


class CZJURollerBearingSurfaceDetectionDlg : public CDialogEx
{
	// 构造
public:
	CZJURollerBearingSurfaceDetectionDlg(CWnd* pParent = NULL);	// 标准构造函数

	BOOL CreateObjects();
	BOOL DestroyObjects();
	void UpdateMenu();
	void UpdateTitleBar();
	static void XferCallback(SapXferCallbackInfo *pInfo);
	static void SignalCallback(SapAcqCallbackInfo *pInfo);
	static void ImageProcessedCallback(SapProCallbackInfo *pInfo);
	static void ViewCallback(SapViewCallbackInfo *pInfo);
	void GetSignalStatus();
	void GetSignalStatus(SapAcquisition::SignalStatus signalStatus);

	BOOL	m_FlatFieldEnabled;
	BOOL  m_FlatFieldUseROI;
	BOOL	m_FlatFieldUseHardware;
	BOOL	m_FlatFieldPixelReplacement;

	void SetFlatFieldEnabled(void);
	void SetFlatFieldDisabled(void);
	void SetFlatFieldUseROIEnable(void);
	void SetFlatFieldUseROIDisable(void);
	void SetFlatFieldUseHardwareEnable(void);
	void SetFlatFieldUseHardwareDisable(void);
	void SetFlatFieldPixelReplacementEnable(void);
	void SetFlatFieldPixelReplacementDisable(void);

	BOOL CheckPixelFormat(char* mode);


	void DrawPicToHDC(IplImage *img, UINT ID);
	// 对话框数据
	enum { IDD = IDD_ZJUROLLERBEARINGSURFACEDETECTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


														// 实现
protected:
	HICON m_hIcon;
	CString  m_appTitle;

	CMenu m_Menu;
	CStatusBar   m_wndStatusBar; //状态栏对象
	CToolBar m_AcqConwndToolBar;
	CToolBar m_FileConwndToolBar;
	CImageWnd		*m_ImageWnd;
	SapAcquisition	*m_Acq;
	SapBuffer		*m_Buffers;
	SapTransfer		*m_Xfer;
	SapView        *m_View;
	SapFlatField*			m_FlatField;
	BOOL m_IsSignalDetected;   // TRUE if camera signal is detected

	SapBuffer		*m_ProcessBuffers;
	void* m_pData;
	int* m_numRead;
	int m_countLine;
	SapGio *m_Gio;
	SapLut *m_Lut;
	SapImageProcessing *m_ProcessingImage;
	SapPerformance *m_Performance;    //detected time in Process Images
									  //BOOL m_IsProBufFinshed;

									  // 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSnap();
	afx_msg void OnGrab();
	afx_msg void OnFreeze();
	afx_msg void OnGeneralOptions();
	afx_msg void OnAreaScanOptions();
	afx_msg void OnLineScanOptions();
	afx_msg void OnCompositeOptions();
	afx_msg void OnLoadAcqConfig();
	afx_msg void OnBufferOptions();
	afx_msg void OnViewOptions();
	afx_msg void OnFileLoad();
	afx_msg void OnFileNew();
	afx_msg void OnFileSave();
	afx_msg void OnExit();
	afx_msg void OnFlatFieldEnable();
	afx_msg void OnFlatFieldCalibrate();
	afx_msg void OnFlatFieldLoad();
	afx_msg void OnFlatFieldSave();
	afx_msg void OnFlatFieldUseROI();
	afx_msg void OnFlatFieldUseHardware();
	afx_msg void OnFlatFieldPixelReplacement();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg BOOL OnQueryEndSession();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_viewWnd;
	CScrollBar m_verticalScr;
	CScrollBar m_horizontalScr;
	CStatic m_statusWnd;
	LONG m_lLine;
	//TCHAR m_szSavePath[MAX_PATH];
	CString m_szSavePath;
	CString m_szTempImageSavePath;
	BOOL m_bSaveImageEnable;
	LONG m_lImageWidth = 2495;

	CString m_szUsbVID;
	CString m_szUsbPID;
	CSerialPort m_serialPort;
	UINT m_intPort;
	UINT m_intBaudRate;
	UINT m_intDataBits;
	char m_cParity;
	UINT m_intStopBits;
	BOOL m_bSerialPortOpen = FALSE;
	UCHAR m_SerialRecv[PKG_SIZE + 1];		//串口接收存放
	UCHAR m_pkg[PKG_SIZE + 1];				//存放串口接收到的数据包 
	LONG  m_lRXCount = 0;
	LONG  m_lTXCount = 0;
	CWinApp *m_App;
	void LoadSettings();
	void SaveSettings();

	CTime m_timeImageSequence;

	afx_msg LRESULT OnCommunication(WPARAM ch, LPARAM port);
	afx_msg void OnParameterssettings();
	afx_msg void OnSerialportsettings();
	afx_msg void OnBearingrollerTest();

	void  CmdToLowerComputer();

	int DoStr2Hex(CString str, char* data);
	char DoHexChar(char c);
	BOOL WChar2MByte(LPCWSTR lpSrc, LPSTR lpDest, int nlen);
	byte DoCheckSum(unsigned char *buffer, int   size);
	BOOL WCharToMByte(LPCWSTR lpSrc, char *  lpDest);
	afx_msg void OnAboutAppShow();
	void showMatImgToWndResult();
	void showMatImgToWndComposite();
	Mat *m_pSrc;
	Mat *m_pDst;
	CWnd* m_pWndImageResult;
	CWnd* m_pWndImageComposite;
	void SetImageFileSaveSetting(Mat*mat, CString HeadString);

	int m_nMImageProcessingPrecision = 99;
	int m_nImagePreprocessingThreshold = 15;
	afx_msg void OnAnalysisHistogram();
};
