#include "Client_Desert_Server.h"
#include "Player.h"
#include "Protocol.h"
class SESSION;


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

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };

class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char send_buf[BUFSIZE];
	char _comp_type;	

	OVER_EXP()
	{
		_wsabuf.len = BUFSIZE;
		_wsabuf.buf = send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = send_buf;
		_comp_type = OP_SEND;

		ZeroMemory(&_over, sizeof(_over));
		memcpy(send_buf, packet, packet[0]);
	}
};

class SESSION
{
	OVER_EXP _recv_over;

public:
	bool	in_use;
	int		_id;
	SOCKET	_socket;
	char	_name[NAME_SIZE];
	int		_prev_remain;

public:
	CPlayer* _pPlayer;

public:
	SESSION()
	{
		_id = -1;
		_socket = 0;
		_name[0] = 0;
		in_use = 0;
		_prev_remain = 0;
		_pPlayer = new CPlayer;
		_pPlayer->Initialize();
	}
	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUFSIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over.send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);

	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.xmf4x4World = _pPlayer->m_xmf4x4World;
		memcpy(p.animInfo, _pPlayer->m_eAnimInfo, sizeof(player_anim) * ANIM::END);
		do_send(&p);
	}

	void send_move_packet(int c_id);

};

array<SESSION, MAX_USER> clients;

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id; // _id로 해서 오류 났었음
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.xmf4x4World = _pPlayer->m_xmf4x4World;
	do_send(&p);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].in_use == false)
			return i;
	}
	return -1;

}

void process_packet(int c_id, char* packet)
{
	// 프로토콜을 클라와 같이 정의해야한다. 헤더파일에 정의하도록

	switch (packet[1])
	{
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();

		// 다른플레이어에게 접속했다고 알림
		for (auto& pl : clients)
		{
			if (!pl.in_use) continue;
			if (pl._id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = c_id;
			strcpy_s(add_packet.name, p->name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.xmf4x4World = clients[c_id]._pPlayer->m_xmf4x4World;
			memcpy(add_packet.animInfo, clients[c_id]._pPlayer->m_eAnimInfo, sizeof(player_anim) * ANIM::END);
			pl.do_send(&add_packet);
		}
		for (auto& pl : clients)
		{
			if (!pl.in_use) continue;
			if (pl._id == c_id) continue;
			SC_ADD_PLAYER_PACKET add_packet;
			add_packet.id = pl._id;
			strcpy_s(add_packet.name, pl._name);
			add_packet.size = sizeof(add_packet);
			add_packet.type = SC_ADD_PLAYER;
			add_packet.xmf4x4World = pl._pPlayer->m_xmf4x4World;
			memcpy(add_packet.animInfo, pl._pPlayer->m_eAnimInfo, sizeof(player_anim) * ANIM::END);
			clients[c_id].do_send(&add_packet);
		}
		break;
	}
	case CS_MOVE: {
		// 일단 받아서 넘겨주는 것만 수행
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		clients[c_id]._pPlayer->m_xmf4x4World = p->xmf4x4World;
		memcpy(clients[c_id]._pPlayer->m_eAnimInfo, p->animInfo, sizeof(player_anim) * ANIM::END);
		for (auto& pl : clients)
			if (true == pl.in_use)
				pl.send_move_packet(c_id);

		clients[c_id].send_move_packet(c_id);
		break;
	}

	default:
		break;
	}

}

void disconnect(int c_id)
{
	for (auto& pl : clients)
	{
		if (pl.in_use == false)
			continue;
		if (pl._id == c_id) continue;
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);
	}
	closesocket(clients[c_id]._socket);
	clients[c_id].in_use = false;
}

int main()
{
	HANDLE h_iocp;

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

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server), h_iocp, 9999, 0); // 동접량

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;

	a_over._comp_type = OP_ACCEPT;
	AcceptEx(server, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	while (true)
	{
		DWORD num_bytes;
		ULONG_PTR key;
		OVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		if (FALSE == ret)
		{
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept\n";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(key);
				if (ex_over->_comp_type == OP_SEND)
					continue;
			}
		}

		switch (ex_over->_comp_type)
		{
		case OP_ACCEPT:
		{
			int client_id = get_new_client_id();
			if (client_id != -1)
			{
				clients[client_id].in_use = true;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, client_id, 0);

				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

			}
			else
			{
				cout << "Max User exceeded.\n";
			}
			ZeroMemory(&a_over._over, sizeof(a_over._over));
			// 서버를 iocp에 등록해야함. -> 핸들 필요
			AcceptEx(server, c_socket, a_over.send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);
			break;
		}
		case OP_RECV:
		{
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->send_buf;
			while (remain_data > 0)
			{
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					// 여러 개가 뭉쳐서 올 수도, 잘려서 올 수도 있음.
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}

		case OP_SEND:
			delete ex_over;
			break;
		default:
			break;
		}
	}

	closesocket(server);
	WSACleanup();
}
