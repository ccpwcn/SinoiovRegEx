
// SinoiovRegExDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CSinoiovRegExDlg �Ի���
class CSinoiovRegExDlg : public CDialogEx
{
// ����
public:
	CSinoiovRegExDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SINOIOVREGEX_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRun();
private:
	// �û������������ʽ
	CEdit m_SourceRegex;
	// �û������Դ�ı�
	CEdit m_SourceText;
	// �����
	CListBox m_Result;
	// ���״̬
	CStatic m_ResultStatus;

	HANDLE m_hWorkThread;
	HANDLE m_hQuitEvent;
	HANDLE m_hRunEvent;

	static DWORD WINAPI m_fnWorkThreadProc(LPVOID lpParam);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
};
