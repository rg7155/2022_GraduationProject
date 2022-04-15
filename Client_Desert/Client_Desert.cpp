// LabProject08-6-7-2.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Client_Desert.h"
#include "GameFramework.h"
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

// Server
char SERVER_ADDR[BUFSIZE] = "127.0.0.1";
SOCKET s_socket;
WSABUF wsabuf_r;
char recv_buf[BUFSIZE];
WSABUF wsabuf_s;
char send_buf[BUFSIZE];

// new
int g_myid;
int g_duoid;

void ProcessPacket(char* ptr);
void process_data(char* net_buf, size_t io_byte);
void send_packet(void* packet);

void Server_PosSend();
void Server_PosRecv();
void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

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

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_CLIENTDESERT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTDESERT));

#ifdef USE_SERVER
	wcout.imbue(locale("korean")); // 에러 메세지 한글로 출력
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData); // 소켓 네트워킹 시작 - 윈도우만
	s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	int ret = connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("connect", err_no);
		//WSARecv에러 겹친 I/O 작업이 진행 중입니다. 는 에러로 판정하지 않아야함
	}
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	strcpy_s(p.name, "TEMP");
	send_packet(&p);

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
			gGameFramework.FrameAdvance();
#ifdef USE_SERVER
			////// 위치 send -> 키입력 있었을때만 Send하는걸로 바꾸고 다른 클라는 입력 값없으면 알아서 돌아가도록
			Server_PosSend();

			//// 위치 받기
			//Server_PosRecv();
			

			//SleepEx(0, true);
			// Sleep -> recv_callback -> send_callback 순으로 실행된다.
#endif // USE_SERVER
		}
		//// Start the Dear ImGui frame
		//ImGui_ImplDX12_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
	}
	gGameFramework.OnDestroy();

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

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
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

void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	size_t received = cbTransferred;
	
	if (received > 0) process_data(recv_buf, received);

	delete lpOverlapped;
}

void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	delete lpOverlapped;
}

void Server_PosRecv()
{
	// 위치 recv
	wsabuf_r.buf = recv_buf; wsabuf_r.len = BUFSIZE;
	DWORD recv_flag = 0;
	WSAOVERLAPPED* r_over = new WSAOVERLAPPED;
	ZeroMemory(r_over, sizeof(WSAOVERLAPPED));

	int ret = WSARecv(s_socket, &wsabuf_r, 1, 0, &recv_flag, r_over, recv_callback);

	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("WSARecv", err_no);
		//WSARecv에러 겹친 I/O 작업이 진행 중입니다. 는 에러로 판정하지 않아야함
	}

}

void Server_PosSend()
{
	CS_MOVE_PACKET p;
	p.size = sizeof(p);
	p.type = CS_MOVE;
	p.xmf4x4World = gGameFramework.m_pPlayer->m_xmf4x4ToParent;
	for (int i = 0; i < ANIM::END; i++)
	{
		p.animInfo[i].fWeight = gGameFramework.m_pPlayer->m_pSkinnedAnimationController->GetTrackWeight(i);
		p.animInfo[i].bEnable = gGameFramework.m_pPlayer->m_pSkinnedAnimationController->GetTrackEnable(i);
		p.animInfo[i].fPosition = gGameFramework.m_pPlayer->m_pSkinnedAnimationController->m_fPosition[i];
	}
	send_packet(&p);
}
void ProcessPacket(unsigned char* ptr)
{
	static bool duo_add = false;

	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;

		// 행렬, 애니메이션
		gGameFramework.m_pPlayer->m_xmf4x4ToParent = packet->xmf4x4World;
		for (int i = 0; i < ANIM::END; i++)
		{
			gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, packet->animInfo[i].fWeight);
			gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, packet->animInfo[i].fPosition);
			gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, packet->animInfo[i].bEnable);
		}
		break;
	}
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			// 행렬, 애니메이션
			gGameFramework.m_pPlayer->m_xmf4x4ToParent = my_packet->xmf4x4World;
			for (int i = 0; i < ANIM::END; i++)
			{
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, my_packet->animInfo[i].fWeight);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, my_packet->animInfo[i].fPosition);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, my_packet->animInfo[i].bEnable);
			}
		}
		else if (id < MAX_USER)
		{
			// duoPlayer 1명만 받아요
			if (duo_add)
				break;
			duo_add = true;
			g_duoid = id;

			// 행렬, 애니메이션
			gGameFramework.m_pScene->m_pDuoPlayer->m_xmf4x4ToParent = my_packet->xmf4x4World;
			for (int i = 0; i < ANIM::END; i++)
			{
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, my_packet->animInfo[i].fWeight);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, my_packet->animInfo[i].fPosition);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, my_packet->animInfo[i].bEnable);
			}
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid)
		{
			gGameFramework.m_pPlayer->m_xmf4x4ToParent = my_packet->xmf4x4World;
			for (int i = 0; i < ANIM::END; i++)
			{
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, my_packet->animInfo[i].fWeight);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, my_packet->animInfo[i].fPosition);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, my_packet->animInfo[i].bEnable);
			}
		}
		else if (other_id == g_duoid)
		{
			// 행렬, 애니메이션
			gGameFramework.m_pScene->m_pDuoPlayer->m_xmf4x4ToParent = my_packet->xmf4x4World;
			for (int i = 0; i < ANIM::END; i++)
			{
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackWeight(i, my_packet->animInfo[i].fWeight);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackPosition(i, my_packet->animInfo[i].fPosition);
				gGameFramework.m_pPlayer->m_pSkinnedAnimationController->SetTrackEnable(i, my_packet->animInfo[i].bEnable);
			}
		}
		break;
	}
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			// 숨기기
		}
		else if (other_id == g_duoid) {
			// duo 숨기기
		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
		break;
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	unsigned char* ptr = reinterpret_cast<unsigned char*>(net_buf);
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static unsigned char packet_buffer[BUFSIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void send_packet(void* packet)
{
	WSABUF mybuf;
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	mybuf.buf = reinterpret_cast<char*>(packet);
	mybuf.len = p[0];
	WSAOVERLAPPED* s_over = new WSAOVERLAPPED;
	ZeroMemory(s_over, sizeof(WSAOVERLAPPED));

	int ret = WSASend(s_socket, &mybuf, 1, 0, 0, s_over, send_callback);
	if (0 != ret)
	{
		int err_no = WSAGetLastError();
		if (err_no != WSA_IO_PENDING)
			error_display("WSASend", err_no);
		//WSARecv에러 겹친 I/O 작업이 진행 중입니다. 는 에러로 판정하지 않아야함
	}
}
