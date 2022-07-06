#include "Client_Desert_Server.h"
#include "Protocol.h"
#include "Timer.h"
class SESSION;

unordered_map<int, SESSION>			clients;
unordered_map<int, SESSION>			monsters;
unordered_map<WSAOVERLAPPED*, int>	over_to_session;
CGameTimer m_GameTimer;
mutex timer_lock;

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

char recv_buf[BUFSIZE];

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



class SEND_DATA {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char send_buf[BUFSIZE];

	SEND_DATA(int size, char* n_data )
	{
		// size id data 순으로 보낸다.
		_wsabuf.len = size;
		_wsabuf.buf = send_buf;
		ZeroMemory(&_over, sizeof(_over));
		ZeroMemory(&send_buf, BUFSIZE);
		memcpy(send_buf, n_data, size);
	}
};

class SESSION
{
	WSAOVERLAPPED _c_over;
	WSABUF	_c_wsabuf[1];

public:
	int		_id;
	char	race;
	float	x, y, z;
	char	direction;
	int		hp, hpmax;
	//char	anim;

public:
	SOCKET	_socket;
	CHAR	_c_mess[BUFSIZE];

public:
	SESSION() {}
	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_c_wsabuf[0].buf = _c_mess;
		_c_wsabuf[0].len = sizeof(_c_mess);
		over_to_session[&_c_over] = id;
	}
	~SESSION() {}

	void do_recv()
	{
		// 받기전에 초기화 해준다.......
		_c_wsabuf[0].buf = _c_mess;
		_c_wsabuf[0].len = BUFSIZE;
		DWORD recv_flag = 0;
		memset(&_c_over, 0, sizeof(_c_over));

		// 키값 받음
		WSARecv(_socket, _c_wsabuf, 1, 0, &recv_flag, &_c_over, recv_callback);
	}

	void do_send(int num_bytes, char* mess)
	{
		SEND_DATA* sdata = new SEND_DATA{ num_bytes, mess };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, send_callback);

	}

	void send_login_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x = 25.f;
		p.y = y = 0.f;
		p.z = z = 25.f;
		do_send(p.size, reinterpret_cast<char*>(&p));
	}

	void send_move_packet(int c_id)
	{
		// 위치, 이동방향만 보내는걸루
		SC_MOVE_OBJECT_PACKET p;
		p.id = c_id; // _id로 해서 오류 났었음
		p.size = sizeof(SC_MOVE_OBJECT_PACKET);
		p.type = SC_MOVE_OBJECT;
		p.x = x;
		p.y = y;
		p.z = z;
		p.direction = direction;

		//// 몬스터 패킷
		//if (g_pGolemMonster)
		//{
		//	SC_MOVE_MONSTER_PACKET monsterpacket;
		//	monsterpacket.id = 0;
		//	monsterpacket.type = SC_MOVE_MONSTER;
		//	monsterpacket.size = sizeof(SC_MOVE_MONSTER_PACKET);
		//	monsterpacket.eCurAnim = g_pGolemMonster->m_eCurAnim;
		//	monsterpacket.xmf3Position = g_pGolemMonster->m_xmf3Position;
		//	monsterpacket.xmf3Look = g_pGolemMonster->m_xmf3Look;
		//	monsterpacket.target_id = g_pGolemMonster->m_pTarget->m_id;
		//	monsterpacket.hp = static_cast<short>(g_pGolemMonster->GetHp());

		//	char buf[BUFSIZE];
		//	memcpy(buf, &p, p.size);
		//	memcpy(buf + p.size, &monsterpacket, monsterpacket.size);
		//	int bufSize = p.size + monsterpacket.size;

		//	do_send(bufSize, p.id, buf);
		//}
		//else
		//	do_send(p.size, p.id, reinterpret_cast<char*>(&p));

	
	}
};

void TimerThread_func()
{
	while (true)
	{
		m_GameTimer.Tick(60.0f);
		float fTimeElapsed = m_GameTimer.GetTimeElapsed();

		// 몬스터 로직짜기

	/*	if(clients.size() >= 1)
			fGolemCreateTime += fTimeElapsed;

		if (!bGolemCreateOn && clients.size() >= 2)
		{
			g_pGolemMonster = new CGolemMonster(clients[0].pPlayer);
			bGolemCreateOn = true;
		}

		if (g_pGolemMonster)
		{
			g_pGolemMonster->Update(fTimeElapsed);

		}*/
	}

}


int main()
{
	srand(unsigned int(time(NULL)));

	m_GameTimer.Start();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	int client_id = 0;

	// 시간재는 용 스레드 만들어서 	m_GameTimer.Tick(60.0f); 하도록

	thread timerThread{ TimerThread_func };

	while (true)
	{
		SOCKET client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, 0, 0);
		clients.try_emplace(client_id, client_id, client); // clients[client] = SESSION{ client };
		clients[client_id].do_recv();
		client_id++;

		cout << "Client " << client_id << '\n';

	}
	timerThread.join();
	closesocket(server);
	WSACleanup();
}

//void send_GolemMonster()
//{
//	SC_MOVE_MONSTER_PACKET p;
//	p.id = 0;
//	p.type = SC_MOVE_MONSTER;
//	p.size = sizeof(SC_MOVE_MONSTER_PACKET);
//	p.eCurAnim = g_pGolemMonster->m_eCurAnim;
//	p.xmf3Position = g_pGolemMonster->m_xmf3Position;
//	p.xmf3Look = g_pGolemMonster->m_xmf3Look;
//	p.target_id = g_pGolemMonster->m_pTarget->m_id;
//	p.hp = static_cast<short>(g_pGolemMonster->GetHp());
//	for (auto& cl : clients)
//	{
//		//if (cl.first == c_id) continue;
//		mylock.lock();
//		cl.second.do_send(p.size, cl.first, reinterpret_cast<char*>(&p));
//		mylock.unlock();
//
//	}
//}

void process_packet(int c_id)
{
	char* packet = clients[c_id]._c_mess;
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		clients[c_id].send_login_packet();

		// 다른 플레이어에게 알림
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;

			SC_ADD_OBJECT_PACKET p;
			p.id = c_id;
			p.size = sizeof(SC_ADD_OBJECT_PACKET);
			p.type = SC_ADD_OBJECT;
			p.x = clients[c_id].x;
			p.y = clients[c_id].y;
			p.z = clients[c_id].z;
			p.race = clients[c_id].race;
			p.hp = clients[c_id].hp;
			p.hpmax = clients[c_id].hpmax;
			cl.second.do_send(p.size, reinterpret_cast<char*>(&p));
		}

		// 다른 플레이어 가져옴
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;

			SC_ADD_OBJECT_PACKET p;
			p.id = c_id;
			p.size = sizeof(SC_ADD_OBJECT_PACKET);
			p.type = SC_ADD_OBJECT;
			p.x = cl.second.x;
			p.y = cl.second.y;
			p.z = cl.second.z;
			p.race = cl.second.race;
			p.hp = cl.second.hp;
			p.hpmax = cl.second.hpmax;
			clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
		}
		break;

	}
	case CS_MOVE:
	{
		// 받은 데이터로 서버 갱신
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id].x = p->x; 
		clients[c_id].y = p->y; 
		clients[c_id].z = p->z;
		clients[c_id].direction = p->direction;

		//// 플레이어가 공격하면 몬스터랑 충돌체크
		//if (g_pGolemMonster && g_pGolemMonster->GetHp() > 0.f)
		//{
		//	if (p->eCurAnim == PLAYER::ATTACK1 || p->eCurAnim == PLAYER::ATTACK2 ||
		//		p->eCurAnim == PLAYER::SKILL1 || p->eCurAnim == PLAYER::SKILL2)
		//	{
		//		g_pGolemMonster->CheckCollision(clients[c_id].pPlayer);
		//	}

		//}
		
		// 모든 클라에게 클라의 위치 전송 (나를 제외-> 클라에서 이동하니까)
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;
			cl.second.send_move_packet(c_id);
		}
		break;
	}
		
	default:
		break;
	}
}

void disconnect(int c_id)
{
	// 모든 클라에게 삭제됨을 전송
	for (auto& cl : clients)
	{
		if (cl.first == c_id) continue;

		SC_REMOVE_OBJECT_PACKET p;
		p.id = c_id;
		p.size = sizeof(SC_REMOVE_OBJECT_PACKET);
		p.type = SC_REMOVE_OBJECT;
		p.race = clients[c_id].race;
		cl.second.do_send(p.size, reinterpret_cast<char*>(&p));
	}
	cout << c_id << "Client Disconnection\n";
	clients.erase(c_id);
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int client_id = over_to_session[over];
	
	if (clients.find(client_id) == clients.end())
		return;

	if (num_bytes == 0)
	{
		disconnect(client_id);
		return;
	}
	process_packet(client_id);
	clients[client_id].do_recv();
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int client_id = over_to_session[over];

	// 삭제된 클라면 over_to_session에서도 삭제해준다. over은 여기에서만 쓰니까.
	auto iter = clients.find(client_id);
	if (iter == clients.end())
		over_to_session.erase(over);

	SEND_DATA* sdata = reinterpret_cast<SEND_DATA*>(over);
	delete sdata;

	return;
}
