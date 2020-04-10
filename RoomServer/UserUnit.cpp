#include "stdafx.h"

#include "RoomUnit.h"
#include "UserUnit.h"
#include "RoomManager.h"

UserUnit::UserUnit(const _Key key)
	: memoryUnit(MEMORY_UNIT_TYPE::RECV_FROM_CLIENT)
	
	, frameworkAccessibleField()
	, userManagerAccessibleField()
	, matchingManagerAccessibleField()

	, key(key)
	, socket()

	//, userState(USER_STATE::NO_LOGIN)
{
}

