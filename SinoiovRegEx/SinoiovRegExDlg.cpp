
// SinoiovRegExDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SinoiovRegEx.h"
#include "SinoiovRegExDlg.h"
#include "afxdialogex.h"

#include <strsafe.h>

#define BOOST_HAS_ICU
#include <boost/regex/mfc.hpp>
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
	DDX_Control(pDX, IDC_CHECK_ECMAScript, m_ECMAScript);
	DDX_Control(pDX, IDC_CHECK_CASE_INSENSITIVE, m_CaseInsensitive);
	DDX_Control(pDX, IDC_CHECK_MULTILINE, m_Multiline);
	DDX_Control(pDX, IDC_LIST_RESULTSET, m_ResultSet);
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
	std::locale loc("");
	std::wcout.imbue(loc);

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
	m_ECMAScript.SetCheck(1);

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
	m_ModeValue = 1;
	m_tNotifyUiEntity.m_nStatus = IDEL;

	m_ResultSet.GetClientRect(&m_ResultSetRect);
	m_ResultSet.SetExtendedStyle(m_ResultSet.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVTILEVIEWINFO ti = { 0 };
	ti.cbSize = sizeof(LVTILEVIEWINFO);
	ti.dwFlags = LVTVIF_AUTOSIZE;
	m_ResultSet.SetTileViewInfo(&ti);

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
	memset(&m_tNotifyUiEntity, 0, sizeof(m_tNotifyUiEntity));
	m_ResultSet.DeleteAllItems();

	BOOL Deleted = TRUE;
	while (Deleted == TRUE)
		Deleted = m_ResultSet.DeleteColumn(0);

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

	typedef boost::basic_regex<TCHAR> tregex;
	typedef boost::match_results<TCHAR const*> tmatch;
	typedef boost::regex_iterator<TCHAR const*>        tregex_iterator;
	typedef boost::regex_token_iterator<TCHAR const*>  tregex_token_iterator;

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
			
			CString srcText;
			pDlg->m_SourceText.GetWindowText(srcText);

			
			boost::regex_constants::syntax_option_type type = boost::regex_constants::normal;
			boost::regex_constants::match_flag_type flag = boost::regex_constants::match_flag_type::match_default;
			if (pDlg->m_ECMAScript.GetCheck() == 1)
			{
				type = boost::regex::ECMAScript;  // ��ȡECMA-262�е�ECMAScript�����﷨
			}

			if (pDlg->m_CaseInsensitive.GetCheck() == 1)
			{
				type = type | boost::regex_constants::icase;   // ��Сд�����е�CheckBoxѡ��ʱ������Ϊ���Դ�Сд
				
			}
			if (pDlg->m_Multiline.GetCheck() == 0)
			{
				flag = flag | boost::regex_constants::match_single_line;
			}
			else
			{
				flag = flag | boost::regex_constants::match_not_bol;
				flag = flag | boost::regex_constants::match_not_eol;
			}

			tregex r(srcRegex, type);

			std::vector<CString> strs;
			pDlg->m_fnSplitString(srcText, _T("\r\n"), strs);
			BOOL result = FALSE;
			std::vector<int> failedVector;
			CString group;
			try
			{
				// ���ݲ�ͬ��ѡ��״ִ̬��
				switch (pDlg->m_ModeValue) {
				case 1:
					if (pDlg->m_Multiline.GetCheck() == 0)
					{
						tmatch what;
						if ((result = boost::regex_search(srcText, what, r, flag)) == TRUE)
						{
							for (size_t i = 0; i < 5; i++)
							{
								group.Format(_T("����%d"), i);
								pDlg->m_ResultSet.InsertColumn(i, group, LVCFMT_LEFT, pDlg->m_ResultSetRect.Width() / 5, i);
							}

							pDlg->m_ResultSet.InsertItem(0, CString(what[2].first));
							for (size_t i = 1; i < 5; i++)
							{
								pDlg->m_ResultSet.SetItemText(0, i, CString(what[i].first, what.length(i)));
							}
						}
					}
					else
					{
						for (size_t i = 0; i < strs.size(); i++)
						{
							tmatch what;
							if (boost::regex_search(strs.at(i), what, r, flag))
							{
								if (i == 0)
								{
									for (short j = 1; j < 5; j++)
									{
										group.Format(_T("����%d"), j);
										pDlg->m_ResultSet.InsertColumn(j, group, LVCFMT_LEFT, pDlg->m_ResultSetRect.Width() / 5, j);
									}

								}
								pDlg->m_ResultSet.InsertItem(i, CString(what[0].first));
								for (size_t j = 1; j < 5; j++)
								{
									pDlg->m_ResultSet.SetItemText(i, j, CString(what[j].first, what[j].length()));
								}
							}
							else
							{
								failedVector.push_back(0);
							}
						}
					}
					
					break;
				case 2:
				{
					tmatch what;
					if ((result = boost::regex_match(srcText, what, r, flag)) == TRUE)
					{
						pDlg->m_ResultSet.InsertItem(0, CString(_T("����0")));
						pDlg->m_ResultSet.SetItemText(0, 1, CString(what.str().c_str()).Trim());
					}
				}
					
					break;
				case 3:
					// TODO: ��ʱû��ʵ��
					break;
				default:
					break;
				}
			}
			catch (std::exception const& e)
			{
				result = FALSE;
				StringCchPrintf(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("ʧ�ܣ�%s"), e.what());
			}
			
			DWORD dwEndTime = GetTickCount();
			result = (result && failedVector.empty());

			// ����UI��Ҫ�õ���״̬���
			if (result)
			{
				pDlg->m_tNotifyUiEntity.m_nStatus = SUCCESSED;
				StringCchPrintf(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("�ɹ�����ʱ%d����"), dwEndTime - dwStartTime);
			}
			else
			{
				pDlg->m_tNotifyUiEntity.m_nStatus = FAILED;
				pDlg->m_tNotifyUiEntity.m_szMessage[0] ? StringCchCopy(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("ʧ��")) : 0;
			}
			::SendMessage(pDlg->m_hWnd, WM_NOTIFY_UI, 0, 0);
		}
	}

	return 0;
}

void CSinoiovRegExDlg::m_fnSplitString(const CString & in, const CString & seperator, std::vector<CString> & out)
{
	out.clear();
	std::vector<CString>().swap(out);

	CString temp;
	short flag = 0;
	for (int i = 0; i < in.GetLength(); i++)
	{
		TCHAR ch = in.GetAt(i);
		if (seperator.Find(ch) == -1)
		{
			if (flag == 0)
			{
				temp.AppendChar(ch);
				flag = 0;
				if (i == in.GetLength() - 1)
				{
					out.push_back(temp);
					temp.Delete(0, temp.GetLength());
				}
			}
		}
		else
		{
			if (flag == 0)
			{
				out.push_back(temp);
				temp.Delete(0, temp.GetLength());
				flag = 1;
			}
			else
			{
				flag = 0;
			}
		}
	}
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
