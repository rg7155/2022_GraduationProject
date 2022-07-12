#include "CactiBullet.h"

CCactiBullet::CCactiBullet()
{
}

CCactiBullet::~CCactiBullet()
{
}

void CCactiBullet::Update(float fTimeElapsed)
{
	if (!m_bActive) return;
	m_fCreateTime -= fTimeElapsed;
	if (m_fCreateTime > 0)
		return;

	m_fTime -= fTimeElapsed;
	if (m_fTime < 0)
	{
		m_bActive = false;
		return;
	}

	XMFLOAT3 xmf3Pos = GetPosition();
	xmf3Pos = Vector3::Add(xmf3Pos, Vector3::ScalarProduct(m_xmf3Target, fTimeElapsed * m_fSpeed, false));
	SetPosition(xmf3Pos.x, xmf3Pos.y, xmf3Pos.z);
}

void CCactiBullet::Send_Packet_To_Clients(int c_id)
{
	SC_MOVE_BULLET_PACKET p;
	//// Ÿ���߰�
	//type;
	//size;
	//id;
	//race;
	//xmf3Look;
	//xmf3Position;
	//target_id;
}

void CCactiBullet::CheckCollision(int c_id)
{
}

void CCactiBullet::SetTarget(XMFLOAT3& xmf3Start, XMFLOAT3& xmf3Target, bool IsYFix)
{
	m_fTime = CACTI_BULLET_TIME;

	SetPosition(xmf3Start.x, xmf3Start.y, xmf3Start.z);

	m_xmf3Target = Vector3::Subtract(xmf3Target, xmf3Start, true, IsYFix);
}
