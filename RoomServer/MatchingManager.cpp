#include "stdafx.h"

#include "UserUnit.h"
#include "RoomUnit.h"
#include "MatchingManager.h"

#include "TimerManager.h"
#include "NetworkManager.h"
#include "UserManager.h"
#include "Utils.h"

MatchingManager::MatchingManager()
	: matchingManagerThread()
	, taskCont()
	, waitingUserCont()
	, roomPool()
	//, roomCont()
{
	// ���� Max Num ���� �޸�Ǯ�� �̸� �Ҵ��մϴ�.
	for (int i = 0; i < MAX_ROOM_NUM; ++i) { roomPool.push(new RoomUnit(static_cast<_RoomKey>(i))); }

	// �� �����带 �����մϴ�.
	matchingManagerThread = static_cast<std::thread>([&]() { this->MatchingThread(); });
}

MatchingManager::~MatchingManager()
{
	// ���� �� �޸𸮵��� �ü������ �ݳ��մϴ�.
	RoomUnit* roomUnit = nullptr;
	while (roomPool.try_pop(roomUnit))
	{
		delete roomUnit;
	}

	for (auto& room : roomCont)
	{
		delete room.second;
	}

	// RoomManager�� ��ϵ� Task �޸𸮵��� ��� �ݳ��մϴ�. 
	_MatchingManagerTask roomManagerTask;
	
	while (taskCont.try_pop(roomManagerTask))
	{
		switch (roomManagerTask.first)
		{
		default:
			break;
		}
	}
}

void MatchingManager::MatchingThread()
{
	_MatchingManagerTask task{};

	while (7)
	{
		if (!taskCont.try_pop(task))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_LOOP_TIME));
			//std::this_thread::yield();
			continue;
		}
	
		using namespace MATCHING_MANAGER;

		switch (task.first)
		{
		case MATCHING_MANAGER::TASK_TYPE::CREATE_ROOM:
			CreateRoom(reinterpret_cast<CreateRoomTaskUnit*>(task.second));
			break;
		case MATCHING_MANAGER::TASK_TYPE::JOIN_ROOM:
			JoinRoom(reinterpret_cast<JoinRoomTaskUnit*>(task.second));
			break;
		case MATCHING_MANAGER::TASK_TYPE::DESTORY_ROOM:
			DestoryRoom(static_cast<RoomUnit*>(task.second));
			break;
		default:
			break;
		}
	}
}

_INLINE void MatchingManager::ProduceTask(MATCHING_MANAGER::TASK_TYPE taskType, void* taskUnit)
{
	taskCont.push({ taskType , taskUnit });
}

void MatchingManager::CreateRoom(MATCHING_MANAGER::CreateRoomTaskUnit* const taskUnit)
{
	RoomUnit* retRoom{ nullptr };

	if (!roomPool.try_pop(retRoom))
	{
		// �ִ� �� ������ŭ ���� �̹� �����߽��ϴ�. �����߽��ϴ�.

		PrintLog(SOURCE_LOCATION, "�� ������ �����Ͽ����ϴ�.");
	}
	
	retRoom->Init(taskUnit->roomName, taskUnit->roomMaxUserNum, taskUnit->roomMapType);

	roomCont.emplace(std::make_pair(retRoom->GetKey(), retRoom));
	PrintLog(SOURCE_LOCATION, "���� �����Ͽ����ϴ�.");
}

void MatchingManager::JoinRoom(MATCHING_MANAGER::JoinRoomTaskUnit* const taskUnit)
{
	if(auto roomIter = roomCont.find(taskUnit->roomUniqueKey)
		; roomIter == roomCont.end())
	{
		PrintLog(SOURCE_LOCATION, "�ش��ϴ� ���� ���� �������� ���߽��ϴ�.");
	}
	else
	{
		auto pRoom = roomIter->second;
		if (pRoom->GetIsRun())
		{
			PrintLog(SOURCE_LOCATION, "�ش� ���� �̹� ������ ���� ���̶� �������� ���߽��ϴ�.");
		}
		else if (pRoom->GetIsFull())
		{
			PrintLog(SOURCE_LOCATION, "�ش� ���� �̹� ������ ���� ���̶� �������� ���߽��ϴ�.");
		}
		else
		{
			PrintLog(SOURCE_LOCATION, "�濡 �����Ͽ����ϴ�.");
		}
	}
}

void MatchingManager::DestoryRoom(RoomUnit* const pRoom)
{
	roomCont.erase(pRoom->GetKey());
	roomPool.push(pRoom);

	PrintLog(SOURCE_LOCATION, "��Ī�Ŵ������� ������� �ʴ� ���� ȸ���߽��ϴ�.");
}

