#include "SendData.h"

CSendData::CSendData(int size, char* n_data)
{
	// size id data ������ ������.
	_wsabuf.len = size;
	_wsabuf.buf = send_buf;
	ZeroMemory(&_over, sizeof(_over));
	ZeroMemory(&send_buf, BUFSIZE);
	memcpy(send_buf, n_data, size);
}
