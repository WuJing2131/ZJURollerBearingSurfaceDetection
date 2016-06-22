//#pragma once
#ifndef BEARINGROLLERSPARAMETERDLG_H
#define BEARINGROLLERSPARAMETERDLG_H
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
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCalculate();
	afx_msg void OnNMCustomdrawDippt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTRBNThumbPosChangingDippt(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawDipp(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMClickDatabaselist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkDatabaselist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDtdb();
	afx_msg void OnCbnEditchangeDatabasecombo();
	DECLARE_MESSAGE_MAP()

private:
	CWinApp *m_App;
	sqlite3* pDB = NULL;

public:
	virtual BOOL OnInitDialog();
	void CalculateLine(double Radius, double Length, double Taper);
	void RefreshListView(SetRefreshStates);
	long GetHeigthLine();
	long GetWeithLine();
	int  GetImageProcessingPrecision();
	int  GetImagePreprocessingThreshold();
	void LoadSettings();
	void SaveSettings();
	CString GetImageSavePath();

	double      m_dRadius;
	double      m_dLength;
	double      m_dTaper;
	long        m_lLine;
	long        m_lCIDIPix;
	int         m_nImageProcessingPrecision;
	// 图像保存路径
	CString     m_szSavePath;
	BOOL        m_bSaveImageEnable;
	CButton     m_CtrSaveImage;
	CSliderCtrl m_CtrDIPP;
	double      m_FreDivFtr;
	CSliderCtrl m_CtrImagePreprocessingThreshold;
	int         m_nImagePreprocessingThreshold;
	CString     m_CtrIPTValue;
	CListCtrl   m_wndList;
	SQLite      m_sqlite;
	TCHAR       sqlCommad[512] = { 0 };
	
};

#endif // !BEARINGROLLERSPARAMETERDLG_H
