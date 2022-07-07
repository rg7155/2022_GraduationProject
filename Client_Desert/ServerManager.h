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
	static void send_packet(void* packet);

	static void send_login_packet();

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

	static WSABUF	m_wsabuf_s;
	static char*	m_send_buf;

	static char		m_prev_buf[BUFSIZE];
	static int		m_prev_bytes;

	static bool		m_isWindow;

	static int		m_myid;
	static CGameFramework* gameFramework;
};

