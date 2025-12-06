#include "ansi_util.h"
#include <stdio.h>
#include <windows.h> // SetConsoleMode, GetStdHandle 등을 사용하기 위함



void enable_ansi_escape_codes() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleMode(hOut, &dwMode)) return;

    // 가상 터미널 시퀀스 처리(ENABLE_VIRTUAL_TERMINAL_PROCESSING) 플래그 설정
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) return;
}

void move_cursor(int x, int y) {
    // \x1b[y;xH 또는 \x1b[y;xf
    printf("\x1b[%d;%dH", y, x);
}

void hide_cursor() {
    printf("\x1b[?25l"); // ?25l: hide
}

void show_cursor() {
    printf("\x1b[?25h"); // ?25h: show
}

void set_foreground_color(int code) {
    printf("\x1b[%dm", code);
}

void set_background_color(int code) {
    printf("\x1b[%dm", code);
}

void reset_color() {
    printf("\x1b[0m");
}

void clear_screen() {
    printf("\x1b[2J"); // 2J: clear screen
}

void flash_screen(int background_color_code, DWORD duration_ms) {
    // 1. 배경색 설정 및 화면 전체를 해당 색으로 채움 (글자 모두 삭제)
    printf("\x1b[%dm", background_color_code);
    clear_screen();
    hide_cursor();

    // 2. 잠시 대기 (100ms)
    Sleep(duration_ms);

    // 3. 색상 초기화 및 화면 다시 지우기
    reset_color();
    clear_screen();
}