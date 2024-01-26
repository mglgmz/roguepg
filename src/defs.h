#ifndef DEFS_H
#define DEFS_H

#include "raylib.h"

#define GAME_TITLE "RoguePG"
#define TARGET_FPS 60

// #define EDITOR_MODE 1

#define CLEAR_COLOR COLOR_3

// PALETTE 1
#define COLOR_1 (Color) { 0x8e,0xca, 0xe6, 0xff }
#define COLOR_2 (Color) { 0x21,0x9e, 0xbc, 0xff }
#define COLOR_3 (Color) { 0x02,0x30, 0x47, 0xff }
#define COLOR_4 (Color) { 0xff,0xb7, 0x03, 0xff }
#define COLOR_5 (Color) { 0xfb,0x85, 0x00, 0xff }
// PALETTE 2
// #define COLOR_1 (Color) { 0x26,0x46, 0x53, 0xff }
// #define COLOR_2 (Color) { 0x2a,0x9d, 0x8f, 0xff }
// #define COLOR_3 (Color) { 0xe9,0xc4, 0x6a, 0xff }
// #define COLOR_4 (Color) { 0xf4,0xa2, 0x61, 0xff }
// #define COLOR_5 (Color) { 0xe7,0x6f, 0x51, 0xff }

static const int screenWidth = 1024;
static const int screenHeight = 768;
static const int gameWidth = 1024;
static const int gameHeight = 768;

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#endif