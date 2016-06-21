
// ZJURollerBearingSurfaceDetection.cpp : ����Ӧ�ó��������Ϊ��
//github

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"


INITIALIZE_EASYLOGGINGPP

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CZJURollerBearingSurfaceDetectionApp

BEGIN_MESSAGE_MAP(CZJURollerBearingSurfaceDetectionApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CZJURollerBearingSurfaceDetectionApp ����

CZJURollerBearingSurfaceDetectionApp::CZJURollerBearingSurfaceDetectionApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CZJURollerBearingSurfaceDetectionApp ����

CZJURollerBearingSurfaceDetectionApp theApp;


// CZJURollerBearingSurfaceDetectionApp ��ʼ��

BOOL CZJURollerBearingSurfaceDetectionApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));


	/*/////////////////////////////////////////////////////////////////////////
	�������ñ�� LoggingFlag::StrictLogFileSizeCheck ����,�����ļ���MAX_LOG_FILE_SIZE = 1048576����Ч
	/////////////////////////////////////////////////////////////////////////*/
	el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
	el::Configurations conf("logConfig.conf");
	el::Loggers::reconfigureAllLoggers(conf);

	/// ���ӱ�� NewLineForContainer��ע��鿴���STL������Ч��  
	el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
	/*LOG(TRACE) << "***** trace log  *****";
	LOG(DEBUG) << "***** debug log  *****";
	LOG(ERROR) << "***** error log  *****";
	LOG(WARNING) << "***** warning log  *****";
	LOG(INFO) << "***** info log  *****";*/

	CZJURollerBearingSurfaceDetectionDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		LOG(WARNING) << L"����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n";
		LOG(WARNING) << L"����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n";
		//TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		//TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

