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

// V1

int maxDungeonSize = 120;
int minSplitSize = 18;
int maxSplitIterations = 9;

#define NO_PARENT -1
#define ROOM_ALLOC 200
#define ROOM_FACTOR 0.9f
#define MAX_TUNNELS_PER_ROOM 4

typedef struct Tunnel {
    int from;
    int to;

    int requirements;
    int used;
} Tunnel;

typedef struct Room {
    int id;
    
    Rectangle container;
    Rectangle bounds;
    Rectangle normalizedBounds;

    Tunnel tunnels[MAX_TUNNELS_PER_ROOM];
    int totalTunnels;

} Room;

Room rooms[ROOM_ALLOC];
int totalRooms = 0;

// Rectangle roomContainers[ROOM_ALLOC];
// Rectangle rooms[ROOM_ALLOC];

static int IterateGeneration(void) {
    int currentTotalRooms = totalRooms;
    int returnValue = 1;
    for(int i = 0; i < currentTotalRooms; i++) {
        Room* room = &rooms[i];

        if(room->container.width >= 2 * minSplitSize || room->container.height >= 2 * minSplitSize) {
            char direction = room->container.width > room->container.height ? 'x' : 'y';
            if(direction == 'x' && room->container.width < 2 * minSplitSize) direction = 'y';
            if(direction == 'y' && room->container.height < 2 * minSplitSize) direction = 'x';
            
            if(direction == 'x') {
                int splitX = GetRandomValue(minSplitSize, room->container.width - minSplitSize);
                float originalWidth = room->container.width;
                room->container.width = splitX;
                Room newRoom = {
                    .id = totalRooms
                };
                newRoom.container = (Rectangle){ room->container.x + splitX, room->container.y, originalWidth-splitX, room->container.height };
                rooms[totalRooms] = newRoom;
                totalRooms++;
            } else {
                int splitY = GetRandomValue(minSplitSize, room->container.height - minSplitSize);
                float originalHeight = room->container.height;
                room->container.height = splitY;
                Room newRoom = {
                    .id = totalRooms
                };
                newRoom.container = (Rectangle){ room->container.x, room->container.y + splitY, room->container.width, originalHeight-splitY };
                rooms[totalRooms] = newRoom;
                totalRooms++;
            }
           returnValue = 0;
        }
    }
    return returnValue;
}

static void GenerateDungeon(void) {
    int shouldStop = 0;
    Rectangle initialContainer = (Rectangle){ -maxDungeonSize / 2, -maxDungeonSize / 2, maxDungeonSize, maxDungeonSize };
    Room initialRoom = {
        .id = 0,
        .container = initialContainer
    };
    rooms[0] = initialRoom;
    totalRooms = 1;
    int iterations = 0;

    while(!shouldStop && iterations++ < maxSplitIterations) {
        int current = 0;
        shouldStop = IterateGeneration();
    }

    for(int i = 0; i < totalRooms; i++) {
        Room *room = &rooms[i];

        int width = room->container.width * ROOM_FACTOR; //GetRandomValue(room->container.width * ROOM_FACTOR, room->container.width);
        int height = room->container.height * ROOM_FACTOR; //GetRandomValue(room->container.height * ROOM_FACTOR, room->container.height);
        int newX = abs(room->container.width - width) / 2 + room->container.x;
        int newY = abs(room->container.height - height) / 2 + room->container.y;

        Rectangle bounds = (Rectangle) { newX, newY, width, height };
        Vector2 normalizedPosition = XYToCoords(newX, newY);
        Rectangle normalizedBounds = (Rectangle) { normalizedPosition.x, normalizedPosition.y, width * TW, height * TW };

        room->bounds = bounds;
        room->normalizedBounds = normalizedBounds;
    }


}


static void GenerateTunnels(void) {
    // every adjacent room will be connected UP to a limit

}

static void DrawDungeon(void) {
    for(int i = 0; i < totalRooms; i++) {
        Room* room = &rooms[i];
        Vector2 position = XYToCoords(room->container.x, room->container.y);
        DrawRectangleLinesEx((Rectangle){ position.x, position.y, room->container.width * TW, room->container.height * TW}, 5.0f, COLOR_4);
        DrawRectangleRec(room->normalizedBounds, COLOR_2);
        DrawText(TextFormat("%i", i), room->normalizedBounds.x + 5, room->normalizedBounds.y + 5, 96, COLOR_4);
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
        ClearBackground(CLEAR_COLOR);

        BeginMode2D(*GetGameCamera());
            DrawDungeon();
            // DrawDebugGrid();
            DrawRectangle(playerPosition.x + TW / 4, playerPosition.y + TW / 4, playerDest.width, playerDest.height, COLOR_3);
            
        EndMode2D();

        DrawText(TextFormat("CurrentTurn: %s", currentTurn == PLAYER_TURN ? "Player" : "Dungeon"), 5, gameHeight - 15, 10, COLOR_3);
        DrawText(TextFormat("TotalRooms: %i", totalRooms), 5, gameHeight - 30, 10, COLOR_3);

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