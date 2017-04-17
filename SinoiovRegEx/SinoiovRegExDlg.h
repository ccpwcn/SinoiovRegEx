
// SinoiovRegExDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CSinoiovRegExDlg 对话框
class CSinoiovRegExDlg : public CDialogEx
{
// 构造
public:
	CSinoiovRegExDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SINOIOVREGEX_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRun();
private:
	// 用户输入的正则表达式
	CEdit m_SourceRegex;
	// 用户输入的源文本
	CEdit m_SourceText;
	// 结果集
	CListBox m_Result;
	// 结果状态
	CStatic m_ResultStatus;

	HANDLE m_hWorkThread;
	HANDLE m_hQuitEvent;
	HANDLE m_hRunEvent;

	static DWORD WINAPI m_fnWorkThreadProc(LPVOID lpParam);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};
