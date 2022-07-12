#pragma once
#include "Client_Desert_Server.h"

class CSendData {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char send_buf[BUFSIZE];

	CSendData(int size, char* n_data);
};

