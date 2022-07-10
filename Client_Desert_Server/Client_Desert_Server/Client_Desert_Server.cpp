#include "Client_Desert_Server.h"
#include "GameObject.h"
#include "GolemMonster.h"
#include "Session.h"
#include "SendData.h"

unordered_map<int, CSession>			clients;
list<CGameObject*>						objects; // monsters & objects

unordered_map<WSAOVERLAPPED*, int>		over_to_session;
CGameTimer	m_GameTimer;
mutex		timer_lock;
char		recv_buf[BUFSIZE];

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
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

void Init_Monsters();

void TimerThread_func()
{
	float	fGolemCreateTime = 0.f;
	bool	bGolemCreateOn = false;

	while (true)
	{
		m_GameTimer.Tick(60.0f);
		float fTimeElapsed = m_GameTimer.GetTimeElapsed();

		for (auto& object : objects) {
			object->Update(fTimeElapsed);
		}
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

void process_packet(int c_id)
{
	char* packet = clients[c_id]._c_mess;
	switch (packet[0])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		clients[c_id].send_login_packet();

		if(c_id == 1)
			Init_Monsters();
		
		// 다른 플레이어에게 알림
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;
			cl.second.send_add_object(c_id);
		}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    

		// 다른 플레이어 가져옴
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;
			clients[c_id].send_add_object(cl.first);
		}
		break;

	}
	case CS_MOVE:
	{
		// 받은 데이터로 서버 갱신
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]._pObject->m_xmf4x4World = p->xmf4x4World;
		clients[c_id]._pObject->m_eCurAnim = p->eCurAnim;
		memcpy(clients[c_id]._pObject->m_eAnimInfo, p->animInfo, sizeof(p->animInfo));

		// 객체들과 충돌체크
		if (p->eCurAnim == PLAYER::ATTACK1 || p->eCurAnim == PLAYER::ATTACK2 ||
			p->eCurAnim == PLAYER::SKILL1 || p->eCurAnim == PLAYER::SKILL2)
		{
			for (auto& object : objects)
			{
				object->CheckCollision(c_id);
			}
		}
		// 모든 클라에게 클라의 위치 전송
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;
			cl.second.send_move_packet(c_id);
			for (auto& object : objects)
			{
				object->Send_Packet_To_Clients(cl.first);
			}
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
		if(clients[c_id]._pObject)
			p.race = clients[c_id]._pObject->race;
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

	CSendData* sdata = reinterpret_cast<CSendData*>(over);
	delete sdata;

	return;
}

void Init_Monsters()
{
	CGameObject* pGolem = new CGolemMonster(0);
	objects.push_back(pGolem);
	pGolem->m_bActive = true;
}
