#pragma once
#include "Client_Desert_Server.h"
#include "GameObject.h"

class CSession
{
	WSAOVERLAPPED _c_over;
	WSABUF	_c_wsabuf[1];

public:
	int				_id;
	bool			_isReady;
	CGameObject* _pObject;

public:
	SOCKET	_socket;
	CHAR	_c_mess[BUFSIZE];

public:
	CSession() {}
	CSession(int id, SOCKET s);
	~CSession() {}

	void do_recv();
	void do_send(int num_bytes, char* mess);

	void send_login_packet();
	void send_add_object(int c_id);
	void send_move_packet(int c_id);
	void send_ready_packet(int c_id);
};

