#include "defs.h"
#include <raylib.h>

#if defined(PLATFORM_WEB)
#define CUSTOM_MODAL_DIALOGS       // Force custom modal dialogs usage
#include <emscripten/emscripten.h> // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>  // Required for: printf()
#include <stdlib.h> // Required for:
#include <string.h>

#include "camera.h"

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

static void GenerateDungeon();
static void UpdateDrawFrame(void);
static Font gameFont;

int main(void) {
#if !defined(DEBUGGER)
    SetTraceLogLevel(LOG_DEBUG);
#else
    SetTraceLogLevel(LOG_NONE);
#endif

    InitWindow(screenWidth, screenHeight, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    gameFont = LoadFont("resources/fonts/shiny.ttf");

#if !defined(EDITOR_MODE)
    InitGameCamera();
    GenerateDungeon();
#else 
    InitEditor();
#endif

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);            // Set our game frames-per-second
    while (!WindowShouldClose()) // Detect window close button
    { 
        UpdateDrawFrame();
    }
#endif
    UnloadFont(gameFont);
    CloseWindow();
    return 0;
}

/////////////////// VARS ////////////////////////
#define TW 24
#define GX 50
#define GY 50

// TODO: multiplayer!!!s

Rectangle playerDest = {
    .x = 1,
    .y = 1,
    .width = TW,
    .height = TW  
};
Vector2 playerPosition;

static int maxPlayerActions = 1;
static int playerRemaingActions = 1;

int playerInRoom = 0;

typedef enum GameState {
    MAP_STATE = 1,
    COMBAT_STATE
} GameState;

typedef enum CurrentTurn {
    PLAYER_TURN = 1,
    DUNGEON_TURN
} CurrentTurn;

GameState currentState = MAP_STATE;
CurrentTurn currentTurn = PLAYER_TURN;

float maxDungeonTime = 0.1f;
float dungeonRemaingTime = 0.0f;

static int gridSizeX = GX;
static int gridSizeY = GY;


//////////////////////////////////////////////////
int GetMoveDistance(int x, int y, int x2, int y2) {
    return abs(x - x2) + abs(y - y2);
}
///////////////////////////////////////////////////

/////////////////// DUNGEON ///////////////////////
static Vector2 gridOrigin = { screenWidth / 2 - TW / 2, screenHeight / 2 - TW / 2};

// V2
#define DEFAULT_DUNGEON_SIZE 50

#define ROLL_MAX 1000

#define BLOCK_CHANCE 130
#define ENEMY_CHANCE 25
#define TRESURE_CHANCE 5

#define ENEMY_ID_START 20
#define NUM_ENEMIES 10

#define TRESURE_ID 99

Vector2 dungeonOrigin = { 0 };
int layout[DEFAULT_DUNGEON_SIZE][DEFAULT_DUNGEON_SIZE];

int enemiesLeft = 0;
int tresuresLeft = 0;

static void GenerateDungeon(void) {
    dungeonOrigin = (Vector2) { -DEFAULT_DUNGEON_SIZE * TW / 2, -DEFAULT_DUNGEON_SIZE * TW / 2 };
    Rectangle initialBounds = (Rectangle){ 0, 0, DEFAULT_DUNGEON_SIZE, DEFAULT_DUNGEON_SIZE };
    for(int x = 0; x < DEFAULT_DUNGEON_SIZE; x++) {
        for(int y = 0; y < DEFAULT_DUNGEON_SIZE; y++) {
            if((playerDest.x == x && playerDest.y == y)) continue;
            int roll = GetRandomValue(0, ROLL_MAX);
            layout[x][y] = 0;
            if(roll < BLOCK_CHANCE) layout[x][y] = 1;
            else {
                roll = GetRandomValue(0, ROLL_MAX);
                if (roll < ENEMY_CHANCE) { 
                    layout[x][y] = ENEMY_ID_START + GetRandomValue(0, NUM_ENEMIES);
                    enemiesLeft++;
                } else {
                    roll = GetRandomValue(0, ROLL_MAX);
                    if (roll < TRESURE_CHANCE) {
                        layout[x][y] = TRESURE_ID;
                        tresuresLeft++;
                    }
                }
            } 
        }
    }
}

static void DrawDungeon(void) {

    for(int y = 0; y < DEFAULT_DUNGEON_SIZE; y++) {
        DrawRectangle(dungeonOrigin.x - TW, dungeonOrigin.y + y * TW, TW, TW, DARKGRAY);
        DrawRectangle(dungeonOrigin.x + (DEFAULT_DUNGEON_SIZE) * TW, dungeonOrigin.y + y * TW, TW, TW, DARKGRAY);
        DrawRectangleLines(dungeonOrigin.x - TW, dungeonOrigin.y + y * TW, TW, TW, LIGHTGRAY);
        DrawRectangleLines(dungeonOrigin.x + (DEFAULT_DUNGEON_SIZE) * TW, dungeonOrigin.y + y * TW, TW, TW, LIGHTGRAY);
    }
    for(int x = -1; x <= DEFAULT_DUNGEON_SIZE; x++) {
        DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y - TW, TW, TW, DARKGRAY);
        DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + (DEFAULT_DUNGEON_SIZE) * TW, TW, TW, DARKGRAY);
        DrawRectangleLines(dungeonOrigin.x + x * TW, dungeonOrigin.y - TW, TW, TW, LIGHTGRAY);
        DrawRectangleLines(dungeonOrigin.x + x * TW, dungeonOrigin.y + (DEFAULT_DUNGEON_SIZE) * TW, TW, TW, LIGHTGRAY);
    }

    for(int x = 0; x < DEFAULT_DUNGEON_SIZE; x++) {
        for(int y = 0; y < DEFAULT_DUNGEON_SIZE; y++) {
            if(layout[x][y] == 1) {
                DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, DARKGRAY);
                DrawRectangleLines(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, LIGHTGRAY);
            }else if(layout[x][y] >= ENEMY_ID_START && layout[x][y] <= ENEMY_ID_START + NUM_ENEMIES) {
                DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, RED);
            }
            else if(layout[x][y] == TRESURE_ID) {
                DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, GOLD);
            }
            else {
                //DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, RAYWHITE);
            }

            //DrawRectangleLines(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, DARKGRAY);
        }
    }

}


static Vector2 CoordsToPos(int x, int y, int tw) {
    return (Vector2){ dungeonOrigin.x + x * tw, dungeonOrigin.y + y * tw };
}

static int CanMove(int x, int y, int xOff, int yOff) {
    return x + xOff >= 0 && y+yOff >=0 && x + xOff <  DEFAULT_DUNGEON_SIZE && y + yOff < DEFAULT_DUNGEON_SIZE
        && layout[x+xOff][y+yOff] == 0;
}

///////////////////////////////////////////////////

/////////////////// UPDATE ////////////////////////

static void PlayerTurn (void) {
    currentTurn = PLAYER_TURN;
    playerRemaingActions = maxPlayerActions;
}

static void DungeonTurn(void) {
    currentTurn = DUNGEON_TURN;
    dungeonRemaingTime = maxDungeonTime;
    //TODO change time and just change turn when dungeon has no remaing actions
}

static void UpdateMap(void) {
    SetGameCameraZoom(GetGameCamera()->zoom + ((float)GetMouseWheelMove()*0.05f));

    // Input
    if(currentTurn == DUNGEON_TURN && dungeonRemaingTime <= 0.0f) {
        PlayerTurn();
    } else if(currentTurn == PLAYER_TURN && playerRemaingActions <= 0) {
        DungeonTurn();
    }
    
    if(currentTurn == PLAYER_TURN) {
        
        if(IsKeyPressed(KEY_S)){
            if(CanMove(playerDest.x, playerDest.y, 0, 1)) {
                playerDest.y += 1;
                playerRemaingActions--;
            }
        }
        else if(IsKeyPressed(KEY_W)){
            if(CanMove(playerDest.x, playerDest.y, 0, -1)) {
                playerDest.y -= 1;
                playerRemaingActions--;
            }
        }
        else if(IsKeyPressed(KEY_A)){
            if(CanMove(playerDest.x, playerDest.y, -1, 0)) { 
                playerDest.x -= 1;
                playerRemaingActions--;
            }
        }
        else if(IsKeyPressed(KEY_D)){
            if(CanMove(playerDest.x, playerDest.y, 1, 0)) {
                playerDest.x += 1;
                playerRemaingActions--;
            }
        }
        
        playerPosition = CoordsToPos(playerDest.x, playerDest.y, TW);
        UpdateGameCamera(playerPosition);

    } else {

        dungeonRemaingTime -= GetFrameTime();
    }

}

static void UpdateCombat(void) {
    
}

///////////////////////////////////////////////////


//////////////////// RENDER ///////////////////////

static void RenderMap(void) {
    // Draw
    BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(*GetGameCamera());
            DrawDungeon();
            
            // DrawDebugGrid();
            DrawRectangle(playerPosition.x, playerPosition.y, playerDest.width, playerDest.height, GREEN);
            
        EndMode2D();

        DrawText(TextFormat("CurrentTurn: %s", currentTurn == PLAYER_TURN ? "Player" : "Dungeon"), 5, gameHeight - 15, 10, COLOR_3);
        // DrawText(TextFormat("TotalRooms: %i", totalRooms), 5, gameHeight - 30, 10, COLOR_3);

    EndDrawing();
}

static void RenderCombat(void) {

}

///////////////////////////////////////////////////

static void UpdateDrawFrame(void) {
#if !defined(EDITOR_MODE)
    if(currentState == MAP_STATE)  {
        UpdateMap();
        RenderMap();
    }
    else if (currentState == COMBAT_STATE)  {
        UpdateCombat();
        RenderCombat();
    }
#else
    UpdateEditor();
    RenderEditor();
#endif

}