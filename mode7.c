#include <emscripten.h>
#include <math.h>
#include <malloc.h>

struct m7_perspective
{
  float fov;
  float near;
  float far;
};

struct m7_view
{
  float angle;
  float x;
  float y;
};

struct m7_plane
{
  int width;
  int height;
  char *data;
};

struct m7_view view;
struct m7_perspective perspective;
struct m7_plane plane;

float linear(float p, float a, float b) {
  return (b - a) * p + a;
}

int clamp(int value, int min, int max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

float clampf(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

EMSCRIPTEN_KEEPALIVE
char* get_safe_memory(unsigned int size) {
  return (char*)malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void delete_safe_memory(char* pointer) {
  free((void*)pointer);
}

EMSCRIPTEN_KEEPALIVE
void fill(int color, int width, int height, char* data) {
  char r = (color >> 24) & 0xFF;
  char g = (color >> 16) & 0xFF;
  char b = (color >> 8) & 0xFF;
  char a = color & 0xFF;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int offset = (y * width + x) * 4;
      data[offset + 0] = r;
      data[offset + 1] = g;
      data[offset + 2] = b;
      data[offset + 3] = a;
    }
  }
}

EMSCRIPTEN_KEEPALIVE
void fill_rect(
  int color,
  int rx,
  int ry,
  int rw,
  int rh,
  int width,
  int height,
  char* data
) {
  char r = (color >> 24) & 0xFF;
  char g = (color >> 16) & 0xFF;
  char b = (color >> 8) & 0xFF;
  char a = color & 0xFF;

  int rex = rx + rw;
  int rey = ry + rh;

  int ex = rex > width ? width : rex;
  int ey = rey > height ? height : rey;

  int y = ry;
  int x = rx;

  for (int y = ry; y < ey; y++) {
    for (int x = rx; x < ex; x++) {
      int offset = ((y * width) + x) * 4;
      data[offset + 0] = r;
      data[offset + 1] = g;
      data[offset + 2] = b;
      data[offset + 3] = a;
    }
  }
}

EMSCRIPTEN_KEEPALIVE
void set_plane(int width, int height, char *data)
{
  plane.width = width;
  plane.height = height;
  plane.data = data;
}

EMSCRIPTEN_KEEPALIVE
void set_perspective(float fov, float near, float far)
{
  perspective.fov = fov;
  perspective.near = near;
  perspective.far = far;
}

EMSCRIPTEN_KEEPALIVE
void set_view(float angle, float x, float y)
{
  view.angle = angle;
  view.x = x;
  view.y = y;
}

EMSCRIPTEN_KEEPALIVE
void render(int width, int height, char *data)
{
  float halfFov = perspective.fov * 0.5f;

  float fovStartX = cosf(view.angle - halfFov);
  float fovStartY = sinf(view.angle - halfFov);

  float fovEndX = cosf(view.angle + halfFov);
  float fovEndY = sinf(view.angle + halfFov);

  float farStartX = view.x + fovStartX * perspective.far;
  float farStartY = view.y + fovStartY * perspective.far;

  float farEndX = view.x + fovEndX * perspective.far;
  float farEndY = view.y + fovEndY * perspective.far;

  float nearStartX = view.x + fovStartX * perspective.near;
  float nearStartY = view.y + fovStartY * perspective.near;

  float nearEndX = view.x + fovEndX * perspective.near;
  float nearEndY = view.y + fovEndY * perspective.near;

  int horizonHeight = height >> 1;

  for (int y = 1; y < horizonHeight; y++) {
    float ny = (float)(y) / (float)(horizonHeight);

    int dy = (horizonHeight + (y - 1));

    float nz = 1.0f / ny;

    float textureStartX = linear(nz, nearStartX, farStartX);
    float textureStartY = linear(nz, nearStartY, farStartY);

    float textureEndX = linear(nz, nearEndX, farEndX);
    float textureEndY = linear(nz, nearEndY, farEndY);

    for (int x = 0; x < width; x++) {
      float nx = (float)(x) / (float)(width);

      int offset = (dy * width + x) * 4;

      float textureX = linear(nx, textureStartX, textureEndX);
      float textureY = linear(nx, textureStartY, textureEndY);

      if (textureX > 1.0f || textureX < 0.0f
       || textureY > 1.0f || textureY < 0.0f) {
        data[offset + 0] = 0x00;
        data[offset + 1] = 0x00;
        data[offset + 2] = 0x00;
        data[offset + 3] = 0x00;
        continue;
      }

      float px = roundf(fabsf(textureX * plane.width));
      float py = roundf(fabsf(textureY * plane.height));

      int sampleX = (int)(px);
      int sampleY = (int)(py);

      int planeOffset = (sampleY * plane.width + sampleX) * 4;
      if (planeOffset < 0 || planeOffset > plane.width * plane.height * 4) {
        data[offset + 0] = 0x00;
        data[offset + 1] = 0x00;
        data[offset + 2] = 0x00;
        data[offset + 3] = 0x00;
        continue;
      }

      data[offset + 0] = plane.data[planeOffset + 0];
      data[offset + 1] = plane.data[planeOffset + 1];
      data[offset + 2] = plane.data[planeOffset + 2];
      data[offset + 3] = 0xFF;
    }
  }
}
