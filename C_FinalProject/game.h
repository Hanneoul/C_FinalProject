#ifndef GAME_H
#define GAME_H

#define MAX_COMMAND_ID 20

#define MAP_WIDTH 15
#define MAP_HEIGHT 7

#define FLASH_NONE 0
#define FLASH_P1 1
#define FLASH_P2 2
#define FLASH_BOTH 3 // P1, P2 모두 공격 성공 시

typedef struct Player Player;

// 함수 포인터 타입 정의: (내 정보, 상대 정보)를 받아 커맨드(int)를 반환함.
typedef int (*CommandFn)(const Player* my_info, const Player* opponent_info);

// Player 구조체의 실제 정의 (api.h에서 전방 선언된 구조체의 내부 정의를 완성)
typedef struct Player Player;

struct Player {
    int id;     // 1 또는 2 (내부 슬롯 구분용)
    int reg_key;     // NEW: 고유 등록 키 (랜덤 학번 역할)
    int x;
    int y;
    int hp;
    int mp;
    char name[10];
    char symbol;
    int last_command;

    // 독 지속 시간
    int poison_duration;
    char secrete_message[500]; // 도발 메시지를 저장할 공간
    
    // 비트마스크 대신 인덱스 배열 사용: [0]=잠김, [1]=해금
    int skill_status[MAX_COMMAND_ID];

    CommandFn get_command;
};

// 게임 상태 구조체
typedef struct {
    Player player1;
    Player player2;
    int turn;
    int game_over;
} GameState;

void init_game(GameState* state);
int execute_turn(GameState* state, int p1_command, int p2_command);
int check_game_over(const GameState* state);

#endif // GAME_H