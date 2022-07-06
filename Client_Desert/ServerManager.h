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
	static void ProcessPacket(char* packet);
public:
	static void error_display(const char* msg, int err_no);

private:
	SOCKET s_socket;
};

