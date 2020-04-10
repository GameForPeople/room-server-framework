#pragma once

#include "../Common/global_header.hh"
#include "../Common/BaseSingleton.h"

struct UserUnit;
struct RoomUnit;

namespace MATCHING_MANAGER
{
	enum class TASK_TYPE
	{
		CREATE_ROOM
		, JOIN_ROOM
		, DESTORY_ROOM
	};

	// 방 생성 요청 시 필요한 정보입니다.
	struct CreateRoomTaskUnit
	{
		UserUnit* pUser;				// 방 생성을 요청한 유저
		char roomMaxUserNum;			// 방의 최고 인원 수
		char roomMapType;				// 방 맵 타입
		std::wstring roomName;			// 방 이름.
	};

	// 방 입장 요청 시 필요한 정보입니다.
	struct JoinRoomTaskUnit
	{
		UserUnit* pUser;				// 방 입장을 요청한 유저
		_RoomKey roomUniqueKey;			// 입장하고자 하는 방의 키
	};
}

/*
	MatchingManager
		- Room들에 대한 작업을 처리하는 Manager입니다.
		- 싱글턴 객체입니다.

	#0. Multi-producer로부터의 Task를 PPL Queue에 집어 넣고, 1개의 thread를 통해 처리합니다.
	#1. 1개의 쓰레드로 처리 중, 비용이 높은 작업에 대해서는 n개의 thread를 추가로 사용하여 처리합니다.
*/
class MatchingManager
	: public TSingleton<MatchingManager>
{
	static constexpr int MAX_ROOM_NUM = 50;	// 최대 생성 가능한 방 개수입니다.
	static constexpr int THREAD_LOOP_TIME = 500;
	using _MatchingManagerTask = std::pair<MATCHING_MANAGER::TASK_TYPE, void*>;

public:
	MatchingManager();
	~MatchingManager();

	DISABLED_COPY(MatchingManager)
	DISABLED_MOVE(MatchingManager)

	void ProduceTask(MATCHING_MANAGER::TASK_TYPE taskType, void* taskUnit);	// 외부로부터 Task를 등록받는 함수입니다.

private:
	void MatchingThread();	// 쓰레드 생성 시 호출하는 함수입니다.

	void CreateRoom(MATCHING_MANAGER::CreateRoomTaskUnit* const taskUnit);
	void JoinRoom(MATCHING_MANAGER::JoinRoomTaskUnit* const taskUnit);
	void DestoryRoom(RoomUnit* const pRooom);

private:
	std::thread matchingManagerThread;	// RoomThread() 함수를 돌리는 thread 객체입니다.
	concurrency::concurrent_queue<_MatchingManagerTask> taskCont; // ProduceTask를 통해 받은 task를 저장하는 컨테이너입니다. 
	
	std::list<_Key> waitingUserCont;
	
	concurrency::concurrent_queue<RoomUnit*> roomPool;	// 사용되지 않는 방의 포인터를 들고 있는 큐입니다. <- 싱글 스레드 변경으로 인해 일반 큐 변경 가능.
	std::map<_RoomKey, RoomUnit*> roomCont;

	// std::array<Room*, MAX_ROOM_NUM> roomCont; // 방을 저장하는 컨테이너입니다.
	// std::map<std::string, RoomUnit*> roomCont; // 방이름을 통해 방인덱스를 빠르게 찾기 위한 컨테이너입니다. -> 메인 룸 컨테이너로 승급했습니다.
};