#pragma once
#include "GameObject.h"
class CCactiMonster : public CGameObject
{
public:
	CCactiMonster();
	~CCactiMonster() {}

public:
	virtual void Send_Packet_To_Clients(int c_id);
	virtual void CheckCollision(int c_id);


};

