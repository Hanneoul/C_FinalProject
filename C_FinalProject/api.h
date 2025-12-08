// api.h (학생에게 제공할 유일한 헤더 파일)

#ifndef API_H
#define API_H

// 1. Player 구조체의 불완전 선언 (Opaque Type)
//    - 학생들은 이 구조체의 내부 멤버(x, y, hp 등)를 알 수 없음.
//    - 따라서 .x, .hp와 같이 직접 접근하여 값을 변경하는 것이 컴파일 단계에서 차단됨.
typedef struct Player Player;

// 2. CommandFn 타입 정의: Player*를 인자로 사용함. (함수 포인터 타입)
typedef int (*CommandFn)(const Player* my_info, const Player* opponent_info);

// 3. AI 등록 API (학생이 자신의 함수를 시스템에 등록하기 위해 호출함)
int register_player_ai(const char* team_name, CommandFn ai_function);

// 4. 스킬 해금 시도 (문자열 정답을 받음)
void attempt_skill_unlock(int registration_key, int skill_command, const char* quiz_answer);

// 5. 스킬 해금 여부 확인 (보안 유지를 위해 Key로 인증)
int is_skill_unlocked(int registration_key, int skill_command);


// 4. 정보 접근 Getter 함수 (학생이 자신의 정보 및 상대 정보를 얻기 위해 호출함)
//    - 모든 함수는 const Player*를 받아 게임 상태를 변경하는 것을 방지함.
extern int get_player_hp(const Player* p);
extern int get_player_mp(const Player* p);
extern int get_player_x(const Player* p);
extern int get_player_y(const Player* p);
extern int get_player_last_command(const Player* p);
extern int get_player_id(const Player* p);

// 5. AI 함수가 사용할 수 있는 커맨드 정의 (매크로)
#define CMD_UP				1	//기본 스킬 :	  위로 이동
#define CMD_DOWN			2	//기본 스킬 :	  아래로 이동
#define CMD_LEFT			3	//기본 스킬 :	  좌로 이동
#define CMD_RIGHT			4	//기본 스킬 :	  우로 이동
#define CMD_ATTACK			5	//기본 스킬 :	  기본 범위 1짜리 공격
#define CMD_POISON          6 	//해금필요 스킬 : 독 (MP 5, DoT 1 for 3 turns)
#define CMD_STRIKE          7   //해금필요 스킬 : 강타 (MP 2, Damage 2, 근접)
#define CMD_BLINK_UP        8   //해금필요 스킬 : 점멸 (상) (MP 1, 2칸 이동)
#define CMD_BLINK_DOWN      9   //해금필요 스킬 : 점멸 (하) (MP 1, 2칸 이동)
#define CMD_BLINK_LEFT     10   //해금필요 스킬 : 점멸 (좌) (MP 1, 2칸 이동)
#define CMD_BLINK_RIGHT    11   //해금필요 스킬 : 점멸 (우) (MP 1, 2칸 이동)
#define CMD_HEAL           12   //기본 스킬 :	  회복 (MP 1, HP 1 회복)
#define CMD_HEAL_ALL       13   //해금필요 스킬 : 회복2 (MP 2, 남은 MP만큼 HP 회복)
#define CMD_RANGE_ATTACK   14   //해금필요 스킬 : 원거리 공격 (MP 1, 거리 2 타격)
#define CMD_REST           15   //기본 스킬 :	  휴식 (MP 2 회복)
#define CMD_SELF_DESTRUCT  16   //해금필요 스킬 : 폭주 (MP 5소모, HP 2이하 일때 발동가능, 상대 3 데미지)
#define CMD_H_ATTACK       17   //해금필요 스킬 : 가로 공격 (MP 3, 가로 전체 1 데미지)
#define CMD_V_ATTACK       18   //해금필요 스킬 : 세로 공격 (MP 3, 세로 전체 1 데미지)
#define CMD_SECRETE        19   //해금필요 스킬 : 비밀 스킬 (소모없음, 채팅창에 메시지 적음)

// (학생들은 ansi_util.h나 game.h의 다른 함수들에 접근할 수 없음.)

#endif // API_H