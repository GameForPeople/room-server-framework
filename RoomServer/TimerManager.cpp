#include "stdafx.h"

#include "TimerManager.h"

#include "UserUnit.h"
#include "UserManager.h"
#include "RoomManager.h"
#include "RoomUnit.h"

#include "Utils.h"

TimerManager::TimerManager()
{
	// Ÿ�̸� �Ŵ������� �ʿ��� ������ �̸� �Ҵ�޽��ϴ�.
	for (int i = 0; i < TIMER_MEMORY_POOL_SIZE; ++i)
	{
		timerMemoryPool.push(new TimerUnit());
	}

	// Ÿ�̸� �����帣 �����մϴ�.
	timerThread = static_cast<std::thread>([&](){ this->TimerThread(); });
}

TimerManager::~TimerManager()
{
	// Ÿ�̸ӿ� �ִ� �޸𸮵��� �ݳ��մϴ�.

	TimerUnit* retTimerUnit = nullptr;
	while (timerMemoryPool.try_pop(retTimerUnit)) { delete retTimerUnit; }
	while (timerCont.try_pop(retTimerUnit)) { delete retTimerUnit; }
}

void TimerManager::AddTimerEvent(const TIMER_TYPE timerType, const TASK_TYPE taskType, const _Key ownerKey, void* data, const TIME waitTime)
{
	// Ÿ�̸� ������ �޸�Ǯ���� �޾ƿɴϴ�.
	TimerUnit* timerUnit = PopTimerUnit();

	// �̺�Ʈ ������ �����մϴ�.
	timerUnit->timerType = timerType;
	timerUnit->taskType = taskType;
	timerUnit->ownerKey = ownerKey;
	timerUnit->data = data;
	timerUnit->eventTime = GetTickCount64() + static_cast<_Time>(waitTime);

	// �̺�Ʈ�� ����մϴ�.
	timerCont.push(timerUnit);
}

void TimerManager::AddTimerEvent(TimerUnit* timerUnit, const TIME waitTime)
{
	// �̺�Ʈ�� �����Ͽ� �ٷ� ����մϴ�.
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

			// ä�ü����̰�, ������ Ÿ�̹��� �䱸�ϴ� �̺�Ʈ�� ���� ������, ��Ա�Ե��
			while (!timerCont.try_pop(retTimerUnit)) { std::this_thread::sleep_for(5s); }

			if (tempNowTime < retTimerUnit->eventTime)
			{
				// ����
				timerCont.push(retTimerUnit);
				break;
			}

			if (ProcessTimerUnit(retTimerUnit)) { PushTimerUnit(retTimerUnit); }
		}
	}
}

bool TimerManager::ProcessTimerUnit(TimerUnit* retTimerUnit)	// return true - ��ȯ �ʿ�, false - ������
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
		// ������ ��Ÿ���� ���� ��Ȳ�Դϴ�.
		UserUnit* pUser;
		UserManager::GetInstance().GetUserPointerWithKey(pUser, retTimerUnit->ownerKey);
		if (RoomUnit* pRoom = pUser->roomManagerUnit.GetRoom()
			; pRoom != nullptr)
		{ 
			pRoom->ProduceTask(TASK_TYPE::INIT_ROLL, reinterpret_cast<void*>(retTimerUnit->ownerKey));
			PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� ������ ��Ÿ�� ����! �̺�Ʈ ó���� ��û�մϴ�. ");
		}
		return true;
	}
	case TIMER_TYPE::ROLL_END:
	{
		// �����Ⱑ ���� ��Ȳ�Դϴ�.
		UserUnit* pUser;
		UserManager::GetInstance().GetUserPointerWithKey(pUser, retTimerUnit->ownerKey);
		if (RoomUnit* pRoom = pUser->roomManagerUnit.GetRoom()
			; pRoom != nullptr)
		{
			pRoom->ProduceTask(TASK_TYPE::ROLL_END, reinterpret_cast<void*>(retTimerUnit->ownerKey));
			PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� ������ ���� �ð��� ����! �̺�Ʈ ó���� ��û�մϴ�. ");
		}
		return true;
	}
	case TIMER_TYPE::GAME_START_COUNT:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->StartGame();
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� ���� ī��Ʈ �ٿ��� �������� �濡 �˸��� ���� ������ ������ ��û�մϴ�.");
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
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� ���� ���Ÿ� �����մϴ�.");
		return true;
	}
	case TIMER_TYPE::REDUCE_REMAIN_TIME:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::REDUCE_REMAIN_TIME, nullptr);
		//PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ���� ���� �ð��� 1�� ���Դϴ�.");
		return true;
	}
	case TIMER_TYPE::END_CHARACTER_NODAMAGE:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_CHARACTER_NODAMAGE, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ĳ������ ������ ���� �մϴ�.");
		return true;
	}
	case TIMER_TYPE::END_CHARACTER_NOMOVE:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_CHARACTER_NOMOVE, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ĳ������ ������ ������ ���� �մϴ�.");
		return true;
	}
	case TIMER_TYPE::KICK_ATTACK:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::KICK_ATTACK , reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� ĳ������ ���� ���� �׽�ũ�� �߰��մϴ�.");
		return true;
	}
	case TIMER_TYPE::KICK_END:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_KICK, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ĳ������ ���� ���� ���� �մϴ�.");
		return true;
	}
	case TIMER_TYPE::KICK_COOLTIME:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::INIT_KICK_COOLTIME, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ĳ������ ű ��Ÿ���� �����մϴ�.");
		return true;
	}
	case TIMER_TYPE::END_STUN:
	{
		static_cast<RoomUnit*>(retTimerUnit->data)->ProduceTask(TASK_TYPE::END_STUN, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ĳ������ ���� ���Ḧ ��û�մϴ�.");
		return true;
	}
	case TIMER_TYPE::RETRY_RANDOM_MATCHING:
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::RANDOM_MATCH, reinterpret_cast<void*>(retTimerUnit->ownerKey));
		PrintLog(SOURCE_LOCATION, "Ÿ�̸ӿ��� �ش� ������ ������Ī ��õ��� �� �Ŵ������� ��û�մϴ�.");
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

