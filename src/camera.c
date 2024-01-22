#include "camera.h"

void InitGameCamera(void) {
    camera.target = (Vector2) { 0.0f, 0.0f };
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateGameCamera(Vector2 target) {
    camera.target = (Vector2){ target.x, target.y };
}

void SetGameCameraZoom(float zoom) {
    // TODO: check if zoom in [0.1f, 3.0f] 
    camera.zoom = zoom;
}

Camera2D* GetGameCamera(void) {
    return &camera;
}