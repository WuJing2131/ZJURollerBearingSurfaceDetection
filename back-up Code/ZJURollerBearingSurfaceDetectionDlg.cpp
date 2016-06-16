// RollerBearingSurfaceDetectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "afxdialogex.h"
#include"BearingRollersDlg.h"
#include "SerialPortDlg.h"

//#include "SapClassBasic.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_STATUSBAR_TX,
	ID_STATUSBAR_RX,
	ID_STATUSBAR_COMM,
	ID_STATUSBAR_SENDKEY,
	IDS_STATUPIXELPOSTION
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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


// CRollerBearingSurfaceDetectionDlg 对话框



CZJURollerBearingSurfaceDetectionDlg::CZJURollerBearingSurfaceDetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CZJURollerBearingSurfaceDetectionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ImageWnd = NULL;
	m_Acq = NULL;
	m_Buffers = NULL;
	m_Xfer = NULL;
	m_View = NULL;
	m_IsSignalDetected = TRUE;

	m_lLine = 640;     //默认采集640行
	m_pData = NULL;
	m_numRead = NULL;
	m_countLine = 0;
	m_Gio = NULL;
	m_Lut = NULL;
	m_ProcessingImage = NULL;
	m_Performance = NULL;
	//	m_IsProBufFinshed = FALSE;
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
	ON_MESSAGE(WM_COMM_RXCHAR, OnCommunication)
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
	ON_WM_ENDSESSION()
	ON_WM_QUERYENDSESSION()
	//ON_COMMAND(IDC_FILE_NEW, &CRollerBearingSurfaceDetectionDlg::OnFileNew)
	ON_COMMAND(ID_PARAMETERSSETTINGS, &CZJURollerBearingSurfaceDetectionDlg::OnParameterssettings)
	ON_COMMAND(ID_SERIALPORTSETTINGS, &CZJURollerBearingSurfaceDetectionDlg::OnSerialportsettings)
	ON_COMMAND(ID_BEARINGROLLER_TEST, &CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest)
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

	if (((pDlg->m_Buffers->GetIndex() + 1) % pDlg->m_lLine) == 0 ||
		pDlg->m_Buffers->GetIndex() == pDlg->m_Buffers->GetCount() - 1)
	{
		pDlg->m_ProcessingImage->Execute();

		//// Refresh view
		//pDlg->m_View->Show();

		//// Refresh controls
		//pDlg->PostMessage(WM_UPDATE_CONTROLS, 0, 0);
	}

	//if (pInfo->IsTrash())
	//{
	//	CString str;
	//	str.Format(_T("Frames acquired in trash buffer: %d"), pInfo->GetEventCount());
	//	pDlg->m_statusWnd.SetWindowText(str);
	//}

	//// Refresh view
	//else
	//{
	//	if (pDlg->m_countLine < pDlg->m_lLine)
	//	{
	//		// Create an array large enough to hold all buffer data
	//		int size = pDlg->m_Buffers->GetWidth() * sizeof(BYTE);
	//		BYTE *dataBuf = new BYTE[size];
	//		pDlg->m_Buffers->Read(0, size, dataBuf);
	//		pDlg->m_ProcessBuffers->Write(pDlg->m_countLine, size, dataBuf);


	//		// Get the buffer pitch in bytes
	//		//int pitch = pDlg->m_Buffers->GetPitch();
	//		// Get the buffer data address
	//		BYTE *pData;
	//		pDlg->m_Buffers->GetAddress((void **)&pData);
	//		int pixelNum;
	//		for (pixelNum = 0; pixelNum < pDlg->m_Buffers->GetWidth(); pixelNum++)
	//		{
	//			if (*pData > 10)   //阈值分割
	//				break;
	//		}
	//		pDlg->m_ProcessBuffers->Write(pDlg->m_countLine, pDlg->m_lImageWidth, pData);
	//		pDlg->m_Buffers->ReleaseAddress(pData);

	//		//pDlg->m_View->GetBuffer()->ReadLine(0, 0, pDlg->m_View->GetBuffer()->GetWidth(), 0, pDlg->m_pData, pDlg->m_numRead);
	//		//pDlg->m_ProcessBuffers->WriteLine(pDlg->m_countLine, 0, pDlg->m_countLine, pDlg->m_View->GetBuffer()->GetWidth(), pDlg->m_pData, pDlg->m_numRead);
	//		pDlg->m_countLine++;
	//		//	pDlg->m_IsProBufFinshed = FALSE;
	//	}
	//	else
	//	{
	//		//Image Processing.........................
	//		// Process current buffer (see Run member function into the SapMyProcessing.cpp file)
	//		pDlg->m_ProcessingImage->Execute();

	//		pDlg->m_countLine = 0;
	//		//	pDlg->m_IsProBufFinshed = TRUE;

	//	}


	//	//pDlg->m_View->Show();

	//}
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

	// Refresh view
	pDlg->m_statusWnd.SetWindowText(str);
	pDlg->m_View->Show();
	pDlg->UpdateTitleBar();
	if (pDlg->m_bSaveImageEnable)
	{
		pDlg->m_Buffers->Save(CStringA(pDlg->m_szSavePath), "-format bmp");
	}
	pDlg->m_Buffers->Clear();

}

//
// This function is called each time a buffer has been shown by the view object
//
void CZJURollerBearingSurfaceDetectionDlg::ViewCallback(SapViewCallbackInfo *pInfo)
{
	CZJURollerBearingSurfaceDetectionDlg *pDlg = (CZJURollerBearingSurfaceDetectionDlg *)pInfo->GetContext();

	if (pDlg->m_ImageWnd->IsRoiTrackerActive())
		pDlg->m_ImageWnd->DisplayRoiTracker();
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
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
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
		TRACE0("Failed   to   Create   Dialog   Toolbar/n");
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

		//m_Gio = new SapGio(m_Acq->GetLocation());
	}
	else
	{
		// Define off-line objects
		m_Buffers = new SapBuffer();
	}

	// Define other objects
	m_View = new SapView(m_Buffers, m_viewWnd.GetSafeHwnd(), ViewCallback, this);
	m_ProcessingImage = new SapImageProcessing(m_Buffers, ImageProcessedCallback, this);

	// Create all objects
	if (!CreateObjects()) { EndDialog(TRUE); return FALSE; }

	// Create an image window object
	m_ImageWnd = new CImageWnd(m_View, &m_viewWnd, &m_horizontalScr, &m_verticalScr, this);
	UpdateMenu();

	// Get current input signal connection status
	GetSignalStatus();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


BOOL CZJURollerBearingSurfaceDetectionDlg::CreateObjects()
{
	CWaitCursor wait;

	// Create acquisition object
	if (m_Acq && !*m_Acq && !m_Acq->Create())
	{
		DestroyObjects();
		return FALSE;
	}

	// Create buffer object
	if (m_Buffers && !*m_Buffers)
	{
		if (!m_Buffers->Create())
		{
			DestroyObjects();
			return FALSE;
		}
		// Clear all buffers
		m_Buffers->Clear();
	}


	// Create view object
	if (m_View && !*m_View && !m_View->Create())
	{
		DestroyObjects();
		return FALSE;
	}

	// Create transfer object
	if (m_Xfer && !*m_Xfer && !m_Xfer->Create())
	{
		DestroyObjects();
		return FALSE;
	}

	// Create processing object
	if (m_ProcessingImage && !*m_ProcessingImage)
	{
		if (!m_ProcessingImage->Create())
		{
			DestroyObjects();
			return FALSE;
		}

		m_ProcessingImage->SetAutoEmpty(TRUE);
	}
	return TRUE;
}

BOOL CZJURollerBearingSurfaceDetectionDlg::DestroyObjects()
{
	// Destroy transfer object
	if (m_Xfer && *m_Xfer) m_Xfer->Destroy();

	// Destroy view object
	if (m_View && *m_View) m_View->Destroy();

	// Destroy buffer object
	if (m_Buffers && *m_Buffers) m_Buffers->Destroy();

	// Destroy acquisition object
	if (m_Acq && *m_Acq) m_Acq->Destroy();


	// Destroy processing object
	if (m_ProcessingImage && *m_ProcessingImage) m_ProcessingImage->Destroy();

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
	if (m_ImageWnd)	delete m_ImageWnd;
	if (m_View)			delete m_View;
	if (m_Buffers)		delete m_Buffers;
	if (m_Acq)			delete m_Acq;
	if (m_ProcessingImage)			 delete m_ProcessingImage;
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

	if (m_ImageWnd) m_ImageWnd->OnSize();
	if (m_wndStatusBar.GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);
		rect.top = rect.bottom - 20;
		m_wndStatusBar.MoveWindow(&rect, TRUE);
	}
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

	//AfxGetApp()->m_pMainWnd ->GetToolb

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
	if (m_Xfer->Freeze())
	{
		if (CAbortDlg(this, m_Xfer).DoModal() != IDOK)
			m_Xfer->Abort();

		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnGrab()
{
	m_statusWnd.SetWindowText(_T(""));

	if (m_Xfer->Grab())
	{
		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnSnap()
{
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
	CAcqDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnAreaScanOptions()
{
	CAScanDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnLineScanOptions()
{
	CLScanDlg dlg(this, m_Acq);
	dlg.DoModal();
}

void CZJURollerBearingSurfaceDetectionDlg::OnCompositeOptions()
{
	if (m_Xfer->Snap())
	{
		CCompDlg dlg(this, m_Acq, m_Xfer);
		dlg.DoModal();

		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnLoadAcqConfig()
{
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


		m_ImageWnd->OnSize();
		InvalidateRect(NULL);
		UpdateWindow();
		UpdateMenu();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnViewOptions()
{
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

	m_Buffers->Clear();
	InvalidateRect(NULL, FALSE);
}

void CZJURollerBearingSurfaceDetectionDlg::OnFileLoad()
{
	CLoadSaveDlg dlg(this, m_Buffers, TRUE);
	if (dlg.DoModal() == IDOK)
	{
		InvalidateRect(NULL);
		UpdateWindow();
	}
}

void CZJURollerBearingSurfaceDetectionDlg::OnFileSave()
{
	CLoadSaveDlg dlg(this, m_Buffers, FALSE);
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
	// TODO:  在此添加命令处理程序代码
	CBearingRollersDlg BRDlg;
	if (IDOK == BRDlg.DoModal())
	{
		SapFormat;
		m_lLine = BRDlg.m_lLine;
		_tcscpy(m_szSavePath, BRDlg.m_szSavePath);
		m_bSaveImageEnable = BRDlg.m_bSaveImageEnable;
		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lLine, m_Buffers->GetFormat(), m_Buffers->GetType(), m_Buffers->GetLocation());
		/*m_Buffers->CopyAll(m_ProcessBuffers);
		m_ProcessBuffers->SetHeight(m_lLine);*/

		//m_ProcessBuffers = new SapBuffer(2, m_Buffers->GetWidth(), m_lLine, m_Buffers->GetFormat(),m_Buffers->GetType(),m_Buffers->GetLocation());

	}

}


void CZJURollerBearingSurfaceDetectionDlg::OnSerialportsettings()
{
	CSerialPortDlg SPDlg;
	if (IDOK == SPDlg.DoModal())
	{
		m_intPort = SPDlg.GetPort();
		m_intBaudRate = SPDlg.GetBaudRate();
		m_intDataBits = SPDlg.GetDateBits();
		m_cParity = SPDlg.GetParity();
		m_intStopBits = SPDlg.GetStopBits();
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
				}
				else
				{
					AfxMessageBox(_T("Serial Port  initialize Falied!"));
					m_bSerialPortOpen = FALSE;
					CString strtemp;
					strtemp.Format(_T("Serial Port  initialize Falied"), m_intPort);
					m_wndStatusBar.SetPaneText(STATUSBAR_COMM, strtemp, TRUE);
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
			}
		}
	}
}


LRESULT CZJURollerBearingSurfaceDetectionDlg::OnCommunication(WPARAM ch, LPARAM port)
{
	static int SerialPortCount = 0;

	m_pkg[SerialPortCount++] = ch;
	if (SerialPortCount == PKG_SIZE) {
		SerialPortCount = 0;
		m_pkg[PKG_SIZE] = '\0';
	}
	if (m_pkg[2] == 23 && !SerialPortCount)
		AfxMessageBox(_T("Hello "));

	m_lRXCount++;
	CString str;
	str.Format(_T("RX:%d"), m_lRXCount);
	m_wndStatusBar.SetPaneText(STATUSBAR_RX, str, TRUE);
	return 0;
}

int CZJURollerBearingSurfaceDetectionDlg::DoStr2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	for (int i = 0; i<len;)
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
	for (int i = 0; i<size; i++)
		myv += *buffer++;
	return myv;
}

void CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest()
{
	// TODO: 在此添加命令处理程序代码
	BYTE kk[9] = { 12 ,22 ,23 ,67,1,11,22,45,89 };
	m_serialPort.WriteToPort(kk, 9);
	//m_serialPort.WriteToPort("Hell0",5);
}


BOOL CZJURollerBearingSurfaceDetectionDlg::WChar2MByte(LPCWSTR lpSrc, LPSTR lpDest, int nlen)
{
	//	int n = 0;

	//	n = WideCharToMultiByte(CP_OEMCP, 0, lpSrc, -1, lpDest, 0, 0, FALSE);
	//	if (n < nlen) return FALSE;
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