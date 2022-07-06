// LabProject08-6-7-2.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Client_Desert.h"
#include "GameFramework.h"
#include "Monster.h"
#include "ServerManager.h"

#include "../Client_Desert_Server/Client_Desert_Server/Protocol.h"
#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);


HWND g_hWnd = NULL;

// Server
WSABUF wsabuf_r;
char recv_buf[BUFSIZE];
WSABUF wsabuf_s;
int g_myid = -1;
char* send_buf = nullptr;
char prev_buf[BUFSIZE];
int prev_bytes = 0;
bool isWindow = false;

void Server_PosSend();
void Server_PosRecv();
void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void send_packet(void* packet);

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러 " << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


#ifdef USE_SERVER
	CServerManager::GetInstance()->Connect();
#endif // USE_SERVER

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_CLIENTDESERT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTDESERT));

#ifdef USE_SERVER
	send_packet(p);
	std::cout << "상대 클라 대기중" << endl;

	//thread serverThread{ Server_PosRecv };
	Server_PosRecv();
#endif // USE_SERVER

	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			


			}
		}
		else
		{
#ifdef USE_SERVER
			
			if (isWindow)
			{
				gGameFramework.FrameAdvance();
			}
#else
			gGameFramework.FrameAdvance();

#endif // USE_SERVER

#ifdef USE_SERVER

			SleepEx(0, true);
			// Sleep -> recv_callback -> send_callback 순으로 실행된다.
#endif // USE_SERVER
		}
		//// Start the Dear ImGui frame
		//ImGui_ImplDX12_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
	}
	gGameFramework.OnDestroy();
	//serverThread.join();
	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDl_CLIENTDESERT));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_CLIENTDESERT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

#ifdef USE_SERVER
	// 클라 2명 연결전 윈도우 안보이게 (임시)
	ShowWindow(hMainWnd, isWindow);
#else
	::ShowWindow(hMainWnd, nCmdShow);
#endif

	g_hWnd = hMainWnd;

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::UpdateWindow(hMainWnd);
	

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	gGameFramework.ChangeSwapChainState();
#endif

	return(TRUE);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:
		hdc = ::BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}

void Server_PosRecv()
{
	// 위치 recv
	wsabuf_r.buf = recv_buf; wsabuf_r.len = BUFSIZE;
	DWORD recv_flag = 0;
	WSAOVERLAPPED* r_over = new WSAOVERLAPPED;
	std::ZeroMemory(r_over, sizeof(WSAOVERLAPPED));

	int ret = WSARecv(s_socket, &wsabuf_r, 1, 0, &recv_flag, r_over, recv_callback);

	//SleepEx(0, true);

	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("WSARecv", err_no);
		//WSARecv에러 겹친 I/O 작업이 진행 중입니다. 는 에러로 판정하지 않아야함
	}

}

void send_packet(void* packet)
{
	char* p = reinterpret_cast<char*>(packet);
	size_t sent = 0;
	WSABUF mybuf;
	mybuf.buf = p;
	mybuf.len = static_cast<unsigned char>(p[0]);
	send_buf = p;
	WSAOVERLAPPED* s_over = new WSAOVERLAPPED;
	std::ZeroMemory(s_over, sizeof(WSAOVERLAPPED));

	//SleepEx(0, true);

	int ret = WSASend(s_socket, &mybuf, 1, 0, 0, s_over, send_callback);
	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("WSASend", err_no);
		//WSARecv에러 겹친 I/O 작업이 진행 중입니다. 는 에러로 판정하지 않아야함
	}
}
void Server_PosSend()
{	
	// 버퍼에 duoPlayer 넣기
	CS_MOVE_PACKET *p = gGameFramework.m_pPlayer->Server_GetParentAndAnimation();
	p->size = sizeof(CS_MOVE_PACKET);
	p->type = CS_MOVE;
	send_packet(p);
}

