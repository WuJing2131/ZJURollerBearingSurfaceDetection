#include "stdafx.h"
#include "CommonMethod.h"


CCommonMethod::CCommonMethod()
{
}


CCommonMethod::~CCommonMethod()
{
}

char CCommonMethod::DoHexChar(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - 0x30;
	else if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;
	else if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;
	else
		return 0x10;
}

int CCommonMethod::DoStr2Hex(CString str, char* data)
{
	int t, t1;
	int rlen = 0, len = str.GetLength();
	for (int i = 0; i < len;)
	{
		char l, h = str[i];
		if (h == ' ')
		{
			i++;
			continue;
		}
		i++;
		if (i >= len) break;
		l = str[i];
		t = DoHexChar(h);
		t1 = DoHexChar(l);
		if ((t == 16) || (t1 == 16))
			break;
		else
			t = t * 16 + t1;
		i++;
		data[rlen] = (char)t;
		rlen++;
	}
	return rlen;
}

BOOL CCommonMethod::WChar2MByte(LPCWSTR lpSrc, LPSTR lpDest, int nlen)
{
	int n = 0;

	n = WideCharToMultiByte(CP_OEMCP, 0, lpSrc, -1, lpDest, 0, 0, FALSE);
	if (n < nlen) return FALSE;
	WideCharToMultiByte(CP_OEMCP, 0, lpSrc, -1, lpDest, nlen, 0, FALSE);
	return TRUE;
}

byte CCommonMethod::DoCheckSum(unsigned char *buffer, int   size)
{
	byte myv = 0x00;
	for (int i = 0; i < size; i++)
		myv += *buffer++;
	return myv;
}

char *  CCommonMethod::WCharToMByte(LPCWSTR lpSrc)
{
	char *  lpDest=NULL;
	int iSize;
	iSize = WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, NULL, 0, NULL, NULL);
	lpDest = (char*)malloc((iSize + 1));
	memset(lpDest, 0, iSize + 1);
	if (!lpDest)
		return NULL;
	WideCharToMultiByte(CP_ACP, 0, lpSrc, -1, lpDest, iSize, NULL, NULL);
	if (iSize)
		return lpDest;
	else
	{
		delete[] lpDest;
		return NULL;
	}
}
