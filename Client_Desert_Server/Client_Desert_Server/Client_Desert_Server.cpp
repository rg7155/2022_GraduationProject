#include "Client_Desert_Server.h"
#include "GameObject.h"
#include "GolemMonster.h"
#include "CactiMonster.h"
#include "BossMonster.h"
#include "Session.h"
#include "SendData.h"

unordered_map<int, CSession>				clients;
list<CGameObject*>							objects[OBJECT_END]; // monsters & objects
unordered_map<string, BoundingOrientedBox>	oobbs;
unordered_map<string, vector<float>>		animTimes;
int											g_Scene = SCENE_0;

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

void Init_Monsters_Scene1();
void LoadingBoundingBox();
void LoadingAnimTime();
void Init_Objects_Scene2();

void TimerThread_func()
{
	while (true)
	{
		m_GameTimer.Tick(60.0f);
		float fTimeElapsed = m_GameTimer.GetTimeElapsed();
		timer_lock.lock();
		for (int i = 0; i < OBJECT::OBJECT_END; i++)
		{
			for (auto& object : objects[i]) {
				object->Update(fTimeElapsed);
			}
		}
		timer_lock.unlock();

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
		clients[c_id]._pObject->m_xmLocalOOBB = oobbs["Player"];

		if(c_id == 1)
			Init_Monsters_Scene1();
		
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

		// 공격력 업뎃!
		if (PLAYER::ATTACK1 == clients[c_id]._pObject->m_eCurAnim || PLAYER::ATTACK2 == clients[c_id]._pObject->m_eCurAnim) {
			clients[c_id]._pObject->m_att = rand() % 51 + 50;
		}
		else if (PLAYER::SKILL1 == clients[c_id]._pObject->m_eCurAnim || PLAYER::SKILL2 == clients[c_id]._pObject->m_eCurAnim) {
			clients[c_id]._pObject->m_att = rand() % 151 + 150;
		}
		timer_lock.lock();

		for (auto& object : objects[OBJECT_MONSTER])
		{
			object->CheckCollision(c_id);
		}
		timer_lock.unlock();

		bool bFoot[2]{};
		if (g_Scene == SCENE_2) {
			// 발판 충돌체크
			for (auto& cl : clients)
			{
				int cnt = 0;
				timer_lock.lock();
				for (auto& object : objects[OBJECT_FOOTHOLD])
				{
					XMFLOAT3 xmf3Pos = object->GetPosition();
					float fDis = Vector3::Distance(object->GetPosition(), cl.second._pObject->GetPosition());
					if (fDis < 1.f)
						bFoot[cnt] = true;
					cnt++;
				}
				timer_lock.unlock();
			}
		}
		// 모든 클라에게 클라의 위치 전송
		for (auto& cl : clients)
		{
			
			if (cl.first == c_id) continue;
			cl.second.send_move_packet(c_id);
			if(g_Scene == SCENE_2)
			{
				SC_FOOTHOLD_PACKET foot_packet;
				foot_packet.size = sizeof(SC_FOOTHOLD_PACKET);
				foot_packet.type = SC_FOOTHOLD;
				foot_packet.flag1 = bFoot[0];
				foot_packet.flag2 = bFoot[1];
				cl.second.do_send(foot_packet.size, reinterpret_cast<char*>(&foot_packet));
			}
			timer_lock.lock();
			for (int i = 0; i < OBJECT::OBJECT_END; ++i) {
				if (i == OBJECT_FOOTHOLD)
					continue;

				for (auto iter = objects[i].begin(); iter != objects[i].end();)
				{
					if (!(*iter)->m_bActive) {
						(*iter)->Send_Remove_Packet_To_Clients(cl.first);
						iter = objects[i].erase(iter);

					}
					else {
						(*iter)->Send_Packet_To_Clients(cl.first);
						iter++;
					}
				}
			}
			timer_lock.unlock();

		}
		break;
	}
	case CS_READY:
	{
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		// 씬전환 관리
		if (SCENE_1 == g_Scene) {
			g_Scene = SCENE_2;
			// 씬2 객체 생성
			Init_Objects_Scene2();
			break;
		}
		else if (clients[c_id]._isReady != p->bReady) {
			clients[c_id]._isReady = p->bReady;
			// 두 플레이어 모두 Ready면 Scene전환
			bool bAll = true;
			for (auto& cl : clients)
			{
				if (!cl.second._isReady)
					bAll = false;
				if (cl.first == c_id) continue;
					clients[c_id].send_ready_packet(cl.first);
			}
			if (bAll)
				g_Scene = SCENE_1;
		}
		break;
	}
	case CS_NPC:
	{
		// 상대플레이어에게 알림
		for (auto& cl : clients)
		{
			if (cl.first == c_id) continue;
			cl.second.send_npc_packet();
		}
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
		p.race = RACE_PLAYER;
		if(clients[c_id]._pObject)
			p.race = clients[c_id]._pObject->m_race;
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

constexpr XMFLOAT3 FOOTHOLD1 = XMFLOAT3(22.11f, 0.f, 28.77f);
constexpr XMFLOAT3 FOOTHOLD2 = XMFLOAT3(18.95f, 0.f, 9.42f);

void Init_Monsters_Scene1()
{
	LoadingBoundingBox();
	LoadingAnimTime();

	CGameObject* pGolem = new CGolemMonster(0);
	pGolem->m_xmLocalOOBB = oobbs["Golem"];

	CGameObject* pCacti1 = new CCactiMonster(0);
	pCacti1->m_xmLocalOOBB = oobbs["Cacti"];

	CGameObject* pCacti2 = new CCactiMonster(1);
	pCacti2->m_xmLocalOOBB = oobbs["Cacti"];

	reinterpret_cast<CCactiMonster*>(pCacti1)->m_pCacti = reinterpret_cast<CCactiMonster*>(pCacti2);
	reinterpret_cast<CCactiMonster*>(pCacti2)->m_pCacti = reinterpret_cast<CCactiMonster*>(pCacti1);

	//timer_lock.lock();
	objects[OBJECT::OBJECT_MONSTER].push_back(pGolem);
	objects[OBJECT::OBJECT_MONSTER].push_back(pCacti1);
	objects[OBJECT::OBJECT_MONSTER].push_back(pCacti2);
	//timer_lock.unlock();

	pGolem->m_bActive = true;
}

void LoadingBoundingBox()
{
	// bound 로딩
	ifstream in{ "Bounds/bounds.txt" };
	string str;
	BoundingOrientedBox OOBB;
	while (!in.eof())
	{
		in >> str;
		in >> OOBB.Center.x >> OOBB.Center.y >> OOBB.Center.z
			>> OOBB.Extents.x >> OOBB.Extents.y >> OOBB.Extents.z;
		oobbs.try_emplace(str, OOBB);
	}
}

void LoadingAnimTime()
{
	// bound 로딩
	ifstream in{ "AnimTime/AnimTime.txt" };

	string str;
	int num, index = 0;
	float time = 0.f;
	while (!in.eof())
	{
		in >> str >> num;
		for (int i = 0; i < num; i++)
		{
			in >> index >> time;
			animTimes[str].push_back(time);
		}
	}
}

void Init_Objects_Scene2()
{
	// 발판 2개 로딩
	CGameObject* pFootHold1 = new CGameObject();
	pFootHold1->SetPosition(FOOTHOLD2.x, FOOTHOLD2.y, FOOTHOLD2.z);
	CGameObject* pFootHold2 = new CGameObject();
	pFootHold2->SetPosition(FOOTHOLD1.x, FOOTHOLD1.y, FOOTHOLD1.z);

	objects[OBJECT::OBJECT_FOOTHOLD].push_back(pFootHold1);
	objects[OBJECT::OBJECT_FOOTHOLD].push_back(pFootHold2);

	// Boss 1마리 로딩
	CGameObject* pBoss = new CBossMonster();
	pBoss->m_xmLocalOOBB = oobbs["Boss"];
	objects[OBJECT::OBJECT_MONSTER].push_back(pBoss);

}

