
// ZJURollerBearingSurfaceDetection.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

#define ELPP_STL_LOGGING  
#define ELPP_UNICODE     //ʹ��־��֧��unicode ����
#include "easylogging++.h"

// CZJURollerBearingSurfaceDetectionApp: 
// �йش����ʵ�֣������ ZJURollerBearingSurfaceDetection.cpp
//


class CZJURollerBearingSurfaceDetectionApp : public CWinApp
{
public:
	CZJURollerBearingSurfaceDetectionApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CZJURollerBearingSurfaceDetectionApp theApp;