#pragma once

#include "stdafx.h"


class CServerManager
{
	DECLARE_SINGLETON(CServerManager)

private:
	CServerManager();
	virtual ~CServerManager();

public:
	static void CALLBACK send_callback(DWORD dwError, DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
	static void CALLBACK recv_callback(DWORD dwError, DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
	static void RecvProcess();
	static void SendProcess();

	static void send_packet(void* packet);

public:
	static void Connect();
	static int ProcessPacket(char* packet);
public:
	static void error_display(const char* msg, int err_no);

public:
	static SOCKET	m_s_socket;
	// Server
	static WSABUF	m_wsabuf_r;
	static char		m_recv_buf[BUFSIZE];

	static WSABUF	m_wsabuf_s;
	static char*	m_send_buf;

	static char		m_prev_buf[BUFSIZE];
	static int		m_prev_bytes;

	static bool		m_isWindow;

	static int		m_myid;
};

