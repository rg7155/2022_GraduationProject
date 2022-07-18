#pragma once

#include "stdafx.h"

class CGameFramework;
class CServerManager
{
	DECLARE_SINGLETON(CServerManager)

private:
	CServerManager();
	~CServerManager();

public:
	static void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
	static void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

	static void recv_packet();
	static void send_packet(void* packet, int size);

	static void send_login_packet();
	static void send_move_packet(char dir);
	static void send_ready_packet();

public:
	static void Connect();
	static int ProcessPacket(char* packet);
public:
	static void error_display(const char* msg, int err_no);

public:
	static SOCKET	m_socket;
	// Server
	static WSABUF	m_wsabuf_r;
	static char		m_recv_buf[BUFSIZE];

	static bool		m_isWindow;

	static int		m_myid;
	static CGameFramework* gameFramework;
	

public:
	static bool m_bFoot[2];
	static queue<char>	m_queue_send_packet;

	// send 여러 개 할 수 있으니 map으로 버퍼 관리하자
	//static queue<char*> m_queueSendBuf;
};

