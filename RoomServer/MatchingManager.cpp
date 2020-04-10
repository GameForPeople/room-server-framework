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
	// 방을 Max Num 까지 메모리풀에 미리 할당합니다.
	for (int i = 0; i < MAX_ROOM_NUM; ++i) { roomPool.push(new RoomUnit(static_cast<_RoomKey>(i))); }

	// 방 쓰레드를 생성합니다.
	matchingManagerThread = static_cast<std::thread>([&]() { this->MatchingThread(); });
}

MatchingManager::~MatchingManager()
{
	// 만든 방 메모리들을 운영체제에게 반납합니다.
	RoomUnit* roomUnit = nullptr;
	while (roomPool.try_pop(roomUnit))
	{
		delete roomUnit;
	}

	for (auto& room : roomCont)
	{
		delete room.second;
	}

	// RoomManager의 등록된 Task 메모리들을 모두 반납합니다. 
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
		// 최대 방 개수만큼 방을 이미 생성했습니다. 실패했습니다.

		PrintLog(SOURCE_LOCATION, "방 생성에 실패하였습니다.");
	}
	
	retRoom->Init(taskUnit->roomName, taskUnit->roomMaxUserNum, taskUnit->roomMapType);

	roomCont.emplace(std::make_pair(retRoom->GetKey(), retRoom));
	PrintLog(SOURCE_LOCATION, "방을 생성하였습니다.");
}

void MatchingManager::JoinRoom(MATCHING_MANAGER::JoinRoomTaskUnit* const taskUnit)
{
	if(auto roomIter = roomCont.find(taskUnit->roomUniqueKey)
		; roomIter == roomCont.end())
	{
		PrintLog(SOURCE_LOCATION, "해당하는 방이 없어 입장하지 못했습니다.");
	}
	else
	{
		auto pRoom = roomIter->second;
		if (pRoom->GetIsRun())
		{
			PrintLog(SOURCE_LOCATION, "해당 방은 이미 게임이 시작 중이라 접속하지 못했습니다.");
		}
		else if (pRoom->GetIsFull())
		{
			PrintLog(SOURCE_LOCATION, "해당 방은 이미 게임이 시작 중이라 접속하지 못했습니다.");
		}
		else
		{
			PrintLog(SOURCE_LOCATION, "방에 입장하였습니다.");
		}
	}
}

void MatchingManager::DestoryRoom(RoomUnit* const pRoom)
{
	roomCont.erase(pRoom->GetKey());
	roomPool.push(pRoom);

	PrintLog(SOURCE_LOCATION, "매칭매니저에서 사용하지 않는 방을 회수했습니다.");
}

