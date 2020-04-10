#pragma once

#ifndef FOR_SERVER
#endif

#pragma region [macro & Attributes]

// macro
#define DISABLED_COPY(type) public: type(const type&) = delete; type& operator=(const type&) = delete;
#define DISABLED_MOVE(type) public: type(type&&) = delete; type& operator=(type&&) = delete;

// Attributes
#define	_NORETURN		[[noreturn]]
#define	_DEPRECATED		[[deprecated]]
#define	_MAYBE_UNUSED	[[maybe_unused]]
#define	_FALLTHROUGH	[[fallthrough]]
#define	_NODISCARD		[[nodiscard]]
#define _INLINE			inline

// #define lock() ERROR
#pragma endregion

#pragma region [const]
namespace GLOBAL
{
#ifdef FOR_SERVER
	inline namespace USING
	{
		enum DEFINE_USING
		{
			NICKNAME_SIZE = 10,
			ID_SIZE = 10,
			PW_SIZE = 10,
			CHAT_MESSAGE_LEN = 50
		};

		using _Time = unsigned long long;
		using _Count = int;
		using _Type = char;
		using _WChar = wchar_t;
		//using _String = std::string;

		using _Key = short;
		using _RoomKey = short;

		using _ID = _WChar[ID_SIZE];
		using _PW = _WChar[PW_SIZE];
		using _Nickname = _WChar[NICKNAME_SIZE];
		using _Chat = _WChar[CHAT_MESSAGE_LEN];
		using _Money = int;
		using _Level = char;
		using _Character = char;
		using _Direction = char;
	};
#else
	inline namespace USING
	{
	};
#endif
	const std::string PUBLIC_SERVER_IP = "127.0.0.1"; // ?
	const std::string LOCAL_HOST_IP = "127.0.0.1";

	const std::string VERSION = "ver 1.1";
	const std::string DATE = "2020/03/26";

	constexpr unsigned short SERVER_LISTEN_PORT_NUMBER = 9000;
	constexpr _Key MAX_USER = 10000;
	constexpr int DATA_BUFFER_SIZE = 1024;

	constexpr int ROOM_MAX_PLAYER = 4;
	constexpr int MAX_BALL_NUM = 20;	// 방에서 스폰할 최대 공의 개수입니다.

	struct PosForPacket
	{
		float posX;
		float posY;
	};

}using namespace GLOBAL;
#pragma endregion

#pragma region [protocol]
namespace PACKET_TYPE
{
	enum class SERVER_TO_CLIENT : char
	{
		// Login Scene ============================================
		ACCEPT_O = 0,			// 엑셉트 실패
		ACCEPT_X = 1,			// 엑셉트 실패
		
		SIGN_UP_O = 2,			// 회원가입 성공
		SIGN_UP_X = 3,			// 회원가입 실패

		LOGIN_O = 4,			// 로그인 성공
		LOGIN_X = 5,			// 로그인 실패

		// Lobby Scene ============================================
		LOBBY_INFO = 6,			// 로비에 필요한 정보
		LOBBY_CHAT = 7,			// 다른 유저의 채팅

		// RANDOM_MATCH,		// 큐 접속 성공..  일단 불필요. (실패할일 없음)
		CANCLE_MATCH_O = 8,		// 매치 취소 성공.

		START_MATCH = 9,		// 매치 시작과, 방 다른 유저 정보
		END_MATCH = 10,
		
		// Room Scene ============================================
		FIXED_MATCH_INFO = 11,	// 일정 프레임마다, 고정으로 보낼 데이터
		USER_DAMAGED = 12,		// 일정 유저가 데미지를 입은 것을 전달함.
		//USER,
		USER_ROLL = 13,			// 일정 유저가 구른 정보를 전달함.
		USER_EMOJI = 14,			// 특정 유저가 이모지를 전달함.
		USER_CHANGE_DIRECTION = 15,

		// ADD_ITEM,			// 아이템...이거 아직 고민중입니다.
		// MOVE_ITEM,			// 아이템...이거 아직 고민중입니다.

		// Other! ============================================
		LOGOUT = 16,

		//
		QUEUE_MATCHED = 17, //  큐가 잡혀서 방이 만들어 질때 // 레벨,  닉네임, 캐릭터 정보
		
		SPAWN_ITEM = 18,	// 아이템 소환시 전송하는 패킷!
		POSITION_ITEM = 19,	// 아이템 위치 전송
		REMOVE_ITEM = 20,
		SPAWN_WALL = 21,	// 벽 생성 시
		POSITION_WALL = 22,	// 벽 움직일 시
		REMOVE_WALL = 23,	// 벽 제거 시
		START_WAVE = 24,
		MOVE_FIREWALL = 25,
		USER_KICK = 26,
		USER_KICK_DAMAGED = 27,
		USER_BLOCKING = 28,
		REMAIN_TIME = 29,
		JOIN_USER_INFO = 30,

		INGAME_RANK = 31
	};

	enum class CLIENT_TO_SERVER : char
	{
		// Login Scene ============================================
		SIGN_UP = 0,			// 회원가입
		LOGIN = 1,				// 로그인

		// Lobby Scene ============================================
		LOBBY_INFO = 2,			// 유저 정보
		LOBBY_CHAT = 3,			// 채팅 입력

		RANDOM_MATCH = 4,		// 매치 큐 요청
		CANCLE_MATCH = 5,

		// ROOM_SCENE ============================================
		CHANGE_DIRECTION = 6,	// 방향 변경
		DO_ROLL = 7,			// 구르기 시도
		EMOJI = 8				// 감정

#ifdef FOR_SERVER
		// For Only Server ============================================

		, LOGOUT = 9			// 로그아웃 처리. 
#endif
		, LOADING_END = 10		// 클라에서 로딩 끝나면 2바이트
		, CHANGE_CHARACTER = 11	// 클라에서 액티브 캐릭터를 변경했을 때
		, BLOCK = 12	// 블로킹
		, KICK = 13		// 킥!
		, LOBBY_USER_INFO = 14	//로비에 들어갔을 떄, 유저 정보 재부탁

		, RANDOM_LOGIN = 15
	};
}

#pragma pack(push, 1)
namespace PACKET_DATA
{
	struct BasePacket
	{
		const unsigned char size;
		const _Type packetType;

		BasePacket(const unsigned char size, const char packetType)
			: size(size)
			, packetType(packetType)
			// typename remove_pointer<this>::type();
		{}
	};

	namespace SERVER_TO_CLIENT
	{
		struct AcceptO : public BasePacket
		{
			AcceptO() noexcept
				: BasePacket(sizeof(AcceptO), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::ACCEPT_O))
			{}
		};

		struct AcceptX : public BasePacket
		{
			const _Type failType;	// 실패 사유

			AcceptX(const _Type failType) noexcept
				: BasePacket(sizeof(AcceptX), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::ACCEPT_X))
				, failType(failType)
			{}
		};

		struct SignUpO : public BasePacket
		{
			SignUpO() noexcept
				: BasePacket(sizeof(SignUpO), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::SIGN_UP_O))
			{}
		};

		struct SignUpX : public BasePacket
		{
			const _Type failType;	// 실패 사유

			SignUpX(const _Type failType) noexcept
				: BasePacket(sizeof(SignUpX), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::SIGN_UP_X))
				, failType(failType)
			{}
		};

		struct LoginO : public BasePacket
		{
			// userInfo
			const _Level level;		// 레벨
			const _Money money;		// 소유 재화
			const _Count winCount;	// 이긴 횟수
			const _Count loseCount;	// 진 횟수
			const _Count pickedCharacterIndex; // 액티브 캐릭터
			_Nickname nickname;	// 닉네임

			LoginO( const _Level level
				, const _Money money
				, const _Count winCount
				, const _Count loseCount
				, const _Count pickedCharacterIndex
				, const std::wstring& nickname ) noexcept
				: BasePacket(sizeof(LoginO), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOGIN_O))
				, level(level)
				, money(money)
				, winCount(winCount)
				, loseCount(loseCount)
				, pickedCharacterIndex(pickedCharacterIndex)
				, nickname()
			{
				wmemcpy(this->nickname, nickname.c_str(), nickname.length());
			}
		};

		struct LoginX : public BasePacket
		{
			const _Type failType;	// 실패 사유

			LoginX(const _Type failType) noexcept
				: BasePacket(sizeof(LoginX), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOGIN_X))
				, failType(failType)
			{}
		};

		struct JoinLobbyInfo : public BasePacket
		{
			const _Level level;		// 레벨
			const _Money money;		// 소유 재화
			const _Count winCount;	// 이긴 횟수
			const _Count loseCount;	// 진 횟수
			const _Count pickedCharacterIndex; // 액티브 캐릭터

			JoinLobbyInfo(const _Level level
				, const _Money money
				, const _Count winCount
				, const _Count loseCount
				, const _Count pickedCharacterIndex) noexcept
				: BasePacket(sizeof(JoinLobbyInfo), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::JOIN_USER_INFO))
				, level(level)
				, money(money)
				, winCount(winCount)
				, loseCount(loseCount)
				, pickedCharacterIndex(pickedCharacterIndex)
			{
			}
		};

		/*
		struct LobbyInfo : public BasePacket
		{
			static constexpr _Count NICKNAMES_COUNT = 10;

		public:
			// const _Count allUser;
			// const _Count lobbyUser;
			_Count allUser;
			_Count lobbyUser;

			_Nickname nicknames[NICKNAMES_COUNT];

			LobbyInfo()
				: BasePacket(sizeof(LobbyInfo), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOBBY_INFO))
			{}

			LobbyInfo(const _Count allUser, const _Count lobbyUser, const _Nickname nicknames) noexcept
				: BasePacket(sizeof(LobbyInfo), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOBBY_INFO))
				, allUser(allUser)
				, lobbyUser(lobbyUser)
				, nicknames()
			{
				memcpy(this->nicknames, nicknames, NICKNAMES_COUNT * DEFINE_USING::NICKNAME_SIZE * 2);
			}
		};
		*/

		struct LobbyChat : public BasePacket
		{
			_Nickname nickname;
			_Chat chatMessage;

			LobbyChat(std::wstring nickname, _Chat chatMessage) noexcept
				: BasePacket(sizeof(LobbyChat), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOBBY_CHAT))
				, nickname()
				, chatMessage()
			{
				wmemcpy(this->nickname, nickname.c_str(), nickname.size());
				wmemcpy(this->chatMessage, chatMessage, CHAT_MESSAGE_LEN);
			}
		};

		// struct RandomMatch : public BasePacket
		// {
		// 	const _Type gameMode;
		// 
		// 	RandomMatch(const _Type gameMode)
		// 		: BasePacket(sizeof(RandomMatch), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::RANDOM_MATCH))
		// 		, gameMode(gameMode)
		// 	{}
		// };

		struct CancleMatching : public BasePacket
		{
			CancleMatching()
				: BasePacket(sizeof(CancleMatching), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::CANCLE_MATCH_O))
			{
			}
		};
		
		struct QueueMatching : public BasePacket
		{
			char myRoomIndex;	// 바이트 희망
			_Nickname nicknames[ROOM_MAX_PLAYER];
			_Level levels[ROOM_MAX_PLAYER];
			_Type characters[ROOM_MAX_PLAYER];

			QueueMatching(_Key myRoomIndex, const std::vector<std::wstring>& nicknames,
				const std::vector<_Level>& levels, const std::vector<_Character>& characters)
				: BasePacket(sizeof(QueueMatching), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::QUEUE_MATCHED))
				, myRoomIndex(myRoomIndex)
				, nicknames()
				, levels()
				, characters()
			{
				for (int i = 0; i < nicknames.size(); ++i)
				{
					wmemcpy(this->nicknames[i], nicknames[i].c_str(), nicknames[i].size());
				}

				for (int i = 0; i < levels.size(); ++i)
				{
					this->levels[i] = levels[i];
				}

				for (int i = 0; i < characters.size(); ++i)
				{
					this->characters[i] = characters[i];
				}
			}
		};

		struct StartMatch : public BasePacket
		{
			StartMatch() noexcept
				: BasePacket(sizeof(StartMatch), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::START_MATCH))
			{
			}
		};

		struct FixedMatchInfo : public BasePacket
		{
			PosForPacket userPositions[ROOM_MAX_PLAYER];
			PosForPacket ballPositions[MAX_BALL_NUM];

			FixedMatchInfo(const std::array<PosForPacket, ROOM_MAX_PLAYER>& userPositions, const std::array<PosForPacket, MAX_BALL_NUM>& ballPositions) noexcept
				: BasePacket(sizeof(FixedMatchInfo), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::FIXED_MATCH_INFO))
				, userPositions()
			 	, ballPositions()
			{
				// memcpy(this->userPositions, userPositions.data(), sizeof(this->userPositions));
				// memcpy(this->ballPositions, ballPositions.data(), sizeof(this->ballPositions));
				for (int i = 0; i < ROOM_MAX_PLAYER; ++i)
				{
					this->userPositions[i] = userPositions[i];
				}

				for (int i = 0; i < MAX_BALL_NUM; ++i)
				{
					this->ballPositions[i] = ballPositions[i];
				}
			}
		};

		struct UserChangeDirection : public BasePacket
		{
			const _Count userIndex;
			const _Direction dir;

			UserChangeDirection(const _Count userIndex, const _Count dir) noexcept
				: BasePacket(sizeof(UserChangeDirection), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_CHANGE_DIRECTION))
				, userIndex(userIndex)
				, dir(dir)
			{
			}
		};

		struct UserDamaged : public BasePacket
		{
			const char damagedUserIndex;
			const char newHp;	// 이거 사실 필요없는데.

			UserDamaged(const _Count damagedUserIndex, const _Count newHp) noexcept
				: BasePacket(sizeof(UserDamaged), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_DAMAGED))
				, damagedUserIndex(damagedUserIndex)
				, newHp(newHp)
			{
			}
		};

		struct UserRolled : public BasePacket
		{
			const _Count rolledUserIndex;
			const _Direction direction;

			UserRolled(const _Count rolledUserIndex, const _Direction direction) noexcept
				: BasePacket(sizeof(UserRolled), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_ROLL))
				, rolledUserIndex(rolledUserIndex)
				, direction(direction)
			{
			}
		};

		struct UserEmoji : public BasePacket
		{
			const _Type emojiUserIndex;
			const _Type emojiType;

			UserEmoji(const _Type emojiUserIndex, const _Type emojiType) noexcept
				: BasePacket(sizeof(UserEmoji), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_EMOJI))
				, emojiUserIndex(emojiUserIndex)
				, emojiType(emojiType)
			{
			}
		};

		struct UserKick : public BasePacket
		{
			const _Type kickUserIndex;

			UserKick(const _Type kickUserIndex) noexcept
				: BasePacket(sizeof(UserKick), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_KICK))
				, kickUserIndex(kickUserIndex)
			{
			}
		};

		struct UserKickDamaged : public BasePacket
		{
			const _Type kickUserIndex;
			const _Direction oldUserDir;
			const _Type kickerIndex;

			UserKickDamaged(const _Type kickUserIndex, const _Direction oldUserDir, const _Type kickerIndex) noexcept
				: BasePacket(sizeof(UserKickDamaged), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_KICK_DAMAGED))
				, kickUserIndex(kickUserIndex)
				, oldUserDir(oldUserDir)
				, kickerIndex(kickerIndex)
			{
			}
		};

		struct UserBlock : public BasePacket
		{
			const _Type blockUserIndex;
			const _Type isOnBlock;

			UserBlock(const _Type blockUserIndex, const _Type isOnBlock) noexcept
				: BasePacket(sizeof(UserBlock), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::USER_BLOCKING))
				, blockUserIndex(blockUserIndex)
				, isOnBlock(isOnBlock)
			{
			}
		};

		struct EndMatchInfo
		{
			char id;
			char survivalTime;
			char rank;
		};

		enum
		{
			ITEM_TOTAL_SIZE = 7
		};

		struct EndMatch : public BasePacket
		{
			EndMatchInfo matchInfo[ROOM_MAX_PLAYER];
			char getItemCountCont[ITEM_TOTAL_SIZE];

			EndMatch(
				std::array<char, ROOM_MAX_PLAYER>& roomIdCont,
				std::array<char, ROOM_MAX_PLAYER>& survivalTimeCont,
				std::array<char, ROOM_MAX_PLAYER>& rankCont,
				std::array<char, ITEM_TOTAL_SIZE>& getItemCountCont
			) noexcept
				: BasePacket(sizeof(EndMatch), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::END_MATCH))
				, matchInfo()
				, getItemCountCont()
			{
				for (int i = 0; i < ROOM_MAX_PLAYER; ++i)
				{
					matchInfo[i].id = roomIdCont[i];
				}

				for (int i = 0; i < ROOM_MAX_PLAYER; ++i)
				{
					matchInfo[i].survivalTime = survivalTimeCont[i];
				}

				for (int i = 0; i < ROOM_MAX_PLAYER; ++i)
				{
					matchInfo[i].rank = rankCont[i];
				}

				for (int i = 0; i < getItemCountCont.size(); ++i)
				{
					this->getItemCountCont[i] = getItemCountCont[i];
				}
			}
		};

		struct Logout : public BasePacket
		{
			Logout() noexcept
				: BasePacket(sizeof(Logout), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOGOUT))
			{}
		};

		struct SpawnItem : public BasePacket
		{
			_Type itemKey;
			_Type itemType;
			PosForPacket itemPos;

			SpawnItem(_Type itemKey, _Type itemType, PosForPacket itemPos)
				: BasePacket(sizeof(SpawnItem), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::SPAWN_ITEM))
				, itemKey(itemKey)
				, itemType(itemType)
				, itemPos(itemPos)
			{}
		};

		struct PosItem : public BasePacket
		{
			const _Type itemKey;
			const PosForPacket itemPos;

			PosItem(_Type itemKey, PosForPacket itemPos)
				: BasePacket(sizeof(PosItem), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::POSITION_ITEM))
				, itemKey(itemKey)
				, itemPos(itemPos)
			{}
		};

		struct RemoveItem : public BasePacket
		{
			const _Type itemKey;
			const char getUserRoomIndex;

			RemoveItem(_Type itemKey, const char getUserRoomIndex)
				: BasePacket(sizeof(RemoveItem), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::REMOVE_ITEM))
				, itemKey(itemKey)
				, getUserRoomIndex(getUserRoomIndex)
			{}
		};

		struct SpawnWall : public BasePacket
		{
			float wallPosY;

			SpawnWall(float wallPosY)
				: BasePacket(sizeof(SpawnWall), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::SPAWN_WALL))
				, wallPosY(wallPosY)
			{}
		};

		struct PosWall : public BasePacket
		{
			float wallPosY;

			PosWall(float wallPosY)
				: BasePacket(sizeof(PosWall), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::POSITION_WALL))
				, wallPosY(wallPosY)
			{}
		};

		struct RemoveWall : public BasePacket
		{
			RemoveWall()
				: BasePacket(sizeof(RemoveWall), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::REMOVE_WALL))
			{}
		};

		struct StartWave : public BasePacket
		{
			const _Type waveType;
			// 1 first wave
			// 2 sercond wave
			// 3 wall spawn
			// 4 third wave
			// 5 Fever Mode
			StartWave(_Type waveType)
				: BasePacket(sizeof(StartWave), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::START_WAVE))
				, waveType(waveType)
			{}
		};

		struct MoveFireWall : public BasePacket
		{
			const float leftX;
			const float rightX;

			MoveFireWall(const float leftX, const float rightX)
				: BasePacket(sizeof(MoveFireWall), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::MOVE_FIREWALL))
				, leftX(leftX)
				, rightX(rightX)
			{}
		};

		struct RemainTime : public BasePacket
		{
			const char remainTime;

			struct RemainTime(const char remainTime)
				: BasePacket(sizeof(RemainTime), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::REMAIN_TIME))
				, remainTime(remainTime)
			{
			}
		};

		struct InGameRank : public BasePacket
		{
			const char yourRank;
		
			struct InGameRank(const char yourRank)
				: BasePacket(sizeof(InGameRank), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::INGAME_RANK))
				, yourRank(yourRank)
			{
			}
		};
	}

	namespace CLIENT_TO_SERVER
	{
		struct SignUp
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
			_ID id;
			_PW pw;
			_Nickname nickname;
		};

		struct Login
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
			_ID id;
			_PW pw;
		};

		struct LobbyChat
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
			_Chat chat;
		};

		struct ChangeDirection
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
			_Direction direction;	// 0왼쪽, 1오른쪽
		};

		struct Roll
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
		};

		struct Emoji
		{
			// 구조체 사이즈 1
			// 구조체 타입 1
			_Type emojiType;	// 이모지 인덱스	
		};

		struct Block
		{
			_Type isOnBlock;
			// 0이면 Do!
			// 1이면 릴리즈!
		};

		struct ChangeCharacter
		{
			_Type characterIndex;
		};

		struct RandomLogin
		{
			// 2
			// RANDOM_LOGIN
		};
	}
}
#pragma pack(pop)
#pragma endregion

#pragma region [TASK_TYPE]
enum class TASK_TYPE
{
	// ROOM ================================================

	CHANGE_DIRECTION,	// 클라이언트 방향 변경 요청
	DO_ROLL,			// 클라이언트 구르기 요청
	INIT_ROLL,			// 클라이언트 구르기 초기화
	ROLL_END,			// 클라이언트 구르기 초기화

	EMOJI,				// 클라이언트 감정 표현 요청

	EXIT_ROOM,			// 방 나가부렷네.

	ROOM_UPDATE,		// 방 정보 갱신

	// Others ==============================================
	PUSH_OLD_KEY		// 사용된 키 반납 및 초기화 작업.
	, LOADING_END		// 클라이언트에서 로딩이 끝나면.
	, GAME_START_COUNT_END // 게임 시작 3초 카운트 다운 끝남!! 게임 시작 필요함!!
	, ROOM_DESTORY // 방 끝내고 폭파시킬 예정입니다.
	, RETRY_RANDOM_MATCHING // 해당 키의 유저 랜덤 매칭 재시도
	, REDUCE_REMAIN_TIME // 타임 제거
	, END_CHARACTER_NODAMAGE // 캐릭터 노 데미지 끝냄.
	, END_CHARACTER_NOMOVE // 캐릭터 움직임 방지.

	, DO_BLOCKING	// 블로킹 시작
	, RELEASE_BLOCKING // 블로킹 릴리즈
	, DO_KICK
	, END_KICK
	, INIT_KICK_COOLTIME

	, CHANGE_CHARACTER	// 캐릭터 변경
	, LOBBY_USER_INFO // 유저정보 전달 요청받았을 시.

	, KICK_ATTACK	// 킥 입렵을 받은 후, 일정 시간 이후, 실제로 킥 충돌이 되었는지 테스트 
	, END_STUN	// 킥에 맞아 스턴인 상태를 종료해줌

	, FORCED_START
	, RANDOM_LOGIN
};
#pragma endregion
