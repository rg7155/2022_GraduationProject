#include "ServerManager.h"
#include "GameMgr.h"
#include "GameFramework.h"

IMPLEMENT_SINGLETON(CServerManager)

SOCKET	CServerManager::m_socket;
WSABUF	CServerManager::m_wsabuf_r;
char	CServerManager::m_recv_buf[BUFSIZE];
WSABUF	CServerManager::m_wsabuf_s;
char*	CServerManager::m_send_buf = nullptr;
char	CServerManager::m_prev_buf[BUFSIZE];
int		CServerManager::m_prev_bytes = 0;
bool	CServerManager::m_isWindow = false;
int		CServerManager::m_myid = -1;
CGameFramework* CServerManager::gameFramework;

CServerManager::CServerManager()
{
}
CServerManager::~CServerManager()
{
}

void CServerManager::send_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	delete lpOverlapped;
	//delete m_send_buf;
}

void CServerManager::recv_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	//// ��Ŷ �߷����°� ó���ؾ���.
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

	recv_packet();
}

void CServerManager::recv_packet()
{
	m_wsabuf_r.buf = m_recv_buf;
	m_wsabuf_r.len = BUFSIZE;
	DWORD recv_flag = 0;
	WSAOVERLAPPED* r_over = new WSAOVERLAPPED;
	ZeroMemory(r_over, sizeof(WSAOVERLAPPED));

	int ret = WSARecv(m_socket, &m_wsabuf_r, 1, 0, &recv_flag, r_over, recv_callback);

	if (0 != ret) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
			error_display("WSASend", err);
	}
}

void CServerManager::send_packet(void* packet)
{
	char* p = reinterpret_cast<char*>(packet);
	size_t sent = 0;
	WSABUF wsabuf;
	wsabuf.buf = p;
	wsabuf.len = static_cast<unsigned char>(p[0]);
	m_send_buf = p;
	WSAOVERLAPPED* s_over = new WSAOVERLAPPED;
	ZeroMemory(s_over, sizeof(WSAOVERLAPPED));

	int ret = WSASend(m_socket, &wsabuf, 1, 0, 0, s_over, send_callback);
	if (0 != ret) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
			error_display("WSASend", err);
	}
}

void CServerManager::send_login_packet()
{
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	std::cout << "��� Ŭ�� �����" << endl;
	send_packet(&p);
}

void CServerManager::Connect()
{
	string ip;
	std::cout << "IP�� �Է��ϼ���:";
	std::cin >> ip;

	if (ip == "1")
		ip = "127.0.0.1";

	std::wcout.imbue(locale("korean")); // ���� �޼��� �ѱ۷� ���
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData); // ���� ��Ʈ��ŷ ���� - �����츸
	m_socket = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	std::ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr);
	int ret = connect(m_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
}

int CServerManager::ProcessPacket(char* packet)
{
	// type�� ��
	switch (packet[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		m_myid = p->id;
		CGameMgr::GetInstance()->SetId(m_myid);
		gameFramework->BuildObjects();
		return p->size;

	}
	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* p = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(packet);
		gameFramework->m_pScene->m_pDuoPlayer->SetPosition(XMFLOAT3(p->x, 0.f, p->z));
		gameFramework->m_pScene->m_pDuoPlayer->SetActiveState(true);
		std::cout << "��� Ŭ�� ����!" << endl;
		m_isWindow = true;
		ShowWindow(g_hWnd, m_isWindow);

		return p->size;
	}
	case SC_MOVE_OBJECT:
	{
		// id�� �ش��ϴ� �÷��̾ �Űܾ��ϴµ� 1�� �ۿ� ������ �׳� Ȯ�� ���ϰ� �ֱ�?
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
