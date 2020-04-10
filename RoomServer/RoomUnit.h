#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct UserUnit;
struct WallUnit;
struct TimerUnit;

struct RoomUserUnit
{
public: 
	RoomUserUnit(const _Key keyInRoom);

public:
	UserUnit* pUserUnit;
	const _Key keyInRoom;
};

namespace ROOM_UNIT
{
	enum class TASK_TYPE
	{

	};

	enum class TIME : unsigned long long
	{

	};

	struct TimerUnit
	{
		ROOM_UNIT::TASK_TYPE taskType;

		_Time eventTime;
		_Key ownerKey;	// 항상 핋요한 것은 아니지만, 자주 사용됨.

		void* data;
		// std::any data;
	};
}

/*
	Room
		- 방입니다.

	#0. 각 방에 대해서는 Single Thread Access가 보장됩니다.
	#1. 내부적으로 roomUserCont를 통해 roomUser에 대한 정보를 갖습니다.
		- room에서 처리되는 모든 유저 정보는 UserCont에 접근하지 않고,
		roomUser에 카피된 데이터들로 처리하는 것을 원칙으로 합니다.
	#2. 추가적으로 방 이름과 호스트 User의 포인터를 갖습니다.
*/
class RoomUnit
{
	using _RoomUnitTask = std::pair<ROOM_UNIT::TASK_TYPE, void*>;

public:
	RoomUnit(const _RoomKey roomKey);
	~RoomUnit();
	
	void ProduceTask(_RoomUnitTask taskUnit);
	
	_RoomKey GetKey() noexcept;
	bool GetIsRun() noexcept;
	bool GetIsFull() noexcept;

	void Init(std::wstring& roomName, const char maxUserNum, const char mapType);

private:
	// 방 생성 시, 돌아갈 쓰레드 함수입니다.
	void RoomThread();
	
private:
	const _RoomKey roomKey;

	std::wstring roomName;
	char maxUserNum;
	char nowUserNum;

	bool isRun = false;
	char mapType;
	_Time playTime = 0;
	
	std::array<RoomUserUnit*, ROOM_MAX_PLAYER> userCont; // 방에 접속한 유저에 대한 정보를 담은 컨테이너입니다.
	
	concurrency::concurrent_queue<_RoomUnitTask> taskCont;
	std::thread roomThread;

private:
	// for Timer
		// 우선순위 큐에 사용되는 비교 함수입니다.
	struct TimerUnitCompareFunction
	{
		_INLINE bool operator()(ROOM_UNIT::TimerUnit* left, ROOM_UNIT::TimerUnit* right) noexcept;
	};

	void TaskProcess(); // TimerThread 내에서, task에 따라 적절히 처리하는 함수입니다.
	void ProcessTaskUnit(_RoomUnitTask taskUnit); // 

	void TimerProcess(); // TimerThread 내에서, task에 따라 적절히 처리하는 함수입니다.
	void ProcessTimerUnit(ROOM_UNIT::TimerUnit* timerUnit); //

	void PushTimerUnit(ROOM_UNIT::TimerUnit* timerUnit); // 사용한 Timer Unit을 메모리풀에 넣는 함수입니다.
	ROOM_UNIT::TimerUnit* PopTimerUnit(); // Timer Unit을 사용하기 위해 메모리풀에서 빼는 함수입니다.

	concurrency::concurrent_priority_queue<ROOM_UNIT::TimerUnit*, TimerUnitCompareFunction> timerCont; // 시간 순서대로 정렬하여 넣는 컨테이너입니다.
	concurrency::concurrent_queue<ROOM_UNIT::TimerUnit*> timerMemoryPool; // timer Memory를 갖고 있는 메모리풀입니다.
};
