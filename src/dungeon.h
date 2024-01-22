/**

// V2
#define MAX_ROOM_EXITS 4
typedef struct DungeonRoom {
    Rectangle bounds;
    DungeonRoomUnion exits[MAX_ROOM_EXITS];
    int usedExits;
} DungeonRoom;

typedef struct DungeonRoomUnion {
    int roomId;
    int toRoomId;
    Vector2 roomPosition;
    Vector2 toRoomPosition;
} DungeonRoomUnion;

typedef struct BSPLeaf {
    Rectangle container;
    Rectangle room;

    BSPLeaf left;
    BSPLeaf right;

    int hasLeft;
    int hasRight;

    DungeonRoomUnion exits[MAX_ROOM_EXITS];
    int usedExits;
} BSPLeaf;

BSPLeaf dungeonLeaves;
DungeonRoom rooms;

static void GenerateDungeonV2(void) {

}

static void IterateDungeonBSPV2(void) {

}

static void GenerateExitsV2(void) {

}

static void DrawDungeonV2(void) {

}

*/