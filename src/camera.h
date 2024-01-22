#ifndef CAMERA_H
#define CAMERA_H

#include "defs.h"

static Camera2D camera = { 0 };
void InitGameCamera(void);

void UpdateGameCamera(Vector2 target);
void SetGameCameraZoom(float zoom);

Camera2D* GetGameCamera(void);


/** TODO
 *  Follow Camera
 *  Move camera to with ease
 *  Rotate camera
*/


#endif