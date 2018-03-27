#include <am.h>
#include <amdev.h>
#include <SDL2/SDL.h>

#define W 400
#define H 300

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }

static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_Texture *texture;
static uint32_t fb[W * H];

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

static void texture_sync() {
  SDL_UpdateTexture(texture, NULL, fb, W * sizeof(Uint32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void video_init() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(W * 2, H * 2, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "Native Application");
  texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, W, H);
  memset(fb, 0, W * H * sizeof(uint32_t));
  texture_sync();
}

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = W;
      info->height = H;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int cp_bytes = sizeof(uint32_t) * min(w, W - x);
      for (int j = 0; j < h && y + j < H; j ++) {
        memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
        pixels += w;
      }
      if (ctl->sync) {
        texture_sync();
      }
      return size;
    }
  }
  return 0;
}