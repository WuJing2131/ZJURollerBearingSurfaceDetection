// RollerBearingSurfaceDetectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "CommonMethod.h"
#include "afxdialogex.h"
#include "BearingRollersParameterDlg.h"
#include "CommunicationConfigurationDlg.h"
#include "LibUSBWin32\include\lusb0_usb.h"
#include "HistogramDlg.h"
#include "CvvImage/CvvImage.h"
#include <crtdbg.h>
#include <iostream>
#include <string>

#define CRTDBG_MAP_ALLOC   //内存泄漏检测
#ifdef _DEBUG
//#include "vld.h"
#define new DEBUG_NEW  // 调试模式下new 会被替换为 DEBUG_NEW 可以定位内存泄露
#endif



// status line indicator
static const UINT indicators[] =
{
	ID_SEPARATOR,
	ID_STATUSBAR_TX,
	ID_STATUSBAR_RX,
	ID_STATUSBAR_COMM,
	ID_STATUSBAR_SENDKEY,
	IDS_STATUPIXELPOSTION
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
#pragma region CAboutDlg
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()
#pragma endregion CAboutDlg
// CRollerBearingSurfaceDetectionDlg 对话框



CZJURollerBearingSurfaceDetectionDlg::CZJURollerBearingSurfaceDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CZJURollerBearingSurfaceDetectionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ImageWnd = NULL;
	m_Acq = NULL;
	m_Buffers = NULL;
	m_ProcessBuffers = NULL;
	m_Xfer = NULL;
	m_View = NULL;
	m_Gio = NULL;
	m_Lut = NULL;
	m_ProcessingImage = NULL;
	m_Performance = NULL;
	m_IsSignalDetected = TRUE;
	m_pData = NULL;
	m_nXferCallbackCount = 0;
	m_pImageProcessResult = NULL;
	m_pImageProcessComposite = NULL;
	m_lImageHeight = 650;     //默认采集640行
	//	m_IsProBufFinshed = FALSE;
#ifdef FLATFIELD
	m_FlatFieldEnabled = FALSE;
	m_FlatFieldUseROI = FALSE;
	m_FlatFieldUseHardware = TRUE;
	m_FlatFieldPixelReplacement = TRUE;
	m_FlatField = NULL;
#endif // FLATFIELD

}

void CZJURollerBearingSurfaceDetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEW_WND, m_viewWnd);
	DDX_Control(pDX, IDC_VERT_SCROLLBAR, m_verticalScr);
	DDX_Control(pDX, IDC_HORZ_SCROLLBAR, m_horizontalScr);
	DDX_Control(pDX, IDC_STATUS, m_statusWnd);
}

BEGIN_MESSAGE_MAP(CZJURollerBearingSurfaceDetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_COMM_RXCHAR, OnSerialPortReceiveCommunication)
	ON_COMMAND(IDC_SNAP, &CZJURollerBearingSurfaceDetectionDlg::OnSnap)
	ON_COMMAND(IDC_GRAB, &CZJURollerBearingSurfaceDetectionDlg::OnGrab)
	ON_COMMAND(IDC_FREEZE, &CZJURollerBearingSurfaceDetectionDlg::OnFreeze)
	ON_COMMAND(IDC_GENERAL_OPTIONS, &CZJURollerBearingSurfaceDetectionDlg::OnGeneralOptions)
	ON_COMMAND(IDC_LINE_SCAN_OPTIONS, &CZJURollerBearingSurfaceDetectionDlg::OnLineScanOptions)
	ON_COMMAND(IDC_COMPOSITE_OPTIONS, &CZJURollerBearingSurfaceDetectionDlg::OnCompositeOptions)
	ON_COMMAND(IDC_LOAD_ACQ_CONFIG, &CZJURollerBearingSurfaceDetectionDlg::OnLoadAcqConfig)
	ON_COMMAND(IDC_BUFFER_OPTIONS, &CZJURollerBearingSurfaceDetectionDlg::OnBufferOptions)
	ON_COMMAND(IDC_VIEW_OPTIONS, &CZJURollerBearingSurfaceDetectionDlg::OnViewOptions)
	ON_COMMAND(IDC_FILE_LOAD, &CZJURollerBearingSurfaceDetectionDlg::OnFileLoad)
	ON_COMMAND(IDC_FILE_NEW, &CZJURollerBearingSurfaceDetectionDlg::OnFileNew)
	ON_COMMAND(IDC_FILE_SAVE, &CZJURollerBearingSurfaceDetectionDlg::OnFileSave)
	ON_COMMAND(IDC_EXIT, &CZJURollerBearingSurfaceDetectionDlg::OnExit)
	ON_COMMAND(ID_FLATFIELDCORRECTION_ENABLE, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldEnable)
	ON_COMMAND(ID_FLATFIELDCORRECTION_CALIBRATE, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldCalibrate)
	ON_COMMAND(ID_FLATFIELDCORRECTION_LOAD, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldLoad)
	ON_COMMAND(ID_FLATFIELDCORRECTION_SAVE, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldSave)
	ON_COMMAND(ID_FLATFIELDCORRECTION_USEAROI, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldUseROI)
	ON_COMMAND(ID_FLATFIELDCORRECTION_HARDWARECORRECTION, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldUseHardware)
	ON_COMMAND(ID_FLATFIELDCORRECTION_PIXELREPLACEMENT, &CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldPixelReplacement)
	ON_WM_ENDSESSION()
	ON_WM_QUERYENDSESSION()
	//ON_COMMAND(IDC_FILE_NEW, &CRollerBearingSurfaceDetectionDlg::OnFileNew)
	ON_COMMAND(ID_PARAMETERSSETTINGS, &CZJURollerBearingSurfaceDetectionDlg::OnParameterssettings)
	ON_COMMAND(ID_SERIALPORTSETTINGS, &CZJURollerBearingSurfaceDetectionDlg::OnCommunicationSettings)
	ON_COMMAND(ID_BEARINGROLLER_TEST, &CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest)
	ON_COMMAND(ID__ABOUT, &CZJURollerBearingSurfaceDetectionDlg::OnAboutAppShow)
	ON_COMMAND(ID_ANALYSIS_HISTOGRAM, &CZJURollerBearingSurfaceDetectionDlg::OnAnalysisHistogram)
END_MESSAGE_MAP()


// CRollerBearingSurfaceDetectionDlg 消息处理程序

/////////////////////////////////////////////////////////////////////////////
// CGrabDemoDlg message handlers

void CZJURollerBearingSurfaceDetectionDlg::XferCallback(SapXferCallbackInfo *pInfo)
{
	CZJURollerBearingSurfaceDetectionDlg *pDlg = (CZJURollerBearingSurfaceDetectionDlg *)pInfo->GetContext();
	//ReadLine(int x1, int y1, int x2, int y2, void *pData, int *numRead);
	//WriteLine(int x1, int y1, int x2, int y2, const void *pData, int *numWritten);
	// If grabbing in trash buffer, do not display the image, update the
	// appropriate number of frames on the status bar instead
	if (pInfo->IsTrash())
	{
		CString str;
		str.Format(_T("Frames acquired in trash buffer: %d"), pInfo->GetEventCount());
		pDlg->m_statusWnd.SetWindowText(str);
	}

	// Refresh view
	else
	{
		//TRACE0("\nm_countLine    **************************%d\n", pDlg->m_countLine);
		if (pDlg->m_nXferCallbackCount < pDlg->m_lImageHeight)
		{
			int PositionOfAImageRowCache = 0;
			for (int ImageFrameCount = 0; ImageFrameCount < pDlg->m_Buffers->GetCount(); ++ImageFrameCount)
			{
				for (int PositionOfAImageRow = PositionOfAImageRowCache*0.99; PositionOfAImageRow < pDlg->m_Buffers->GetWidth(); ++PositionOfAImageRow)
				{
					BYTE ElementDataValue;
					pDlg->m_Buffers->ReadElement(PositionOfAImageRow, 0, &ElementDataValue);
					if (ElementDataValue > pDlg->m_nImagePreprocessingThreshold)
					{
						PositionOfAImageRowCache = PositionOfAImageRow;
						break;
					}
				}

				pDlg->m_ProcessBuffers->CopyRect(pDlg->m_Buffers, ImageFrameCount, PositionOfAImageRowCache, 0, pDlg->m_lImageWidth, 1, ImageFrameCount, 0, pDlg->m_nXferCallbackCount);
				//pDlg->m_ProcessBuffers->CopyRect(pDlg->m_Buffers, nimageCount, 0, 0, pDlg->m_Buffers->GetWidth(), 1, nimageCount, 0, pDlg->m_countLine);
			}
			++(pDlg->m_nXferCallbackCount);
		}
		else if (pDlg->m_nXferCallbackCount == pDlg->m_lImageHeight)
		{
			//Image Processing.........................
			// Process current buffer (see Run member function into the SapMyProcessing.cpp file)
			++(pDlg->m_nXferCallbackCount);

			LOG(TRACE) << " A frame image acquisition is completed! Image processing...";
			//pDlg->m_ProcessingImage->Init();
			pDlg->m_ProcessingImage->Execute();

			//	pDlg->m_IsProBufFinshed = TRUE;

		}
		else
			return;
		//pDlg->m_View->Show();

	}
}

//
// This function is called each time a buffer has been processed by the processing object
//
void CZJURollerBearingSurfaceDetectionDlg::ImageProcessedCallback(SapProCallbackInfo *pInfo)
{
	CZJURollerBearingSurfaceDetectionDlg* pDlg = (CZJURollerBearingSurfaceDetectionDlg *)pInfo->GetContext();
	CString			str;

	// Show current buffer index and execution time in millisecond
	str.Format(_T("Image Process spend time = %5.2f ms"), pDlg->m_ProcessingImage->GetTime());
	LOG(TRACE) << " Image processing is completed! Image Process spend time = " << pDlg->m_ProcessingImage->GetTime() << "ms";
	// Refresh view
	pDlg->m_statusWnd.SetWindowText(str);
	pDlg->m_View->Show();
	//	pDlg->UpdateTitleBar();
	pDlg->SetImageFileSaveSetting(NULL, _T("SapBuffer"));

	pDlg->showMatImgToWnd(IDC_IMAGERESULT, pDlg->m_pImageProcessResult, _T("Result"));
	pDlg->showMatImgToWnd(IDC_IMAGECOMPOSITE, pDlg->m_pImageProcessComposite, _T("Composite"));

	//pDlg->showMatImgToWnd(pDlg->m_pWndImageComposite, pDlg->m_pDst); 
	/*if (pDlg->m_bSaveImageEnable)
	{
		pDlg->m_ProcessBuffers->Save(CStringA(pDlg->m_szSavePath), "-format bmp");
	}*/
	//pDlg->m_ProcessBuffers->Clear();   

}

//
// This function is called each time a buffer has been shown by the view object
//
void CZJURollerBearingSurfaceDetectionDlg::ViewCallback(SapViewCallbackInfo *pInfo)
{
	CZJURollerBearingSurfaceDetectionDlg *pDlg = (CZJURollerBearingSurfaceDetectionDlg *)pInfo->GetContext();

#ifdef FLATFIELD
	if (pDlg->m_FlatFieldUseROI)
		pDlg->m_ImageWnd->DisplayRoiTracker();
	else
		pDlg->m_ImageWnd->HideRoiTracker();
#else
	if (pDlg->m_ImageWnd->IsRoiTrackerActive())
		pDlg->m_ImageWnd->DisplayRoiTracker();
#endif // FLATFIELD



}

void CZJURollerBearingSurfaceDetectionDlg::SignalCallback(SapAcqCallbackInfo *pInfo)
{
	CZJURollerBearingSurfaceDetectionDlg *pDlg = (CZJURollerBearingSurfaceDetectionDlg *)pInfo->GetContext();
	pDlg->GetSignalStatus(pInfo->GetSignalStatus());
}

BOOL CZJURollerBearingSurfaceDetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
		pSysMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		pSysMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	_CrtSetBreakAlloc(242);
	LOG(TRACE) << " Roller Bearing Surface Detection Program Initialization ...";
	//LOG(TRACE) << L"程序初始化";
	m_App = AfxGetApp();
	LoadSettings();
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		LOG(TRACE) << "Failed to create status bar";
		return -1;      // fail to create
	}

	CRect rect;
	GetClientRect(&rect);

	m_wndStatusBar.SetPaneInfo(StatusBarPosition::SEPARATOR,         ID_SEPARATOR,          SBPS_STRETCH, 60);
	m_wndStatusBar.SetPaneInfo(StatusBarPosition::STATUSBAR_TX,      ID_STATUSBAR_TX,       SBPS_NORMAL,  60);
	m_wndStatusBar.SetPaneInfo(StatusBarPosition::STATUSBAR_RX,      ID_STATUSBAR_RX,       SBPS_NORMAL,  60);
	m_wndStatusBar.SetPaneInfo(StatusBarPosition::STATUSBAR_COMM,    ID_STATUSBAR_COMM,     SBPS_NORMAL, 150);
	m_wndStatusBar.SetPaneInfo(StatusBarPosition::STATUSBAR_SENDKEY, ID_STATUSBAR_SENDKEY,  SBPS_NORMAL,  60);
	m_wndStatusBar.SetPaneInfo(StatusBarPosition::STATUPIXELPOSTION, IDS_STATUPIXELPOSTION, SBPS_NORMAL, 165);

	m_Menu.LoadMenu(IDR_MENU);
	SetMenu(&m_Menu);

	if (!m_AcqConwndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_AcqConwndToolBar.LoadToolBar(IDR_ACQ_CON_TOOLBAR))
	{
		LOG(TRACE) << "Failed   to   Create   Dialog   Toolbar";
		//TRACE0("Failed   to   Create   Dialog   Toolbar/n");
		EndDialog(IDCANCEL);
	}
	m_AcqConwndToolBar.ShowWindow(SW_SHOW);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	//CRect rect;
	//GetClientRect(rect);
	//if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))
	//{
	//	TRACE0("Can't create status bar\n");
	//	return false;
	//}
	//m_wndStatusBar.MoveWindow(0, rect.bottom - 20, rect.right, 20);// 调整状态栏的位置和大小

	// Initialize variables
	GetWindowText(m_appTitle);

	// Are we operating on-line?
	CAcqConfigDlg dlg(this, NULL);
	if (dlg.DoModal() == IDOK)
	{
		// Define on-line objects
		m_Acq = new SapAcquisition(dlg.GetAcquisition());
		m_Buffers = new SapBufferWithTrash(2, m_Acq);
		m_Xfer = new SapAcqToBuf(m_Acq, m_Buffers, XferCallback, this);
		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lImageHeight, m_Buffers->GetFormat(), m_Buffers->GetType(), m_Buffers->GetLocation());
		m_ProcessBuffers = new SapBuffer();
		//m_ProcessBuffers = new SapBufferWithTrash(2, m_Acq);
		//	m_ProcessBuffers->SetHeight(m_lImageHeight);

		//m_Gio = new SapGio(m_Acq->GetLocation());
		LOG(TRACE) << " SapAcquisition Config Dlalog does open when the program is initialized! Drive loading success... ";
	}
	else
	{
		// Define off-line objects
		m_Buffers = new SapBuffer();
		m_Buffers->SetWidth(m_lCameraSamplingWeith);     //#####################
		m_Buffers->SetHeight(m_lImageHeight);    //#####################
		m_ProcessBuffers = new SapBuffer();
		LOG(TRACE) << " SapAcquisition Config Dlalog does not open when the program is initialized! Drive not loaded successfully ... ";
	}

	m_pImageProcessResult = new  cv::Mat(0, 0, CV_8UC3);
	m_pImageProcessComposite = new  cv::Mat(0, 0, CV_8UC3);
	// Define other objects
	m_View = new SapView(m_ProcessBuffers, m_viewWnd.GetSafeHwnd(), ViewCallback, this);
	m_ProcessingImage = new SapImageProcessing(m_ProcessBuffers, ImageProcessedCallback, this, \
		m_pImageProcessResult, m_pImageProcessComposite, m_nMImageProcessingPrecision);  

	// Create all objects
	if (!CreateObjects()) { EndDialog(TRUE); return FALSE; }

	// Create processing object
	/*if (m_ProcessingImage && !*m_ProcessingImage)
	{
		if (!m_ProcessingImage->Create())
		{
			DestroyObjects();
			EndDialog(TRUE);
			return FALSE;
		}

		m_ProcessingImage->SetAutoEmpty(TRUE);

		if (m_Xfer)
			m_Xfer->SetAutoEmpty(FALSE);
	}*/

	// Create an image window object
	m_ImageWnd = new CImageWnd(m_View, &m_viewWnd, &m_horizontalScr, &m_verticalScr, this);
	UpdateMenu();

	// Get current input signal connection status
	GetSignalStatus();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


BOOL CZJURollerBearingSurfaceDetectionDlg::CreateObjects()
{
	LOG(TRACE) << " Creating Objects ...";
	CWaitCursor wait;

	// Create acquisition object
	if (m_Acq && !*m_Acq && !m_Acq->Create())
	{
		LOG(TRACE) << " Create acquisition object failed...";
		DestroyObjects();
		return FALSE;
	}
	else
		LOG(TRACE) << " Create acquisition object success...";

	// Create buffer object
	if (m_Buffers && !*m_Buffers)
	{
		if (!m_Buffers->Create())
		{
			LOG(TRACE) << "Create buffer object failed...";
			DestroyObjects();
			return FALSE;
		}
		LOG(TRACE)\
			<< "\n					m_Buffers Count: "      << m_Buffers->GetCount()\
			<< "\n					m_Buffers GFrameRate: " << m_Buffers->GetFrameRate()\
			<< "\n					m_Buffers Height: "     << m_Buffers->GetHeight()\
			<< "\n					m_Buffers Width: "      << m_Buffers->GetWidth()\
			<< "\n					m_Buffers Type: "       << m_Buffers->GetType();
		// Clear all buffers
		m_Buffers->Clear();
	}
	else
		LOG(TRACE) << " Create buffer object success...";

	// Create m_ProcessBuffers object
	if (m_ProcessBuffers && !*m_ProcessBuffers)
	{
		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lImageHeight, m_Buffers->GetFormat(), m_Buffers->GetType(), m_Buffers->GetLocation());
		//m_ProcessBuffers->SetWidth(m_Buffers->GetWidth());
		m_ProcessBuffers->SetWidth(m_lImageWidth);
		m_ProcessBuffers->SetHeight(m_lImageHeight);
		m_ProcessBuffers->SetFormat(m_Buffers->GetFormat());
		m_ProcessBuffers->SetType(m_Buffers->GetType());
		m_ProcessBuffers->SetLocation(m_Buffers->GetLocation());
		m_ProcessBuffers->SetCount(m_Buffers->GetCount());
		LOG(TRACE)\
			<< "\n					m_ProcessBuffers Count: "      << m_ProcessBuffers->GetCount()\
			<< "\n					m_ProcessBuffers GFrameRate: " << m_ProcessBuffers->GetFrameRate()\
			<< "\n					m_ProcessBuffers Height: "     << m_ProcessBuffers->GetHeight()\
			<< "\n					m_ProcessBuffers Width: "      << m_ProcessBuffers->GetWidth()\
			<< "\n					m_ProcessBuffers  Type: "      << m_ProcessBuffers->GetType();

		LOG(TRACE)\
			<< "\n		*************************************************************************************\n"\
			<< "		TypeDefault               = -1" << "   TypeVirtual   = 0" << "   TypeContiguous            =  1\n"\
			<< "		TypeScatterGather         = 32" << "   TypeOffscreen = 8" << "   TypeOffscreenVideo        = 10\n"\
			<< "		TypeOverlay               =  6" << "   TypeDummy     =64" << "   TypePhysical              =128\n"\
			<< "		TypeScatterGatherPhysical =160" << "   TypeUnmapped =512" << "   TypeScatterGatherUnmapped =544\n"\
			<< "		*************************************************************************************";

		//m_ProcessBuffers->SetLocation(m_Buffers->GetLocation());
		if (!m_ProcessBuffers->Create())
		{
			LOG(TRACE) << " Create image Process buffer object failed...";
			DestroyObjects();
			return FALSE;
		}
		// Clear all buffers
		m_ProcessBuffers->Clear();
	}
	else
		LOG(TRACE) << " Create image Process buffer object success...";

#ifdef FLATFIELD
	LOG(TRACE) << "Flat Field Function is Setup ...";
	// Create flat field object
	if (!m_Acq && *m_Acq && m_Acq->IsFlatFieldAvailable())
		m_FlatField = new SapFlatField(m_Acq);
	else
		m_FlatField = new SapFlatField(m_Buffers);

	if (m_FlatField && !*m_FlatField && !m_FlatField->Create())
	{
		LOG(TRACE) << " Create flat field object failed...";
		DestroyObjects();
		return FALSE;
	}
	else
		LOG(TRACE) << " Create flat field object success...";
#else
	LOG(TRACE) << " Flat Field Function is not Setup ...";
#endif //FLATFIELD

	// Create view object
	if (m_View && !*m_View && !m_View->Create())
	{
		LOG(TRACE) << " Create view object failed...";
		DestroyObjects();
		return FALSE;
	}
	else
		LOG(TRACE) << " Create view object success...";

	// Create transfer object
	if (m_Xfer && !*m_Xfer && !m_Xfer->Create())
	{
		LOG(TRACE) << " Create transfer object failed...";
		DestroyObjects();
		return FALSE;
	}
	else
		LOG(TRACE) << " Create transfer object success...";

	// Create processing object
	if (m_ProcessingImage && !*m_ProcessingImage)
	{

		if (!m_ProcessingImage->Create())
		{
			LOG(TRACE) << " Create processing object failed...";
			DestroyObjects();
			return FALSE;
		}
		else
			LOG(TRACE) << " Create processing object success...";
		m_ProcessingImage->SetAutoEmpty(TRUE);
		m_ProcessingImage->Init();
	}


	return TRUE;
}

BOOL CZJURollerBearingSurfaceDetectionDlg::DestroyObjects()
{
	LOG(TRACE) << " Destroy Objects ...";
	// Destroy transfer object
	if (m_Xfer && *m_Xfer) m_Xfer->Destroy();

	// Destroy processing object
	if (m_ProcessingImage && *m_ProcessingImage) m_ProcessingImage->Destroy();

	// Destroy view object
	if (m_View && *m_View) m_View->Destroy();

#ifdef FLATFIELD
	// Destroy flat field object
	if (m_FlatField && *m_FlatField)	m_FlatField->Destroy();
#endif // FLATFIELD


	// Destroy buffer object
	if (m_Buffers && *m_Buffers) m_Buffers->Destroy();

	// Destroy acquisition object
	if (m_Acq && *m_Acq) m_Acq->Destroy();

	// Destroy Processbuffer object
	if (m_ProcessBuffers && *m_ProcessBuffers) m_ProcessBuffers->Destroy();

#ifdef FLATFIELD
	// Disable flat field correction
	m_FlatFieldEnabled = FALSE;
#endif

	return TRUE;
}

void CZJURollerBearingSurfaceDetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CZJURollerBearingSurfaceDetectionDlg::OnPaint()
{
	//CRect   rectLL;
	//CPaintDC   dc(this);
	//GetClientRect(rectLL);
	//dc.FillSolidRect(rectLL, RGB(0, 0, 0));   //设置为绿色背景

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		INT32 cxIcon = GetSystemMetrics(SM_CXICON);
		INT32 cyIcon = GetSystemMetrics(SM_CYICON);

		CRect rect;
		GetClientRect(&rect);
		INT32 x = (rect.Width() - cxIcon + 1) / 2;
		INT32 y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();

		if (m_ImageWnd)
		{
			// Display last acquired image
			m_ImageWnd->OnPaint();
		}
	}
}


void CZJURollerBearingSurfaceDetectionDlg::OnDestroy()
{

	CDialog::OnDestroy();

	// Destroy all objects
	DestroyObjects();

	// Delete all objects
	if (m_Xfer)			             delete m_Xfer;
	if (m_ProcessingImage)			 delete m_ProcessingImage;
	if (m_ImageWnd)	                 delete m_ImageWnd;
	if (m_View)			             delete m_View;
	if (m_FlatField)                 delete m_FlatField;
	if (m_Buffers)		             delete m_Buffers;
	if (m_ProcessBuffers)            delete m_ProcessBuffers;
	if (m_Acq)			             delete m_Acq;
	if (m_pData)                     delete m_pData;
	if (m_Gio)                       delete m_Gio;
	if (m_Lut)                       delete m_Lut;
	if (m_Performance)               delete m_Performance;
	if (m_pImageProcessResult)       delete m_pImageProcessResult;
	if (m_pImageProcessComposite)    delete m_pImageProcessComposite;
	if (m_pWndImageResult)           delete m_pWndImageResult;
	if (m_pWndImageComposite)        delete m_pWndImageComposite;
	LOG(TRACE) << " Roller Bearing Surface Detection Program Over ...";
	_CrtDumpMemoryLeaks();  //检测内存泄漏

}

void CZJURollerBearingSurfaceDetectionDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_IsSignalDetected)
	{
		CString str = m_appTitle;

		if (!nFlags)
			str = m_ImageWnd->GetPixelString(point);
		str += _T("  ") + m_ImageWnd->GetPixelString(point);

		//SetWindowText(str);
		m_wndStatusBar.SetPaneText(5, str, TRUE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CZJURollerBearingSurfaceDetectionDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	if (m_ImageWnd) m_ImageWnd->OnMove();
}


void CZJURollerBearingSurfaceDetectionDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(&rect);
	//ScreenToClient(&rect);
	/*if (m_pWndImageResult->GetSafeHwnd())
	{
		m_pWndImageResult->MoveWindow(rect.left + 30, rect.top + 50, 200, 45);
	}*/
	if (m_ImageWnd) m_ImageWnd->OnSize();
	if (m_wndStatusBar.GetSafeHwnd())
	{
		GetClientRect(&rect);
		rect.top = rect.bottom - 20;
		m_wndStatusBar.MoveWindow(&rect, TRUE);
	}

	//GetWindowRect(&rect);
	/*rect.top = 30;
	rect.left = 30;
	rect.Height = 265;*/
	//rect.
	//GetDlgItem(IDC_IMAGERESULT) ->
	//if (m_pWndImageResult) m_pWndImageResult->OnSize();
	//m_pWndImageResult->MoveWindow(rect.left+10, 100, 200, 45);
	//GetDlgItem(IDC_IMAGERESULT)->MoveWindow(100, 100, 200, 45);
	//GetDlgItem(IDC_IMAGECOMPOSITE)->MoveWindow(rect.Width() / 2, 0, rect.Width() / 2, 265);

}


void CZJURollerBearingSurfaceDetectionDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetDlgCtrlID() == IDC_HORZ_SCROLLBAR)
	{
		// Adjust source's horizontal origin
		m_ImageWnd->OnHScroll(nSBCode, nPos);
		OnPaint();
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CZJURollerBearingSurfaceDetectionDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetDlgCtrlID() == IDC_VERT_SCROLLBAR)
	{
		// Adjust source's vertical origin
		m_ImageWnd->OnVScroll(nSBCode, nPos);
		OnPaint();
	}

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CZJURollerBearingSurfaceDetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CZJURollerBearingSurfaceDetectionDlg::OnExit()
{
	EndDialog(TRUE);
}

void CZJURollerBearingSurfaceDetectionDlg::OnEndSession(BOOL bEnding)
{
	CDialog::OnEndSession(bEnding);

	if (bEnding)
	{
		// If ending the session, free the resources.
		OnDestroy();
	}
}

BOOL CZJURollerBearingSurfaceDetectionDlg::OnQueryEndSession()
{
	if (!CDialog::OnQueryEndSession())
		return FALSE;

	return TRUE;
}

//**************************************************************************************
// Updates the menu items enabling/disabling the proper items depending on the state
//  of the application
//**************************************************************************************
void CZJURollerBearingSurfaceDetectionDlg::UpdateMenu(void)
{
	BOOL bAcqNoGrab = m_Xfer && *m_Xfer && !m_Xfer->IsGrabbing();
	BOOL bAcqGrab = m_Xfer && *m_Xfer && m_Xfer->IsGrabbing();
	BOOL bNoGrab = !m_Xfer || !m_Xfer->IsGrabbing();
	INT32	 scan = 0;
	BOOL bLineScan = m_Acq && m_Acq->GetParameter(CORACQ_PRM_SCAN, &scan) && (scan == CORACQ_VAL_SCAN_LINE);
	INT32 iInterface = CORACQ_VAL_INTERFACE_DIGITAL;
	if (m_Acq)
		m_Acq->GetCapability(CORACQ_CAP_INTERFACE, (void *)&iInterface);

	// Acquisition Control
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_GRAB, bAcqNoGrab ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_SNAP, bAcqNoGrab ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_FREEZE, bAcqGrab ? MF_ENABLED : MF_GRAYED);

	// Acquisition Options
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_GENERAL_OPTIONS, bAcqNoGrab ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_LINE_SCAN_OPTIONS, (bAcqNoGrab && bLineScan) ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_COMPOSITE_OPTIONS, (bAcqNoGrab && (iInterface == CORACQ_VAL_INTERFACE_ANALOG)) ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_LOAD_ACQ_CONFIG, (m_Xfer && !m_Xfer->IsGrabbing()) ? MF_ENABLED : MF_GRAYED);

	// File Options
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_FILE_NEW, bNoGrab ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_FILE_LOAD, bNoGrab ? MF_ENABLED : MF_GRAYED);
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_FILE_SAVE, bNoGrab ? MF_ENABLED : MF_GRAYED);

	// General Options
	AfxGetApp()->m_pMainWnd->GetMenu()->EnableMenuItem(IDC_BUFFER_OPTIONS, bNoGrab ? MF_ENABLED : MF_GRAYED);


	//AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_ENABLE, MF_CHECKED);

	//// If last control was disabled, set default focus
	//if (!GetFocus())
	//	GetDlgItem(IDC_EXIT)->SetFocus();
}

void CZJURollerBearingSurfaceDetectionDlg::UpdateTitleBar()
{
	// Update pixel information
	CString str = m_appTitle;
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	//str += _T("  ") + m_ImageWnd->GetPixelString(point);
	str = m_ImageWnd->GetPixelString(point);
	//SetWindowText(str);
	m_wndStatusBar.SetPaneText(5, str, TRUE);
}


//*****************************************************************************************
//
//					Acquisition Control
//
//*****************************************************************************************
void CZJURollerBearingSurfaceDetectionDlg::OnFreeze()
{
	LOG(TRACE) << " Image Freeze button clicked ...";
	if (m_Xfer->Freeze())
	{
		if (CAbortDlg(this, m_Xfer).DoModal() != IDOK)
			m_Xfer->Abort();

		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnGrab()
{
	LOG(TRACE) << " Image Grab button clicked ...";
	m_statusWnd.SetWindowText(_T(""));

	if (m_Xfer->Grab())
	{
		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnSnap()
{
	LOG(TRACE) << " Image Snap button clicked ...";
	m_statusWnd.SetWindowText(_T(""));
	if (m_Xfer->Snap())
	{
		if (CAbortDlg(this, m_Xfer).DoModal() != IDOK)
			m_Xfer->Abort();

		UpdateMenu();
	}
}


//*****************************************************************************************
//
//					Acquisition Options
//
//*****************************************************************************************
void CZJURollerBearingSurfaceDetectionDlg::OnGeneralOptions()
{
	LOG(TRACE) << " Acquisition  General Options Setting ...";
	CAcqDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnAreaScanOptions()
{
	LOG(TRACE) << " Acquisition  Area Scan Options Setting ...";
	CAScanDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnLineScanOptions()
{
	LOG(TRACE) << " Acquisition  Line Scan Options Setting ...";
	CLScanDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnCompositeOptions()
{
	LOG(TRACE) << " Acquisition  Composite Options Setting ...";
	if (m_Xfer->Snap())
	{
		CCompDlg dlg(this, m_Acq, m_Xfer);
		dlg.DoModal();

		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnLoadAcqConfig()
{
	LOG(TRACE) << " Acquisition Set acquisition parameters Options Setting ...";
	// Set acquisition parameters
	CAcqConfigDlg dlg(this, m_Acq);
	if (dlg.DoModal() == IDOK)
	{
		// Destroy objects
		DestroyObjects();

		// Update acquisition object
		SapAcquisition acq = *m_Acq;
		*m_Acq = dlg.GetAcquisition();

		// Recreate objects
		if (!CreateObjects())
		{
			*m_Acq = acq;
			CreateObjects();
		}

		GetSignalStatus();
		m_ImageWnd->OnSize();
		InvalidateRect(NULL);
		UpdateWindow();
		UpdateMenu();
	}
}

//*****************************************************************************************
//
//					General Options
//
//*****************************************************************************************
void CZJURollerBearingSurfaceDetectionDlg::OnBufferOptions()
{
	LOG(TRACE) << " General Buffer Options Setting ...";
	//CBufDlg dlg(this, m_Buffers, m_View->GetDisplay());
	CBufDlg dlg(this, m_Buffers);
	if (dlg.DoModal() == IDOK)
	{
		// Destroy objects
		DestroyObjects();

		// Update buffer object
		SapBuffer buf = *m_Buffers;
		*m_Buffers = dlg.GetBuffer();

		// Recreate objects
		if (!CreateObjects())
		{
			*m_Buffers = buf;
			//*m_ProcessBuffers = buf;
			CreateObjects();
		}

		/*int np = m_Buffers->GetWidth();
		np = m_Buffers->GetHeight();
		np = m_ProcessBuffers->GetWidth();
		np = m_ProcessBuffers->GetHeight();*/
		m_ImageWnd->OnSize();
		InvalidateRect(NULL);
		UpdateWindow();
		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnViewOptions()
{
	LOG(TRACE) << " General View Options Setting ...";
	CViewDlg dlg(this, m_View);
	if (dlg.DoModal() == IDOK)
	{
		m_ImageWnd->Invalidate();
		m_ImageWnd->OnSize();
	}
}

//*****************************************************************************************
//
//					File Options
//
//*****************************************************************************************
void CZJURollerBearingSurfaceDetectionDlg::OnFileNew()
{
	LOG(TRACE) << " File New Options Setting ...";
	//m_Buffers->Clear();
	m_ProcessBuffers->Clear();
	InvalidateRect(NULL, FALSE);
}

void CZJURollerBearingSurfaceDetectionDlg::OnFileLoad()
{
	LOG(TRACE) << " File Load Options Setting ...";
	CLoadSaveDlg dlg(this, m_Buffers, TRUE);
	if (dlg.DoModal() == IDOK)
	{
		LOG(TRACE)\
			<< "\n					m_Buffers Count: " << m_Buffers->GetCount()\
			<< "\n					m_Buffers GFrameRate: " << m_Buffers->GetFrameRate()\
			<< "\n					m_Buffers Height: " << m_Buffers->GetHeight()\
			<< "\n					m_Buffers Width: " << m_Buffers->GetWidth()\
			<< "\n					m_Buffers Type: " << m_Buffers->GetType();
		//m_ProcessBuffers->CopyAll(m_Buffers);   ImageRowPosition
		int PositionOfAImageRowCache = 0;
		for (int ImageFrameCount = 0; ImageFrameCount < m_Buffers->GetCount(); ++ImageFrameCount)
		{
			for (int ImageRowPosition = 0; ImageRowPosition < m_Buffers->GetHeight(); ++ImageRowPosition)
			{

				//m_ProcessBuffers->CopyRect(m_Buffers, j, 0, i, m_Buffers->GetWidth(), 1, j, 0, i);
				for (int PositionOfAImageRow = PositionOfAImageRowCache*0.99; PositionOfAImageRow < m_Buffers->GetWidth(); PositionOfAImageRow++)
				{
					BYTE ElementDataValue;
					m_Buffers->ReadElement(PositionOfAImageRow, ImageRowPosition, &ElementDataValue);
					if (ElementDataValue > m_nImagePreprocessingThreshold)
					{
						PositionOfAImageRowCache = PositionOfAImageRow;
						break;
					}
				}

				m_ProcessBuffers->CopyRect(m_Buffers, ImageFrameCount, PositionOfAImageRowCache, ImageRowPosition, m_lImageWidth, 1, ImageFrameCount, 0, ImageRowPosition);
			}
		}
		//m_ProcessingImage->Init();
		m_ProcessingImage->Execute();

		//SetImageFileSaveSetting(NULL, _T("SapBuffer"));

		InvalidateRect(NULL);
		UpdateWindow();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnFileSave()
{
	LOG(TRACE) << " File Save Options Setting ...";
	CLoadSaveDlg dlg(this, m_ProcessBuffers, FALSE);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::GetSignalStatus()
{
	SapAcquisition::SignalStatus signalStatus;

	if (m_Acq && m_Acq->IsSignalStatusAvailable())
	{
		if (m_Acq->GetSignalStatus(&signalStatus, SignalCallback, this))
			GetSignalStatus(signalStatus);
	}
}

void CZJURollerBearingSurfaceDetectionDlg::GetSignalStatus(SapAcquisition::SignalStatus signalStatus)
{
	m_IsSignalDetected = (signalStatus != SapAcquisition::SignalNone);

	if (m_IsSignalDetected)
		SetWindowText(m_appTitle);
	else
	{
		CString newTitle = m_appTitle;
		newTitle += " (No camera signal detected)";
		SetWindowText(newTitle);
	}
}


void CZJURollerBearingSurfaceDetectionDlg::OnParameterssettings()
{
	LOG(TRACE) << " Bearing Roller Parameters Setting ...";
	CBearingRollersParameterDlg BRDlg;
	if (IDOK == BRDlg.DoModal())
	{
		// Destroy objects
		DestroyObjects();

		//SapFormat;
		m_lImageHeight = BRDlg.m_lLine;
		m_szSavePath = BRDlg.GetImageSavePath();
		//_tcscpy(m_szSavePath, BRDlg.m_szSavePath);
		m_bSaveImageEnable = BRDlg.m_bSaveImageEnable;
		m_nMImageProcessingPrecision = BRDlg.GetImageProcessingPrecision();
		m_nImagePreprocessingThreshold = BRDlg.GetImagePreprocessingThreshold();
		m_lImageHeight = BRDlg.GetHeigthLine();
		m_lImageWidth = BRDlg.GetWeithLine();

		// Recreate objects
		if (!CreateObjects())
		{
			CreateObjects();
		}
		/*m_ProcessBuffers->SetWidth(m_lImageWidth);
		m_ProcessBuffers->SetHeight(m_lLine);*/

		SaveSettings();
		/*m_Buffers->CopyAll(m_ProcessBuffers);
		m_ProcessBuffers->SetHeight(m_lLine);*/

		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lLine, m_Buffers->GetFormat(),m_Buffers->GetType(),m_Buffers->GetLocation());
	}

}

void CZJURollerBearingSurfaceDetectionDlg::OnAboutAppShow()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::LoadSettings()
{
	m_szSavePath = m_App->GetProfileStringW(_T("ZJURollerBearingSurfaceDetectionDlg"), _T("m_szSavePath"), _T("C:\\Users\\WuJing\\Desktop\\TempImages\\"));
}

void CZJURollerBearingSurfaceDetectionDlg::SaveSettings()
{
	m_App->WriteProfileStringW(_T("ZJURollerBearingSurfaceDetectionDlg"), _T("m_szSavePath"), m_szSavePath);
}


void CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest()
{
	int viewWidth;
	int viewHeight;
	float	m_scaleHeightFactor;
	float	m_scaleWidthFactor;
	BOOL	m_bLockAspectRatio=TRUE;
	DWORD	m_scaleHeight;
	DWORD	m_scaleWidth;
	m_View->GetViewArea(&viewWidth, &viewHeight);

	m_scaleWidthFactor = 100.0f * viewWidth / m_View->GetBuffer()->GetWidth();
	m_scaleHeightFactor = 100.0f * viewHeight / m_View->GetBuffer()->GetHeight();

	if (m_bLockAspectRatio)
	{
		if (m_scaleWidthFactor < m_scaleHeightFactor)
		{
			m_scaleHeightFactor = m_scaleWidthFactor;
		}
		else
		{
			m_scaleWidthFactor = m_scaleHeightFactor;
		}
	}

	m_scaleWidth = (DWORD)floor(m_View->GetBuffer()->GetWidth()  * m_scaleWidthFactor / 100 + 0.5f);
	m_scaleHeight = (DWORD)floor(m_View->GetBuffer()->GetHeight() * m_scaleHeightFactor / 100 + 0.5f);

	m_scaleWidthFactor = 100.0f * m_scaleWidth / m_View->GetBuffer()->GetWidth();
	m_scaleHeightFactor = 100.0f * m_scaleHeight / m_View->GetBuffer()->GetHeight();

	m_View->SetScalingMode(m_scaleWidthFactor / 100.0f, m_scaleHeightFactor / 100.0f);
	m_ImageWnd->Invalidate();
	m_ImageWnd->OnSize();
	/*//CSerialPort Test
	BYTE kk[9] = { 12 ,22 ,23 ,67,1,11,22,45,89 };
	m_serialPort.WriteToPort(kk, 9);
	//m_serialPort.WriteToPort("Hell0",5);
	*/
	/*//LIBUSBWIN32 TEST
	char data[512];
	CString szSend;
	int len;
	CString szSend(_T("EE 01 AA FF FC FF FF"));
	len = DoStr2Hex(szSend, data);
	usb_bulk_write(gusb_handle, EP_OUT, data, len, 100);
	*/
}

void CZJURollerBearingSurfaceDetectionDlg::OnAnalysisHistogram()
{
	BYTE *pData;
	m_Buffers->GetAddress((void **)&pData);
	cv::Mat HistogramMat(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1, (void*)pData);
	cv::Mat HistDst;
	equalizeHist(HistogramMat, HistDst);    //直方图   Debug
	CHistogramDlg histDlg(&HistDst);
	histDlg.DoModal();
	LOG(TRACE) << " Image Analysis Histogram...";
}
