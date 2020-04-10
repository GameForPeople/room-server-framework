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
	// 메모리 할당
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
	// 등록받은 테스크를 처리합니다.
	switch (taskUnit.first)
	{
	default:
		PrintLog(SOURCE_LOCATION, "어떤 방에서 이상한 패킷을 받았습니다. 패킷 타입 : " + std::to_string((int)taskUnit.first));
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
		// 타이머에서 처리해야할 이벤트가 있는 지 확인합니다.
		TimerProcess();
		TaskProcess();
	}
}

_INLINE void RoomUnit::ProduceTask(_RoomUnitTask taskUnit)
{
	// TaskCont에 등록해줍니다.
	taskCont.push(taskUnit);
}

void RoomUnit::TimerProcess()
{
	ROOM_UNIT::TimerUnit* timerUnit;

	// 발생하지 않을 예외처리를 체크할 확률이 더 높음.
	//if (timerCont.size() == 0) { return; }

	const auto tempNowTime = GetTickCount64();

	while (timerCont.try_pop(timerUnit))
	{
		if (tempNowTime < timerUnit->eventTime)
		{
			// 재등록
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

	// 테스크가 있는지 확인합니다.
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
		PrintLog(SOURCE_LOCATION, "경고! TimerUnit이 부족합니다.");
		return new ROOM_UNIT::TimerUnit();
	}();
}
