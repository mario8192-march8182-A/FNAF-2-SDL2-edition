#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// Renderer Types
// ============================================================================

typedef enum {
    BLEND_MODE_NONE = 0,
    BLEND_MODE_BLEND,
    BLEND_MODE_ADD,
    BLEND_MODE_MOD,
    BLEND_MODE_MUL,
} BlendMode;

typedef enum {
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 1,
    FLIP_VERTICAL = 2,
    FLIP_BOTH = 3,
} FlipMode;

// Texture wrapper
typedef struct {
    SDL_Texture* sdl_texture;
    int width;
    int height;
    uint32_t id;
} Texture;

// Sprite definition (part of a sprite sheet)
typedef struct {
    Texture* texture;
    SDL_Rect source;     // Position in texture
    int pivot_x;         // Rotation pivot point
    int pivot_y;
    int frame_width;
    int frame_height;
} Sprite;

// Animation frame
typedef struct {
    Sprite sprite;
    float duration;      // How long to display this frame
} AnimationFrame;

// Animation controller
typedef struct {
    AnimationFrame* frames;
    int frame_count;
    int current_frame;
    float elapsed_time;
    bool is_playing;
    bool is_looping;
    float playback_speed;
} Animation;

// Color structure
typedef struct {
    uint8_t r, g, b, a;
} Color;

// Main renderer
typedef struct {
    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    int screen_width;
    int screen_height;
    bool is_fullscreen;
    bool vsync_enabled;
    Color clear_color;
    uint32_t frame_count;
} Renderer;

// Drawing context
typedef struct {
    float x, y;           // Position
    float scale_x, scale_y;
    float rotation;       // In degrees
    float alpha;          // 0.0 - 1.0
    Color tint;
    BlendMode blend_mode;
    FlipMode flip;
} DrawContext;

// ============================================================================
// Renderer Functions
// ============================================================================

// Core renderer management
Renderer* renderer_create(const char* title, int width, int height, bool fullscreen, bool vsync);
void renderer_destroy(Renderer* renderer);
void renderer_clear(Renderer* renderer);
void renderer_present(Renderer* renderer);

// Texture management
Texture* renderer_load_texture(Renderer* renderer, const char* filepath);
void renderer_free_texture(Texture* texture);
Texture* renderer_create_blank_texture(Renderer* renderer, int width, int height);

// Drawing primitives
void renderer_draw_rect(Renderer* renderer, int x, int y, int w, int h, Color color, bool filled);
void renderer_draw_circle(Renderer* renderer, int x, int y, int radius, Color color, bool filled);
void renderer_draw_line(Renderer* renderer, int x1, int y1, int x2, int y2, Color color, int thickness);
void renderer_draw_filled_polygon(Renderer* renderer, const SDL_Point* points, int count, Color color);

// Sprite rendering
void renderer_draw_sprite(Renderer* renderer, Sprite* sprite, DrawContext* context);
void renderer_draw_texture(Renderer* renderer, Texture* texture, int x, int y, DrawContext* context);
void renderer_draw_texture_scaled(Renderer* renderer, Texture* texture, int x, int y, float scale_x, float scale_y, DrawContext* context);
void renderer_draw_texture_rotated(Renderer* renderer, Texture* texture, int x, int y, float rotation, DrawContext* context);

// Animation functions
Animation* animation_create(int frame_count);
void animation_destroy(Animation* animation);
void animation_add_frame(Animation* anim, Sprite sprite, float duration);
void animation_play(Animation* anim, bool loop);
void animation_stop(Animation* anim);
void animation_update(Animation* anim, float delta_time);
Sprite* animation_get_current_sprite(Animation* anim);

// Batch rendering
void renderer_begin_batch(Renderer* renderer);
void renderer_end_batch(Renderer* renderer);

// Effects
void renderer_set_alpha(Renderer* renderer, uint8_t alpha);
void renderer_set_blend_mode(Renderer* renderer, BlendMode mode);
void renderer_set_tint(Renderer* renderer, Color color);

// Utilities
void renderer_screenshot(Renderer* renderer, const char* filepath);
Color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
DrawContext draw_context_create(void);

#endif // RENDERER_H
