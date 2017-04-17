
// SinoiovRegExDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SinoiovRegEx.h"
#include "SinoiovRegExDlg.h"
#include "afxdialogex.h"

#include <strsafe.h>
#include <regex>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSinoiovRegExDlg �Ի���



CSinoiovRegExDlg::CSinoiovRegExDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SINOIOVREGEX_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hWorkThread = NULL;
	m_hRunEvent = NULL;
	m_hQuitEvent = NULL;
	memset(&m_tNotifyUiEntity, 0, sizeof(m_tNotifyUiEntity));
}

void CSinoiovRegExDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REGEX, m_SourceRegex);
	DDX_Control(pDX, IDC_EDIT_SOURCE, m_SourceText);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_ResultStatus);
	DDX_Control(pDX, IDC_LIST_RESULT, m_ResultSet);
	DDX_Control(pDX, IDC_CHECK_ECMAScript, m_ECMAScript);
	DDX_Control(pDX, IDC_CHECK_CASE_SENSITIVE, m_CaseSensitive);
	DDX_Control(pDX, IDC_CHECK_MULTILINE, m_Multiline);
}

BEGIN_MESSAGE_MAP(CSinoiovRegExDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CSinoiovRegExDlg::OnBnClickedButtonRun)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_RADIO1, &CSinoiovRegExDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CSinoiovRegExDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CSinoiovRegExDlg::OnBnClickedRadio3)
	ON_MESSAGE(WM_NOTIFY_UI, &CSinoiovRegExDlg::OnNotifyUi)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CSinoiovRegExDlg ��Ϣ�������

BOOL CSinoiovRegExDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_hWorkThread = CreateThread(
		NULL,	// pointer to security attributes
		0,		// initial thread stack size
		LPTHREAD_START_ROUTINE(m_fnWorkThreadProc),	// pointer to thread function
		this,	// argument for new thread
		0,		// creation flags
		NULL	// pointer to receive thread ID
	);
	m_hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // �˳��¼����ֶ������ź�������ʼ״̬Ϊ����Ϣ
	m_hRunEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // �����¼����Զ������ź�������ʼ״̬Ϊ���ź�

	if (m_hWorkThread == NULL || m_hQuitEvent == NULL || m_hRunEvent == NULL) {
		AfxMessageBox(_T("Ӧ�ó�������ʧ�ܣ�"), MB_OK | MB_ICONSTOP);
		exit(255);
	}

	// Ĭ��ģʽ
	m_CaseSensitive.SetCheck(1);
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
	m_ModeValue = 1;
	m_tNotifyUiEntity.m_nStatus = IDEL;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSinoiovRegExDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSinoiovRegExDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSinoiovRegExDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSinoiovRegExDlg::OnBnClickedButtonRun()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_ResultSet.ResetContent();
	m_ResultStatus.SetWindowText(_T(""));

	SetEvent(m_hRunEvent);
}

BOOL CSinoiovRegExDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			if (GetFocus() != GetDlgItem(IDC_EDIT_SOURCE))
				return true;
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSinoiovRegExDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	SetEvent(m_hRunEvent);
	SetEvent(m_hQuitEvent);

	CDialogEx::OnClose();
}

void CSinoiovRegExDlg::OnBnClickedRadio1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_ModeValue = 1;
}


void CSinoiovRegExDlg::OnBnClickedRadio2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_ModeValue = 2;
}


void CSinoiovRegExDlg::OnBnClickedRadio3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_ModeValue = 3;
}


DWORD WINAPI CSinoiovRegExDlg::m_fnWorkThreadProc(LPVOID lpParam)
{
	CSinoiovRegExDlg *pDlg = (CSinoiovRegExDlg *)lpParam;
	if (pDlg == NULL) {
		AfxMessageBox(_T("Ӧ�ó����ʼ��ʧ�ܣ�"), MB_OK | MB_ICONSTOP);
		exit(254);
	}

	TCHAR szMsg[BUFSIZ] = { 0 };
	while (TRUE) {
		if (WaitForSingleObject(pDlg->m_hRunEvent, 100) == WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(pDlg->m_hQuitEvent, 50) == WAIT_OBJECT_0)
			{
				OutputDebugString(_T("sinoiov regex quit"));
				break;
			}
			
			DWORD dwStartTime = GetTickCount();

			CString srcRegex;
			pDlg->m_SourceRegex.GetWindowText(srcRegex);
			std::wstring wRegexStr(srcRegex.GetBuffer());
			srcRegex.ReleaseBuffer();
			std::wregex wrx;
			std::regex_constants::syntax_option_type type;

			CString srcText;
			pDlg->m_SourceText.GetWindowText(srcText);
			std::wstring wSrcText(srcText.GetBuffer());
			srcText.ReleaseBuffer();
			OutputDebugString(wSrcText.c_str());

			std::wsmatch wideMatch;

			std::regex_constants::match_flag_type flag;;
			if (pDlg->m_ECMAScript.GetCheck() == 1)
			{
				flag = std::regex_constants::format_default;
				type = std::regex::ECMAScript;
			}

			if (pDlg->m_CaseSensitive.GetCheck() == 0)
			{
				type = type | std::regex_constants::icase;
				
			}
			if (pDlg->m_Multiline.GetCheck() == 1)
			{
				type = type | std::regex_constants::multiline;
			}
			wrx = std::wregex(wRegexStr.c_str(), type);

			BOOL result = FALSE;
			// ���ݲ�ͬ��ѡ��״ִ̬��
			std::wstring buffer;
			switch (pDlg->m_ModeValue) {
			case 1:
				if (buffer.length() > 0 && (result = std::regex_search(buffer.cbegin(), buffer.cend(), wideMatch, wrx, flag)) == TRUE)
				{
					for (unsigned i = 0; i < wideMatch.size(); i++)
					{
						pDlg->m_ResultSet.AddString(wideMatch.str().c_str());
					}
				}
				break;
			case 2:
				if (result = std::regex_match(wSrcText.cbegin(), wSrcText.cend(), wideMatch, wrx, flag))
				{
					pDlg->m_ResultSet.AddString(wideMatch.str().c_str());
				}
				break;
			case 3:
				// TODO: ��ʱû��ʵ��
				break;
			default:
				break;
			}
			DWORD dwEndTime = GetTickCount();

			// ����UI��Ҫ�õ���״̬���
			if (result)
			{
				pDlg->m_tNotifyUiEntity.m_nStatus = SUCCESSED;
				StringCchPrintf(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("�ɹ�����ʱ%d����"), dwEndTime - dwStartTime);
			}
			else
			{
				pDlg->m_tNotifyUiEntity.m_nStatus = FAILED;
				StringCchCopy(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("ʧ��"));
			}
			::SendMessage(pDlg->m_hWnd, WM_NOTIFY_UI, 0, 0);
		}
	}

	return 0;
}


afx_msg LRESULT CSinoiovRegExDlg::OnNotifyUi(WPARAM wParam, LPARAM lParam)
{
	m_ResultStatus.SetWindowText(m_tNotifyUiEntity.m_szMessage);
	m_ResultSet.UpdateData();

	return 0;
}


HBRUSH CSinoiovRegExDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if (IDC_STATIC_STATUS == pWnd->GetDlgCtrlID())  // �жϷ�����Ϣ�Ŀռ��Ƿ��Ǹþ�̬�ı���
	{
		// �����ı���ɫ
		if (m_tNotifyUiEntity.m_nStatus == SUCCESSED)
			pDC->SetTextColor(RGB(33, 197, 41)); // �ɹ�̬
		else if (m_tNotifyUiEntity.m_nStatus == FAILED)
			pDC->SetTextColor(RGB(255, 0, 0)); // ʧ��̬
	}

	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}
