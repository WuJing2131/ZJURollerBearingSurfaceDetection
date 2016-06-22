//#pragma once
#ifndef COMMUNICATIONCONFIGURATIONDLG_H
#define COMMUNICATIONCONFIGURATIONDLG_H
#include "afxwin.h"


// CCommunicationConfigurationDlg 对话框

class CCommunicationConfigurationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCommunicationConfigurationDlg)

public:
	explicit CCommunicationConfigurationDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCommunicationConfigurationDlg();

	// 对话框数据
	enum { IDD = IDD_CSERIALPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()

	CWinApp *m_App;
	

public:
	virtual BOOL OnInitDialog();
	UINT GetPort();
	UINT GetBaudRate();
	UINT GetDateBits();
	char GetParity();
	UINT GetStopBits();
	CString GetUSBVID();
	CString GetUSBPID();
	BOOL IsEnableUSBCommunication();
	BOOL IsEnableSerialPortCommunication();

	void LoadSettings();
	void SaveSettings();

	CString   m_intPort;
	CString   m_intBaudRate;
	CString   m_intDataBits;
	CString   m_cParity;
	CString   m_intStopBits;
	CComboBox m_CtrPort;
	CComboBox m_CtrBaudRate;
	CComboBox m_CtrDataBits;
	CComboBox m_CtrParity;
	CComboBox m_CtrStopBits;
	CButton   m_EnableSerialPortCommunication;
	CButton   m_CtrEnableUSBCommunication;
	CString   m_szUSBVID;
	CString   m_szUSBPID;
	BOOL      m_SerialPortEnable;
	BOOL      m_bUSBEnable;
};

#endif