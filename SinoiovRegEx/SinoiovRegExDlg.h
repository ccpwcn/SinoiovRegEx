
// SinoiovRegExDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


#define WM_NOTIFY_UI (WM_USER+8)

enum STATUS {IDEL = 0, SUCCESSED = 1, FAILED = 2};

typedef struct _tagNotifyUiTag 
{
	short m_nStatus;
	TCHAR m_szMessage[BUFSIZ];
}NOTIFY_UI_ENTITY;

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
	// ���״̬
	CStatic m_ResultStatus;

	HANDLE m_hWorkThread;
	HANDLE m_hQuitEvent;
	HANDLE m_hRunEvent;

	static DWORD WINAPI m_fnWorkThreadProc(LPVOID lpParam);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
private:
	// ƥ��ģʽ
	BOOL m_ModeValue;
public:
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
protected:
	afx_msg LRESULT OnNotifyUi(WPARAM wParam, LPARAM lParam);
private:
	NOTIFY_UI_ENTITY m_tNotifyUiEntity;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	// ECMAScript
	CButton m_ECMAScript;
	// ��Сд���ؿؼ�
	CButton m_CaseInsensitive;
	// ����ģʽ
	CButton m_Multiline;
	// �����
	CListCtrl m_ResultSet;
	CRect m_ResultSetRect;
};
