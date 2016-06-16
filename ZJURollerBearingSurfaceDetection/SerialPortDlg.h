#pragma once
#include "afxwin.h"


// CSerialPortDlg �Ի���

class CSerialPortDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSerialPortDlg)

public:
	CSerialPortDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSerialPortDlg();

// �Ի�������
	enum { IDD = IDD_CSERIALPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	CWinApp *m_App;
	void LoadSettings();
	void SaveSettings();

public:
	CString m_intPort;
	CString m_intBaudRate;
	CString m_intDataBits;
	CString m_cParity;
	CString m_intStopBits;
	virtual BOOL OnInitDialog();
	CComboBox m_CtrPort;
	CComboBox m_CtrBaudRate;
	CComboBox m_CtrDataBits;
	CComboBox m_CtrParity;
	CComboBox m_CtrStopBits;
	CButton m_EnableSerialPortCommunication;
	UINT GetPort();
	UINT GetBaudRate();
	UINT GetDateBits();
	char GetParity();
	UINT GetStopBits();
	BOOL IsEnableSerialPortCommunication();
	
	CString GetUSBVID();
	CString GetUSBPID();
	BOOL IsEnableUSBCommunication();
	afx_msg void OnBnClickedOk();
	BOOL m_SerialPortEnable;
	CButton m_CtrEnableUSBCommunication;
	CString m_szUSBVID;
	CString m_szUSBPID;
	BOOL m_bUSBEnable;
};
