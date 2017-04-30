
// SinoiovRegExDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SinoiovRegEx.h"
#include "SinoiovRegExDlg.h"
#include "afxdialogex.h"
#include <cmath>
#include <strsafe.h>

#define BOOST_HAS_ICU
#include <boost/regex.hpp>
#include <boost/regex/mfc.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CSinoiovRegExDlg 对话框



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


// CSinoiovRegExDlg 消息处理程序

BOOL CSinoiovRegExDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
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
	m_hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // 退出事件，手动设置信号量，初始状态为无信息
	m_hRunEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // 运行事件，自动设置信号量，初始状态为无信号

	if (m_hWorkThread == NULL || m_hQuitEvent == NULL || m_hRunEvent == NULL) {
		AfxMessageBox(_T("应用程序启动失败！"), MB_OK | MB_ICONSTOP);
		exit(255);
	}

	// 默认模式
	m_ECMAScript.SetCheck(1);

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
	m_ModeValue = 1;
	m_tNotifyUiEntity.m_nStatus = IDEL;
	m_SourceRegex.SetWindowText(_T("(\\d{6})(19[4-9][0-9]|20[0-9]{2})(0[0-9]|1[0-2])([0-3][0-9])(\\d{4}|\\d{3}[xX])"));
	m_SourceText.SetWindowText(_T("34072119790807393X\r\n140123198112194055"));

	m_ResultSet.GetClientRect(&m_ResultSetRect);
	m_ResultSet.SetExtendedStyle(m_ResultSet.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVTILEVIEWINFO ti = { 0 };
	ti.cbSize = sizeof(LVTILEVIEWINFO);
	ti.dwFlags = LVTVIF_AUTOSIZE;
	m_ResultSet.SetTileViewInfo(&ti);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSinoiovRegExDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSinoiovRegExDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSinoiovRegExDlg::OnBnClickedButtonRun()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ResultSet.DeleteAllItems();

	BOOL Deleted = TRUE;
	while (Deleted == TRUE)
		Deleted = m_ResultSet.DeleteColumn(0);

	SetEvent(m_hRunEvent);
}

BOOL CSinoiovRegExDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
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
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SetEvent(m_hRunEvent);
	SetEvent(m_hQuitEvent);

	CDialogEx::OnClose();
}

void CSinoiovRegExDlg::OnBnClickedRadio1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ModeValue = 1;
}


void CSinoiovRegExDlg::OnBnClickedRadio2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ModeValue = 2;
}


void CSinoiovRegExDlg::OnBnClickedRadio3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ModeValue = 3;
}


DWORD WINAPI CSinoiovRegExDlg::m_fnWorkThreadProc(LPVOID lpParam)
{
	CSinoiovRegExDlg *pDlg = (CSinoiovRegExDlg *)lpParam;
	if (pDlg == NULL) {
		AfxMessageBox(_T("应用程序初始化失败！"), MB_OK | MB_ICONSTOP);
		exit(254);
	}

	TCHAR szMsg[BUFSIZ] = { 0 };
	while (TRUE) {
		short nWorkMode = 0;
		if (WaitForSingleObject(pDlg->m_hRunEvent, 100) == WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(pDlg->m_hQuitEvent, 50) == WAIT_OBJECT_0)
			{
				OutputDebugString(_T("sinoiov regex quit"));
				break;
			}
			if (nWorkMode == 1)
			{
				::SendMessage(pDlg->m_hWnd, WM_NOTIFY_UI, 1, 0);
				continue;
			}
			nWorkMode = 1;
			DWORD dwStartTime = GetTickCount();
			memset(&(pDlg->m_tNotifyUiEntity), 0, sizeof(m_tNotifyUiEntity));

			CString srcRegex;
			pDlg->m_SourceRegex.GetWindowText(srcRegex);
			
			CString srcText;
			pDlg->m_SourceText.GetWindowText(srcText);

			
			boost::regex_constants::syntax_option_type type = boost::regex_constants::normal;
			boost::regex_constants::match_flag_type flag = boost::regex_constants::match_flag_type::match_default;
			if (pDlg->m_ECMAScript.GetCheck() == 1)
			{
				type = boost::regex::ECMAScript;  // 采取ECMA-262中的ECMAScript所用语法
			}

			if (pDlg->m_CaseInsensitive.GetCheck() == 1)
			{
				type = type | boost::regex_constants::icase;   // 大小写不敏感的CheckBox选定时，设置为忽略大小写
				
			}
			if (pDlg->m_Multiline.GetCheck() == 1)
			{
				flag |= boost::match_prev_avail;
				flag |= boost::match_not_bob;;
			}

			const boost::tregex r(srcRegex, type);
			CString group;
			BOOL result = TRUE;
			try
			{
				// 根据不同的选定状态执行
				switch (pDlg->m_ModeValue) {
				case 1:
					{
						if (pDlg->m_Multiline.GetCheck() == 1)
						{
							std::vector<STD_STRING> tokens = pDlg->m_fnSplitString(srcText.GetBuffer(), _T("\r\n")); srcText.ReleaseBuffer();
							boost::wregex exp(srcRegex.GetBuffer()); srcRegex.ReleaseBuffer();
							size_t nLineCount = tokens.size();
							for (size_t line = 0; line < nLineCount; line++)
							{
								boost::wsmatch results;
								if (regex_search(tokens.at(line), results, exp))
								{
									size_t nResultSize = results.size();
									for (size_t i = 0; line == 0 && i < nResultSize; i++)
									{
										group.Format(_T("分组%d"), i);
										pDlg->m_ResultSet.InsertColumn(i, group, LVCFMT_LEFT, pDlg->m_ResultSetRect.Width() / nResultSize, i);
									}

									pDlg->m_ResultSet.InsertItem(line, STD_STRING(results[0]).c_str());
									for (size_t i = 0; i < nResultSize; i++)
									{
										pDlg->m_ResultSet.SetColumnWidth(line, LVSCW_AUTOSIZE_USEHEADER);
										pDlg->m_ResultSet.SetItemText(line, i, STD_STRING(results[i]).c_str());
									}
								}
							}
						}
						else
						{
							int nLineIndex = 0;
							boost::tmatch what;
							if ((result = boost::regex_search(srcText, what, r)) == TRUE)
							{
								size_t nResultSize = what.size();
								for (size_t i = 0; nLineIndex == 0 && i < nResultSize; i++)
								{
									group.Format(_T("分组%d"), i);
									pDlg->m_ResultSet.InsertColumn(i, group, LVCFMT_LEFT, pDlg->m_ResultSetRect.Width() / nResultSize, i);
								}

								pDlg->m_ResultSet.InsertItem(nLineIndex, CString(what[0].first));
								for (size_t i = 0; i < nResultSize; i++)
								{
									pDlg->m_ResultSet.SetColumnWidth(nLineIndex, LVSCW_AUTOSIZE_USEHEADER);
									pDlg->m_ResultSet.SetItemText(nLineIndex, i, CString(what[i].first, what[i].length()));
								}
								nLineIndex++;
							}
						}
					}
					break;
				case 2:
					{
						int nLineIndex = 0;
						boost::tregex_iterator it(boost::make_regex_iterator(srcText, r)), end;
						while (it != end)
						{
							size_t nGroupSize = (*it).size();
							for (size_t i = 0; nLineIndex == 0 && i < nGroupSize; i++)
							{
								group.Format(_T("分组%d"), i);
								int nColumnWidth = pDlg->m_ResultSetRect.Width() / nGroupSize + (*it)[i].length() * 4;
								pDlg->m_ResultSet.InsertColumn(i, group, LVCFMT_LEFT, nColumnWidth, i);
							}
							pDlg->m_ResultSet.InsertItem(nLineIndex, CString((*it)[0].first));
							for (size_t i = 0; i < nGroupSize; i++)
							{
								pDlg->m_ResultSet.SetColumnWidth(nLineIndex, LVSCW_AUTOSIZE_USEHEADER);
								pDlg->m_ResultSet.SetItemText(nLineIndex, i, CString((*it)[i].first, (*it)[i].length()));
							}
							it++;
							nLineIndex++;
						}
					}
					break;
				case 3:
					// TODO: 暂时没有实现
					throw std::runtime_error("暂不支持");
					break;
				default:
					break;
				}
			}
			catch (std::exception const& e)
			{
				result = FALSE;
				MultiByteToWideChar(CP_ACP, 0, e.what(), -1, pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ);
			}
			
			DWORD dwEndTime = GetTickCount();

			// 设置UI需要用到的状态标记
			if (result)
			{
				pDlg->m_tNotifyUiEntity.m_nStatus = SUCCESSED;
				StringCchPrintf(pDlg->m_tNotifyUiEntity.m_szMessage, BUFSIZ, _T("完成，耗时%d毫秒"), dwEndTime - dwStartTime);
			}
			
			::SendMessage(pDlg->m_hWnd, WM_NOTIFY_UI, 0, 0);
			nWorkMode = 0;
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

//字符串分割函数
std::vector<STD_STRING> CSinoiovRegExDlg::m_fnSplitString(STD_STRING str, STD_STRING pattern)
{
	STD_STRING::size_type pos;
	std::vector<STD_STRING> result;
	str += pattern;//扩展字符串以方便操作
	size_t size = str.size();

	for (size_t i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			STD_STRING s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}


afx_msg LRESULT CSinoiovRegExDlg::OnNotifyUi(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
	{
		m_ResultStatus.SetWindowText(m_tNotifyUiEntity.m_szMessage);
		m_ResultSet.UpdateData();
	}
	else if (wParam == 1)
	{
		m_ResultStatus.SetWindowText(_T("程序繁忙，请等待。"));
		m_ResultSet.UpdateData();
	}

	return 0;
}


HBRUSH CSinoiovRegExDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (IDC_STATIC_STATUS == pWnd->GetDlgCtrlID())  // 判断发出消息的空间是否是该静态文本框
	{
		// 设置文本颜色
		if (m_tNotifyUiEntity.m_nStatus == SUCCESSED)
			pDC->SetTextColor(RGB(33, 197, 41)); // 成功态
		else if (m_tNotifyUiEntity.m_nStatus == FAILED)
			pDC->SetTextColor(RGB(255, 0, 0)); // 失败态
	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
