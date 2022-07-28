#include "ServerManager.h"
#include "GameMgr.h"
#include "GameFramework.h"
#include "Monster.h"

IMPLEMENT_SINGLETON(CServerManager)

SOCKET					CServerManager::m_socket;
char					CServerManager::m_recv_buf[BUFSIZE];
WSABUF					CServerManager::m_wsabuf_r;
bool					CServerManager::m_isWindow = false;
int						CServerManager::m_myid = -1;
//queue<char*>			CServerManager::m_queueSendBuf;
bool					CServerManager::m_bFoot[2]{};

queue<char>				CServerManager::m_queue_send_packet;
CGameFramework*			CServerManager::gameFramework;

CServerManager::CServerManager()
{
}
CServerManager::~CServerManager()
{
}

void CServerManager::send_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	delete lpOverlapped;
}

void CServerManager::recv_callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	char* m_start = m_recv_buf;
	while (true)
	{
		int* size = reinterpret_cast<int*>(&m_start[1]);

		if (cbTransferred < *size || !*size) {

			cout << "받은 데이터가 더 작음!" << endl;
			cout << cbTransferred << ' ' << *size << endl;
			break;
		}
		int msg_size = ProcessPacket(m_start);

		cbTransferred -= msg_size;
		if (0 >= cbTransferred) break;
		m_start += msg_size;
		// 30 171
	}
	delete lpOverlapped;
	//CServerManager::GetInstance()->send_anim_change_packet();


	char dir = CGameMgr::GetInstance()->GetPlayer()->m_dir;
	CServerManager::GetInstance()->send_move_packet(dir);

	// 큐에 있는거 send
	while (!m_queue_send_packet.empty())
	{
		char type = m_queue_send_packet.front();
		switch (type)
		{
		case CS_READY:
			send_ready_packet();
			break;
		case CS_NPC:
			send_npc_packet();
			break;
		default:
			break;
		}
		m_queue_send_packet.pop();
	}

	recv_packet();

	
}

void CServerManager::recv_packet()
{
	ZeroMemory(m_recv_buf, sizeof(m_recv_buf));

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

void CServerManager::send_packet(void* packet, int size)
{
	char* p = reinterpret_cast<char*>(packet);
	size_t sent = 0;
	WSABUF wsabuf;
	wsabuf.buf = p;
	wsabuf.len = size;
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
	std::cout << "상대 클라 대기중" << endl;
	send_packet(&p, p.size);
}

void CServerManager::send_move_packet(char dir)
{
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();

	CS_MOVE_PACKET p;
	p.size = sizeof(CS_MOVE_PACKET);
	p.type = CS_MOVE;
	p.xmf4x4World = pPlayer->m_xmf4x4ToParent;
	p.eCurAnim = pPlayer->GetCurAnim();
	pPlayer->Set_object_anim(p.animInfo);
	send_packet(&p, p.size);
}

void CServerManager::send_ready_packet()
{
	CPlayer* pPlayer = CGameMgr::GetInstance()->GetPlayer();

	CS_READY_PACKET p;
	p.size = sizeof(CS_READY_PACKET);
	p.type = CS_READY;
	p.bReady = pPlayer->m_isReadyToggle;
	send_packet(&p, p.size);
}

void CServerManager::send_npc_packet()
{
	CS_NPC_PACKET p;
	p.size = sizeof(CS_NPC_PACKET);
	p.type = CS_NPC;
	send_packet(&p, p.size);

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
	// type을 비교
	switch (packet[0])
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
		// 플레이어
		if (p->id < 2) {
			gameFramework->m_pScene->m_pDuoPlayer->SetActiveState(true);
			std::cout << "상대 클라 접속!" << endl;
			m_isWindow = true;
			ShowWindow(g_hWnd, m_isWindow);
		}
		// 몬스터
		return p->size;
	}
	case SC_MOVE_OBJECT:
	{
		// id에 해당하는 플레이어를 옮겨야하는데 1명 밖에 없으니 그냥 확인 안하고 넣기?
		SC_MOVE_OBJECT_PACKET* p = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(packet);
		if (p->id < 2) {
			CDuoPlayer* pDuoPlayer = gameFramework->m_pScene->m_pDuoPlayer;
			pDuoPlayer->m_xmf4x4ToParent = p->xmf4x4World;
			pDuoPlayer->Update_object_anim(p->animInfo);
			pDuoPlayer->m_eCurAnim = (PLAYER::ANIM)p->eCurAnim;
		}

		return p->size;
	}
	case SC_STAT_CHANGE:
	{
		SC_STAT_CHANGE_PACKET* p;
		p = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(packet);
		//gameFramework->m_pScene->m_pDuoPlayer->set_stat_change(p);
		return p->size;
	}
	case SC_MOVE_MONSTER:
	{
		SC_MOVE_MONSTER_PACKET* p = reinterpret_cast<SC_MOVE_MONSTER_PACKET*>(packet);
		if (p->race == RACE_GOLEM) {
			CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Golem").front();
			CGolemObject* pGolem = reinterpret_cast<CGolemObject*>(pObj);

			if (!pGolem->m_isActive)
				pGolem->SetActiveState(true);

			pGolem->Change_Animation((GOLEM::ANIM)p->eCurAnim);
			pGolem->SetLookAt(p->xmf3Look);
			pGolem->SetPosition(p->xmf3Position);
			pGolem->m_targetId = p->target_id;
			pGolem->SetHp(static_cast<int>(p->hp));
			//cout << static_cast<int>(p->hp) << endl;
		}
		else if (p->race == RACE_CACTI) {
			// id: 0,1
			CGameObject* pObj;
			if (p->id == 0) {
				pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cacti").front();
			}
			else
				pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cacti").back();

			CCactiObject* pCacti = reinterpret_cast<CCactiObject*>(pObj);
			pCacti->m_index = p->id;
			pCacti->Change_Animation((CACTI::ANIM)p->eCurAnim);
			//pCacti->SetLookAt(p->xmf3Look);
			pCacti->SetPosition(p->xmf3Position);
			pCacti->SetHp(static_cast<int>(p->hp));
			// verse 바뀌면
			if ((p->verse != pCacti->m_nowVerse) && (p->verse == VERSE3)) {
				pCacti->m_nowVerse = p->verse;
				pCacti->Rotate(0.f, 180.f, 0.f);
			}
		}
		else if (p->race == RACE_CACTUS) {
			CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cactus").front();
			CCactusObject* pCactus = reinterpret_cast<CCactusObject*>(pObj);
			if (!pCactus->m_isActive)
				pCactus->SetActiveState(true);
			pCactus->Change_Animation((CACTUS::ANIM)p->eCurAnim);
			pCactus->SetHp(static_cast<int>(p->hp));

		}
		else if (p->race == RACE_BOSS) {
			CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Boss").front();
			CBossObject* pBoss = reinterpret_cast<CBossObject*>(pObj);
			if (!pBoss->m_isActive)
				pBoss->SetActiveState(true);
			pBoss->Change_Animation((BOSS::ANIM)p->eCurAnim);
			pBoss->SetHp(static_cast<int>(p->hp));
			if (p->verse == VERSE2) {
				pBoss->SetLookAt(p->xmf3Look);
				pBoss->m_targetId = p->target_id;
			}
			if (p->verse == VERSE3) {
				pBoss->m_targetId = p->target_id;
				pBoss->SetLookAt(p->xmf3Look);
				pBoss->SetPosition(p->xmf3Position);
			}
		}
		
		return p->size;
	}
	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* p = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(packet);
		if(p->race == RACE_PLAYER)
			gameFramework->m_pScene->m_pDuoPlayer->SetActiveState(false);
		else if (p->race == RACE_CACTI) {
			if (p->id == 0) {
				CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cacti").front()->SetActiveState(false);
			}
			else
				CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cacti").back()->SetActiveState(false);
		}
		else if (p->race == RACE_CACTUS) {
			CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Cactus").front();
			CCactusObject* pCactus = reinterpret_cast<CCactusObject*>(pObj);
			pCactus->Change_Animation(CACTUS::ANIM::DIE);
		}
		else if (p->race == RACE_BOSS) {
			CGameObject* pObj = CGameMgr::GetInstance()->GetScene()->m_pMonsterObjectShader->GetObjectList(L"Boss").front();
			CBossObject* pBoss = reinterpret_cast<CBossObject*>(pObj);
			pBoss->SetHp(0);
			pBoss->Change_Animation(BOSS::ANIM::DIE);
		}
		return p->size;
	}
	case SC_FOOTHOLD:
	{
		SC_FOOTHOLD_PACKET* p = reinterpret_cast<SC_FOOTHOLD_PACKET*>(packet);
		m_bFoot[0] = p->flag1;
		m_bFoot[1] = p->flag2;
		return p->size;
	}
	case SC_READY:
	{
		SC_READY_PACKET* p = reinterpret_cast<SC_READY_PACKET*>(packet);
		gameFramework->m_pScene->m_pDuoPlayer->m_isReadyToggle = p->bReady;
		return p->size;
	}
	case SC_NPC:
	{
		SC_NPC_PACKET* p = reinterpret_cast<SC_NPC_PACKET*>(packet);
		CGameMgr::GetInstance()->GetScene()->AddTextToUILayer(NPC_TEXT);
		return p->size;
	}
	case SC_DAMAGED:
	{
		SC_DAMAGED_PACKET* p = reinterpret_cast<SC_DAMAGED_PACKET*>(packet);
		CGameMgr::GetInstance()->GetPlayer()->SetDamaged();
		return p->size;

	}
	default:
		break;
	}

	return 0;
}

void CServerManager::error_display(const char* msg, int err_no)
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
