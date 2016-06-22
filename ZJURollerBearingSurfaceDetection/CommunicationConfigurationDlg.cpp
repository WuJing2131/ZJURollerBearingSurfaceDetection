// SerialPortDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "CommunicationConfigurationDlg.h"
#include "afxdialogex.h"

// CCommunicationConfigurationDlg 对话框

IMPLEMENT_DYNAMIC(CCommunicationConfigurationDlg, CDialogEx)

CCommunicationConfigurationDlg::CCommunicationConfigurationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCommunicationConfigurationDlg::IDD, pParent)
	, m_intPort(_T(""))
	, m_intBaudRate(_T(""))
	, m_intDataBits(_T(""))
	, m_cParity(_T(""))
	, m_intStopBits(_T(""))
	, m_SerialPortEnable(TRUE)
	, m_szUSBVID(_T(""))
	, m_szUSBPID(_T(""))
	, m_bUSBEnable(FALSE)
{

}

CCommunicationConfigurationDlg::~CCommunicationConfigurationDlg()
{
}

void CCommunicationConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_PORT, m_intPort);
	DDX_CBString(pDX, IDC_BAUDBATE, m_intBaudRate);
	DDX_CBString(pDX, IDC_DATABITS, m_intDataBits);
	DDX_CBString(pDX, IDC_PARITY, m_cParity);
	DDX_CBString(pDX, IDC_STOPBITS, m_intStopBits);
	DDX_Control (pDX, IDC_PORT, m_CtrPort);
	DDX_Control (pDX, IDC_BAUDBATE, m_CtrBaudRate);
	DDX_Control (pDX, IDC_DATABITS, m_CtrDataBits);
	DDX_Control (pDX, IDC_PARITY, m_CtrParity);
	DDX_Control (pDX, IDC_STOPBITS, m_CtrStopBits);
	DDX_Control (pDX, IDC_ENABLESERIALPORTCOMMUNICATION, m_EnableSerialPortCommunication);
	DDX_Check   (pDX, IDC_ENABLESERIALPORTCOMMUNICATION, m_SerialPortEnable);
	DDX_Text    (pDX, IDC_USBVID, m_szUSBVID);
	DDX_Text    (pDX, IDC_USBPID, m_szUSBPID);
}


BEGIN_MESSAGE_MAP(CCommunicationConfigurationDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CCommunicationConfigurationDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCommunicationConfigurationDlg 消息处理程序


BOOL CCommunicationConfigurationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_App = AfxGetApp();
	m_CtrPort.AddString(_T("COM1"));
	m_CtrPort.AddString(_T("COM2"));
	m_CtrPort.AddString(_T("COM3"));
	m_CtrPort.AddString(_T("COM4"));
	m_CtrPort.AddString(_T("COM5"));
	m_CtrPort.AddString(_T("COM6"));
	m_CtrPort.AddString(_T("COM7"));
	m_CtrPort.AddString(_T("COM8"));
	m_CtrPort.AddString(_T("COM9"));
	m_CtrPort.AddString(_T("COM10"));
	m_CtrPort.AddString(_T("COM11"));
	m_CtrPort.AddString(_T("COM12"));
	//m_CtrPort.SetCurSel(4);
	m_CtrBaudRate.AddString(_T("2400"));
	m_CtrBaudRate.AddString(_T("4800"));
	m_CtrBaudRate.AddString(_T("9600"));
	m_CtrBaudRate.AddString(_T("2400"));
	m_CtrBaudRate.AddString(_T("14400"));
	m_CtrBaudRate.AddString(_T("19200"));
	m_CtrBaudRate.AddString(_T("38400"));
	m_CtrBaudRate.AddString(_T("56000"));
	m_CtrBaudRate.AddString(_T("57600"));
	m_CtrBaudRate.AddString(_T("115200"));
	//m_CtrPort.SetCurSel(5);
	m_CtrDataBits.AddString(_T("8"));
	m_CtrDataBits.AddString(_T("7"));
	m_CtrDataBits.AddString(_T("6"));
	m_CtrDataBits.AddString(_T("5"));
	//m_CtrDataBits.SetCurSel(m_CtrDataBits.SelectString(0, _T("8")));
	m_CtrParity.AddString(_T("NO"));
	m_CtrParity.AddString(_T("ODD"));
	m_CtrParity.AddString(_T("EVEN"));
	m_CtrParity.AddString(_T("MASK"));
	m_CtrParity.AddString(_T("SPACE"));
	m_CtrStopBits.AddString(_T("0"));
	m_CtrStopBits.AddString(_T("1"));
	m_CtrStopBits.AddString(_T("1.5"));
	m_CtrStopBits.AddString(_T("2"));
	m_EnableSerialPortCommunication.SetCheck(TRUE);
	LoadSettings();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CCommunicationConfigurationDlg::LoadSettings()
{
	m_intPort     = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intPort"), _T("COM5"));
	m_intBaudRate = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intBaudRate"), _T("115200"));
	m_intDataBits = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intDataBits"), _T("8"));
	m_cParity     = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_cParity"), _T("NO"));
	m_intStopBits = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intStopBits"), _T("1"));
	m_szUSBVID    = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_szUSBVID"), _T("C255"));
	m_szUSBPID    = m_App->GetProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_szUSBPID"), _T("4355"));
	//m_bUSBEnable = m_App->GetProfileIntW(_T("CCommunicationConfigurationDlg"), _T("m_bUSBEnable"), FALSE);
}
void CCommunicationConfigurationDlg::SaveSettings()
{
	CString szTemp;
	GetDlgItemText(IDC_PORT, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intPort"), szTemp);
	GetDlgItemText(IDC_BAUDBATE, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intBaudRate"), szTemp);
	GetDlgItemText(IDC_DATABITS, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intDataBits"), szTemp);
	GetDlgItemText(IDC_PARITY, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_cParity"), szTemp);
	GetDlgItemText(IDC_STOPBITS, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_intStopBits"), szTemp);
	GetDlgItemText(IDC_USBVID, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_szUSBVID"), szTemp);
	GetDlgItemText(IDC_USBPID, szTemp);
	m_App->WriteProfileStringW(_T("CCommunicationConfigurationDlg"), _T("m_szUSBPID"), szTemp);

	//m_App->WriteProfileInt(_T("CCommunicationConfigurationDlg"), _T("m_bUSBEnable"), m_CtrEnableUSBCommunication.GetCheck());
}

UINT CCommunicationConfigurationDlg::GetPort()
{
	m_intPort.TrimLeft(_T("COM"));
	return _wtoi(m_intPort);
}
UINT CCommunicationConfigurationDlg::GetBaudRate()
{
	return _wtoi(m_intBaudRate);
}
UINT CCommunicationConfigurationDlg::GetDateBits()
{
	return _wtoi(m_intDataBits);
}
char CCommunicationConfigurationDlg::GetParity()
{
	return m_cParity[0];
}
UINT CCommunicationConfigurationDlg::GetStopBits()
{
	if (m_intStopBits == _T("1"))
		return 1;
	else if (m_intStopBits == _T("1.5"))
		return 1;
	else
		return 1;
}

BOOL CCommunicationConfigurationDlg::IsEnableSerialPortCommunication()
{
	return m_SerialPortEnable;
}

CString CCommunicationConfigurationDlg::GetUSBVID()
{
	return m_szUSBVID;
}
CString CCommunicationConfigurationDlg::GetUSBPID()
{
	return m_szUSBPID;
}
BOOL CCommunicationConfigurationDlg::IsEnableUSBCommunication()
{
	return m_bUSBEnable;
}

void CCommunicationConfigurationDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	SaveSettings();
	LoadSettings();
	UpdateData(FALSE);
	CDialogEx::OnOK();
}
