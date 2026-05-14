#include "assets.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Asset Manager Creation/Destruction
// ============================================================================

AssetManager* asset_manager_create(Renderer* renderer, const char* base_path, int initial_capacity) {
    if (!renderer || !base_path) return NULL;

    AssetManager* manager = (AssetManager*)malloc(sizeof(AssetManager));
    if (!manager) return NULL;

    manager->assets = (Asset*)malloc(sizeof(Asset) * initial_capacity);
    if (!manager->assets) {
        free(manager);
        return NULL;
    }

    manager->renderer = renderer;
    manager->asset_count = 0;
    manager->asset_capacity = initial_capacity;
    manager->asset_id_counter = 1;

    strcpy(manager->base_path, base_path);

    // Statistics
    manager->loaded_textures = 0;
    manager->loaded_spritesheets = 0;
    manager->total_memory_used = 0;

    return manager;
}

void asset_manager_destroy(AssetManager* manager) {
    if (!manager) return;

    // Unload all assets
    for (int i = 0; i < manager->asset_count; i++) {
        Asset* asset = &manager->assets[i];

        switch (asset->metadata.type) {
            case ASSET_TYPE_TEXTURE:
                if (asset->data.texture) {
                    renderer_free_texture(asset->data.texture);
                }
                break;

            case ASSET_TYPE_SPRITESHEET: {
                SpriteSheet* sheet = asset->data.spritesheet;
                if (sheet) {
                    if (sheet->sprites) {
                        free(sheet->sprites);
                    }
                    if (sheet->texture) {
                        renderer_free_texture(sheet->texture);
                    }
                    free(sheet);
                }
                break;
            }

            default:
                break;
        }
    }

    if (manager->assets) {
        free(manager->assets);
    }

    free(manager);
}

// ============================================================================
// Texture Management
// ============================================================================

Asset* asset_manager_load_texture(AssetManager* manager, const char* name, const char* filepath) {
    if (!manager || !name || !filepath) return NULL;

    // Check if already loaded
    Asset* existing = asset_manager_get_asset(manager, name);
    if (existing) {
        existing->metadata.reference_count++;
        return existing;
    }

    // Expand assets array if needed
    if (manager->asset_count >= manager->asset_capacity) {
        manager->asset_capacity *= 2;
        Asset* new_assets = (Asset*)realloc(manager->assets, sizeof(Asset) * manager->asset_capacity);
        if (!new_assets) return NULL;
        manager->assets = new_assets;
    }

    // Load texture
    clock_t start = clock();
    Texture* texture = renderer_load_texture(manager->renderer, filepath);
    clock_t end = clock();

    if (!texture) {
        fprintf(stderr, "Failed to load texture: %s\n", filepath);
        return NULL;
    }

    // Create asset
    Asset* asset = &manager->assets[manager->asset_count];
    asset->metadata.id = manager->asset_id_counter++;
    asset->metadata.type = ASSET_TYPE_TEXTURE;
    asset->metadata.reference_count = 1;
    asset->metadata.is_loaded = true;
    asset->metadata.load_time = (float)(end - start) / CLOCKS_PER_SEC;

    strcpy(asset->metadata.name, name);
    strcpy(asset->metadata.filepath, filepath);

    asset->data.texture = texture;

    manager->asset_count++;
    manager->loaded_textures++;
    manager->total_memory_used += texture->width * texture->height * 4;

    return asset;
}

Texture* asset_manager_get_texture(AssetManager* manager, const char* name) {
    if (!manager || !name) return NULL;

    Asset* asset = asset_manager_get_asset(manager, name);
    if (asset && asset->metadata.type == ASSET_TYPE_TEXTURE) {
        return asset->data.texture;
    }

    return NULL;
}

// ============================================================================
// Spritesheet Management
// ============================================================================

Asset* asset_manager_load_spritesheet(
    AssetManager* manager,
    const char* name,
    const char* filepath,
    int cols, int rows,
    int sprite_width, int sprite_height) {

    if (!manager || !name || !filepath || cols <= 0 || rows <= 0) return NULL;

    // Check if already loaded
    Asset* existing = asset_manager_get_asset(manager, name);
    if (existing) {
        existing->metadata.reference_count++;
        return existing;
    }

    // Expand assets array if needed
    if (manager->asset_count >= manager->asset_capacity) {
        manager->asset_capacity *= 2;
        Asset* new_assets = (Asset*)realloc(manager->assets, sizeof(Asset) * manager->asset_capacity);
        if (!new_assets) return NULL;
        manager->assets = new_assets;
    }

    // Load spritesheet texture
    clock_t start = clock();
    Texture* texture = renderer_load_texture(manager->renderer, filepath);
    clock_t end = clock();

    if (!texture) {
        fprintf(stderr, "Failed to load spritesheet texture: %s\n", filepath);
        return NULL;
    }

    // Create spritesheet
    SpriteSheet* sheet = (SpriteSheet*)malloc(sizeof(SpriteSheet));
    if (!sheet) {
        renderer_free_texture(texture);
        return NULL;
    }

    int sprite_count = cols * rows;
    sheet->sprites = (Sprite*)malloc(sizeof(Sprite) * sprite_count);
    if (!sheet->sprites) {
        free(sheet);
        renderer_free_texture(texture);
        return NULL;
    }

    sheet->texture = texture;
    sheet->cols = cols;
    sheet->rows = rows;
    sheet->sprite_width = sprite_width;
    sheet->sprite_height = sprite_height;
    sheet->sprite_count = sprite_count;

    // Generate sprites
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int index = row * cols + col;
            sheet->sprites[index].texture = texture;
            sheet->sprites[index].source.x = col * sprite_width;
            sheet->sprites[index].source.y = row * sprite_height;
            sheet->sprites[index].source.w = sprite_width;
            sheet->sprites[index].source.h = sprite_height;
            sheet->sprites[index].frame_width = sprite_width;
            sheet->sprites[index].frame_height = sprite_height;
            sheet->sprites[index].pivot_x = sprite_width / 2;
            sheet->sprites[index].pivot_y = sprite_height / 2;
        }
    }

    // Create asset
    Asset* asset = &manager->assets[manager->asset_count];
    asset->metadata.id = manager->asset_id_counter++;
    asset->metadata.type = ASSET_TYPE_SPRITESHEET;
    asset->metadata.reference_count = 1;
    asset->metadata.is_loaded = true;
    asset->metadata.load_time = (float)(end - start) / CLOCKS_PER_SEC;

    strcpy(asset->metadata.name, name);
    strcpy(asset->metadata.filepath, filepath);

    asset->data.spritesheet = sheet;

    manager->asset_count++;
    manager->loaded_spritesheets++;
    manager->total_memory_used += texture->width * texture->height * 4;
    manager->total_memory_used += sizeof(Sprite) * sprite_count;

    return asset;
}

SpriteSheet* asset_manager_get_spritesheet(AssetManager* manager, const char* name) {
    if (!manager || !name) return NULL;

    Asset* asset = asset_manager_get_asset(manager, name);
    if (asset && asset->metadata.type == ASSET_TYPE_SPRITESHEET) {
        return asset->data.spritesheet;
    }

    return NULL;
}

// ============================================================================
// Asset Queries
// ============================================================================

Asset* asset_manager_get_asset(AssetManager* manager, const char* name) {
    if (!manager || !name) return NULL;

    for (int i = 0; i < manager->asset_count; i++) {
        if (strcmp(manager->assets[i].metadata.name, name) == 0) {
            return &manager->assets[i];
        }
    }

    return NULL;
}

bool asset_manager_is_loaded(AssetManager* manager, const char* name) {
    Asset* asset = asset_manager_get_asset(manager, name);
    return asset != NULL && asset->metadata.is_loaded;
}

void asset_manager_unload_asset(AssetManager* manager, const char* name) {
    if (!manager || !name) return;

    Asset* asset = asset_manager_get_asset(manager, name);
    if (!asset) return;

    asset->metadata.reference_count--;

    if (asset->metadata.reference_count <= 0) {
        switch (asset->metadata.type) {
            case ASSET_TYPE_TEXTURE:
                if (asset->data.texture) {
                    renderer_free_texture(asset->data.texture);
                    asset->data.texture = NULL;
                }
                manager->loaded_textures--;
                break;

            case ASSET_TYPE_SPRITESHEET: {
                SpriteSheet* sheet = asset->data.spritesheet;
                if (sheet) {
                    if (sheet->sprites) free(sheet->sprites);
                    if (sheet->texture) renderer_free_texture(sheet->texture);
                    free(sheet);
                    asset->data.spritesheet = NULL;
                }
                manager->loaded_spritesheets--;
                break;
            }

            default:
                break;
        }

        asset->metadata.is_loaded = false;
    }
}

// ============================================================================
// Batch Loading
// ============================================================================

void asset_manager_load_animatronic(AssetManager* manager, const char* name, const char* base_filepath) {
    if (!manager || !name || !base_filepath) return;

    char full_path[512];

    // Load idle animation
    snprintf(full_path, sizeof(full_path), "%s_idle.png", base_filepath);
    asset_manager_load_spritesheet(manager, name, full_path, 4, 1, 128, 128);

    // Load attack animation
    snprintf(full_path, sizeof(full_path), "%s_attack.png", base_filepath);
    asset_manager_load_spritesheet(manager, name, full_path, 4, 1, 128, 128);
}

void asset_manager_load_ui_assets(AssetManager* manager) {
    // Load common UI assets
    // Buttons, panels, etc.
}

void asset_manager_load_effect_assets(AssetManager* manager) {
    // Load effect sprites
    // Particles, animations, etc.
}

// ============================================================================
// Memory Management
// ============================================================================

void asset_manager_clear_unused(AssetManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->asset_count; i++) {
        if (manager->assets[i].metadata.reference_count <= 0) {
            asset_manager_unload_asset(manager, manager->assets[i].metadata.name);
        }
    }
}

int asset_manager_get_memory_usage(AssetManager* manager) {
    if (!manager) return 0;
    return manager->total_memory_used;
}

// ============================================================================
// Statistics
// ============================================================================

void asset_manager_print_stats(AssetManager* manager) {
    if (!manager) return;

    printf("\n========== Asset Manager Statistics ==========\n");
    printf("Total Assets: %d\n", manager->asset_count);
    printf("Loaded Textures: %d\n", manager->loaded_textures);
    printf("Loaded Spritesheets: %d\n", manager->loaded_spritesheets);
    printf("Total Memory Used: %d bytes (%.2f MB)\n",
           manager->total_memory_used,
           manager->total_memory_used / (1024.0f * 1024.0f));
    printf("\nLoaded Assets:\n");

    for (int i = 0; i < manager->asset_count; i++) {
        Asset* asset = &manager->assets[i];
        const char* type_name;

        switch (asset->metadata.type) {
            case ASSET_TYPE_TEXTURE:
                type_name = "Texture";
                break;
            case ASSET_TYPE_SPRITESHEET:
                type_name = "Spritesheet";
                break;
            default:
                type_name = "Unknown";
        }

        printf("  [%d] %s (Type: %s, Refs: %d, Load Time: %.3fs)\n",
               asset->metadata.id,
               asset->metadata.name,
               type_name,
               asset->metadata.reference_count,
               asset->metadata.load_time);
    }

    printf("==============================================\n\n");
}
