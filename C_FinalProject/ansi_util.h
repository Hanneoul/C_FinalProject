#ifndef ANSI_UTIL_H
#define ANSI_UTIL_H
#include <Windows.h>

void flash_screen(int background_color_code, DWORD duration_ms);

// ANSI Escape Code를 활성화하는 함수 (Visual Studio 환경 필수)
void enable_ansi_escape_codes();

// 커서를 원하는 위치(x, y)로 이동 (1-based index)
void move_cursor(int x, int y);

// 커서 숨기기
void hide_cursor();

// 커서 보이기
void show_cursor();

// 전경색 설정 (30-37)
void set_foreground_color(int code);

// 배경색 설정 (40-47)
void set_background_color(int code);

// 색상 초기화 (초기값으로 복원)
void reset_color();

// 화면 지우기 (Clear Screen)
void clear_screen();

// ANSI 색상 코드 정의
#define ANSI_BLACK 30
#define ANSI_RED 31
#define ANSI_GREEN 32
#define ANSI_YELLOW 33
#define ANSI_BLUE 34
#define ANSI_MAGENTA 35
#define ANSI_CYAN 36
#define ANSI_WHITE 37

#define ANSI_BG_BLACK 40
#define ANSI_BG_RED 41
#define ANSI_BG_GREEN 42
#define ANSI_BG_BLUE 44

// ... 필요한 다른 색상 코드를 여기에 추가

#endif // ANSI_UTIL_H