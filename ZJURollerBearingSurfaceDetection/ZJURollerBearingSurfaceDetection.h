
// ZJURollerBearingSurfaceDetection.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

#define ELPP_STL_LOGGING  
#define ELPP_UNICODE     //使日志库支持unicode 编码
#include "easylogging++.h"

// CZJURollerBearingSurfaceDetectionApp: 
// 有关此类的实现，请参阅 ZJURollerBearingSurfaceDetection.cpp
//


class CZJURollerBearingSurfaceDetectionApp : public CWinApp
{
public:
	CZJURollerBearingSurfaceDetectionApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CZJURollerBearingSurfaceDetectionApp theApp;