#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "sqlite\SQLite.h"

enum SetRefreshStates {
	SelectAllToRefresh,
	SelectSpecialToRefresh
};
// CBearingRollersParameterDlg 对话框

class CBearingRollersParameterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBearingRollersParameterDlg)

public:
	CBearingRollersParameterDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBearingRollersParameterDlg();

	// 对话框数据
	enum { IDD = IDD_PARAMETERS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	CWinApp *m_App;
	void LoadSettings();
	void SaveSettings();

private:
	sqlite3* pDB = NULL;
	void CalculateLine(double Radius, double Length, double Taper);

public:
	virtual BOOL OnInitDialog();
	void RefreshListView(SetRefreshStates);
	double m_dRadius;
	double m_dLength;
	double m_dTaper;
	long m_lLine;
	long m_lCIDIPix;
	int m_nImageProcessingPrecision;
	// 图像保存路径
	CString m_szSavePath;
	BOOL m_bSaveImageEnable;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCalculate();
	long GetHeigthLine();
	long GetWeithLine();
	int GetImageProcessingPrecision();
	int GetImagePreprocessingThreshold();
	CButton m_CtrSaveImage;

	CString GetImageSavePath();
	CSliderCtrl m_CtrDIPP;
	double m_FreDivFtr;
	CSliderCtrl m_CtrImagePreprocessingThreshold;
	int m_nImagePreprocessingThreshold;
	afx_msg void OnNMCustomdrawDippt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTRBNThumbPosChangingDippt(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_CtrIPTValue;
	afx_msg void OnNMCustomdrawDipp(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_wndList;
	SQLite sqlite;
	TCHAR sql[512] = { 0 };
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMClickDatabaselist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkDatabaselist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDtdb();
	afx_msg void OnCbnEditchangeDatabasecombo();
};
