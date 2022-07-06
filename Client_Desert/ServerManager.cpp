#include "ServerManager.h"

IMPLEMENT_SINGLETON(CServerManager)

SOCKET	CServerManager::m_s_socket;
WSABUF	CServerManager::m_wsabuf_r;
char	CServerManager::m_recv_buf[BUFSIZE];
WSABUF	CServerManager::m_wsabuf_s;
char*	CServerManager::m_send_buf = nullptr;
char	CServerManager::m_prev_buf[BUFSIZE];
int		CServerManager::m_prev_bytes = 0;
bool	CServerManager::m_isWindow = false;
int		CServerManager::m_myid = -1;

CServerManager::CServerManager()
{
}
CServerManager::~CServerManager()
{
}

void CServerManager::send_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	delete lpOverlapped;
	delete m_send_buf;
}

void CServerManager::recv_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
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

	char* m_start = m_recv_buf;
	while (true)
	{
		int msg_size = static_cast<unsigned char>(m_start[0]);

		ProcessPacket(m_start);

		if (cbTransferred < msg_size)
		{
			std::cout << "recv_callback Error" << endl;
			std::cout << msg_size << endl;
			std::cout << cbTransferred << endl;
			std::cout << m_start[0] << endl;
			break;
		}

		cbTransferred -= msg_size;
		if (0 >= cbTransferred) break;
		m_start += msg_size;
	}
	delete lpOverlapped;

	RecvProcess();
}

void CServerManager::RecvProcess()
{
}

void CServerManager::SendProcess()
{
}

void CServerManager::send_packet(void* packet)
{
}

void CServerManager::Connect()
{
	string ip;
	std::cout << "IP를 입력하세요:";
	std::cin >> ip;

	if (ip == "1")
		ip = "127.0.0.1";

	std::wcout.imbue(locale("korean")); // 에러 메세지 한글로 출력
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData); // 소켓 네트워킹 시작 - 윈도우만
	m_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	std::ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
	int ret = connect(m_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	CS_LOGIN_PACKET* p = new CS_LOGIN_PACKET;
	p->size = sizeof(CS_LOGIN_PACKET);
	p->type = CS_LOGIN;
	//strcpy_s(p->name, "PLAYER");


	//// 위치 받기
	//Server_PosRecv();
}

int CServerManager::ProcessPacket(char* packet)
{
	// type을 비교
	switch (packet[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		m_myid = p->id;
	/*	gGameFramework.m_iId = g_myid;
		CGameMgr::GetInstance()->SetId(g_myid);
		gGameFramework.BuildObjects();
		CGameMgr::GetInstance()->GetPlayer()->m_iId = g_myid;*/
		return p->size;

	}
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* p = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);
		/*gGameFramework.m_pScene->m_pDuoPlayer->SetPosition(XMFLOAT3(p->x, 0.f, p->z));
		gGameFramework.m_pScene->m_pDuoPlayer->SetActiveState(true);
		std::cout << "상대 클라 접속!" << endl;
		isWindow = true;
		ShowWindow(g_hWnd, isWindow);*/

		return p->size;
	}
	case SC_MOVE_OBJECT:
	{
		// id에 해당하는 플레이어를 옮겨야하는데 1명 밖에 없으니 그냥 확인 안하고 넣기?
		SC_MOVE_OBJECT_PACKET* p;
		p = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(packet);
		//gGameFramework.m_pScene->m_pDuoPlayer->Server_SetParentAndAnimation(p);
		return p->size;
	}
	//case SC_MOVE_MONSTER:
	//{
	//	SC_MOVE_MONSTER_PACKET* p;
	//	p = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(ptr);
	//	CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Golem").front();
	//	CGolemObject* pGolem = reinterpret_cast<CGolemObject*>(pObj);

	//	if (!pGolem->m_isActive)
	//		pGolem->SetActiveState(true);

	//	pGolem->Change_Animation(p->eCurAnim);
	//	pGolem->SetLookAt(p->xmf3Look);
	//	pGolem->SetPosition(p->xmf3Position);
	//	pGolem->m_targetId = p->target_id;
	//	pGolem->SetHp(static_cast<int>(p->hp));
	//	//cout << static_cast<int>(p->hp) << endl;
	//	return p->size;
	//}
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* p = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet);
		//gGameFramework.m_pScene->m_pDuoPlayer->SetActiveState(false);
		return p->size;
	}
	default:
		break;
	}
}

void CServerManager::error_display(const char* msg, int err_no)
{
}
