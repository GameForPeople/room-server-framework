#include "stdafx.h"

#include "UserUnit.h"

#include "RoomUnit.h"

#include "TimerManager.h"
#include "NetworkManager.h"
#include "UserManager.h"

#include "Utils.h"

RoomUserUnit::RoomUserUnit(const _Key keyInRoom)
	: keyInRoom(keyInRoom)
	, pUserUnit(nullptr)
{
}

RoomUnit::RoomUnit(const _RoomKey roomKey)
	: roomKey()
	
	, userCont()

	, isRun()
	, playTime(0)

	, taskCont()
	, roomThread()

	, timerCont()
	, timerMemoryPool()
{
	// �޸� �Ҵ�
	for (int i = 0; i < ROOM_MAX_PLAYER; ++i) { userCont[i] = new RoomUserUnit(i); }
}

RoomUnit::~RoomUnit()
{
	for (auto& roomUser : userCont) { delete roomUser; }
}

void RoomUnit::ProcessTimerUnit(ROOM_UNIT::TimerUnit* timerUnit)
{
	switch (timerUnit->taskType)
	{
	default:
		break;
	}
}

void RoomUnit::ProcessTaskUnit(_RoomUnitTask taskUnit)
{
	// ��Ϲ��� �׽�ũ�� ó���մϴ�.
	switch (taskUnit.first)
	{
	default:
		PrintLog(SOURCE_LOCATION, "� �濡�� �̻��� ��Ŷ�� �޾ҽ��ϴ�. ��Ŷ Ÿ�� : " + std::to_string((int)taskUnit.first));
		break;
	}
}

//----------------------------------------------------------------------------------------------------------------
// FIXED
//----------------------------------------------------------------------------------------------------------------
_INLINE _RoomKey RoomUnit::GetKey() noexcept
{
	return roomKey;
}

_INLINE bool RoomUnit::GetIsRun() noexcept
{
	return isRun;
}

_INLINE bool RoomUnit::GetIsFull() noexcept
{
	return maxUserNum == nowUserNum;
}

void RoomUnit::Init(std::wstring& roomName, const char maxUserNum, const char mapType)
{
	this->roomName = roomName;
	this->maxUserNum = maxUserNum;
	this->mapType = mapType;
}

void RoomUnit::RoomThread()
{
	while (isRun)
	{
		// Ÿ�̸ӿ��� ó���ؾ��� �̺�Ʈ�� �ִ� �� Ȯ���մϴ�.
		TimerProcess();
		TaskProcess();
	}
}

_INLINE void RoomUnit::ProduceTask(_RoomUnitTask taskUnit)
{
	// TaskCont�� ������ݴϴ�.
	taskCont.push(taskUnit);
}

void RoomUnit::TimerProcess()
{
	ROOM_UNIT::TimerUnit* timerUnit;

	// �߻����� ���� ����ó���� üũ�� Ȯ���� �� ����.
	//if (timerCont.size() == 0) { return; }

	const auto tempNowTime = GetTickCount64();

	while (timerCont.try_pop(timerUnit))
	{
		if (tempNowTime < timerUnit->eventTime)
		{
			// ����
			timerCont.push(timerUnit);
			break;
		}
		else
		{
			ProcessTimerUnit(timerUnit);
		}
		PushTimerUnit(timerUnit);
	}
}

void RoomUnit::TaskProcess()
{
	_RoomUnitTask task{};

	// �׽�ũ�� �ִ��� Ȯ���մϴ�.
	while (taskCont.try_pop(task))
	{
		ProcessTaskUnit(task);
	}
}

_INLINE void RoomUnit::PushTimerUnit(ROOM_UNIT::TimerUnit* timerUnit)
{
	timerMemoryPool.push(timerUnit);
}

_NODISCARD ROOM_UNIT::TimerUnit* RoomUnit::PopTimerUnit()
{
	ROOM_UNIT::TimerUnit* retTimerUnit{ nullptr };

	return timerMemoryPool.try_pop(retTimerUnit)
		? retTimerUnit
		: []()->ROOM_UNIT::TimerUnit*
	{
		PrintLog(SOURCE_LOCATION, "���! TimerUnit�� �����մϴ�.");
		return new ROOM_UNIT::TimerUnit();
	}();
}
