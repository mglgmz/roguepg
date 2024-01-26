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

/////////////////// UTILS ////////////////////////

static Vector2 gridOrigin = { screenWidth / 2 - TW / 2, screenHeight / 2 - TW / 2};
static Vector2 CoordsToGrid(Vector2 coords) {
    return (Vector2) { (coords.x - gridOrigin.x) / TW, (coords.y - gridOrigin.y) / TW };
}

static Vector2 GridToCoords(Vector2 grid) {
    return (Vector2) { grid.x * TW + gridOrigin.x , grid.y * TW + gridOrigin.x };
}

static Vector2 XYToCoords(float x, float y) {
    return (Vector2) { x * TW + gridOrigin.x , y * TW + gridOrigin.x };
}

static void DrawDebugGrid(void) {
    for(int gx = -gridSizeX; gx <= gridSizeX; gx++) {
        for(int gy = -gridSizeX; gy <= gridSizeX; gy++) { 
            Vector2 coords = GridToCoords((Vector2){ gx, gy});
            DrawRectangleLines(coords.x, coords.y, TW, TW, LIGHTGRAY);
            // const char* text = TextFormat("%d, %d", gx, gy);
            // DrawText(text, coords.x, coords.y, 8, LIGHTGRAY);
        }   
    }
}

///////////////////////////////////////////////////

///////////////////////////////////////////////////

/////////////////// DUNGEON ///////////////////////

// V2
#define DEFAULT_DUNGEON_SIZE 50

Vector2 dungeonOrigin = { 0 };


Dungeon dungeon = { 0 };

static void GenerateDungeon(void) {
    dungeonOrigin = (Vector2) { -DEFAULT_DUNGEON_SIZE * TW / 2, -DEFAULT_DUNGEON_SIZE * TW / 2 };

    Rectangle initialBounds = (Rectangle){ 0, 0, DEFAULT_DUNGEON_SIZE, DEFAULT_DUNGEON_SIZE };

    

}

static void DrawDungeon(void) {
    
    

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
            playerDest.y += 1;
            playerRemaingActions--;
        }
        else if(IsKeyPressed(KEY_W)){
            playerDest.y -= 1;
            playerRemaingActions--;
        }
        else if(IsKeyPressed(KEY_A)){
            playerDest.x -= 1;
            playerRemaingActions--;
        }
        else if(IsKeyPressed(KEY_D)){
            playerDest.x += 1;
            playerRemaingActions--;
        }

    } else {

        dungeonRemaingTime -= GetFrameTime();
    }

    // playerPosition = CoordsToPos(playerDest.x, playerDest.y, TW);
    // UpdateGameCamera(playerPosition);

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
            DrawRectangle(playerPosition.x, playerPosition.y, playerDest.width, playerDest.height, COLOR_4);
            
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