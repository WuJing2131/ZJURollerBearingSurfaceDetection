﻿//#pragma once
#ifndef COMMONMETHOD_H
#define  COMMONMETHOD_H
#include "stdafx.h"
#include "afxwin.h"

class CCommonMethod
{
public:
	CCommonMethod();
	~CCommonMethod();
	static char DoHexChar(char c);
	static int  DoStr2Hex(CString str, char* data);
	static BOOL WChar2MByte(LPCWSTR lpSrc, LPSTR lpDest, int nlen);
	static byte DoCheckSum(unsigned char *buffer, int   size);
	static char* WCharToMByte(LPCWSTR lpSrc);
	char* ConvertStringToChar(const CString &str);
};
#endif
