#include "stdafx.h"

#include "UserUnit.h"
#include "TaskManager.h"

#include "RoomUnit.h"

#include "NetworkManager.h"
#include "UserManager.h"
#include "RoomManager.h"
#include "TimerManager.h"
#include "Utils.h"

TaskManager::TaskManager()
	// : taskTypeCont()
{
}

TaskManager::~TaskManager()
{
	TimerUnit* retTimerUnit;

	while (retryTaskQueue.try_pop(retTimerUnit))
	{
		delete retTimerUnit;
	}
}

void TaskManager::ProduceTask(UserUnit* userUnit)
{
	using namespace PACKET_TYPE;
	std::cout << "받은 Task는 : " << magic_enum::enum_name<CLIENT_TO_SERVER>(static_cast<CLIENT_TO_SERVER>(reinterpret_cast<PACKET_DATA::BasePacket*>
		(userUnit->taskManagerUnit.loadedBuffer)->packetType)) << std::endl;

	switch (static_cast<CLIENT_TO_SERVER>(reinterpret_cast<PACKET_DATA::BasePacket*>
		(userUnit->taskManagerUnit.loadedBuffer)->packetType))
	{
	//---------------------------------------------------------------------------------------------------------
	case CLIENT_TO_SERVER::SIGN_UP:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::SignUp*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		UserManager::GetInstance().ProduceTask(TASK_TYPE::USER_SIGNUP, new USER_MANAGER::SignUpTaskUnit{ userUnit->key, std::wstring(packet->id), std::wstring(packet->pw), std::wstring(packet->nickname) });
		break;
	}
	case CLIENT_TO_SERVER::LOGIN:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::Login*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		UserManager::GetInstance().ProduceTask(TASK_TYPE::USER_LOGIN, new USER_MANAGER::LoginTaskUnit{ userUnit->key, packet->id, packet->pw });
		break;
	}
	// case CLIENT_TO_SERVER::LOBBY_INFO:
	// {
	// 	UserManager::GetInstance().ProduceTask(TASK_TYPE::LOBBY_INFO, reinterpret_cast<void*>(userUnit->key));
	// 	break;
	// }
	case CLIENT_TO_SERVER::LOBBY_CHAT:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::LobbyChat*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		UserManager::GetInstance().ProduceTask(TASK_TYPE::LOBBY_CHAT, new USER_MANAGER::ChatLobbyTaskUnit({ userUnit->key, packet->chat }));
		break;
	}
	case CLIENT_TO_SERVER::CHANGE_CHARACTER:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::ChangeCharacter*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		UserManager::GetInstance().ProduceTask(TASK_TYPE::CHANGE_CHARACTER, new USER_MANAGER::ChangeCharacterTaskUnit({ userUnit->key, packet->characterIndex }));
		break;
	}
	case CLIENT_TO_SERVER::LOBBY_USER_INFO:
	{
		UserManager::GetInstance().ProduceTask(TASK_TYPE::LOBBY_USER_INFO, reinterpret_cast<void*>(userUnit->key));
		break;
	}

	case CLIENT_TO_SERVER::RANDOM_LOGIN:
	{
		UserManager::GetInstance().ProduceTask(TASK_TYPE::RANDOM_LOGIN, reinterpret_cast<void*>(userUnit->key));
		break;
	}

	//---------------------------------------------------------------------------------------------------------
	case CLIENT_TO_SERVER::RANDOM_MATCH:
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::RANDOM_MATCH, reinterpret_cast<void*>(userUnit->key));
		break;
	}
	case CLIENT_TO_SERVER::CANCLE_MATCH:
	{
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::CANCLE_MATCH, reinterpret_cast<void*>(userUnit->key));
		break;
	}
	//---------------------------------------------------------------------------------------------------------
	case CLIENT_TO_SERVER::CHANGE_DIRECTION:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::ChangeDirection*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::CHANGE_DIRECTION, new ROOM_UNIT::ChangeDirection({ userUnit->key, packet->direction }));
		break;
	}
	case CLIENT_TO_SERVER::DO_ROLL:
	{
		//auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::Roll*>(userUnit->taskManagerUnit.loadedBuffer);
		userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::DO_ROLL, reinterpret_cast<void*>(userUnit->key));
		break;
	}
	case CLIENT_TO_SERVER::EMOJI:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::Emoji*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::EMOJI, new ROOM_UNIT::EmojiTaskUnit({ userUnit->key, packet->emojiType }));
		break;
	}
	case CLIENT_TO_SERVER::BLOCK:
	{
		auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::Block*>(userUnit->taskManagerUnit.loadedBuffer + 2);
		
		if (packet->isOnBlock == 0)
		{
			userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::DO_BLOCKING, reinterpret_cast<void*>(userUnit->key));
		}
		else
		{
			userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::RELEASE_BLOCKING, reinterpret_cast<void*>(userUnit->key));
		}
		break;
	}
	case CLIENT_TO_SERVER::KICK:
	{
		userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::DO_KICK, reinterpret_cast<void*>(userUnit->key));
		break;
	}
	case CLIENT_TO_SERVER::LOADING_END:
	{
		//auto packet = reinterpret_cast<PACKET_DATA::CLIENT_TO_SERVER::Emoji*>(userUnit->taskManagerUnit.loadedBuffer);
		//userUnit->roomManagerUnit.GetRoom()->ProduceTask(TASK_TYPE::LOADING_END, reinterpret_cast<void*>(userUnit->key));
		//userUnit->roomManagerUnit.GetRoom()->LoadingEnd(userUnit->key);
		RoomManager::GetInstance().ProduceTask(TASK_TYPE::LOADING_END, reinterpret_cast<void*>(userUnit->key));
		break;
	}
	//--------------------------------------------------------------------------------------------------------------
	case CLIENT_TO_SERVER::LOGOUT:
	{
		PrintLog(SOURCE_LOCATION, "로그아웃 : " + userUnit->key);
		UserManager::GetInstance().ProduceTask(TASK_TYPE::USER_LOGOUT, new USER_MANAGER::LogoutTaskUnit({ userUnit->key, true }));
		break;
	}
	default:
	{
		PrintLog(SOURCE_LOCATION, "미정의 패킷 수신. 패킷 타입 : " 
			+ std::to_string((int)((PACKET_DATA::BasePacket*)(userUnit->taskManagerUnit.loadedBuffer))->packetType));
		break;
	}
	}
}

void TaskManager::LoginSceneTask(UserUnit* userUnit)
{
}

void TaskManager::LobbySceneTask(UserUnit* userUnit)
{
}

void TaskManager::RoomSceneTask(UserUnit* userUnit)
{
}
