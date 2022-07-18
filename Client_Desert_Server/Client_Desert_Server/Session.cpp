#include "Session.h"
#include "SendData.h"

CSession::CSession(int id, SOCKET s)
	: _id(id), _socket(s)
{
	_c_wsabuf[0].buf = _c_mess;
	_c_wsabuf[0].len = sizeof(_c_mess);
	over_to_session[&_c_over] = id;
	_isReady = false;
}

void CSession::do_recv()
{
	// 받기전에 초기화 해준다.......
	_c_wsabuf[0].buf = _c_mess;
	_c_wsabuf[0].len = BUFSIZE;
	DWORD recv_flag = 0;
	memset(&_c_over, 0, sizeof(_c_over));

	// 키값 받음
	WSARecv(_socket, _c_wsabuf, 1, 0, &recv_flag, &_c_over, recv_callback);
}

void CSession::do_send(int num_bytes, char* mess)
{
	CSendData* sdata = new CSendData{ num_bytes, mess };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, send_callback);

}

void CSession::send_login_packet()
{
	_pObject = new CGameObject;
	_pObject->Initialize();
	_pObject->m_bActive = true;
	XMFLOAT3 pos = _pObject->GetPosition();
	SC_LOGIN_INFO_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = pos.x;
	p.y = pos.y;
	p.z = pos.z;
	do_send(p.size, reinterpret_cast<char*>(&p));
}

void CSession::send_add_object(int c_id)
{
	CGameObject* pObject = clients[c_id]._pObject;

	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.xmf4x4World = pObject->m_xmf4x4World;
	p.eCurAnim = pObject->m_eCurAnim;
	memcpy(p.animInfo, pObject->m_eAnimInfo, sizeof(p.animInfo));
	p.race = clients[c_id]._pObject->m_race;
	p.hp = clients[c_id]._pObject->m_hp;
	p.hpmax = clients[c_id]._pObject->m_hpmax;
	do_send(p.size, reinterpret_cast<char*>(&p));
}

void CSession::send_move_packet(int c_id)
{
	CGameObject* pObject = clients[c_id]._pObject;

	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id; // _id로 해서 오류 났었음
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.xmf4x4World = pObject->m_xmf4x4World;
	p.eCurAnim = pObject->m_eCurAnim;
	memcpy(p.animInfo, pObject->m_eAnimInfo, sizeof(p.animInfo));
	p.race = pObject->m_race;
	do_send(p.size, reinterpret_cast<char*>(&p));
}

void CSession::send_ready_packet(int c_id)
{
	SC_READY_PACKET p;
	p.size = sizeof(SC_READY_PACKET);
	p.type = SC_READY;
	p.bReady = _isReady;
	clients[c_id].do_send(p.size, reinterpret_cast<char*>(&p));
}
