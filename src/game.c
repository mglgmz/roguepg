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

    InitGameCamera();
    GenerateDungeon();

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
    .x = 0,
    .y = 0,
    .width = TW / 2,
    .height = TW / 2  
};
Vector2 playerPosition;

static int maxPlayerActions = 1;
static int playerRemaingActions = 1;

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

// V1
int totalRooms = 0;
int maxDungeonSize = 120;
int minSplitSize = 18;
int maxSplitIterations = 9;

#define NO_PARENT -1
#define ROOM_ALLOC 200

Rectangle roomContainers[ROOM_ALLOC];
int parents[ROOM_ALLOC];
Rectangle rooms[ROOM_ALLOC];

static int IterateGeneration(void) {
    int currentTotalRooms = totalRooms;
    int returnValue = 1;
    for(int i = 0; i < currentTotalRooms; i++) {
        Rectangle* room = &roomContainers[i];
        if(room->width >= 2 * minSplitSize || room->height >= 2 * minSplitSize) {
            char direction = room->width > room->height ? 'x' : 'y';
            if(direction == 'x' && room->width < 2 * minSplitSize) direction = 'y';
            if(direction == 'y' && room->height < 2 * minSplitSize) direction = 'x';
            
            if(direction == 'x') {
                int splitX = GetRandomValue(minSplitSize, room->width - minSplitSize);
                float originalWidth = room->width;
                room->width = splitX;
                roomContainers[totalRooms] = (Rectangle){ room->x + splitX, room->y, originalWidth-splitX, room->height };
                totalRooms++;
                parents[totalRooms - 1] = i;
            } else {
                int splitY = GetRandomValue(minSplitSize, room->height - minSplitSize);
                float originalHeight = room->height;
                room->height = splitY;
                roomContainers[totalRooms] = (Rectangle){ room->x, room->y + splitY, room->width, originalHeight-splitY };
                totalRooms++;
                parents[totalRooms - 1] = i;
            }
           returnValue = 0;
        }
    }
    return returnValue;
}

static void GenerateDungeon(void) {
    int shouldStop = 0;
    roomContainers[0] = (Rectangle){ -maxDungeonSize / 2, -maxDungeonSize / 2, maxDungeonSize, maxDungeonSize };
    totalRooms = 1;
    int iterations = 0;
    for(int i =0; i < ROOM_ALLOC;i++) parents[i] = NO_PARENT;

    while(!shouldStop && iterations++ < maxSplitIterations) {
        int current = 0;
        shouldStop = IterateGeneration();
    }

    for(int i = 0; i < totalRooms; i++) {
        Rectangle* room = &roomContainers[i];
        int width = GetRandomValue(room->width * 0.6f, room->width);
        int height = GetRandomValue(room->height * 0.6f, room->height);
        int newX = abs(room->width - width) / 2 + room->x;
        int newY = abs(room->height - height) / 2 + room->y;
        rooms[i] = (Rectangle) { newX, newY, width, height };
    }
}

#ifdef DEBUGGER
bool showRoomsInfo = false;
static void ShowRoomsInfo(void) {
    if(!showRoomsInfo) return;
    int currentY = 5; 
    int fontSize = 12;
    for(int i = 0; i < totalRooms; i++, currentY += fontSize * 1.5f) {
        Rectangle* room = &roomContainers[i];
        Vector2 position = XYToCoords(room->x, room->y);
        const char* roomInfo = TextFormat("i: %i, X: %f, Y: %f, pX: %f, pY: %f, w: %f, h:%f", i, room->x, room->y, position.x, position.y, room->width, room->height);
        DrawText(roomInfo, 5, currentY, fontSize, DARKBLUE);
    }
}
#endif

static void DrawDungeon(void) {
    for(int i = 0; i < totalRooms; i++) {
        Rectangle* roomContainer = &roomContainers[i];
        Vector2 position = XYToCoords(roomContainer->x, roomContainer->y);

        DrawRectangleLinesEx((Rectangle){ position.x, position.y, roomContainer->width * TW, roomContainer->height * TW}, 5.0f, RED);
        #ifdef DEBUGGER
            if(showRoomsInfo) {
                const char* roomInfo = TextFormat("i: %i", i);
                DrawText(roomInfo, position.x + 5, position.y + 5, 24, RED);
            }
        #endif

        Rectangle* room = &rooms[i];
        Vector2 rPosition = XYToCoords(room->x, room->y);
        DrawRectangleRec((Rectangle){ rPosition.x, rPosition.y, room->width * TW, room->height * TW}, BEIGE);
    }

    
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

#ifdef DEBUGGER
    if(IsKeyPressed(KEY_BACKSLASH)) {
        GenerateDungeon();
    }

    if(IsKeyPressed(KEY_F12))
        showRoomsInfo = !showRoomsInfo;
#endif

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

    playerPosition = XYToCoords(playerDest.x, playerDest. y);
    UpdateGameCamera(playerPosition);

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
            DrawDebugGrid();
            DrawRectangle(playerPosition.x + TW / 4, playerPosition.y + TW / 4, playerDest.width, playerDest.height, BROWN);
            
        EndMode2D();

        DrawText(TextFormat("CurrentTurn: %s", currentTurn == PLAYER_TURN ? "Player" : "Dungeon"), 5, gameHeight - 15, 10, RED);
        DrawText(TextFormat("TotalRooms: %i", totalRooms), 5, gameHeight - 30, 10, RED);

#ifdef DEBUGGER
        ShowRoomsInfo();
#endif
    EndDrawing();
}

static void RenderCombat(void) {

}

///////////////////////////////////////////////////

static void UpdateDrawFrame(void) {
    
    if(currentState == MAP_STATE)  {
        UpdateMap();
        RenderMap();
    }
    else if (currentState == COMBAT_STATE)  {
        UpdateCombat();
        RenderCombat();
    }

}