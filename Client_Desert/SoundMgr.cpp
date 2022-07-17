#include "stdafx.h"
#include "SoundMgr.h"

IMPLEMENT_SINGLETON(CSoundMgr)

CSoundMgr::CSoundMgr()
{
	m_pSystem = nullptr; 
}


CSoundMgr::~CSoundMgr()
{
	Release(); 
}

void CSoundMgr::Initialize()
{
	FMOD_System_Create(&m_pSystem);
	
	// 1. 시스템 포인터, 2. 사용할 가상채널 수 , 초기화 방식) 
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

	LoadSoundFile("Sound/*.*", "Sound/");
	//LoadSoundFile("../Sound/BGM/*.*", "../Sound/BGM/");
	//LoadSoundFile("../Sound/Stage/*.*", "../Sound/Stage/");
	//LoadSoundFile("../Sound/Player/*.*", "../Sound/Player/");
	//LoadSoundFile("../Sound/Boss/*.*", "../Sound/Boss/");
	//LoadSoundFile("../Sound/Monster/*.*", "../Sound/Monster/");

	std::random_device rd;
	std::mt19937 gen2(rd());

	gen = gen2;

}
void CSoundMgr::Release()
{
	for (auto& Mypair : m_mapSound)
	{
		delete[] Mypair.first;
		FMOD_Sound_Release(Mypair.second);
	}
	m_mapSound.clear(); 
	FMOD_System_Release(m_pSystem);
	FMOD_System_Close(m_pSystem);
}

void CSoundMgr::PlaySound(TCHAR * pSoundKey, CHANNELID eID)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter; 

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter) 
	{
		return !lstrcmp(pSoundKey, iter.first);
	});

	if (iter == m_mapSound.end())
		return;

	FMOD_BOOL bPlay = FALSE; 
	if (FMOD_Channel_IsPlaying(m_pChannelArr[eID], &bPlay))
	{
		FMOD_System_PlaySound(m_pSystem, FMOD_CHANNEL_FREE, iter->second, FALSE, &m_pChannelArr[eID]);
	}
	FMOD_System_Update(m_pSystem);
}

void CSoundMgr::PlayBGM(TCHAR * pSoundKey)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
	{
		return !lstrcmp(pSoundKey, iter.first);
	});

	if (iter == m_mapSound.end())
		return;

	FMOD_System_PlaySound(m_pSystem, FMOD_CHANNEL_FREE, iter->second, FALSE, &m_pChannelArr[BGM]);
	FMOD_Channel_SetMode(m_pChannelArr[BGM], FMOD_LOOP_NORMAL);
	FMOD_System_Update(m_pSystem);
}


void CSoundMgr::StopSound(CHANNELID eID)
{
	FMOD_Channel_Stop(m_pChannelArr[eID]);
}

void CSoundMgr::StopAll()
{
	for (int i = 0 ; i < MAXCHANNEL ; ++i)
		FMOD_Channel_Stop(m_pChannelArr[i]);
}


void CSoundMgr::PlayShot()
{
	//if (m_NormalAtt.fDelay > 0.2f)
	//{
	//	std::uniform_int_distribution<int> dis(0, 0);

	//	wstring wstrkey;

	//	TCHAR szKey[32];
	//	int iIndex = dis(gen);
	//	switch (iIndex)
	//	{
	//	case 0:
	//		wstrkey = __T("DLCstaff_shotsmall1.ogg");
	//		break;
	//	case 1:
	//		wstrkey = __T("DLCstaff_shotsmall2.ogg");
	//		break;
	//	}
	//	lstrcpy(szKey, wstrkey.c_str());

	//	switch (m_NormalAtt.iIndex++)
	//	{
	//	case 0:
	//		PlaySound(szKey, CSoundMgr::NORMALATT_0);
	//		break;
	//	case 1:
	//		PlaySound(szKey, CSoundMgr::NORMALATT_1);
	//		break;
	//	case 2:
	//		PlaySound(szKey, CSoundMgr::NORMALATT_2);
	//		break;

	//	}

	//	if (m_NormalAtt.iIndex > 2)
	//		m_NormalAtt.iIndex = 0;

	//	m_NormalAtt.fDelay = 0.f;
	//}
}



void CSoundMgr::Update(float fTimedelta)
{
	//////////////////////////////////////////////////////////
	m_NormalAtt.fDelay += fTimedelta;

	//////////////////////////////////////////////////////////

}

void CSoundMgr::LoadSoundFile(const char* pPath, const char* pPath2)
{
	_finddata_t fd; 

	intptr_t handle = _findfirst(pPath, &fd);

	if (handle == 0)
		return; 

	int iResult = 0; 

	char szFullPath[128] = ""; 

	while (iResult != -1)
	{
		strcpy_s(szFullPath, pPath2);
		strcat_s(szFullPath, fd.name);
		FMOD_SOUND* pSound = nullptr; 

		FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem, szFullPath, FMOD_HARDWARE, 0, &pSound);

		if (eRes == FMOD_OK)
		{
			int iLength = int(strlen(fd.name) + 1); 

			TCHAR* pSoundKey = new TCHAR[iLength];
			ZeroMemory(pSoundKey, sizeof(TCHAR) * iLength);
			MultiByteToWideChar(CP_ACP, 0, fd.name, iLength, pSoundKey, iLength);

			m_mapSound.emplace(pSoundKey, pSound);
		}
		iResult = _findnext(handle, &fd);
	}
	FMOD_System_Update(m_pSystem);
	_findclose(handle);
}


