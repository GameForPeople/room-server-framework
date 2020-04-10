#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct UserUnit;
struct TimerUnit;

/*
	TaskManager
		- 단순히 Task를 조립하고, task에 따른 함수를 실행하기 위한 Manager입니다.
		- 싱글턴 객체입니다.

	#0. 해당 클래스는 최초 구조 계획과는 다르게 역할이 많이 축소되었습니다. 단순히 패킷 조립만을 처리합니다.
	#1. 자체 쓰레드는 갖고 있지 않지만, I/O worker thread 개수 만큼 병렬적으로 실행됩니다.

	!0. 수정 피드백을 가장 많이 받은 객체입니다! 각 task에 대한 함수 분할이 요구됩니다.
	!1. 해당 manager는 현재 구조상, 상수가 아닌 변수를 갖지 않아야합니다.
*/

class TaskManager
	: public TSingleton<TaskManager>
{
public:
	TaskManager();
	~TaskManager();

	DISABLED_COPY(TaskManager)
	DISABLED_MOVE(TaskManager)

	void ProduceTask(UserUnit* userUnit);	// 유저에게 받은 스트림을 조립합니다.

private:
	void LoginSceneTask(UserUnit* userUnit);
	void LobbySceneTask(UserUnit* userUnit);
	void RoomSceneTask(UserUnit* userUnit);

private:
	std::thread retryThread;	// 재시도 쓰레드 입니다.
	concurrency::concurrent_queue<TimerUnit*> retryTaskQueue; // 재시도 쓰레드 Task Quee입니다.
};
