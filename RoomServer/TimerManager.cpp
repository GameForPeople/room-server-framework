#include "stdafx.h"

#include "TimerManager.h"

#include "UserUnit.h"
#include "UserManager.h"
#include "RoomManager.h"
#include "RoomUnit.h"

#include "Utils.h"

TimerManager::TimerManager()
{
	// 타이머 매니저에서 필요한 정보를 미리 할당받습니다.
	for (int i = 0; i < TIMER_MEMORY_POOL_SIZE; ++i)
	{
		timerMemoryPool.push(new TimerUnit());
	}

	// 타이머 쓰레드르 생성합니다.
	timerThread = static_cast<std::thread>([&](){ this->TimerThread(); });
}

TimerManager::~TimerManager()
{
	// 타이머에 있는 메모리들을 반납합니다.

	TimerUnit* retTimerUnit = nullptr;
	while (timerMemoryPool.try_pop(retTimerUnit)) { delete retTimerUnit; }
	while (timerCont.try_pop(retTimerUnit)) { delete retTimerUnit; }
}

void TimerManager::AddTimerEvent(const TIMER_TYPE timerType, const TASK_TYPE taskType, const _Key ownerKey, void* data, const TIME waitTime)
{
	// 타이머 유닛을 메모리풀에서 받아옵니다.
	TimerUnit* timerUnit = PopTimerUnit();

	// 이벤트 정보를 설정합니다.
	timerUnit->timerType = timerType;
	timerUnit->taskType = taskType;
	timerUnit->ownerKey = ownerKey;
	timerUnit->data = data;
	timerUnit->eventTime = GetTickCount64() + static_cast<_Time>(waitTime);

	// 이벤트를 등록합니다.
	timerCont.push(timerUnit);
}

void TimerManager::AddTimerEvent(TimerUnit* timerUnit, const TIME waitTime)
{
	// 이벤트를 재사용하여 바로 등록합니다.
	timerUnit->eventTime = GetTickCount64() + static_cast<_Time>(waitTime);
	timerCont.push(timerUnit);
}

void TimerManager::TimerThread()
{
	using namespace std::literals;
	while (7)
	{
		std::this_thread::yield();
		const auto tempNowTime = GetTickCount64();

		while (timerCont.size())
		{
			TimerUnit* retTimerUnit{ nullptr };

			// 채팅서버이고, 세밀한 타이밍을 요구하는 이벤트가 없기 떄문에, 길게길게등록
			while (!timerCont.try_pop(retTimerUnit)) { std::this_thread::sleep_for(5s); }

			if (tempNowTime < retTimerUnit->eventTime)
			{
				// 재등록
				timerCont.push(retTimerUnit);
				break;
			}

			if (ProcessTimerUnit(retTimerUnit)) { PushTimerUnit(retTimerUnit); }
		}
	}
}

bool TimerManager::ProcessTimerUnit(TimerUnit* retTimerUnit)	// return true - 반환 필요, false - 재사용함
{
	switch (retTimerUnit->timerType)
	{
	case TIMER_TYPE::ROOM_UPDATE:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::ROOM_UPDATE, nullptr);
		return true;
	}
	case TIMER_TYPE::ROLL_COOL_TIMER:
	{
		// 구르기 쿨타임이 도는 상황입니다.
		UserUnit* pUser;
		UserManager::GetInstance().GetUserPointerWithKey(pUser, retTimerUnit->ownerKey);
		if (RoomUnit* pRoom = pUser->roomManagerUnit.GetRoom()
			; pRoom != nullptr)
		{ 
			pRoom->ProduceTask(TASK_TYPE::INIT_ROLL, reinterpret_cast<void*>(retTimerUnit->ownerKey));
			PrintLog(SOURCE_LOCATION, "타이머에서 구르기 쿨타임 끝남! 이벤트 처리를 요청합니다. ");
		}
		return true;
	}
	case TIMER_TYPE::ROLL_END:
	{
		// 구르기가 끝난 상황입니다.
		UserUnit* pUser;
		UserManager::GetInstance().GetUserPointerWithKey(pUser, retTimerUnit->ownerKey);
		if (RoomUnit* pRoom = pUser->roomManagerUnit.GetRoom()
			; pRoom != nullptr)
		{
			pRoom->ProduceTask(TASK_TYPE::ROLL_END, reinterpret_cast<void*>(retTimerUnit->ownerKey));
			PrintLog(SOURCE_LOCATION, "타이머에서 구르기 지속 시간이 끝남! 이벤트 처리를 요청합니다. ");
		}
		return true;
	}
	case TIMER_TYPE::GAME_START_COUNT:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->StartGame();
		PrintLog(SOURCE_LOCATION, "타이머에서 게임 카운트 다운이 끝났음을 방에 알리고 게임 쓰레드 생성을 요청합니다.");
		return true;
	}
	case TIMER_TYPE::FORCED_START:
	{
		return true;
	}
	case TIMER_TYPE::ROOM_DESTORY:
	{
		//static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::ROOM_DESTORY, nullptr);
		//RoomManager::GetInstance().ProduceTask(TASK_TYPE::ROOM_DESTORY, static_cast<RoomUnit*>(retTimerUnit->data));
		static_cast<RoomUnit*>(retTimerUnit->data)->DestoryRoom();
		PrintLog(SOURCE_LOCATION, "타이머에서 방의 제거를 시작합니다.");
		return true;
	}
	case TIMER_TYPE::REDUCE_REMAIN_TIME:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::REDUCE_REMAIN_TIME, nullptr);
		//PrintLog(SOURCE_LOCATION, "타이머에서 해당 방의 남은 시간을 1초 줄입니다.");
		return true;
	}
	case TIMER_TYPE::END_CHARACTER_NODAMAGE:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_CHARACTER_NODAMAGE, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 캐릭터의 무적을 제거 합니다.");
		return true;
	}
	case TIMER_TYPE::END_CHARACTER_NOMOVE:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_CHARACTER_NOMOVE, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 캐릭터의 움직임 방지를 제거 합니다.");
		return true;
	}
	case TIMER_TYPE::KICK_ATTACK:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::KICK_ATTACK , reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 캐릭터의 실제 공격 테스크를 추가합니다.");
		return true;
	}
	case TIMER_TYPE::KICK_END:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_KICK, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 캐릭터의 공격 중을 제거 합니다.");
		return true;
	}
	case TIMER_TYPE::KICK_COOLTIME:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::INIT_KICK_COOLTIME, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 캐릭터의 킥 쿨타임을 갱신합니다.");
		return true;
	}
	case TIMER_TYPE::END_STUN:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_STUN, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 캐릭터의 스턴 종료를 요청합니다.");
		return true;
	}
	case TIMER_TYPE::RETRY_RANDOM_MATCHING:
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::RANDOM_MATCH, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "타이머에서 해당 유저의 랜덤매칭 재시도를 방 매니저에게 요청합니다.");
		return true;
	}
	case TIMER_TYPE::PUSH_OLD_KEY:
	{
		UserManager::GetInstance().ProduceTask(TASK_TYPE::PUSH_OLD_KEY, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		return true;
	}
	default:
		return true;
		break;
	}
}

_NODISCARD TimerUnit* TimerManager::PopTimerUnit()
{
	TimerUnit* retTimerUnit{ nullptr };

	return timerMemoryPool.try_pop(retTimerUnit)
		? retTimerUnit
		: []()->TimerUnit*
	{
		return new TimerUnit();
	}();
}

void TimerManager::PushTimerUnit(TimerUnit* timerUnit)
{
	timerMemoryPool.push(timerUnit);
}

