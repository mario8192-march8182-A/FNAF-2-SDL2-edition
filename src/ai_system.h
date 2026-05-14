#ifndef AI_SYSTEM_H
#define AI_SYSTEM_H

#include "entity.h"
#include <stdint.h>
#include <stdbool.h>

// Animatronic states
typedef enum {
    AI_STATE_INACTIVE,      // Dormant/charging
    AI_STATE_IDLE,          // Moving slowly in stage
    AI_STATE_ALERT,         // Detected player activity
    AI_STATE_HUNTING,       // Actively pursuing player
    AI_STATE_ATTACKING,     // At office door/window
    AI_STATE_OFFICE,        // Inside the office
    AI_STATE_DESPAWNED,     // Waiting to respawn
    AI_STATE_GLITCHED       // Withered state (corrupted behavior)
} AIState;

// Animatronic types
typedef enum {
    ANIMATRONIC_MAGNUS,          // Main bear (Toy Freddy replacement)
    ANIMATRONIC_STRIKER,         // Rabbit (Toy Bonnie replacement)
    ANIMATRONIC_SCRAPBIRD,       // Bird (Toy Chica replacement)
    ANIMATRONIC_RAVAGER,         // Fox (Mangle replacement)
    ANIMATRONIC_VOLT,            // Dog (custom)
    ANIMATRONIC_SOVEREIGN,       // Golden (Golden Freddy replacement)
    ANIMATRONIC_WITHERED_MAGNUS, // Withered/Corrupted Magnus
    ANIMATRONIC_PUPPET,          // Marionette
    ANIMATRONIC_COUNT
} AnimatronicType;

// Location/Camera system
typedef enum {
    LOCATION_STAGE,           // Main performance stage
    LOCATION_PARTS_CORRIDOR,  // Parts and service corridor
    LOCATION_BACK_ROOM,       // Storage/back room
    LOCATION_OFFICE_DOOR,     // Outside player office
    LOCATION_OFFICE_ENTRANCE, // Office entrance
    LOCATION_OFFICE_INSIDE,   // Inside the office
    LOCATION_VENT_SYSTEM,     // Ventilation ducts
    LOCATION_GAME_OVER,       // Game over state
    LOCATION_COUNT
} LocationType;

// AI behavior modes
typedef struct {
    float aggression;           // 0.0 - 1.0 (how active they are)
    float intelligence;         // 0.0 - 1.0 (how they hunt)
    float speed_multiplier;     // Movement speed factor
    float awareness_range;      // How far they can "sense" the player
    float randomness;           // How predictable they are
    bool can_use_vents;         // Can enter ventilation system
    bool can_cut_power;         // Can cause blackouts
    bool can_move_fast;         // Sprint capability
} AIBehavior;

// AI Navigation waypoint
typedef struct {
    LocationType location;
    float x, y;
    float pause_duration;       // How long to wait here
    bool is_decision_point;     // Branch in path
} Waypoint;

// AI system for individual animatronic
typedef struct {
    AnimatronicType type;
    AIState current_state;
    AIState previous_state;
    
    // Behavioral parameters
    AIBehavior behavior;
    
    // Movement
    LocationType current_location;
    LocationType target_location;
    Waypoint* waypoints;
    int waypoint_count;
    int current_waypoint;
    
    // Timing
    float state_timer;
    float action_cooldown;
    float hunt_timer;
    
    // Activity tracking
    int night_level;            // Difficulty scaling
    float activity_level;       // Current activity (0.0 - 1.0)
    int times_attacked;
    
    // Awareness
    float player_detection_chance;
    bool has_detected_player;
    float detection_confidence;
    
    // Status
    bool is_active;
    bool is_in_office;
    bool is_attacking;
    bool is_glitched;           // For withered animatronics
    
    // Entity reference
    Entity* entity;
    
    // Special abilities
    bool can_disable_door;
    bool can_disable_camera;
    bool can_cause_blackout;
} AIController;

// Global AI manager
typedef struct {
    AIController* animatronics[ANIMATRONIC_COUNT];
    float game_time;
    int night;
    float player_power_usage;
    float ambient_noise;        // Affects detection
    bool blackout_active;
    float blackout_timer;
    
    // Spawn management
    float respawn_timer[ANIMATRONIC_COUNT];
    bool has_spawned[ANIMATRONIC_COUNT];
    
    // Difficulty scaling
    float difficulty_multiplier;
} AIManager;

// Function declarations
AIManager* ai_manager_create(void);
void ai_manager_destroy(AIManager* manager);

AIController* ai_controller_create(AnimatronicType type, int night);
void ai_controller_destroy(AIController* controller);

void ai_manager_update(AIManager* manager, float delta_time);
void ai_controller_update(AIController* controller, AIManager* manager, float delta_time);

void ai_change_state(AIController* controller, AIState new_state);
void ai_set_target_location(AIController* controller, LocationType location);

// Behavior queries
float ai_calculate_aggression(AIController* controller, int night);
float ai_calculate_detection_chance(AIController* controller, AIManager* manager);
bool ai_should_attack_player(AIController* controller, AIManager* manager);
bool ai_can_reach_player(AIController* controller, AIManager* manager);

// Special abilities
void ai_trigger_attack(AIController* controller);
void ai_trigger_blackout(AIManager* manager, AnimatronicType attacker);
void ai_trigger_glitch(AIController* controller);

// Debugging
void ai_debug_print_state(AIController* controller);

#endif // AI_SYSTEM_H
