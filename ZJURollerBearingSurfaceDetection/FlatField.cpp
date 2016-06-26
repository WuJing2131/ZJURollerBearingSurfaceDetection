#include "stdafx.h"
#include "ZJURollerBearingSurfaceDetection.h"
#include "ZJURollerBearingSurfaceDetectionDlg.h"
#include "afxdialogex.h"
#include "CommonMethod.h"

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
