/*!
 * \class CRollerBearingSurfaceDetectionDlg
 *
 * \brief 轴承滚子表面探伤
 *
 * \author WuJing
 * \date 六月 2016
 */
// 
// #pragma once
#ifndef ZJUROLLERBEARINGSURFACEDETECTIONDLG_H
#define ZJUROLLERBEARINGSURFACEDETECTIONDLG_H
#include "afxwin.h"
#include "SapClassBasic.h"
#include "SapClassGui.h"
#include "SapImageProcessing.h"
#include "CSerialPort/SerialPort.h"
#include <queue>


//#define  WM_MYMESSAGEFORCHANGETHREAD (WM_USER+100)

//#define FLATFIELD    //Flat Field


#define PKG_SIZE 7   //host computer Commad Length

enum StatusBarPosition {
	SEPARATOR,
	STATUSBAR_TX,
	STATUSBAR_RX,
	STATUSBAR_COMM,
	STATUSBAR_SENDKEY,
	STATUPIXELPOSTION
};

//USB reader threads (global) function
UINT ReadReportThread(LPVOID lpParam);

class CZJURollerBearingSurfaceDetectionDlg : public CDialogEx
{
	// Constructor
public:
	explicit CZJURollerBearingSurfaceDetectionDlg(CWnd* pParent = NULL);	// Standard Constructor
	static void XferCallback(SapXferCallbackInfo *pInfo);
	static void SignalCallback(SapAcqCallbackInfo *pInfo);
	static void ImageProcessedCallback(SapProCallbackInfo *pInfo);
	static void ViewCallback(SapViewCallbackInfo *pInfo);
	BOOL CreateObjects();
	BOOL DestroyObjects();
	BOOL CheckPixelFormat(char* mode);
	void UpdateMenu();
	void UpdateTitleBar();
	void GetSignalStatus();
	void GetSignalStatus(SapAcquisition::SignalStatus signalStatus);
	
	//Cameras flat field correction parameters
	BOOL	m_FlatFieldEnabled;
	BOOL    m_FlatFieldUseROI;
	BOOL	m_FlatFieldUseHardware;
	BOOL	m_FlatFieldPixelReplacement;

	//Cameras flat field correction related functions
	void SetFlatFieldEnabled(void);
	void SetFlatFieldDisabled(void);
	void SetFlatFieldUseROIEnable(void);
	void SetFlatFieldUseROIDisable(void);
	void SetFlatFieldUseHardwareEnable(void);
	void SetFlatFieldUseHardwareDisable(void);
	void SetFlatFieldPixelReplacementEnable(void);
	void SetFlatFieldPixelReplacementDisable(void);

	//Configuration information (INI file) Load and Save
	void LoadSettings();
	void SaveSettings();

	void CmdToLowerComputer();    //Sends a command to the next crew procedures
	void SetImageFileSaveSetting(cv::Mat*mat, CString HeadString);   //图像保存  HeadString==Null 保存采集的原图像
	void showMatImgToWnd(int nID, cv::Mat *mat, CString ImageSavePrefix);   //将处理后的图像显示在窗口控件上
	void DrawPicToHDC(IplImage *img, UINT ID);       //使用CvvImage将Mat绘制到Picture Control

	// Dialog Data
	enum { IDD = IDD_ZJUROLLERBEARINGSURFACEDETECTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV Support

protected:
	HICON          m_hIcon;
	CString        m_appTitle;

	CMenu          m_Menu;
	CStatusBar     m_wndStatusBar; //Status Bar Object
	CToolBar       m_AcqConwndToolBar;
	CToolBar       m_FileConwndToolBar;
	CImageWnd	  *m_ImageWnd;

	SapAcquisition     *m_Acq;
	SapBuffer	       *m_Buffers;
	SapTransfer	       *m_Xfer;
	SapView            *m_View;
	SapFlatField       *m_FlatField;
	SapBuffer	       *m_ProcessBuffers;
	SapGio             *m_Gio;
	SapLut             *m_Lut;
	SapImageProcessing *m_ProcessingImage;
	SapPerformance     *m_Performance;    //detected time in Process Images
	BOOL                m_IsSignalDetected;   // TRUE if camera signal is detected

	//Generated message mapping function
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
	afx_msg LRESULT OnSerialPortReceiveCommunication(WPARAM ch, LPARAM port);
	afx_msg void OnParameterssettings();
	afx_msg void OnCommunicationSettings();
	afx_msg void OnBearingrollerTest();
	afx_msg void OnAboutAppShow();
	afx_msg void OnAnalysisHistogram();
	DECLARE_MESSAGE_MAP()
public:
	CWinApp *  m_App;
	CTime      m_timeImageSequence;
	CStatic    m_viewWnd;
	CScrollBar m_verticalScr;
	CScrollBar m_horizontalScr;
	CStatic    m_statusWnd;
	CString    m_szSavePath;
	CString    m_szTempImageSavePath;
	BOOL       m_bSaveImageEnable;

	//Communication type  USB parameters
	CString m_szUsbVID;
	CString m_szUsbPID;

	//Communication type  SeialPort parameters
	CSerialPort m_serialPort;
	UINT        m_intPort;
	UINT        m_intBaudRate;
	UINT        m_intDataBits;
	char        m_cParity;
	UINT        m_intStopBits;
	BOOL        m_bSerialPortOpen = FALSE;
	UCHAR       m_SerialRecv[PKG_SIZE + 1];		//Serial Port Receive Array
	UCHAR       m_pkg[PKG_SIZE + 1];				//Store the serial port to receive packets
	LONG        m_lRXCount = 0;
	LONG        m_lTXCount = 0;
	
	//OpenCV Image Data Structure
	cv::Mat *  m_pImageProcessResult;
	cv::Mat *  m_pImageProcessComposite;
	CWnd*  m_pWndImageResult;
	CWnd*  m_pWndImageComposite;
	void*  m_pData;
	int    m_nXferCallbackCount;

	//BOOL m_IsProBufFinshed;
	LONG  m_lCameraSamplingWeith=8192;
	LONG   m_lImageWidth = 2495;
	LONG   m_lImageHeight;
	int    m_nMImageProcessingPrecision = 99;  
	int    m_nImagePreprocessingThreshold = 15;
	
};

#endif
