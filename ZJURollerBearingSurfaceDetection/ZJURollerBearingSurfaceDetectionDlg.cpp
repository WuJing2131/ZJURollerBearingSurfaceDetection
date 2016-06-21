// RollerBearingSurfaceDetectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "afxdialogex.h"
#include "BearingRollersParameterDlg.h"
#include "CommunicationConfigurationDlg.h"
#include "LibUSBWin32\include\lusb0_usb.h"
#include "HistogramDlg.h"
#include "CvvImage.h"
#include <crtdbg.h>
#include <iostream>
#include <string>

#define CRTDBG_MAP_ALLOC   //内存泄漏检测
#ifdef _DEBUG
//#include "vld.h"
#define new DEBUG_NEW
#endif

using namespace std;

#pragma region USB Parameters Settings
#define MY_CONFIG 1
#define MY_INTF 0
// Device endpoint(s)
#define EP_IN 0x81
#define EP_OUT 0x02
usb_dev_handle *gusb_handle;
HANDLE m_hDevice = INVALID_HANDLE_VALUE;
CWinThread *m_pReadReportThread = NULL;
OVERLAPPED ReadOverlapped;
char ReadReportBuffer[16] = { 0 };
BYTE KeyStatus = 0;
volatile BOOL m_bReadReportRunFlag = FALSE;
#pragma endregion

// status line indicator
static UINT indicators[] =
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
	m_numRead = NULL;
	m_countLine = 0;
	m_pSrc = NULL;
	m_pDst = NULL;
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
		if (pDlg->m_countLine < pDlg->m_lImageHeight)
		{
			for (int nimageCount = 0; nimageCount < pDlg->m_Buffers->GetCount(); nimageCount++)
			{
				int count = 0;
				for (int k = 0; k < pDlg->m_Buffers->GetWidth(); k++)
				{
					BYTE data;
					pDlg->m_Buffers->ReadElement(k, 0, &data);
					if (data > pDlg->m_nImagePreprocessingThreshold)
					{
						count = k;
						break;
					}
				}

				pDlg->m_ProcessBuffers->CopyRect(pDlg->m_Buffers, nimageCount, count, 0, pDlg->m_lImageWidth, 1, nimageCount, 0, pDlg->m_countLine);
				//pDlg->m_ProcessBuffers->CopyRect(pDlg->m_Buffers, nimageCount, 0, 0, pDlg->m_Buffers->GetWidth(), 1, nimageCount, 0, pDlg->m_countLine);
			}
			pDlg->m_countLine++;
		}
		else if (pDlg->m_countLine == pDlg->m_lImageHeight)
		{
			//Image Processing.........................
			// Process current buffer (see Run member function into the SapMyProcessing.cpp file)
			pDlg->m_countLine++;

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

	pDlg->showMatImgToWndResult();
	pDlg->showMatImgToWndComposite();
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
		//TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CRect rect;
	GetClientRect(&rect);

	m_wndStatusBar.SetPaneInfo(SEPARATOR, ID_SEPARATOR, SBPS_STRETCH, 60);
	m_wndStatusBar.SetPaneInfo(STATUSBAR_TX, ID_STATUSBAR_TX, SBPS_NORMAL, 60);
	m_wndStatusBar.SetPaneInfo(STATUSBAR_RX, ID_STATUSBAR_RX, SBPS_NORMAL, 60);
	m_wndStatusBar.SetPaneInfo(STATUSBAR_COMM, ID_STATUSBAR_COMM, SBPS_NORMAL, 150);
	m_wndStatusBar.SetPaneInfo(STATUSBAR_SENDKEY, ID_STATUSBAR_SENDKEY, SBPS_NORMAL, 60);
	m_wndStatusBar.SetPaneInfo(STATUPIXELPOSTION, IDS_STATUPIXELPOSTION, SBPS_NORMAL, 165);
	//m_wndStatusBar.SetPaneText(4, _T("hello"), TRUE);

	m_Menu.LoadMenu(IDR_MENU);
	SetMenu(&m_Menu);

	if (!m_AcqConwndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_AcqConwndToolBar.LoadToolBar(IDR_ACQ_CON_TOOLBAR))
	{
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
		m_Buffers->SetWidth(8192);     //#####################
		m_Buffers->SetHeight(650);    //#####################
		m_ProcessBuffers = new SapBuffer();
		LOG(TRACE) << " SapAcquisition Config Dlalog does not open when the program is initialized! Drive not loaded successfully ... ";
	}

	m_pSrc = new  Mat(0, 0, CV_8UC3);
	m_pDst = new  Mat(0, 0, CV_8UC3);
	// Define other objects
	m_View = new SapView(m_ProcessBuffers, m_viewWnd.GetSafeHwnd(), ViewCallback, this);
	m_ProcessingImage = new SapImageProcessing(m_ProcessBuffers, ImageProcessedCallback, this,
		m_pSrc, m_pDst, m_nMImageProcessingPrecision);         //m_nImageProcessingPrecision




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
		LOG(TRACE) << "\n					m_Buffers Count: " << m_Buffers->GetCount()
			<< "\n					m_Buffers GFrameRate: " << m_Buffers->GetFrameRate()
			<< "\n					m_Buffers Height: " << m_Buffers->GetHeight()
			<< "\n					m_Buffers Width: " << m_Buffers->GetWidth()
			<< "\n					m_Buffers Type: " << m_Buffers->GetType();
		// Clear all buffers
		m_Buffers->Clear();
	}
	else
		LOG(TRACE) << " Create buffer object success...";

	// Create m_ProcessBuffers object
	if (m_ProcessBuffers && !*m_ProcessBuffers)
	{
		int np = 0;
		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lImageHeight, m_Buffers->GetFormat(), m_Buffers->GetType(), m_Buffers->GetLocation());
		//m_ProcessBuffers->SetWidth(m_Buffers->GetWidth());
		m_ProcessBuffers->SetWidth(m_lImageWidth);
		m_ProcessBuffers->SetHeight(m_lImageHeight);
		np = m_Buffers->GetFormat();
		m_ProcessBuffers->SetFormat(m_Buffers->GetFormat());
		m_ProcessBuffers->SetType(m_Buffers->GetType());
		m_ProcessBuffers->SetLocation(m_Buffers->GetLocation());
		m_ProcessBuffers->SetCount(m_Buffers->GetCount());
		LOG(TRACE)
			<< "\n					m_ProcessBuffers Count: " << m_ProcessBuffers->GetCount()
			<< "\n					m_ProcessBuffers GFrameRate: " << m_ProcessBuffers->GetFrameRate()
			<< "\n					m_ProcessBuffers Height: " << m_ProcessBuffers->GetHeight()
			<< "\n					m_ProcessBuffers Width: " << m_ProcessBuffers->GetWidth()
			<< "\n					m_ProcessBuffers  Type: " << m_ProcessBuffers->GetType();
		LOG(TRACE)
			<< "\n		*************************************************************************************\n"
			<< "		TypeDefault               = -1" << "   TypeVirtual   = 0" << "   TypeContiguous            =  1\n"
			<< "		TypeScatterGather         = 32" << "   TypeOffscreen = 8" << "   TypeOffscreenVideo        = 10\n"
			<< "		TypeOverlay               =  6" << "   TypeDummy     =64" << "   TypePhysical              =128\n"
			<< "		TypeScatterGatherPhysical =160" << "   TypeUnmapped =512" << "   TypeScatterGatherUnmapped =544\n"
			<< "		*************************************************************************************";
		//np =m_Buffers->GetLocation();
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
	if (m_Xfer)			delete m_Xfer;
	if (m_ProcessingImage)			 delete m_ProcessingImage;
	if (m_ImageWnd)	delete m_ImageWnd;
	if (m_View)			delete m_View;
	if (m_FlatField)  delete m_FlatField;
	if (m_Buffers)		delete m_Buffers;
	if (m_ProcessBuffers) delete m_ProcessBuffers;
	if (m_Acq)			delete m_Acq;
	if (m_pData) delete m_pData;
	if (m_numRead) delete m_numRead;
	if (m_Gio) delete m_Gio;
	if (m_Lut) delete m_Lut;
	if (m_Performance) delete m_Performance;
	if (m_pSrc) delete m_pSrc;
	if (m_pDst) delete m_pDst;
	if (m_pWndImageResult) delete m_pWndImageResult;
	if (m_pWndImageComposite) delete m_pWndImageComposite;
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
	int count = 0;
	CLoadSaveDlg dlg(this, m_Buffers, TRUE);
	if (dlg.DoModal() == IDOK)
	{
		LOG(TRACE)
			<< "\n					m_Buffers Count: " << m_Buffers->GetCount()
			<< "\n					m_Buffers GFrameRate: " << m_Buffers->GetFrameRate()
			<< "\n					m_Buffers Height: " << m_Buffers->GetHeight()
			<< "\n					m_Buffers Width: " << m_Buffers->GetWidth()
			<< "\n					m_Buffers Type: " << m_Buffers->GetType();
		//m_ProcessBuffers->CopyAll(m_Buffers);
		for (int j = 0; j < m_Buffers->GetCount(); j++)
		{
			for (int i = 0; i < m_Buffers->GetHeight(); i++)
			{

				//m_ProcessBuffers->CopyRect(m_Buffers, j, 0, i, m_Buffers->GetWidth(), 1, j, 0, i);
				for (int k = 0; k < m_Buffers->GetWidth(); k++)
				{
					BYTE data;
					m_Buffers->ReadElement(k, i, &data);
					if (data > m_nImagePreprocessingThreshold)
					{
						count = k;
						break;
					}
				}

				m_ProcessBuffers->CopyRect(m_Buffers, j, count, i, m_lImageWidth, 1, j, 0, i);
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
	// TODO:  在此添加命令处理程序代码
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

// USB SerialPort Communication Parameters Settings
void CZJURollerBearingSurfaceDetectionDlg::OnCommunicationSettings()  //
{
	LOG(TRACE) << " Serial Port || USB Parameters Setting ...";
	CCommunicationConfigurationDlg SPDlg;
	if (IDOK == SPDlg.DoModal())
	{
		m_intPort = SPDlg.GetPort();
		m_intBaudRate = SPDlg.GetBaudRate();
		m_intDataBits = SPDlg.GetDateBits();
		m_cParity = SPDlg.GetParity();
		m_intStopBits = SPDlg.GetStopBits();
		m_szUsbVID = SPDlg.GetUSBVID();
		m_szUsbPID = SPDlg.GetUSBPID();
		if (SPDlg.IsEnableSerialPortCommunication())
		{
			if (!m_bSerialPortOpen)
			{
				if (m_serialPort.InitPort(this, m_intPort, m_intBaudRate, m_cParity, m_intDataBits, m_intStopBits, EV_RXFLAG | EV_RXCHAR, 512))
				{
					m_serialPort.StartMonitoring();
					m_bSerialPortOpen = TRUE;
					CString strtemp;
					strtemp.Format(_T("COM%d Connected"), m_intPort);
					/*SetDlgItemTextW(ID_STATUSBAR_COMM, strtemp);
					m_wndStatusBar.SetPaneInfo(STATUSBAR_COMM, ID_STATUSBAR_COMM, SBPS_NORMAL, strtemp.GetLength());*/
					//SetPaneInfo(int nIndex, UINT nID, UINT nStyle, int cxWidth);
					m_wndStatusBar.SetPaneText(STATUSBAR_COMM, strtemp, TRUE);
					LOG(TRACE)
						<< "\n					Serial Port Port:     " << m_intPort
						<< "\n					Serial Port Baud Rate " << m_intBaudRate
						<< "\n					Serial Port Date Bits " << m_intDataBits
						<< "\n					Serial Port Parity    " << m_cParity
						<< "\n					Serial Port Stop Bits " << m_intStopBits
						<< "\n					Serial Port Date Bits " << m_intDataBits
						<< "\n					Connected ... ";

				}
				else
				{
					AfxMessageBox(_T("Serial Port  initialize Falied!"));
					m_bSerialPortOpen = FALSE;
					CString strtemp;
					strtemp.Format(_T("Serial Port  initialize Falied"), m_intPort);
					m_wndStatusBar.SetPaneText(STATUSBAR_COMM, strtemp, TRUE);
					LOG(TRACE) << " Serial Port  initialize Falied!";
				}
			}
		}
		else
		{
			if (m_bSerialPortOpen)
			{
				m_serialPort.ClosePort();
				m_bSerialPortOpen = FALSE;
				CString strtemp;
				strtemp.Format(_T("Serial Port Closed"), m_intPort);
				m_wndStatusBar.SetPaneText(STATUSBAR_COMM, strtemp, TRUE);
				LOG(TRACE) << " Serial Port Closed ! ";
			}
		}

		if (SPDlg.IsEnableUSBCommunication())
		{
			char VIDdata[2] = { 0 };
			char PIDdata[2] = { 0 };
			DoStr2Hex(m_szUsbVID, VIDdata);
			DoStr2Hex(m_szUsbPID, PIDdata);

			usb_init();
			usb_find_busses();
			usb_find_devices();
			struct usb_bus *bus;
			struct usb_device *dev;
			for (bus = usb_get_busses(); bus; bus = bus->next)
			{
				for (dev = bus->devices; dev; dev = dev->next)
				{
					if ((dev->descriptor.idVendor == (unsigned char)VIDdata[0] * 256 + (unsigned char)VIDdata[1]) &&
						(dev->descriptor.idProduct == (unsigned char)PIDdata[0] * 256 + (unsigned char)PIDdata[1]))
					{
						gusb_handle = usb_open(dev);
						if (!gusb_handle)
						{
							CString str;
							str.Format(_T("Error Open USB Device Handle \n%s\n"), usb_strerror());
							m_statusWnd.SetWindowText(str);
							LOG(TRACE) << " Error Open USB Device Handle";
						}
						else
						{
							CString str;
							str.Format(_T("success: device %04X:%04X opened\n"),
								(unsigned char)VIDdata[0] * 256 + (unsigned char)VIDdata[1],
								(unsigned char)PIDdata[0] * 256 + (unsigned char)PIDdata[1]);
							m_statusWnd.SetWindowText(str);
							LOG(TRACE) << "\n					USB idVendor  " << (unsigned char)VIDdata[0] * 256 + (unsigned char)VIDdata[1]
								<< "\n					USB idProduct " << (unsigned char)PIDdata[0] * 256 + (unsigned char)PIDdata[1]
								<< "\n					USB opened!";
						}
					}
				}
			}
			if (usb_set_configuration(gusb_handle, MY_CONFIG) < 0)
			{
				CString str;
				str.Format(_T("error setting config #%d: %s\n"), MY_CONFIG, usb_strerror());
				m_statusWnd.SetWindowText(str);
				usb_close(gusb_handle);
				LOG(TRACE) << " Error setting config #" << MY_CONFIG;
				return;
			}
			else
			{
				CString str;
				str.Format(_T("success: set configuration #%d\n"), MY_CONFIG);
				m_statusWnd.SetWindowText(str);
				LOG(TRACE) << " Success: set configuration #" << MY_CONFIG;
			}
			if (usb_claim_interface(gusb_handle, 0) < 0)
			{
				CString str;
				str.Format(_T("error claiming interface #%d:\n%s\n"), MY_INTF, usb_strerror());
				m_statusWnd.SetWindowText(str);
				usb_close(gusb_handle);
				LOG(TRACE) << " Error claiming interface #" << MY_INTF;
				return;
			}
			else
			{
				CString str;
				str.Format(_T("success: claim_interface #%d\n"), MY_INTF);
				m_statusWnd.SetWindowText(str);
				LOG(TRACE) << " Success: claim_interface #" << MY_INTF;
			}
			m_pReadReportThread = AfxBeginThread(ReadReportThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
			if (m_pReadReportThread == NULL)
			{
				CString strLog;
				strLog = _T("Thread Create Failed!");
				m_statusWnd.SetWindowText(strLog);
				CloseHandle(m_hDevice);
				m_hDevice = INVALID_HANDLE_VALUE;
				LOG(TRACE) << " Thread Create Failed!....";
				exit(0);
			}
			m_pReadReportThread->m_bAutoDelete = FALSE;
			m_pReadReportThread->ResumeThread();
		}
	}
}

void  CZJURollerBearingSurfaceDetectionDlg::CmdToLowerComputer()
{
	//do something for test
}

UINT ReadReportThread(LPVOID lpParam)
{
	LOG(TRACE) << " Read Report Thread Created....";
	CZJURollerBearingSurfaceDetectionDlg *pDlg = (CZJURollerBearingSurfaceDetectionDlg *)lpParam;
	ReadOverlapped.Offset = 0;
	ReadOverlapped.OffsetHigh = 0;
	ReadOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bReadReportRunFlag = TRUE;
	while (TRUE)
	{
		if (!m_bReadReportRunFlag)
		{
			if (ReadOverlapped.hEvent != NULL)
			{
				CloseHandle(ReadOverlapped.hEvent);
				ReadOverlapped.hEvent = NULL;
			}
			return 0;
		}
		ResetEvent(ReadOverlapped.hEvent);
		//usb_bulk_read(gusb_handle, 0x81, ReadReportBuffer, sizeof(ReadReportBuffer), 100);
		int ret = usb_interrupt_read(gusb_handle, 0x81, ReadReportBuffer, sizeof(ReadReportBuffer), 0);
		std::vector<int> vecNum;
		for (int i = 0; i < ret; i++)
			vecNum.push_back(ReadReportBuffer[i]);
		el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
		LOG(TRACE) << " USB Receive:" << vecNum;

		if ((unsigned char)ReadReportBuffer[0] == 0x01 && (unsigned char)ReadReportBuffer[3] == 0xFF && (unsigned char)ReadReportBuffer[4] == 0xFC
			&& (unsigned char)ReadReportBuffer[5] == 0xFF && (unsigned char)ReadReportBuffer[6] == 0xFF)
			AfxMessageBox(_T("Receive Right!"));
		//if (ReadReportBuffer[0])
		/*if (ret == 7)
		{
		if (ReadReportBuffer[0] == 0xEE && ReadReportBuffer[3] == 0xFF && ReadReportBuffer[4] == 0xFC
		&& ReadReportBuffer[5] == 0xFF && ReadReportBuffer[6] == 0xFF)
		{
		CString str;
		str.Format(_T("ret=%d"), ReadReportBuffer[1]);
		AfxMessageBox(str);
		}
		}*/
		/*	unsigned char x=(unsigned char)ReadReportBuffer[0];
		CString str;
		str.Format(_T("ret=%d,%d"), ret, (unsigned char)ReadReportBuffer[0]);
		AfxMessageBox(str);*/
		/*KeyStatus = ReadReportBuffer[0];
		if (KeyStatus==0xEE)
		AfxMessageBox(_T("Recevie Ok!"));*/
		//pDlg->SetKeyStatus();
		ResetEvent(ReadOverlapped.hEvent);
	}
}

LRESULT CZJURollerBearingSurfaceDetectionDlg::OnSerialPortReceiveCommunication(WPARAM ch, LPARAM  port)
{
	static int SerialPortCount = 0;
	static int cmd_state = 0;

	m_lRXCount++;

	if (SerialPortCount == 0 && ch != 0xEE)  //EE 0x XX FF FC FF FF   
		return 0;

	if (SerialPortCount < PKG_SIZE)
		m_pkg[SerialPortCount++] = ch;
	if (ch == 0xFF)
	{
		switch (cmd_state)
		{
		case 2:cmd_state = 3; break;
		case 3:cmd_state = 4; break;
		default:cmd_state = 1; break;
		}
	}
	else if (ch == 0xFC)
	{
		switch (cmd_state)
		{
		case 1:cmd_state = 2; break;
		case 3:cmd_state = 2; break;
		default:cmd_state = 0; break;
		}
	}
	else
		cmd_state = 0;
	if (cmd_state == 4)
	{
		switch (m_pkg[1])
		{
		case 1:  m_countLine = 0; break;
		case 2:                   break;  //其余定义功能
		default:
			break;
		}

		std::vector<int> vecNum;
		for (int i = 0; i < SerialPortCount; i++)
			vecNum.push_back(m_pkg[i]);
		el::Loggers::removeFlag(el::LoggingFlag::NewLineForContainer);
		LOG(TRACE) << " Serial Port Receive:" << vecNum;

		cmd_state = 0;
		SerialPortCount = 0;
		return 0;
	}

	CString str;
	str.Format(_T("RX:%d"), m_lRXCount);
	m_wndStatusBar.SetPaneText(STATUSBAR_RX, str, TRUE);
	return 0;
}

int CZJURollerBearingSurfaceDetectionDlg::DoStr2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	for (int i = 0; i < len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len) break;
		l = str[i];
		t = DoHexChar(h);
		t1 = DoHexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;
}

char CZJURollerBearingSurfaceDetectionDlg::DoHexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}

byte CZJURollerBearingSurfaceDetectionDlg::DoCheckSum(unsigned char *buffer, int   size)
{
	byte myv = 0x00;
	for (int i = 0; i < size; i++)
		myv += *buffer++;
	return myv;
}

void CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest()
{
	// TODO: 在此添加命令处理程序代码
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

BOOL CZJURollerBearingSurfaceDetectionDlg::WCharToMByte(LPCWSTR lpSrc, char * lpDest)
{
	int iSize;
	iSize = WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, NULL, 0, NULL, NULL);
	lpDest = (char*)malloc((iSize + 1));
	WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, lpDest, iSize, NULL, NULL);
	return TRUE;
}

BOOL CZJURollerBearingSurfaceDetectionDlg::WChar2MByte(LPCWSTR lpSrc, LPSTR lpDest, int nlen)
{


	int n = 0;

	n = WideCharToMultiByte(CP_OEMCP, 0, lpSrc, -1, lpDest, 0, 0, FALSE);
	if (n < nlen) return FALSE;
	WideCharToMultiByte(CP_OEMCP, 0, lpSrc, -1, lpDest, nlen, 0, FALSE);
	return TRUE;
}

//BOOL WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
//{
//	DWORD dwMinSize;
//	dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
//	if (dwSize < dwMinSize)
//	{
//		return FALSE;
//	}
//	WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, lpszStr, dwSize, NULL, FALSE);
//	return TRUE;
//}

void CZJURollerBearingSurfaceDetectionDlg::OnAboutAppShow()
{
	// TODO: 在此添加命令处理程序代码
	CAboutDlg dlg;
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::showMatImgToWndResult()
{

	//CWnd* pWnd = GetDlgItem(IDC_IMAGERESULT);
	//if (m_pSrc->empty())
	//	return;
	//static BITMAPINFO *bitMapinfo = NULL;
	//static bool First = TRUE;
	//if (First)
	//{
	//	BYTE *bitBuffer = new BYTE[40 + 4 * 256];
	//	if (bitBuffer == NULL)
	//	{
	//		return;
	//	}
	//	First = FALSE;
	//	memset(bitBuffer, 0, 40 + 4 * 256);
	//	bitMapinfo = (BITMAPINFO *)bitBuffer;
	//	bitMapinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//	bitMapinfo->bmiHeader.biPlanes = 1;      // 目标设备的级别，必须为1  
	//	for (int i = 0; i<256; i++)
	//	{   //颜色的取值范围 (0-255)  
	//		bitMapinfo->bmiColors[i].rgbBlue = bitMapinfo->bmiColors[i].rgbGreen = bitMapinfo->bmiColors[i].rgbRed = (BYTE)i;
	//	}

	//}

	//bitMapinfo->bmiHeader.biHeight = -m_pSrc->rows;   //如果高度为正的，位图的起始位置在左下角。如果高度为负，起始位置在左上角。  
	//bitMapinfo->bmiHeader.biWidth = m_pSrc->cols;
	//bitMapinfo->bmiHeader.biBitCount = m_pSrc->channels() * 8;     // 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一  

	//CRect drect;
	//pWnd->GetClientRect(drect);    //(drect);  (&drect);  两种方式均可，竟然    

	//CClientDC dc(pWnd);
	//HDC hDC = dc.GetSafeHdc();
	//SetStretchBltMode(hDC, COLORONCOLOR);  //此句不能少哦  
	//									   //内存中的图像数据拷贝到屏幕上  
	//StretchDIBits(hDC,
	//	0,
	//	0,
	//	700,        //显示窗口宽度  
	//	265,       //显示窗口高度  
	//	0,
	//	0,
	//	m_pSrc->cols,          //图像宽度  
	//	m_pSrc->rows,          //图像高度  
	//	m_pSrc->data,
	//	bitMapinfo,
	//	DIB_RGB_COLORS,
	//	SRCCOPY
	//);


	SetImageFileSaveSetting(m_pSrc, _T("Result"));
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
	CWnd *pWnd = GetDlgItem(IDC_IMAGERESULT); //获得pictrue控件窗口的句柄
	CRect rect;
	pWnd->GetClientRect(&rect); //获得pictrue控件所在的矩形区域
	CDC *pDC = pWnd->GetDC(); //获得pictrue控件的DC
	pDC->SetStretchBltMode(COLORONCOLOR);
	/*StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0,
	bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);*/
	GetClientRect(&rect);
	StretchDIBits(pDC->GetSafeHdc(), 0, 0, 700, 265, 0, 0,
		bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);



	//IplImage qImg1;
	//qImg1 = IplImage(*m_pSrc); // cv::Mat -> IplImage
	//						   //DrawPicToHDC(&qImg1, IDC_SHOWRESULTIMAGE);
	//CDC *pDC = GetDlgItem(IDC_IMAGERESULT)->GetDC();
	//HDC hDC = pDC->GetSafeHdc();
	//CRect rect;
	//GetClientRect(&rect);
	//GetDlgItem(IDC_IMAGERESULT)->MoveWindow(rect.left + 10, rect.top + 50, rect.Width() / 2 - 10, 265);
	////GetDlgItem(IDC_IMAGERESULT)->MoveWindow(rect.left + 10, rect.top + 50, 0, 0);
	////SetRect(rect, 10, 50, 600, 265);
	//SetRect(rect, rect.left, rect.top , rect.left + rect.Width() / 2 - 10, rect.top + 265);
	////GetDlgItem(IDC_IMAGERESULT)->GetClientRect(&rect);
	//CvvImage cimg;
	//cimg.CopyOf(&qImg1);
	///*rect.left += 10;
	//rect.top += 50;*/
	/*rect.bottom = rect.top + 265;
	rect.right = rect.left + rect.Width() / 2-10;*/
	/*cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);*/

	LOG(TRACE) << " Show Image Processed Result...";
}

//std::string TCHAR2STRING（TCHAR *STR）
//
//{
//
//	int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);
//
//	char* chRtn = new char[iLen * sizeof(char)];
//
//	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
//
//	std::string str(chRtn);
//
//	return str;
//
//}

void CZJURollerBearingSurfaceDetectionDlg::showMatImgToWndComposite()
{
	//IplImage qImg1;
	//qImg1 = IplImage(*m_pDst); // cv::Mat -> IplImage
	////DrawPicToHDC(&qImg1, IDC_SHOWRESULTIMAGE);
	//CDC *pDC = GetDlgItem(IDC_IMAGECOMPOSITE)->GetDC();
	//HDC hDC = pDC->GetSafeHdc();
	//CRect rect;
	//GetClientRect(&rect);
	////GetDlgItem(IDC_IMAGECOMPOSITE)->MoveWindow(rect.Width() / 2, rect.top + 50, rect.Width() / 2-10, 265);
	//GetDlgItem(IDC_IMAGECOMPOSITE)->MoveWindow(rect.Width() / 2, rect.top + 50, rect.Width() / 2-10, 265);
	////GetDlgItem(IDC_IMAGECOMPOSITE)->GetClientRect(&rect);
	//SetRect(rect, rect.Width() / 2, rect.top, rect.Width() - 10, rect.top + 265);
	//CvvImage cimg;
	//cimg.CopyOf(&qImg1);
	////rect.left += rect.Width() / 2;
	////rect.top += 50;
	///*rect.bottom = rect.top + 265;
	//rect.right =  rect.Width()- 10;*/
	//cimg.DrawToHDC(hDC, &rect);
	//ReleaseDC(pDC);

	SetImageFileSaveSetting(m_pDst, _T("Composite"));
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
	CWnd *pWnd = GetDlgItem(IDC_IMAGECOMPOSITE); //获得pictrue控件窗口的句柄
	CRect rect;
	pWnd->GetClientRect(&rect); //获得pictrue控件所在的矩形区域
	CDC *pDC = pWnd->GetDC(); //获得pictrue控件的DC
	pDC->SetStretchBltMode(COLORONCOLOR);
	/*StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0,
	bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);*/
	GetClientRect(&rect);
	StretchDIBits(pDC->GetSafeHdc(), 200, 0, 700, 265, 0, 0,
		bmpInfo.biWidth, bmpInfo.biHeight, pBmpData, pBmpInfo, DIB_RGB_COLORS, SRCCOPY);

	//CWnd* pWnd = GetDlgItem(IDC_IMAGECOMPOSITE);
	//if (m_pDst->empty())
	//	return;
	//static BITMAPINFO *bitMapinfo = NULL;
	//static bool First = TRUE;
	//if (First)
	//{
	//	BYTE *bitBuffer = new BYTE[40 + 4 * 256];
	//	if (bitBuffer == NULL)
	//	{
	//		return;
	//	}
	//	First = FALSE;
	//	memset(bitBuffer, 0, 40 + 4 * 256);
	//	bitMapinfo = (BITMAPINFO *)bitBuffer;
	//	bitMapinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	//	bitMapinfo->bmiHeader.biPlanes = 1;      // 目标设备的级别，必须为1  
	//	for (int i = 0; i<256; i++)
	//	{   //颜色的取值范围 (0-255)  
	//		bitMapinfo->bmiColors[i].rgbBlue = bitMapinfo->bmiColors[i].rgbGreen = bitMapinfo->bmiColors[i].rgbRed = (BYTE)i;
	//	}

	//}

	//bitMapinfo->bmiHeader.biHeight = -m_pDst->rows;   //如果高度为正的，位图的起始位置在左下角。如果高度为负，起始位置在左上角。  
	//bitMapinfo->bmiHeader.biWidth = m_pDst->cols;
	//bitMapinfo->bmiHeader.biBitCount = m_pDst->channels() * 8;     // 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一  

	//CRect drect;
	//pWnd->GetClientRect(drect);    //(drect);  (&drect);  两种方式均可，竟然    
	//CClientDC dc(pWnd);
	//HDC hDC = dc.GetSafeHdc();
	//SetStretchBltMode(hDC, COLORONCOLOR);  //此句不能少哦  
	//									   //内存中的图像数据拷贝到屏幕上  
	//BOOL TF=StretchDIBits(hDC,
	//	200,
	//	0,
	//	700,        //显示窗口宽度  
	//	265,       //显示窗口高度  
	//	0,
	//	0,
	//	m_pDst->cols,          //图像宽度  
	//	m_pDst->rows,          //图像高度  
	//	m_pDst->data,
	//	bitMapinfo,
	//	DIB_RGB_COLORS,
	//	SRCCOPY
	//);

	LOG(TRACE) << " Show Image Processed Composite...";
}

void CZJURollerBearingSurfaceDetectionDlg::DrawPicToHDC(IplImage *img, UINT ID)
{
	CDC *pDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pDC->GetSafeHdc();
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);
	CvvImage cimg;
	cimg.CopyOf(img);
	cimg.DrawToHDC(hDC, &rect);
	ReleaseDC(pDC);
}

void CZJURollerBearingSurfaceDetectionDlg::SetImageFileSaveSetting(Mat *pmat, CString HeadString)
{
	m_timeImageSequence = CTime::GetCurrentTime();
	CString szTime = m_timeImageSequence.Format("%Y-%m-%d_%H-%M-%S.bmp");
	m_szTempImageSavePath = m_szSavePath + HeadString + szTime;
	if (pmat != NULL)
	{
		char* pszMultiByte = NULL;
		int iSize;
		iSize = WideCharToMultiByte(CP_ACP, 0, m_szTempImageSavePath, -1, NULL, 0, NULL, NULL);
		pszMultiByte = (char*)malloc((iSize + 1));
		WideCharToMultiByte(CP_ACP, 0, m_szTempImageSavePath, -1, pszMultiByte, iSize, NULL, NULL);
		//WCharToMByte(szImageSavePath, pszMultiByte);
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

void CZJURollerBearingSurfaceDetectionDlg::LoadSettings()
{
	m_szSavePath = m_App->GetProfileStringW(_T("ZJURollerBearingSurfaceDetectionDlg"), _T("m_szSavePath"), _T("C:\\Users\\WuJing\\Desktop\\TempImages\\"));
}

void CZJURollerBearingSurfaceDetectionDlg::SaveSettings()
{
	m_App->WriteProfileStringW(_T("ZJURollerBearingSurfaceDetectionDlg"), _T("m_szSavePath"), m_szSavePath);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldEnabled()
{
	m_FlatFieldEnabled = TRUE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_ENABLE, MF_CHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldDisabled()
{
	m_FlatFieldEnabled = FALSE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_ENABLE, MF_UNCHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldUseROIEnable()
{
	m_FlatFieldUseROI = TRUE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_USEAROI, MF_CHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldUseROIDisable()
{
	m_FlatFieldUseROI = FALSE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_USEAROI, MF_UNCHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldUseHardwareEnable()
{
	m_FlatFieldUseHardware = TRUE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_HARDWARECORRECTION, MF_CHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldUseHardwareDisable()
{
	m_FlatFieldUseHardware = FALSE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_HARDWARECORRECTION, MF_UNCHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldPixelReplacementEnable()
{
	m_FlatFieldPixelReplacement = TRUE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_PIXELREPLACEMENT, MF_CHECKED);
}

void CZJURollerBearingSurfaceDetectionDlg::SetFlatFieldPixelReplacementDisable()
{
	m_FlatFieldPixelReplacement = FALSE;
	AfxGetApp()->m_pMainWnd->GetMenu()->CheckMenuItem(ID_FLATFIELDCORRECTION_PIXELREPLACEMENT, MF_UNCHECKED);
}

//*****************************************************************************************
//
//					Flat field Options
//
//*****************************************************************************************
#define DEFAULT_FFC_FILENAME			"FFC.tif"
#define STANDARD_FILTER					"TIFF Files (*.tif)|*.tif||"

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldEnable()
{
	LOG(TRACE) << " Flat Field Enable Setting...";
	CWaitCursor wait;

	UpdateData();

	// To enable/disable flat field correction, the transfer object must first be disconnected from the hardware
	if (m_Xfer && *m_Xfer)
	{
		m_Xfer->Destroy();
		LOG(TRACE) << " Success: the transfer object is disconnected from the hardware";
	}


	BOOL success = TRUE;

	// Check for invalid pixel format
	if (m_FlatFieldEnabled && !m_FlatFieldUseHardware)
	{
		success = CheckPixelFormat("software correction");
		LOG(TRACE) << " Check for invalid pixel format: " << success
			<< "\n            *****   1->Success    0->failed    ******";
	}


	// Enable/disable flat field correction
	if (success)
	{
		success = m_FlatField->Enable(m_FlatFieldEnabled, m_FlatFieldUseHardware);
		LOG(TRACE) << " Enable/disable flat field correction: " << success
			<< "\n           *****   1->Enable    0->Disable    ******";
	}


	if (!success)
	{
		m_FlatFieldEnabled = !m_FlatFieldEnabled;
		UpdateData(FALSE);
	}

	if (m_Xfer && !*m_Xfer)
	{
		LOG(TRACE) << " Recreate the transfer object to reconnect it to the hardware";
		// Recreate the transfer object to reconnect it to the hardware
		if (!m_Xfer->Create() || !m_Xfer->Init(TRUE))
		{
			m_FlatFieldUseHardware = !m_FlatFieldUseHardware;
			m_FlatField->Enable(m_FlatFieldEnabled, m_FlatFieldUseHardware);
			m_Xfer->Create();
			m_Xfer->Init(TRUE);
			UpdateData(FALSE);
			if (!m_FlatFieldUseHardware)
			{
				AfxMessageBox(_T("There was an error enabling hardware correction.  Software correction enabled in its place."));
				LOG(TRACE) << " There was an error enabling hardware correction.  Software correction enabled in its place.";
			}

		}
		m_ProcessingImage->Init();
	}

	UpdateMenu();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldUseROI()
{
	LOG(TRACE) << " Flat Field Use ROI...";
	UpdateData();

	m_FlatField->ResetRegionOfInterest();

	if (!m_FlatFieldUseROI)
		m_ImageWnd->HideRoiTracker();

	m_ImageWnd->OnPaint();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldUseHardware()
{
	LOG(TRACE) << " Flat Field Use Hardware...";
	CWaitCursor wait;

	UpdateData();

	// Get Parameter of Bayer Decoder
	int bayerDecoder;
	m_Acq->GetParameter(CORACQ_PRM_BAYER_DECODER_ENABLE, &bayerDecoder);

	if (m_Acq && *m_Acq && m_Acq->IsBayerAvailable())
	{
		if (bayerDecoder)
		{
			m_FlatFieldUseHardware = true;
			AfxMessageBox(_T("Hardware correction is always used when Bayer decoder is enabled."));
			LOG(TRACE) << " Hardware correction is always used when Bayer decoder is enabled.";
			UpdateData();
		}
	}

	OnFlatFieldEnable();

	UpdateMenu();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldPixelReplacement()
{
	LOG(TRACE) << " Flat Field Pixel Replacement...";
	UpdateData();

	// Enable/disable pixel replacement flat field correction
	m_FlatField->EnablePixelReplacement(m_FlatFieldPixelReplacement);

	UpdateMenu();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldCalibrate()
{
	LOG(TRACE) << " Flat Field Calibrate...";
	if (!CheckPixelFormat("calibration"))
		return;

	CFlatFieldDlg dlg(this, m_FlatField, m_Xfer, m_Buffers);
	dlg.DoModal();
	UpdateMenu();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldLoad()
{
	LOG(TRACE) << " Flat Field Load...";
	CFileDialog dlgFFC(TRUE, _T(""), _T(DEFAULT_FFC_FILENAME), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T(STANDARD_FILTER), this);

	dlgFFC.m_ofn.lpstrTitle = _T("Open Flat Field Correction");
	if (dlgFFC.DoModal() == IDOK)
	{
		// Load flat field correction file
		if (!m_FlatField->Load(CStringA(dlgFFC.GetPathName())))
			return;
	}

	UpdateMenu();
}

void CZJURollerBearingSurfaceDetectionDlg::OnFlatFieldSave()
{
	LOG(TRACE) << " Flat Field Save...";
	CFileDialog dlgFFC(FALSE, _T(""), _T(DEFAULT_FFC_FILENAME), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T(STANDARD_FILTER), this);

	dlgFFC.m_ofn.lpstrTitle = _T("Save Flat Field Correction As");
	if (dlgFFC.DoModal() == IDOK)
	{
		// Save flat field correction file
		m_FlatField->Save(CStringA(dlgFFC.GetPathName()));
	}

	UpdateMenu();
}

BOOL CZJURollerBearingSurfaceDetectionDlg::CheckPixelFormat(char* mode)
{
	SapFormat format = m_Buffers->GetFormat();

	if (format != SapFormatMono8 && format != SapFormatMono16)
	{
		CString message;
		message.Format(_T("Pixel format must be 8-bit or 16-bit monochrome for %s\n"), mode);
		MessageBox(message);
		return FALSE;
	}

	return TRUE;
}

void CZJURollerBearingSurfaceDetectionDlg::OnAnalysisHistogram()
{
	BYTE *pData;
	m_Buffers->GetAddress((void **)&pData);
	Mat HistogramMat(m_ProcessBuffers->GetHeight(), m_ProcessBuffers->GetWidth(), CV_8UC1, (void*)pData);
	Mat HistDst;
	equalizeHist(HistogramMat, HistDst);    //直方图   Debug
	CHistogramDlg histDlg(&HistDst);
	histDlg.DoModal();
	LOG(TRACE) << " Image Analysis Histogram...";
}
