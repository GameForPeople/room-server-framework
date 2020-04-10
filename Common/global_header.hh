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
	constexpr int MAX_BALL_NUM = 20;	// �濡�� ������ �ִ� ���� �����Դϴ�.

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
		ACCEPT_O = 0,			// ����Ʈ ����
		ACCEPT_X = 1,			// ����Ʈ ����
		
		SIGN_UP_O = 2,			// ȸ������ ����
		SIGN_UP_X = 3,			// ȸ������ ����

		LOGIN_O = 4,			// �α��� ����
		LOGIN_X = 5,			// �α��� ����

		// Lobby Scene ============================================
		LOBBY_INFO = 6,			// �κ� �ʿ��� ����
		LOBBY_CHAT = 7,			// �ٸ� ������ ä��

		// RANDOM_MATCH,		// ť ���� ����..  �ϴ� ���ʿ�. (�������� ����)
		CANCLE_MATCH_O = 8,		// ��ġ ��� ����.

		START_MATCH = 9,		// ��ġ ���۰�, �� �ٸ� ���� ����
		END_MATCH = 10,
		
		// Room Scene ============================================
		FIXED_MATCH_INFO = 11,	// ���� �����Ӹ���, �������� ���� ������
		USER_DAMAGED = 12,		// ���� ������ �������� ���� ���� ������.
		//USER,
		USER_ROLL = 13,			// ���� ������ ���� ������ ������.
		USER_EMOJI = 14,			// Ư�� ������ �̸����� ������.
		USER_CHANGE_DIRECTION = 15,

		// ADD_ITEM,			// ������...�̰� ���� ������Դϴ�.
		// MOVE_ITEM,			// ������...�̰� ���� ������Դϴ�.

		// Other! ============================================
		LOGOUT = 16,

		//
		QUEUE_MATCHED = 17, //  ť�� ������ ���� ����� ���� // ����,  �г���, ĳ���� ����
		
		SPAWN_ITEM = 18,	// ������ ��ȯ�� �����ϴ� ��Ŷ!
		POSITION_ITEM = 19,	// ������ ��ġ ����
		REMOVE_ITEM = 20,
		SPAWN_WALL = 21,	// �� ���� ��
		POSITION_WALL = 22,	// �� ������ ��
		REMOVE_WALL = 23,	// �� ���� ��
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
		SIGN_UP = 0,			// ȸ������
		LOGIN = 1,				// �α���

		// Lobby Scene ============================================
		LOBBY_INFO = 2,			// ���� ����
		LOBBY_CHAT = 3,			// ä�� �Է�

		RANDOM_MATCH = 4,		// ��ġ ť ��û
		CANCLE_MATCH = 5,

		// ROOM_SCENE ============================================
		CHANGE_DIRECTION = 6,	// ���� ����
		DO_ROLL = 7,			// ������ �õ�
		EMOJI = 8				// ����

#ifdef FOR_SERVER
		// For Only Server ============================================

		, LOGOUT = 9			// �α׾ƿ� ó��. 
#endif
		, LOADING_END = 10		// Ŭ�󿡼� �ε� ������ 2����Ʈ
		, CHANGE_CHARACTER = 11	// Ŭ�󿡼� ��Ƽ�� ĳ���͸� �������� ��
		, BLOCK = 12	// ���ŷ
		, KICK = 13		// ű!
		, LOBBY_USER_INFO = 14	//�κ� ���� ��, ���� ���� ���Ź

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
			const _Type failType;	// ���� ����

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
			const _Type failType;	// ���� ����

			SignUpX(const _Type failType) noexcept
				: BasePacket(sizeof(SignUpX), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::SIGN_UP_X))
				, failType(failType)
			{}
		};

		struct LoginO : public BasePacket
		{
			// userInfo
			const _Level level;		// ����
			const _Money money;		// ���� ��ȭ
			const _Count winCount;	// �̱� Ƚ��
			const _Count loseCount;	// �� Ƚ��
			const _Count pickedCharacterIndex; // ��Ƽ�� ĳ����
			_Nickname nickname;	// �г���

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
			const _Type failType;	// ���� ����

			LoginX(const _Type failType) noexcept
				: BasePacket(sizeof(LoginX), static_cast<_Type>(PACKET_TYPE::SERVER_TO_CLIENT::LOGIN_X))
				, failType(failType)
			{}
		};

		struct JoinLobbyInfo : public BasePacket
		{
			const _Level level;		// ����
			const _Money money;		// ���� ��ȭ
			const _Count winCount;	// �̱� Ƚ��
			const _Count loseCount;	// �� Ƚ��
			const _Count pickedCharacterIndex; // ��Ƽ�� ĳ����

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
			char myRoomIndex;	// ����Ʈ ���
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
			const char newHp;	// �̰� ��� �ʿ���µ�.

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
			// ����ü ������ 1
			// ����ü Ÿ�� 1
			_ID id;
			_PW pw;
			_Nickname nickname;
		};

		struct Login
		{
			// ����ü ������ 1
			// ����ü Ÿ�� 1
			_ID id;
			_PW pw;
		};

		struct LobbyChat
		{
			// ����ü ������ 1
			// ����ü Ÿ�� 1
			_Chat chat;
		};

		struct ChangeDirection
		{
			// ����ü ������ 1
			// ����ü Ÿ�� 1
			_Direction direction;	// 0����, 1������
		};

		struct Roll
		{
			// ����ü ������ 1
			// ����ü Ÿ�� 1
		};

		struct Emoji
		{
			// ����ü ������ 1
			// ����ü Ÿ�� 1
			_Type emojiType;	// �̸��� �ε���	
		};

		struct Block
		{
			_Type isOnBlock;
			// 0�̸� Do!
			// 1�̸� ������!
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

	CHANGE_DIRECTION,	// Ŭ���̾�Ʈ ���� ���� ��û
	DO_ROLL,			// Ŭ���̾�Ʈ ������ ��û
	INIT_ROLL,			// Ŭ���̾�Ʈ ������ �ʱ�ȭ
	ROLL_END,			// Ŭ���̾�Ʈ ������ �ʱ�ȭ

	EMOJI,				// Ŭ���̾�Ʈ ���� ǥ�� ��û

	EXIT_ROOM,			// �� �����ηǳ�.

	ROOM_UPDATE,		// �� ���� ����

	// Others ==============================================
	PUSH_OLD_KEY		// ���� Ű �ݳ� �� �ʱ�ȭ �۾�.
	, LOADING_END		// Ŭ���̾�Ʈ���� �ε��� ������.
	, GAME_START_COUNT_END // ���� ���� 3�� ī��Ʈ �ٿ� ����!! ���� ���� �ʿ���!!
	, ROOM_DESTORY // �� ������ ���Ľ�ų �����Դϴ�.
	, RETRY_RANDOM_MATCHING // �ش� Ű�� ���� ���� ��Ī ��õ�
	, REDUCE_REMAIN_TIME // Ÿ�� ����
	, END_CHARACTER_NODAMAGE // ĳ���� �� ������ ����.
	, END_CHARACTER_NOMOVE // ĳ���� ������ ����.

	, DO_BLOCKING	// ���ŷ ����
	, RELEASE_BLOCKING // ���ŷ ������
	, DO_KICK
	, END_KICK
	, INIT_KICK_COOLTIME

	, CHANGE_CHARACTER	// ĳ���� ����
	, LOBBY_USER_INFO // �������� ���� ��û�޾��� ��.

	, KICK_ATTACK	// ű �Է��� ���� ��, ���� �ð� ����, ������ ű �浹�� �Ǿ����� �׽�Ʈ 
	, END_STUN	// ű�� �¾� ������ ���¸� ��������

	, FORCED_START
	, RANDOM_LOGIN
};
#pragma endregion
