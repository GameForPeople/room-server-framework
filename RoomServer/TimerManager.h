#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

enum class TIMER_TYPE
{
	PUSH_OLD_KEY,
	RETRY_TASK,
	ROOM_UPDATE,
	ROLL_COOL_TIMER,	// 구르기 쿨타임 끝나면 콜!
	ROLL_END,	// 구르기가 끝나 무적등을 해제해줘야함.
	GAME_START_COUNT,	// 게임 3초 후 시작,
	ROOM_DESTORY,	// 방 제거를 위한 Timer
	RETRY_RANDOM_MATCHING,	// 랜덤매칭 진입 실패 시, 일정 시간 이후 재시도
	REDUCE_REMAIN_TIME,	// 남은 시간 확인
	END_CHARACTER_NODAMAGE,	// 데미지를 입은 후, 일정 시간 노데미지
	END_CHARACTER_NOMOVE,	// 데미지를 입은 후, 일정 시간 움직임 금지.
	KICK_END,		// 킥 상태 끝남
	KICK_COOLTIME,	// 킥 쿨타임이후 초기화
	KICK_ATTACK,	//킥 실제 공격 확인.
	END_STUN,		// 스턴 종료

	FORCED_START	// 강제 실행
};

enum class TIME : unsigned long long
{
	SECOND = 1000,
	MINUATE = 60000,

	PUSH_OLD_KEY = 5000,	// 5초
	RETRY_TASK = 500,

	ROOM_UPDATE = 50,
	ROLL_COOLTIME = 5000,	// 5초
	ROLL_TIME = 500,	// 구르기 지속 시간 1초

	GAME_START_COUNT = 2500,	// 게임 시작 카운트
	ROOM_DESTORY = 5000,		// 룸 파괴 5초
	RETRY_RANDOM_MATCHING = 3000,	// 3초 뒤 랜덤매칭 재시도,
	REDUCE_REMAIN_TIME = 1000,	// 1초마다 한번씩. 
	END_CHARACTER_NODAMAGE = 1900,	//2초마다 캐릭터 노데미지 제거(갱신)
	END_CHARACTER_NOMOVE = 500,	//2초마다 캐릭터 노데미지 제거(갱신)
	KICK_END = 550,	// 밀치키 End?
	KICK_COOLTIME = 10000,	// 밀치기 쿨타임
	KICK_ATTACK = 250,	// 실제 밀치는 것 검사
	END_STUN = 300,		// 0.3초
	FORCED_START = 7000	// 강제 실행. 7초
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
		- 원하는 시간에 따라, task를 실행하기 위한 Manager입니다.
		- 싱글턴 객체입니다.

	#0. 현재 용도는 PUSH_OLD_KEY를 위해서만 사용됩니다.
*/
class TimerManager
	: public TSingleton<TimerManager>
{
	static constexpr int TIMER_MEMORY_POOL_SIZE = 10000;	// Init단계에서, Timer Memory Pool에 넣어줄 메모리 유닛 개수입니다.
	static constexpr int ADD_TIMER_MEMORY_POOL_SIZE = 100; // 런타임 중, Timer Memory Pool에서 꺼낼 메모리가 없는 경우, 추가로 Timer Memory Pool에 넣을 메모리 유닛 개수입니다.

public:
	TimerManager();
	~TimerManager();

	DISABLED_COPY(TimerManager)
	DISABLED_MOVE(TimerManager)

	void AddTimerEvent(const TIMER_TYPE, const TASK_TYPE taskType, const _Key ownerKey, void* data, const TIME waitTime); // 타이머 이벤트를 등록하는 함수입니다.
	void AddTimerEvent(TimerUnit* timerUnit, const TIME waitTime); // 이미 사용한 timerUnit을 시간만 변경하여 재등록하는 함수입니다.

private:
	// 우선순위 큐에 사용되는 비교 함수입니다.
	struct TimerUnitCompareFunction
	{
		_INLINE bool operator()(TimerUnit* left, TimerUnit* right) noexcept
		{
			return (left->eventTime) > (right->eventTime);
		}
	};

private:
	void TimerThread(); // 쓰레드 생성 시 호출하는 함수입니다.

	bool ProcessTimerUnit(TimerUnit* timerUnit); // TimerThread 내에서, task에 따라 적절히 처리하는 함수입니다.
	void PushTimerUnit(TimerUnit* timerUnit); // 사용한 Timer Unit을 메모리풀에 넣는 함수입니다.
	TimerUnit* PopTimerUnit(); // Timer Unit을 사용하기 위해 메모리풀에서 빼는 함수입니다.

private:
	std::thread timerThread; // 쓰레드 객체입니다.

	concurrency::concurrent_priority_queue<TimerUnit*, TimerUnitCompareFunction> timerCont; // 시간 순서대로 정렬하여 넣는 컨테이너입니다.
	concurrency::concurrent_queue<TimerUnit*> timerMemoryPool; // timer Memory를 갖고 있는 메모리풀입니다.
};


