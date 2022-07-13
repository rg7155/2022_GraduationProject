#ifndef Eunm_h__
#define Eunm_h__

enum MOUSEKEYSTATE { DIM_LB, DIM_RB, DIM_MB, DIM_END };
enum MOUSEMOVESTATE { DIMS_X, DIMS_Y, DIMS_Z, DIMS_END };

enum COMPONENT { COM_FRUSTUM, COM_COLLISION, COM_TRAIL, COM_END };

enum OBJ_ID { OBJ_PLAYER, OBJ_MAP, OBJ_SWORD, OBJ_MONSTER, OBJ_BULLET, OBJ_END };

enum SCENE { SCENE_1, SCENE_2, SCENE_END};

namespace PLAYER
{
	enum ANIM {
		IDLE_RELAXED = 0, RUN = 1, ATTACK1 = 2, ATTACK2 = 3, SKILL1 = 4,
		SKILL2 = 5, IDLE = 6, GET_RESOURCE = 7, DIE = 8, TAKE_DAMAGED = 9, END = 10,
	};
}

namespace GOLEM
{
	enum ANIM {
		IDLE = 0, RUN = 1, ATTACK1 = 2, ATTACK2 = 3,
		DAMAGED_RIGHT = 4, DAMAGED_LEFT = 5, DIE = 6, GETUP = 7, END = 8
	};
}

namespace CACTI
{
	enum ANIM {
		WALK = 0, IDLE = 1, ATTACK1 = 2, ATTACK2 = 3,
		DIE = 4, BITE = 5, END = 6
	};
}

namespace CACTUS
{
	enum ANIM {
		SPAWN = 0, IDLE = 1, ATTACK1 = 2, ATTACK2 = 3,
		ATTACK3 = 4, DIE = 5, TAKE_DAMAGED = 6, END = 7
	};
}
//string COM_FRUSTUM = "Frustum";
#endif // Eunm_h__
