#include "camera.h"

#include "raymath.h"

#include <math.h>

#define CAMERA_FOVY            90
#define CAMERA_INIT_MAG_FACTOR 2.0f
#define CAMERA_MIN_MAG_FACTOR  1.25f
#define CAMERA_MAX_MAG_FACTOR  2.5f
#define CAMERA_INIT_THETA      (PI / 5)
#define CAMERA_INIT_PHI        (PI / 3)
#define CAMERA_PHI_EPSILON     0.01f

#define MOUSE_ROTATE_SENSITIVITY 0.005f
#define MOUSE_WHEEL_GAIN         10.0f
#define MOUSE_WHEEL_DAMPING      0.9f

/* Spherical coords for camera position around the origin, with a velocity
 * term on the magnitude for inertial wheel-zoom. */
static float camera_mag;
static float camera_mag_vel;
static float camera_theta;
static float camera_phi;

Camera camera = {
    { 0 },
    { 0, 0, 0 },
    { 0, 1, 0 },
    CAMERA_FOVY, CAMERA_PERSPECTIVE
};

void camera_init (int cube_size)
{
    camera_mag     = CAMERA_INIT_MAG_FACTOR * cube_size;
    camera_mag_vel = 0.0f;
    camera_theta   = CAMERA_INIT_THETA;
    camera_phi     = CAMERA_INIT_PHI;
}

void camera_update (int cube_size)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) camera_init(cube_size);

    float dt = GetFrameTime();

    camera_mag += camera_mag_vel * dt;
    if (camera_mag < CAMERA_MIN_MAG_FACTOR * cube_size) camera_mag = CAMERA_MIN_MAG_FACTOR * cube_size;
    if (camera_mag > CAMERA_MAX_MAG_FACTOR * cube_size) camera_mag = CAMERA_MAX_MAG_FACTOR * cube_size;
    camera_mag_vel -= GetMouseWheelMove() * MOUSE_WHEEL_GAIN;
    camera_mag_vel *= MOUSE_WHEEL_DAMPING;

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 delta = GetMouseDelta();
        camera_theta -= delta.x * MOUSE_ROTATE_SENSITIVITY;
        camera_phi -= delta.y * MOUSE_ROTATE_SENSITIVITY;
    }
    if (camera_phi >= PI) camera_phi = PI - CAMERA_PHI_EPSILON;
    if (camera_phi <= 0) camera_phi = CAMERA_PHI_EPSILON;

    camera.position.z = sinf(camera_phi) * cosf(camera_theta) * camera_mag;
    camera.position.x = sinf(camera_phi) * sinf(camera_theta) * camera_mag;
    camera.position.y = cosf(camera_phi) * camera_mag;
}
