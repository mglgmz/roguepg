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
/////////////////// PLAYER ////////////////////////
Rectangle playerDest = {
    .x = 1,
    .y = 1,
    .width = TW,
    .height = TW  
};
Vector2 playerPosition;


#define DEFAULT_PLAYER_ACTIONS 1
static int maxPlayerActions = DEFAULT_PLAYER_ACTIONS;
static int playerRemaingActions = DEFAULT_PLAYER_ACTIONS;

int maxHp = 10;
int hp = 10;



int power = 1;
int mPower = 1;

int crit = 1;
float critPower = 1.2f; 

float dodge = 0.0f;

#define NUM_DAMAGE_TYPES 7
typedef enum DamageTypes {
    TRUE = 1,
    PHYSICAL = 2,
    FIRE = 4,
    ICE = 8,
    LIGHTNING = 16,
    EARTH = 32,
    DARK = 64,
    MAGIC = 128
} DamageTypes;

// True cannot be resisted
int shields[NUM_DAMAGE_TYPES] = { 0, 0, 0, 0, 0, 0, 0 };
int resistances[NUM_DAMAGE_TYPES] = { 0, 0, 0, 0, 0, 0, 0 };

///////////////////////////////////////////////////
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

#define BLOCK_ID 0
#define EMPTY_ID 1

#define BLOCK_CHANCE 130
#define ENEMY_CHANCE 25
#define TRESURE_CHANCE 5

#define ENEMY_ID_START 20
#define NUM_ENEMIES 10

#define TRESURE_ID 99

Vector2 dungeonOrigin = { 0 };
int layout[DEFAULT_DUNGEON_SIZE][DEFAULT_DUNGEON_SIZE];

#define MAX_ENEMIES 40
#define MAX_TREASURES 20

int numTreasures = 0;

int enemiesLeft = 0;
int tresuresLeft = 0;

typedef struct Treasure {
    Vector2 position;
    int claimed;
    int numItems;
    int items[5];
    int gp;
    int rarity;
} Treasure;

Treasure treasure[MAX_TREASURES];

// Enemies
typedef struct Enemy {
    int type;
    Vector2 position;
    int detection;
    int actionsPerTurn;
    int inEncounter;
} Enemy;
Enemy enemies[MAX_ENEMIES];


static void GenerateDungeon(void) {
    dungeonOrigin = (Vector2) { -DEFAULT_DUNGEON_SIZE * TW / 2, -DEFAULT_DUNGEON_SIZE * TW / 2 };
    Rectangle initialBounds = (Rectangle){ 0, 0, DEFAULT_DUNGEON_SIZE, DEFAULT_DUNGEON_SIZE };
    for(int x = 0; x < DEFAULT_DUNGEON_SIZE; x++) {
        for(int y = 0; y < DEFAULT_DUNGEON_SIZE; y++) {

            layout[x][y] = EMPTY_ID;

            if((playerDest.x == x && playerDest.y == y)) continue;
            
            int roll = GetRandomValue(0, ROLL_MAX);
            
            if(roll < BLOCK_CHANCE) layout[x][y] = BLOCK_ID;
            else {
                roll = GetRandomValue(0, ROLL_MAX);
                if (enemiesLeft < MAX_ENEMIES && roll < ENEMY_CHANCE) { 
                    // layout[DEFAULT_DUNGEON_SIZE - x][DEFAULT_DUNGEON_SIZE - y] = ENEMY_ID_START + GetRandomValue(0, NUM_ENEMIES);
                    Enemy enemy = {
                        .type = ENEMY_ID_START + GetRandomValue(0, NUM_ENEMIES),
                        .actionsPerTurn = 1
                    };
                    enemy.position = (Vector2) { x, y };
                    enemy.detection = GetRandomValue(5, 15);
                    enemy.inEncounter = 0;
                    enemies[enemiesLeft] = enemy;
                    enemiesLeft++;

                    // TODO: Generate one random enemy, save it's details

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

static Vector2 CoordsToPos(int x, int y, int tw) {
    return (Vector2){ dungeonOrigin.x + x * tw, dungeonOrigin.y + y * tw };
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
            if(layout[x][y] == BLOCK_ID) {
                DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, DARKGRAY);
                DrawRectangleLines(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, LIGHTGRAY);
            }else if(layout[x][y] >= ENEMY_ID_START && layout[x][y] <= ENEMY_ID_START + NUM_ENEMIES) {
                //DrawRectangle(dungeonOrigin.x + x * TW, dungeonOrigin.y + y * TW, TW, TW, RED);
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

    for(int i=0; i< MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        Vector2 pos = CoordsToPos(enemy->position.x, enemy->position.y, TW);
        DrawRectangle(pos.x, pos.y, TW, TW, RED);
    }

}




static int GetValue(int x, int y) {
    if ( x < 0 || x >= DEFAULT_DUNGEON_SIZE || y < 0 || y >= DEFAULT_DUNGEON_SIZE) return 0;
    return layout[x][y];
}

static int CanMove(int x, int y, int xOff, int yOff) {
    return GetValue(x + xOff, y + yOff); 
}

///////////////////////////////////////////////////

/////////////////// UPDATE ////////////////////////

int numEncounterEnemies = 0;
int encounterEnemies[10];

static void DetectEncounters(void) {
    for(int i=0; i< MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if(enemy->inEncounter) continue;
        if(enemy->position.x == playerDest.x && enemy->position.y == playerDest.y) {
            encounterEnemies[numEncounterEnemies] = i;
            numEncounterEnemies++;
            enemy->inEncounter = 1;
        }
    }

    if(numEncounterEnemies > 0) {
        currentState = COMBAT_STATE;
        currentTurn = PLAYER_TURN;
    }
}

static void PlayerTurn (void) {
    currentTurn = PLAYER_TURN;
    playerRemaingActions = maxPlayerActions;
}

static void MoveEnemies(void) {
    for(int i=0; i< MAX_ENEMIES; i++) {
        Enemy* enemy = &enemies[i];
        if(enemy->inEncounter) continue;
        // TODO check if enemy detected player
        int distanceToPlayer = GetMoveDistance(enemy->position.x , enemy->position.y, playerDest.x, playerDest.y);
        if(distanceToPlayer > enemy->detection || distanceToPlayer == 0) continue;

        int actionsTaken = 0;

        while(actionsTaken < enemy->actionsPerTurn) {
            
            int dx = playerDest.x < enemy->position.x ? -1 :
                    playerDest.x > enemy->position.x ? 1 : 0;
            // TODO Account for number of actions

            if(dx != 0 && CanMove(enemy->position.x, enemy->position.y, dx, 0)) {
                enemy->position = (Vector2) { enemy->position.x + dx, enemy->position.y };
            } else {
                int dy = playerDest.y < enemy->position.y ? -1 :
                    playerDest.y > enemy->position.y ? 1 : 0;
                if(dy != 0 && CanMove(enemy->position.x, enemy->position.y, 0, dy)) {
                    enemy->position = (Vector2) { enemy->position.x, enemy->position.y + dy };
                }
            }
            actionsTaken++;
        }
    }
}

static void DungeonTurn(void) {
    currentTurn = DUNGEON_TURN;
    dungeonRemaingTime = maxDungeonTime;
    //TODO change time and just change turn when dungeon has no remaing actions
    MoveEnemies();

    DetectEncounters();
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
    
    if(numEncounterEnemies == 0) {
        currentState = MAP_STATE;
        PlayerTurn();
    }
}

///////////////////////////////////////////////////


//////////////////// RENDER ///////////////////////

static void RenderMap(void) {

    BeginMode2D(*GetGameCamera());
        DrawDungeon();
        
        // DrawDebugGrid();
        DrawRectangle(playerPosition.x, playerPosition.y, playerDest.width, playerDest.height, GREEN);
        
    EndMode2D();

    DrawText(TextFormat("Enemies Left: %i", enemiesLeft), 5, gameHeight - 45, 10, COLOR_3);
    DrawText(TextFormat("CurrentState: %s", currentState == MAP_STATE ? "Map" : "Combat"), 5, gameHeight - 30, 10, COLOR_3);
    DrawText(TextFormat("CurrentTurn: %s", currentTurn == PLAYER_TURN ? "Player" : "Dungeon"), 5, gameHeight - 15, 10, COLOR_3);
    // DrawText(TextFormat("TotalRooms: %i", totalRooms), 5, gameHeight - 30, 10, COLOR_3);

}

int combatMargin = 15;
const int enemyWidth = 80;
static void RenderCombat(void) {
    DrawRectangle(0 + combatMargin, 0 + combatMargin, screenWidth - 2 * combatMargin, screenHeight - 2 * combatMargin, Fade(BLACK, 0.75f));
numEncounterEnemies = 2;
    // enemies
    int offset = (gameWidth - combatMargin * 4) / (numEncounterEnemies + 1);
    int currentPosition = offset;
    for(int i = 0; i < numEncounterEnemies; i++) {
        DrawRectangle(currentPosition, 150, enemyWidth, enemyWidth, RED);
        //Enemy* enemy = &enemies[encounterEnemies[i]];
        currentPosition += offset;
    }


}

static void RenderGame(void) {
    BeginDrawing();
        ClearBackground(RAYWHITE);
        RenderMap();
        if (currentState == COMBAT_STATE)
            RenderCombat();
    EndDrawing();
}
 
///////////////////////////////////////////////////

static void UpdateDrawFrame(void) {
#if !defined(EDITOR_MODE)
    if(currentState == MAP_STATE)  {
        UpdateMap();
    }
    else if (currentState == COMBAT_STATE)  {
        UpdateCombat();
    }
    
    RenderGame();

#else
    UpdateEditor();
    RenderEditor();
#endif

}