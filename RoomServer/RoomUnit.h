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
		_Key ownerKey;	// �׻� ������ ���� �ƴ�����, ���� ����.

		void* data;
		// std::any data;
	};
}

/*
	Room
		- ���Դϴ�.

	#0. �� �濡 ���ؼ��� Single Thread Access�� ����˴ϴ�.
	#1. ���������� roomUserCont�� ���� roomUser�� ���� ������ �����ϴ�.
		- room���� ó���Ǵ� ��� ���� ������ UserCont�� �������� �ʰ�,
		roomUser�� ī�ǵ� �����͵�� ó���ϴ� ���� ��Ģ���� �մϴ�.
	#2. �߰������� �� �̸��� ȣ��Ʈ User�� �����͸� �����ϴ�.
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
	// �� ���� ��, ���ư� ������ �Լ��Դϴ�.
	void RoomThread();
	
private:
	const _RoomKey roomKey;

	std::wstring roomName;
	char maxUserNum;
	char nowUserNum;

	bool isRun = false;
	char mapType;
	_Time playTime = 0;
	
	std::array<RoomUserUnit*, ROOM_MAX_PLAYER> userCont; // �濡 ������ ������ ���� ������ ���� �����̳��Դϴ�.
	
	concurrency::concurrent_queue<_RoomUnitTask> taskCont;
	std::thread roomThread;

private:
	// for Timer
		// �켱���� ť�� ���Ǵ� �� �Լ��Դϴ�.
	struct TimerUnitCompareFunction
	{
		_INLINE bool operator()(ROOM_UNIT::TimerUnit* left, ROOM_UNIT::TimerUnit* right) noexcept;
	};

	void TaskProcess(); // TimerThread ������, task�� ���� ������ ó���ϴ� �Լ��Դϴ�.
	void ProcessTaskUnit(_RoomUnitTask taskUnit); // 

	void TimerProcess(); // TimerThread ������, task�� ���� ������ ó���ϴ� �Լ��Դϴ�.
	void ProcessTimerUnit(ROOM_UNIT::TimerUnit* timerUnit); //

	void PushTimerUnit(ROOM_UNIT::TimerUnit* timerUnit); // ����� Timer Unit�� �޸�Ǯ�� �ִ� �Լ��Դϴ�.
	ROOM_UNIT::TimerUnit* PopTimerUnit(); // Timer Unit�� ����ϱ� ���� �޸�Ǯ���� ���� �Լ��Դϴ�.

	concurrency::concurrent_priority_queue<ROOM_UNIT::TimerUnit*, TimerUnitCompareFunction> timerCont; // �ð� ������� �����Ͽ� �ִ� �����̳��Դϴ�.
	concurrency::concurrent_queue<ROOM_UNIT::TimerUnit*> timerMemoryPool; // timer Memory�� ���� �ִ� �޸�Ǯ�Դϴ�.
};
