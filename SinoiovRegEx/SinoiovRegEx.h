
// SinoiovRegEx.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSinoiovRegExApp: 
// �йش����ʵ�֣������ SinoiovRegEx.cpp
//

class CSinoiovRegExApp : public CWinApp
{
public:
	CSinoiovRegExApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSinoiovRegExApp theApp;