/**
 * =================================================================================================
 * [C Command Battle AI Challenge]
 * =================================================================================================
 * * 과제 설명:
 * 본 파일(PlayerX.c)은 학생이 구현할 AI 로직을 담고 있습니다.
 * 학생은 simple_killer_ai와 같은 CommandFn 형태의 AI 함수를 작성하여,
 * 매 턴 게임 상태(my_info, opponent_info)를 분석하고 다음 행동(CMD_ID)을 반환해야 합니다.
 * * -------------------------------------------------------------------------------------------------
 * * 🚨 핵심 제약 사항 (절대 준수해야 함) 🚨
 * * 1. 헤더 파일 제한:
 * - 프로젝트 내에서 "api.h" 외의 다른 헤더 파일을 include 하는 것은 엄격히 금지됩니다.
 * - 표준 라이브러리(Standard Library) 함수는 <stdlib.h>, <stdio.h> 등에 포함된
 * 기본적인 함수(abs, rand, printf, scanf 등)만 사용해야 합니다.
 * * 2. 실행 흐름:
 * - 본 파일은 main.c의 main 함수 실행 전에 student1_ai_entry() 함수를 통해 시스템에 연결됩니다.
 * - **AI 로직은 반드시 CommandFn 형태의 함수로 구현**되어야 합니다.
 * * -------------------------------------------------------------------------------------------------
 * * 🎯 학생의 주요 임무 (매 턴 수행):
 * * - AI 함수는 오직 하나의 커맨드 ID (1 ~ 19)를 반환하는 것에 집중합니다.
 * - 함수는 get_player_x() 등의 API Getter 함수만을 사용하여 정보를 조회하고,
 * 게임 상태를 직접 변경해서는 안 됩니다.
 * * -------------------------------------------------------------------------------------------------
 * * 💡 시스템 작동 보장 및 폴백 (Fallback)
 * * - AI 함수 미구현 시: 만약 학생이 AI 함수를 구현하지 않았거나 등록에 실패한 경우,
 * 프로그램은 자동으로 내장된 수동 입력 모드로 전환되어, 사용자가 직접 콘솔에 커맨드를
 * 입력하여 캐릭터를 제어할 수 있도록 합니다. (과제 제출 시에는 반드시 AI 함수를 등록해야 함.)
 * * =================================================================================================
 */

#include "api.h"
#include <stdlib.h> 
#include <stdio.h> 

//학생의 고유번호를 발급받아 저장할 변수
int my_secret_key;
 // =================================================================================================
 // [학생 구현 영역 1] AI 로직 구현부
 // =================================================================================================

 // [AI용 별도 제작함수] 간단한 맨하탄 거리 계산 유틸리티 함수
static int calculate_distance(const Player* p1, const Player* p2) {
    int dx = abs(get_player_x(p1) - get_player_x(p2));
    int dy = abs(get_player_y(p1) - get_player_y(p2));
    return dx + dy;
}

// [AI로직] MP를 사용하지 않고 공격만 시도하는 AI 로직 (예시)
int simple_killer_ai(const Player* my_info, const Player* opponent_info) {
    int distance = calculate_distance(my_info, opponent_info);

    int my_x = get_player_x(my_info);
    int opp_x = get_player_x(opponent_info);
    int my_y = get_player_y(my_info);
    int opp_y = get_player_y(opponent_info);

    // 1. 공격 판정 
    if (distance <= 1) {
        //대사를 넣을 수 있는 기믹 (19번 스킬 해금시 사용가능)
        set_custom_secrete_message(my_secret_key, "보통 펀치!!!");
        return CMD_ATTACK;
    }

    // 2. 추격 이동 (X축 우선)
    if (my_x != opp_x) {
        if (my_x < opp_x) {
            set_custom_secrete_message(my_secret_key, "오른쪽으로 슈슉!!!");
            return CMD_RIGHT;
        }
        else {
            set_custom_secrete_message(my_secret_key, "왼쪽으로 슈슉!!!");
            return CMD_LEFT;
        }
    }

    // 3. Y축 추격
    if (my_y != opp_y) {
        if (my_y < opp_y) {
            set_custom_secrete_message(my_secret_key, "아래로 간다!!!");
            return CMD_DOWN;
        }
        else {
            set_custom_secrete_message(my_secret_key, "위로 올라간다!!!");
            return CMD_UP;
        }
    }

    // 4. 예외 상황 
    set_custom_secrete_message(my_secret_key, "예외상황이구나 귀여운 은석펀치!!!");
    return CMD_ATTACK;
}


// =================================================================================================
// [학생 구현 영역 2] 시스템 진입 및 해금 영역
// =================================================================================================

// 이 함수는 main.c에서 extern으로 호출되는 학생 코드의 진입점입니다.
void student1_ai_entry() {
    // 이 섹션의 모든 코드는 예제를 참고하세요


    // 팀이름과 AI함수를 NPC에 등록하는 register_player_ai함수와 리턴값으로 플레이어 고유번호 발급하는 코드
    // 주석한 부분처럼 함수에 아무것도 등록이 안되면 NPC가 아니라 플레이어로써 플레이할 수 있는 모드가 실행됩니다.
    my_secret_key = register_player_ai("TEAM-ALPHA", simple_killer_ai);
    //my_secret_key = register_player_ai("TEAM-ALPHA", 0);

    // ------------------------------------------------------------------
    // [COMMAND UNLOCK SECTION] 
    // 학생은 아래의 코드를 복사하여 필요한 스킬 수만큼 반복해야 합니다.
    // ------------------------------------------------------------------

    //attempt_skill_unlock(고유번호, 스킬번호, 시험문제 정답) 함수를 실행하여 시험문제의 답을 입력하면 기술이 해금됩니다.
    attempt_skill_unlock(my_secret_key, CMD_POISON, "Ancient_Relic|Doom_Greatsword|Immortal_Sword");

    //주의: 시험문제에서 제공되는 CSV는 데이터를 바꾸어 최종 시험때 실행됩니다.
    //위와 같이 string을 직접 입력하지마시오 시험문제를 풀어서 CSV로부터 저 string을 뽑아내는 코드를 작성해서 해금하시기 바랍니다.
    //그래야 시험 당일날에도 스킬이 정상적으로 해금됩니다.


    //해금됐는지 확인하려면 아래와 같이 is_skill_unlocked 사용해볼것!!!
    if (is_skill_unlocked(my_secret_key, CMD_POISON))
        printf("TEAM-ALPHA : CMD_POISON 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_POISON 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_STRIKE, "2key");
    if (is_skill_unlocked(my_secret_key, CMD_STRIKE))
        printf("TEAM-ALPHA : CMD_STRIKE 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_STRIKE 해금 실패 ㅜㅜ\n");


    attempt_skill_unlock(my_secret_key, CMD_BLINK_DOWN, "*A**C**F**T*");
    if (is_skill_unlocked(my_secret_key, CMD_BLINK_DOWN))
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_BLINK 4종 해금 실패 ㅜㅜ\n");


    attempt_skill_unlock(my_secret_key, CMD_HEAL_ALL, "*H*");
    if (is_skill_unlocked(my_secret_key, CMD_HEAL_ALL))
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_HEAL_ALL 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_RANGE_ATTACK, "\"LOT,A\"");
    if (is_skill_unlocked(my_secret_key, CMD_RANGE_ATTACK))
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_RANGE_ATTACK 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_BLESS, "T");
    if (is_skill_unlocked(my_secret_key, CMD_BLESS))
        printf("TEAM-ALPHA : CMD_BLESS 해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_BLESS 해금 실패 ㅜㅜ\n");

    attempt_skill_unlock(my_secret_key, CMD_H_ATTACK, "Inf_03");
    if (is_skill_unlocked(my_secret_key, CMD_H_ATTACK))
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK  해금 완료\n");
    else
        printf("TEAM-ALPHA : CMD_H_ATTACK,CMD_V_ATTACK 해금 실패 ㅜㅜ\n");

    // CMD_SECRETE (비밀 메시지) 해금 및 설정 예시
    attempt_skill_unlock(my_secret_key, CMD_SECRETE, "wn_St");
    if (is_skill_unlocked(my_secret_key, CMD_SECRETE))
    {
        printf("TEAM-ALPHA : CMD_SECRETE 해금 완료\n");
        // set_custom_secrete_message 함수를 사용하여 도발 메시지를 등록합니다.
        set_custom_secrete_message(my_secret_key, "후후후 좁밥들...");
    }
    else
        printf("TEAM-ALPHA : CMD_SECRETE 해금 실패 ㅜㅜ\n");

    // ------------------------------------------------------------------

    printf("TEAM-ALPHA : 플레이어 초기화 완료. 아무키나 누르시오.\n");

    // getchar()는 그냥 멈추려고 사용한거에요
    getchar();
}