#ifndef SoundMgr_h__
#define SoundMgr_h__

//#include "Define.h"
//#include "stdafx.h"

class CSoundMgr
{
public:
	DECLARE_SINGLETON(CSoundMgr)

public:
	enum CHANNELID 
	{
		BGM,
		PLAYER,
		BUTTON,
		MAXCHANNEL
	};
private:
	CSoundMgr();
	~CSoundMgr();

private:
	typedef struct tagSoundInfo
	{
		int iIndex = 0;
		float fDelay = 0.f;

	}SOUND_INFO;

public:
	void Initialize(); 

	void Release(); 
public:
	void PlaySound(TCHAR* pSoundKey, CHANNELID eID);
	void PlayBGM(TCHAR* pSoundKey);
	void StopSound(CHANNELID eID);
	void StopAll();
	void Update(float fTimedelta);

	void PlayShot();




public:

private:
	void LoadSoundFile(const char* pPath, const char* pPath2);

private:
	// 사운드 리소스 정보를 갖는 객체 
	map<TCHAR*, FMOD_SOUND*> m_mapSound; 
	// FMOD_CHANNEL : 재생하고 있는 사운드를 관리할 객체 
	FMOD_CHANNEL* m_pChannelArr[MAXCHANNEL]; 
	// 사운드 ,채널 객체 및 장치를 관리하는 객체 
	FMOD_SYSTEM* m_pSystem; 

	mt19937 gen;

	SOUND_INFO m_NormalAtt;



};

#endif // SoundMgr_h__
