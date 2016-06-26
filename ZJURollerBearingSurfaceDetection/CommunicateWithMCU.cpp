#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "CommonMethod.h"
#include "BearingRollersParameterDlg.h"
#include "CommunicationConfigurationDlg.h"
#include "LibUSBWin32\include\lusb0_usb.h"

#pragma region USB Parameters Settings
const int MY_CONFIG = 1;
const int MY_INTF = 0;
// Device endpoint(s)
const int EP_IN = 0x81;
const int EP_OUT = 0x02;
usb_dev_handle *gusb_handle;
HANDLE m_hDevice = INVALID_HANDLE_VALUE;
CWinThread *m_pReadReportThread = NULL;
OVERLAPPED ReadOverlapped;
char ReadReportBuffer[16] = { 0 };
BYTE KeyStatus = 0;
volatile BOOL m_bReadReportRunFlag = FALSE;
#pragma endregion

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

			CCommonMethod::DoStr2Hex(m_szUsbVID, VIDdata);
			CCommonMethod::DoStr2Hex(m_szUsbPID, PIDdata);

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
		for (int i = 0; i < ret; ++i)
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

	++m_lRXCount;

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
		case 1:  m_nXferCallbackCount = 0; break;
		case 2:                   break;  //其余定义功能
		default:
			break;
		}

		std::vector<int> vecNum;
		for (int i = 0; i < SerialPortCount; ++i)
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

void CZJURollerBearingSurfaceDetectionDlg::OnBearingrollerTest()
{

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