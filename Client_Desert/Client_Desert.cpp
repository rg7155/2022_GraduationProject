// LabProject08-6-7-2.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Client_Desert.h"
#include "GameFramework.h"
#include "Monster.h"
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
char SERVER_ADDR[BUFSIZE] = /*"211.109.112.11"*//*"210.99.123.127"*/ "127.0.0.1";
SOCKET s_socket;
WSABUF wsabuf_r;
char recv_buf[BUFSIZE];
WSABUF wsabuf_s;
int g_myid = -1;
char* send_buf = nullptr;
char prev_buf[BUFSIZE];
int prev_bytes = 0;

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

	CS_LOGIN_PACKET* p = new CS_LOGIN_PACKET;
	p->size = sizeof(CS_LOGIN_PACKET);
	p->type = CS_LOGIN;
	strcpy_s(p->name, "PLAYER");
	send_packet(p);

	//// 위치 받기
	Server_PosRecv();

#endif // USE_SERVER

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_CLIENTDESERT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTDESERT));


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

			if (g_myid != -1)
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

int Process_Packet(char* ptr)
{
	// type을 비교
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = p->id;
		gGameFramework.m_iId = g_myid;
		gGameFramework.BuildObjects();
		gGameFramework.m_pPlayer->m_iId = g_myid;
		CGameMgr::GetInstance()->SetId(g_myid);
		return p->size;

	}
	case SC_ADD_PLAYER: 
	{
		SC_ADD_PLAYER_PACKET* p = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		gGameFramework.m_pScene->m_pDuoPlayer->SetPosition(XMFLOAT3(p->x, 0.f, p->z));
		gGameFramework.m_pScene->m_pDuoPlayer->SetActiveState(true);
		return p->size;
	}
	case SC_MOVE_PLAYER:
	{
		// id에 해당하는 플레이어를 옮겨야하는데 1명 밖에 없으니 그냥 확인 안하고 넣기?
		SC_MOVE_PLAYER_PACKET* p;
		p = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		gGameFramework.m_pScene->m_pDuoPlayer->Server_SetParentAndAnimation(p);
		return p->size;
	}
	case SC_MOVE_MONSTER:
	{
		SC_MOVE_MONSTER_PACKET* p;
		p = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(ptr);
		CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Golem").front();
		CGolemObject* pGolem = reinterpret_cast<CGolemObject*>(pObj);
		pGolem->Change_Animation(p->eCurAnim);
		pGolem->SetLookAt(p->xmf3Look);
		pGolem->SetPosition(p->xmf3Position);
		pGolem->m_targetId = p->target_id;
		return p->size;
	}
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* p = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		gGameFramework.m_pScene->m_pDuoPlayer->SetActiveState(false);
		return p->size;
	}
	default:
		break;
	}
}
void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	//// 패킷 잘려오는거 처리해야함.
	//
	//if (prev_bytes)
	//{
	//	//memcpy(recv_buf,prev_buf, prev_bytes);
	//	char* p = recv_buf;

	//	memcpy(recv_buf, prev_buf, prev_bytes);
	//	memcpy(recv_buf + prev_bytes, p, cbTransferred);
	//}
	//int remain_data = cbTransferred + prev_bytes;

	//char* p = recv_buf;
	//while (remain_data > 0)
	//{
	//	int packet_size = static_cast<unsigned char>(p[0]);
	//	if (packet_size == 0)
	//	{
	//		int a;
	//		a = packet_size;
	//	}
	//	cout << packet_size << endl;
	//	if (packet_size <= remain_data)
	//	{
	//		Process_Packet(p);
	//		p = p + packet_size;
	//		remain_data -= packet_size;
	//	}
	//	else break;
	//}
	//prev_bytes = remain_data;
	//if (remain_data > 0)
	//{
	//	memcpy(prev_buf, p, remain_data);
	//	cout << "cut"<< remain_data << endl;
	//}

	char* m_start = recv_buf;
	while (true)
	{
		int msg_size = static_cast<unsigned char>(m_start[0]);
		
		Process_Packet(m_start);

		if (cbTransferred < msg_size)
		{
			cout << "recv_callback Error" << endl;
			cout << msg_size << endl;
			cout << cbTransferred << endl;
			cout << m_start[0] << endl;
			break;
		}

		cbTransferred -= msg_size;
		if (0 >= cbTransferred) break;
		m_start += msg_size;
	}
	delete lpOverlapped;


	if (g_myid != -1)
		Server_PosSend();

	Server_PosRecv();



}

void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	delete lpOverlapped;
	delete send_buf;
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

void send_packet(void* packet)
{
	char* p = reinterpret_cast<char*>(packet);
	size_t sent = 0;
	WSABUF mybuf;
	mybuf.buf = p;
	mybuf.len = static_cast<unsigned char>(p[0]);
	send_buf = p;
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
void Server_PosSend()
{	
	// 버퍼에 duoPlayer 넣기
	CS_MOVE_PACKET *p = gGameFramework.m_pPlayer->Server_GetParentAndAnimation();
	p->size = sizeof(CS_MOVE_PACKET);
	p->type = CS_MOVE;
	send_packet(p);
}

