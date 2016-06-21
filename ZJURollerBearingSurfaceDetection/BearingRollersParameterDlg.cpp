// BearingRollersDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "BearingRollersParameterDlg.h"
#include "afxdialogex.h"
#include "SapClassGui.h"
#include "sqlite\sqlite3.h"

// CBearingRollersParameterDlg �Ի���

IMPLEMENT_DYNAMIC(CBearingRollersParameterDlg, CDialogEx)

CBearingRollersParameterDlg::CBearingRollersParameterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBearingRollersParameterDlg::IDD, pParent)
	, m_dRadius(0)
	, m_dLength(0)
	, m_dTaper(0)
	, m_lLine(0)
	, m_szSavePath(_T(""))
	, m_bSaveImageEnable(FALSE)
	, m_lCIDIPix(0)
	, m_nImageProcessingPrecision(0)
	, m_FreDivFtr(0)
	, m_nImagePreprocessingThreshold(0)
	, m_CtrIPTValue(_T(""))
{

}

CBearingRollersParameterDlg::~CBearingRollersParameterDlg()
{
}

void CBearingRollersParameterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RADIUS, m_dRadius);
	DDV_MinMaxDouble(pDX, m_dRadius, 0.00, 50);
	DDX_Text(pDX, IDC_LENGTH, m_dLength);
	DDV_MinMaxDouble(pDX, m_dLength, 0.00, 50.00);
	DDX_Text(pDX, IDC_TAPER, m_dTaper);
	DDV_MinMaxDouble(pDX, m_dTaper, -30.00, +30.00);
	DDX_Text(pDX, IDC_LINE, m_lLine);
	DDX_Text(pDX, IDC_SAVEPATH, m_szSavePath);
	DDX_Check(pDX, IDC_SAVEIMAGEENABLE, m_bSaveImageEnable);
	DDX_Control(pDX, IDC_SAVEIMAGEENABLE, m_CtrSaveImage);
	DDX_Control(pDX, IDC_DIPP, m_CtrDIPP);
	DDX_Text(pDX, IDC_CIDILINE, m_lCIDIPix);
	DDX_Slider(pDX, IDC_DIPP, m_nImageProcessingPrecision);
	DDX_Text(pDX, IDC_TAPER2, m_FreDivFtr);
	DDX_Control(pDX, IDC_DIPPT, m_CtrImagePreprocessingThreshold);
	DDX_Slider(pDX, IDC_DIPPT, m_nImagePreprocessingThreshold);
	DDX_Text(pDX, IDC_IPT, m_CtrIPTValue);
	DDX_Control(pDX, IDC_DATABASELIST, m_wndList);
}


BEGIN_MESSAGE_MAP(CBearingRollersParameterDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBearingRollersParameterDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CALCULATE, &CBearingRollersParameterDlg::OnBnClickedCalculate)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DIPPT, &CBearingRollersParameterDlg::OnNMCustomdrawDippt)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_DIPPT, &CBearingRollersParameterDlg::OnTRBNThumbPosChangingDippt)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DIPP, &CBearingRollersParameterDlg::OnNMCustomdrawDipp)
	ON_BN_CLICKED(IDCANCEL, &CBearingRollersParameterDlg::OnBnClickedCancel)
	ON_NOTIFY(NM_CLICK, IDC_DATABASELIST, &CBearingRollersParameterDlg::OnNMClickDatabaselist)
	ON_NOTIFY(NM_DBLCLK, IDC_DATABASELIST, &CBearingRollersParameterDlg::OnNMDblclkDatabaselist)
	ON_BN_CLICKED(IDC_DTDB, &CBearingRollersParameterDlg::OnBnClickedDtdb)
	ON_CBN_EDITCHANGE(IDC_DATABASECOMBO, &CBearingRollersParameterDlg::OnCbnEditchangeDatabasecombo)
END_MESSAGE_MAP()


// CBearingRollersParameterDlg ��Ϣ�������


BOOL CBearingRollersParameterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	LOG(TRACE) << " Bearing Roller Parameters Setup";
	m_App = AfxGetApp();
	m_CtrImagePreprocessingThreshold.SetRange(0, 50);
	m_CtrDIPP.SetRange(0, 255);
	m_CtrDIPP.SetPos(100);
	LoadSettings();
	CalculateLine(m_dRadius, m_dLength, m_dTaper);


	GetDlgItem(IDC_DATABASECOMBO)->SetWindowTextW(_T("Search Radius Select Bearing Roller Type"));
	CRect rect;
	// ��ȡ��������б���ͼ�ؼ���λ�úʹ�С   
	m_wndList.GetClientRect(&rect);
	m_wndList.SetExtendedStyle(m_wndList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	//���·���а������ģ���Ҫ���б���ת��
	m_wndList.DeleteAllItems();//���  
	m_wndList.InsertColumn(0, _T("Radius"), LVCFMT_CENTER, rect.Width() / 3, 0);//�����
	m_wndList.InsertColumn(1, _T("Length"), LVCFMT_CENTER, rect.Width() / 3, 1);
	m_wndList.InsertColumn(1, _T("Taper"), LVCFMT_CENTER, rect.Width() / 3, 2);

	LOG(TRACE) << " Load DataBase sqlite/BearingRollerType.db...";
	TCHAR *szDbPath = _T("sqlite/BearingRollerType.db");

	if (!m_sqlite.Open(szDbPath))
	{
		_tprintf(_T("%s\n"), m_sqlite.GetLastErrorMsg());
		LOG(TRACE) << " Load DataBase Failed...";
		return FALSE;
	}

	RefreshListView(SetRefreshStates::SelectAllToRefresh);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣:  OCX ����ҳӦ���� FALSE
}


void CBearingRollersParameterDlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	// �ر����ݿ�  
	m_sqlite.Close();

	UpdateData(TRUE);
	SaveSettings();
	LoadSettings();
	UpdateData(FALSE);
	LOG(TRACE) << " Database closed! Parameters Save/load...";
	LOG(TRACE)
		<< "\n					Bearing Roller Radius:" << m_dRadius
		<< "\n					Bearing Roller Length:" << m_dLength
		<< "\n					Bearing Roller Taper:" << m_dTaper
		<< "\n					Bearing Roller FreDivFtr:" << m_FreDivFtr
		<< "\n					Bearing Roller Line:" << m_lLine
		<< "\n					Bearing Roller CIDIPix:" << m_lCIDIPix
		<< "\n					Bearing Roller SaveImageEnable:" << m_bSaveImageEnable
		<< "\n					Bearing Roller SavePath:" << CStringA(m_szSavePath)
		<< "\n					Bearing Roller ImageProcessingPrecision:" << m_nImageProcessingPrecision
		<< "\n					Bearing Roller ImagePreprocessingThreshold:" << m_nImagePreprocessingThreshold;
	CDialogEx::OnOK();
}


void CBearingRollersParameterDlg::OnBnClickedCalculate()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	CalculateLine(m_dRadius, m_dLength, m_dTaper);
	UpdateData(FALSE);

}

void CBearingRollersParameterDlg::CalculateLine(double Radius, double Length, double Taper)
{
	m_lLine = 4800 * 3.5 * 2 * Radius / 67 / m_FreDivFtr;
	m_lCIDIPix = Length / 20 * 2495;
	//m_lLine = Radius + Length + Taper;
	SaveSettings();
}


void CBearingRollersParameterDlg::LoadSettings()
{
	m_dRadius = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_dRadius"), _T("8.5")));
	m_dLength = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_dLength"), _T("30.0")));
	m_dTaper = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_dTaper"), _T("3.0")));
	m_FreDivFtr = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_FreDivFtr"), _T("1.0")));
	m_lLine = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_lLine"), _T("650")));
	m_lCIDIPix = _wtof(m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_lCIDIPix"), _T("2495")));
	m_bSaveImageEnable = m_App->GetProfileIntW(_T("BearingRollersDlg"), _T("m_bSaveImageEnable"), TRUE);
	m_szSavePath = m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_szSavePath"), _T("D://"));
	m_nImageProcessingPrecision = m_App->GetProfileIntW(_T("BearingRollersDlg"), _T("m_nImageProcessingPrecision"), 100);
	m_nImagePreprocessingThreshold = m_App->GetProfileIntW(_T("BearingRollersDlg"), _T("m_nImagePreprocessingThreshold"), 15);
	//StringCbCopyW(m_szSavePath, m_App->GetProfileStringW(_T("BearingRollersDlg"), _T("m_szSavePath"), _T("D:/")), sizeof(m_szSavePath));
}

void CBearingRollersParameterDlg::SaveSettings()
{
	CString szTemp;
	szTemp.Format(_T("%ld"), m_lLine);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_lLine"), szTemp);
	szTemp.Format(_T("%f"), m_dRadius);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_dRadius"), szTemp);
	szTemp.Format(_T("%f"), m_dLength);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_dLength"), szTemp);
	szTemp.Format(_T("%f"), m_dTaper);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_dTaper"), szTemp);
	GetDlgItem(IDC_SAVEPATH)->GetWindowTextW(szTemp);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_szSavePath"), szTemp);
	szTemp.Format(_T("%ld"), m_lCIDIPix);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_lCIDIPix"), szTemp);
	szTemp.Format(_T("%f"), m_FreDivFtr);
	m_App->WriteProfileStringW(_T("BearingRollersDlg"), _T("m_FreDivFtr"), szTemp);
	m_App->WriteProfileInt(_T("BearingRollersDlg"), _T("m_bSaveImageEnable"), m_CtrSaveImage.GetCheck());
	m_App->WriteProfileInt(_T("BearingRollersDlg"), _T("m_nImageProcessingPrecision"), m_CtrDIPP.GetPos());
	m_App->WriteProfileInt(_T("BearingRollersDlg"), _T("m_nImagePreprocessingThreshold"), m_CtrImagePreprocessingThreshold.GetPos());

}


CString CBearingRollersParameterDlg::GetImageSavePath()
{
	return m_szSavePath;
}

long CBearingRollersParameterDlg::GetHeigthLine()
{
	return m_lLine;
}

long CBearingRollersParameterDlg::GetWeithLine()
{
	return m_lCIDIPix;
}

int CBearingRollersParameterDlg::GetImageProcessingPrecision()
{
	return m_nImageProcessingPrecision;
}

int CBearingRollersParameterDlg::GetImagePreprocessingThreshold()
{
	return m_nImagePreprocessingThreshold;
}

void CBearingRollersParameterDlg::OnNMCustomdrawDippt(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	CString str;
	str.Format(_T("%d"), m_CtrImagePreprocessingThreshold.GetPos());
	GetDlgItem(IDC_IPT)->SetWindowTextW(str);
}


void CBearingRollersParameterDlg::OnTRBNThumbPosChangingDippt(NMHDR *pNMHDR, LRESULT *pResult)
{
	// �˹���Ҫ�� Windows Vista ����߰汾��
	// _WIN32_WINNT ���ű��� >= 0x0600��
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}


void CBearingRollersParameterDlg::OnNMCustomdrawDipp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
	CString str;
	str.Format(_T("%d"), m_CtrDIPP.GetPos());
	GetDlgItem(IDC_IPP)->SetWindowTextW(str);
}


void CBearingRollersParameterDlg::RefreshListView(SetRefreshStates state)
{
	m_wndList.DeleteAllItems();//���   
	memset(sqlCommad, 0, sizeof(sqlCommad));
	if (state == SetRefreshStates::SelectAllToRefresh)
	{
		_stprintf(sqlCommad, _T("%s"), _T("select * from BearingRoller"));
	}
	else
	{
		CString str;
		GetDlgItem(IDC_COMBO1)->GetWindowTextW(str);
		_stprintf(sqlCommad, _T("select * from BearingRoller where Radius like '%%%s%%'"), str);
	}
	LOG(TRACE) << sqlCommad;
	SQLiteDataReader Reader = m_sqlite.ExcuteQuery(sqlCommad);

	int index = 0;
	int len = 0;
	while (Reader.Read())
	{

		m_wndList.InsertItem(index, Reader.GetStringValue(0));
		m_wndList.SetItemText(index, 1, Reader.GetStringValue(1));
		m_wndList.SetItemText(index, 2, Reader.GetStringValue(2));
		index++;
		//// ��ȡͼƬ�������ļ�  
		//const BYTE *ImageBuf = Reader.GetBlobValue(6, len);
	}
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	Reader.Close();
	//******************************************************  

}


void CBearingRollersParameterDlg::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// �ر����ݿ�  
	m_sqlite.Close();
	CDialogEx::OnCancel();
}


void CBearingRollersParameterDlg::OnNMClickDatabaselist(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nItem = -1;
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (pNMItemActivate != NULL)
	{
		nItem = pNMItemActivate->iItem;
	}
	CString str;
	str = m_wndList.GetItemText(nItem, 0);
	GetDlgItem(IDC_RADIUS)->SetWindowTextW(str);
	str = m_wndList.GetItemText(nItem, 1);
	GetDlgItem(IDC_LENGTH)->SetWindowTextW(str);
	str = m_wndList.GetItemText(nItem, 2);
	GetDlgItem(IDC_TAPER)->SetWindowTextW(str);
	UpdateData(TRUE);
	CalculateLine(m_dRadius, m_dLength, m_dTaper);
	UpdateData(FALSE);
	*pResult = 0;
}


void CBearingRollersParameterDlg::OnNMDblclkDatabaselist(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nItem = -1;
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (pNMItemActivate != NULL)
	{
		nItem = pNMItemActivate->iItem;
	}
	m_sqlite.BeginTransaction();
	memset(sqlCommad, 0, sizeof(sqlCommad));
	CString s1 = m_wndList.GetItemText(nItem, 0);
	CString s2 = m_wndList.GetItemText(nItem, 1);
	CString s3 = m_wndList.GetItemText(nItem, 2);
	_stprintf(sqlCommad, _T("delete from BearingRoller where Radius='%s' and Length='%s' and Taper='%s'"), s1, s2, s3);
	LOG(TRACE) << sqlCommad;
	SQLiteCommand cmd(&m_sqlite, sqlCommad);
	//cmd.BindParam(1, s);
	if (!m_sqlite.ExcuteNonQuery(sqlCommad))
	{
		_tprintf(_T("%s\n"), m_sqlite.GetLastErrorMsg());
		return;
	}
	// ���cmd  
	cmd.Clear();
	// �ύ����  
	m_sqlite.CommitTransaction();
	m_wndList.DeleteItem(nItem);
	*pResult = 0;
}


void CBearingRollersParameterDlg::OnBnClickedDtdb()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	// ��һ���Բ��������¼ʱ�򣬲�������ķ�ʽ�����Ч��  
	m_sqlite.BeginTransaction();
	memset(sqlCommad, 0, sizeof(sqlCommad));
	_stprintf(sqlCommad, _T("insert into BearingRoller(Radius,Length,Taper) values(?,?,?)"));
	LOG(TRACE) << sqlCommad;
	SQLiteCommand cmd(&m_sqlite, sqlCommad);
	CString str;
	GetDlgItem(IDC_RADIUS)->GetWindowTextW(str);
	cmd.BindParam(1, str);
	GetDlgItem(IDC_LENGTH)->GetWindowTextW(str);
	cmd.BindParam(2, str);
	GetDlgItem(IDC_TAPER)->GetWindowTextW(str);
	cmd.BindParam(3, str);
	if (!m_sqlite.ExcuteNonQuery(&cmd))
	{
		_tprintf(_T("%s\n"), m_sqlite.GetLastErrorMsg());
		LOG(TRACE) << CStringA(m_sqlite.GetLastErrorMsg());
		return;
	}

	// ������������  
	//for (int i = 0; i<5; i++)
	//{
	//	TCHAR strValue[16] = { 0 };
	//	_stprintf(strValue, _T("%d"), i);
	//	// �󶨵�һ��������name�ֶ�ֵ��  
	//	cmd.BindParam(1, strValue);
	//	// �󶨵ڶ���������catagory_id�ֶ�ֵ��  
	//	cmd.BindParam(2, 20);
	//	//BYTE imageBuf[] = { 0xff,0xff,0xff,0xff };
	//	//// �󶨵�����������image�ֶ�ֵ,���������ݣ�  
	//	//cmd.BindParam(3, imageBuf, sizeof(imageBuf));
	//	if (!sqlite.ExcuteNonQuery(&cmd))
	//	{
	//		_tprintf(_T("%s\n"), sqlite.GetLastErrorMsg());
	//		break;
	//	}
	//}
	// ���cmd  
	cmd.Clear();
	// �ύ����  
	m_sqlite.CommitTransaction();
	RefreshListView(SetRefreshStates::SelectAllToRefresh);
}


void CBearingRollersParameterDlg::OnCbnEditchangeDatabasecombo()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	RefreshListView(SetRefreshStates::SelectSpecialToRefresh);
}
