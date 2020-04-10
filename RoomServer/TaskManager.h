#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct UserUnit;
struct TimerUnit;

/*
	TaskManager
		- �ܼ��� Task�� �����ϰ�, task�� ���� �Լ��� �����ϱ� ���� Manager�Դϴ�.
		- �̱��� ��ü�Դϴ�.

	#0. �ش� Ŭ������ ���� ���� ��ȹ���� �ٸ��� ������ ���� ��ҵǾ����ϴ�. �ܼ��� ��Ŷ �������� ó���մϴ�.
	#1. ��ü ������� ���� ���� ������, I/O worker thread ���� ��ŭ ���������� ����˴ϴ�.

	!0. ���� �ǵ���� ���� ���� ���� ��ü�Դϴ�! �� task�� ���� �Լ� ������ �䱸�˴ϴ�.
	!1. �ش� manager�� ���� ������, ����� �ƴ� ������ ���� �ʾƾ��մϴ�.
*/

class TaskManager
	: public TSingleton<TaskManager>
{
public:
	TaskManager();
	~TaskManager();

	DISABLED_COPY(TaskManager)
	DISABLED_MOVE(TaskManager)

	void ProduceTask(UserUnit* userUnit);	// �������� ���� ��Ʈ���� �����մϴ�.

private:
	void LoginSceneTask(UserUnit* userUnit);
	void LobbySceneTask(UserUnit* userUnit);
	void RoomSceneTask(UserUnit* userUnit);

private:
	std::thread retryThread;	// ��õ� ������ �Դϴ�.
	concurrency::concurrent_queue<TimerUnit*> retryTaskQueue; // ��õ� ������ Task Quee�Դϴ�.
};
