#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

enum class TIMER_TYPE
{
	PUSH_OLD_KEY,
	RETRY_TASK,
	ROOM_UPDATE,
	ROLL_COOL_TIMER,	// ������ ��Ÿ�� ������ ��!
	ROLL_END,	// �����Ⱑ ���� �������� �����������.
	GAME_START_COUNT,	// ���� 3�� �� ����,
	ROOM_DESTORY,	// �� ���Ÿ� ���� Timer
	RETRY_RANDOM_MATCHING,	// ������Ī ���� ���� ��, ���� �ð� ���� ��õ�
	REDUCE_REMAIN_TIME,	// ���� �ð� Ȯ��
	END_CHARACTER_NODAMAGE,	// �������� ���� ��, ���� �ð� �뵥����
	END_CHARACTER_NOMOVE,	// �������� ���� ��, ���� �ð� ������ ����.
	KICK_END,		// ű ���� ����
	KICK_COOLTIME,	// ű ��Ÿ������ �ʱ�ȭ
	KICK_ATTACK,	//ű ���� ���� Ȯ��.
	END_STUN,		// ���� ����

	FORCED_START	// ���� ����
};

enum class TIME : unsigned long long
{
	SECOND = 1000,
	MINUATE = 60000,

	PUSH_OLD_KEY = 5000,	// 5��
	RETRY_TASK = 500,

	ROOM_UPDATE = 50,
	ROLL_COOLTIME = 5000,	// 5��
	ROLL_TIME = 500,	// ������ ���� �ð� 1��

	GAME_START_COUNT = 2500,	// ���� ���� ī��Ʈ
	ROOM_DESTORY = 5000,		// �� �ı� 5��
	RETRY_RANDOM_MATCHING = 3000,	// 3�� �� ������Ī ��õ�,
	REDUCE_REMAIN_TIME = 1000,	// 1�ʸ��� �ѹ���. 
	END_CHARACTER_NODAMAGE = 1900,	//2�ʸ��� ĳ���� �뵥���� ����(����)
	END_CHARACTER_NOMOVE = 500,	//2�ʸ��� ĳ���� �뵥���� ����(����)
	KICK_END = 550,	// ��ġŰ End?
	KICK_COOLTIME = 10000,	// ��ġ�� ��Ÿ��
	KICK_ATTACK = 250,	// ���� ��ġ�� �� �˻�
	END_STUN = 300,		// 0.3��
	FORCED_START = 7000	// ���� ����. 7��
};

struct TimerUnit
{
	TIMER_TYPE timerType;
	TASK_TYPE taskType;

	_Time eventTime;
	_Key ownerKey;

	void* data;
	// std::any data;
};

/*
	TimerManager
		- ���ϴ� �ð��� ����, task�� �����ϱ� ���� Manager�Դϴ�.
		- �̱��� ��ü�Դϴ�.

	#0. ���� �뵵�� PUSH_OLD_KEY�� ���ؼ��� ���˴ϴ�.
*/
class TimerManager
	: public TSingleton<TimerManager>
{
	static constexpr int TIMER_MEMORY_POOL_SIZE = 10000;	// Init�ܰ迡��, Timer Memory Pool�� �־��� �޸� ���� �����Դϴ�.
	static constexpr int ADD_TIMER_MEMORY_POOL_SIZE = 100; // ��Ÿ�� ��, Timer Memory Pool���� ���� �޸𸮰� ���� ���, �߰��� Timer Memory Pool�� ���� �޸� ���� �����Դϴ�.

public:
	TimerManager();
	~TimerManager();

	DISABLED_COPY(TimerManager)
	DISABLED_MOVE(TimerManager)

	void AddTimerEvent(const TIMER_TYPE, const TASK_TYPE taskType, const _Key ownerKey, void* data, const TIME waitTime); // Ÿ�̸� �̺�Ʈ�� ����ϴ� �Լ��Դϴ�.
	void AddTimerEvent(TimerUnit* timerUnit, const TIME waitTime); // �̹� ����� timerUnit�� �ð��� �����Ͽ� �����ϴ� �Լ��Դϴ�.

private:
	// �켱���� ť�� ���Ǵ� �� �Լ��Դϴ�.
	struct TimerUnitCompareFunction
	{
		_INLINE bool operator()(TimerUnit* left, TimerUnit* right) noexcept
		{
			return (left->eventTime) > (right->eventTime);
		}
	};

private:
	void TimerThread(); // ������ ���� �� ȣ���ϴ� �Լ��Դϴ�.

	bool ProcessTimerUnit(TimerUnit* timerUnit); // TimerThread ������, task�� ���� ������ ó���ϴ� �Լ��Դϴ�.
	void PushTimerUnit(TimerUnit* timerUnit); // ����� Timer Unit�� �޸�Ǯ�� �ִ� �Լ��Դϴ�.
	TimerUnit* PopTimerUnit(); // Timer Unit�� ����ϱ� ���� �޸�Ǯ���� ���� �Լ��Դϴ�.

private:
	std::thread timerThread; // ������ ��ü�Դϴ�.

	concurrency::concurrent_priority_queue<TimerUnit*, TimerUnitCompareFunction> timerCont; // �ð� ������� �����Ͽ� �ִ� �����̳��Դϴ�.
	concurrency::concurrent_queue<TimerUnit*> timerMemoryPool; // timer Memory�� ���� �ִ� �޸�Ǯ�Դϴ�.
};


