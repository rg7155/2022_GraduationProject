#include "GameMgr.h"
#include "InputDev.h"
IMPLEMENT_SINGLETON(CGameMgr)

CGameMgr::CGameMgr(void)
{
	m_iId = 0;
}

CGameMgr::~CGameMgr(void)
{
	//Free();
}

void CGameMgr::GameUpdate(float& fTimeElapsed)
{

}

