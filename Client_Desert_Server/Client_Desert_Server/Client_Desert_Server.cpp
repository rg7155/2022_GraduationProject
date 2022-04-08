#include "Client_Desert_Server.h"
#include "Player.h"
#include "Protocol.h"
class SESSION;

unordered_map<int, SESSION> clients;
unordered_map<WSAOVERLAPPED*, int> over_to_session;

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

	SEND_DATA(int size, int client_id, char* n_data )
	{
		// size id data 순으로 보낸다.
		_wsabuf.len = size + 2;
		_wsabuf.buf = send_buf;
		ZeroMemory(&_over, sizeof(_over));
		ZeroMemory(&send_buf, BUFSIZE);

		send_buf[0] = size + 2;
		send_buf[1] = client_id;
		memcpy(send_buf + 2, n_data, size);
	}
};

class SESSION
{
	WSAOVERLAPPED _c_over;
	WSABUF _c_wsabuf[1];
	int _id;

public:
	SOCKET	_socket;
	CHAR	_c_mess[BUFSIZE];

	CPlayer* pPlayer;

public:
	SESSION() {}
	SESSION(int id, SOCKET s) : _id(id), _socket(s)
	{
		_c_wsabuf[0].buf = _c_mess;
		_c_wsabuf[0].len = sizeof(_c_mess);
		over_to_session[&_c_over] = id;
		pPlayer = new CPlayer;
		pPlayer->Initialize();
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

	void do_send(int num_bytes, int client_id, char* mess)
	{
		SEND_DATA* sdata = new SEND_DATA{ num_bytes, client_id, mess };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, send_callback);

	}
};

int main()
{
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
	while (true)
	{
		SOCKET client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, 0, 0);
		clients.try_emplace(client_id, client_id, client); // clients[client] = SESSION{ client };
		clients[client_id].do_recv();
		client_id++;

		cout << "Client " << client_id << '\n';

	}

	closesocket(server);
	WSACleanup();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int client_id = over_to_session[over];
	if (clients.find(client_id) == clients.end())
		return;

	duoPlayer* duoPl = reinterpret_cast<duoPlayer*>(clients[client_id]._c_mess);

	if (num_bytes == 0)
	{
		// 모든 클라에게 삭제됨을 전송
		for (auto& cl : clients)
		{
			duoPl->xmf4x4World = clients[client_id].pPlayer->m_xmf4x4World;
			memcpy(duoPl->animInfo, clients[client_id].pPlayer->m_eAnimInfo, sizeof(player_anim) * ANIM::END);
			cl.second.do_send(sizeof(duoPlayer), client_id, reinterpret_cast<char*>(duoPl));
		}
		cout << client_id << "Client Disconnection\n";
		clients.erase(client_id);
		return;
	}
	clients[client_id].pPlayer->m_xmf4x4World = duoPl->xmf4x4World;
	for (int i = 0; i < ANIM::END; i++)
	{
		clients[client_id].pPlayer->m_eAnimInfo[i] = duoPl->animInfo[i];
	}
	// 모든 클라에게 클라의 위치 전송 (나를 제외)
	for (auto& cl : clients)
	{
		if (cl.first == client_id) continue;
		cl.second.do_send(sizeof(duoPlayer), client_id, reinterpret_cast<char*>(duoPl));
	}
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
