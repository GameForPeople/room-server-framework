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

	// �� ���� ��û �� �ʿ��� �����Դϴ�.
	struct CreateRoomTaskUnit
	{
		UserUnit* pUser;				// �� ������ ��û�� ����
		char roomMaxUserNum;			// ���� �ְ� �ο� ��
		char roomMapType;				// �� �� Ÿ��
		std::wstring roomName;			// �� �̸�.
	};

	// �� ���� ��û �� �ʿ��� �����Դϴ�.
	struct JoinRoomTaskUnit
	{
		UserUnit* pUser;				// �� ������ ��û�� ����
		_RoomKey roomUniqueKey;			// �����ϰ��� �ϴ� ���� Ű
	};
}

/*
	MatchingManager
		- Room�鿡 ���� �۾��� ó���ϴ� Manager�Դϴ�.
		- �̱��� ��ü�Դϴ�.

	#0. Multi-producer�κ����� Task�� PPL Queue�� ���� �ְ�, 1���� thread�� ���� ó���մϴ�.
	#1. 1���� ������� ó�� ��, ����� ���� �۾��� ���ؼ��� n���� thread�� �߰��� ����Ͽ� ó���մϴ�.
*/
class MatchingManager
	: public TSingleton<MatchingManager>
{
	static constexpr int MAX_ROOM_NUM = 50;	// �ִ� ���� ������ �� �����Դϴ�.
	static constexpr int THREAD_LOOP_TIME = 500;
	using _MatchingManagerTask = std::pair<MATCHING_MANAGER::TASK_TYPE, void*>;

public:
	MatchingManager();
	~MatchingManager();

	DISABLED_COPY(MatchingManager)
	DISABLED_MOVE(MatchingManager)

	void ProduceTask(MATCHING_MANAGER::TASK_TYPE taskType, void* taskUnit);	// �ܺηκ��� Task�� ��Ϲ޴� �Լ��Դϴ�.

private:
	void MatchingThread();	// ������ ���� �� ȣ���ϴ� �Լ��Դϴ�.

	void CreateRoom(MATCHING_MANAGER::CreateRoomTaskUnit* const taskUnit);
	void JoinRoom(MATCHING_MANAGER::JoinRoomTaskUnit* const taskUnit);
	void DestoryRoom(RoomUnit* const pRooom);

private:
	std::thread matchingManagerThread;	// RoomThread() �Լ��� ������ thread ��ü�Դϴ�.
	concurrency::concurrent_queue<_MatchingManagerTask> taskCont; // ProduceTask�� ���� ���� task�� �����ϴ� �����̳��Դϴ�. 
	
	std::list<_Key> waitingUserCont;
	
	concurrency::concurrent_queue<RoomUnit*> roomPool;	// ������ �ʴ� ���� �����͸� ��� �ִ� ť�Դϴ�. <- �̱� ������ �������� ���� �Ϲ� ť ���� ����.
	std::map<_RoomKey, RoomUnit*> roomCont;

	// std::array<Room*, MAX_ROOM_NUM> roomCont; // ���� �����ϴ� �����̳��Դϴ�.
	// std::map<std::string, RoomUnit*> roomCont; // ���̸��� ���� ���ε����� ������ ã�� ���� �����̳��Դϴ�. -> ���� �� �����̳ʷ� �±��߽��ϴ�.
};