#pragma once
#ifndef __STRUCT_H__
#define __STRUCT_H__

typedef struct tagInfo
{
	float	fX;
	float	fY;
	int		iCX;
	int		iCY;
}INFO;

typedef struct tagLinePos
{
	tagLinePos() { ZeroMemory(this, sizeof(tagLinePos)); }
	tagLinePos(float _x, float _y) : fX(_x), fY(_y) {}

	float	fX;
	float	fY;
}LINEPOS;

typedef struct tagLineInfo
{
	tagLineInfo() { ZeroMemory(this, sizeof(tagLineInfo)); }
	tagLineInfo(LINEPOS& _Left, LINEPOS& _Right) : tLeftPos(_Left), tRightPos(_Right) {}

	LINEPOS		tLeftPos;
	LINEPOS		tRightPos;
}LINEINFO;


typedef struct tagFrame
{
	int		iFrameStart;
	int		iFrameEnd;
	int		iFrameScene;
	DWORD	dwFrameSpeed;
	DWORD	dwFrameTime;        
}FRAME;


//////////////////////////////////// ����
struct HpPotionCreate
{
	int		cnt; // ��� Ŭ�� ���´���
	int		index; // ü�¾� ����Ʈ �� ���°?
	bool	bCreateOn;
	POINT	pos;
};

struct HpPotionDelete
{
	int		cnt; // ��� Ŭ�� ���´���
	int		index; // ü�¾� ����Ʈ �� ���°?
	bool	bDeleteOn;
};

struct HpPotionInfo
{
	HpPotionCreate thpPotionCreate;
	HpPotionDelete thpPotionDelete;
};


//struct HpPotion
typedef struct tagHpPotionRes
{
	bool	bCollision;
	int		iIndex;
}POTIONRES;
#endif // !__STRUCT_H__
