#include "ai_system.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// AI Manager Creation/Destruction
// ============================================================================

AIManager* ai_manager_create(void) {
    AIManager* manager = (AIManager*)malloc(sizeof(AIManager));
    if (!manager) return NULL;
    
    manager->game_time = 0.0f;
    manager->night = 1;
    manager->player_power_usage = 0.0f;
    manager->ambient_noise = 0.0f;
    manager->blackout_active = false;
    manager->blackout_timer = 0.0f;
    manager->difficulty_multiplier = 1.0f + (manager->night * 0.15f);
    
    // Initialize animatronics
    for (int i = 0; i < ANIMATRONIC_COUNT; i++) {
        manager->animatronics[i] = ai_controller_create((AnimatronicType)i, manager->night);
        manager->respawn_timer[i] = 0.0f;
        manager->has_spawned[i] = false;
    }
    
    return manager;
}

void ai_manager_destroy(AIManager* manager) {
    if (!manager) return;
    
    for (int i = 0; i < ANIMATRONIC_COUNT; i++) {
        if (manager->animatronics[i]) {
            ai_controller_destroy(manager->animatronics[i]);
        }
    }
    
    free(manager);
}

// ============================================================================
// AI Controller Creation/Destruction
// ============================================================================

AIController* ai_controller_create(AnimatronicType type, int night) {
    AIController* controller = (AIController*)malloc(sizeof(AIController));
    if (!controller) return NULL;
    
    controller->type = type;
    controller->current_state = AI_STATE_INACTIVE;
    controller->previous_state = AI_STATE_INACTIVE;
    controller->current_location = LOCATION_STAGE;
    controller->target_location = LOCATION_STAGE;
    
    controller->state_timer = 0.0f;
    controller->action_cooldown = 0.0f;
    controller->hunt_timer = 0.0f;
    
    controller->night_level = night;
    controller->activity_level = 0.0f;
    controller->times_attacked = 0;
    controller->player_detection_chance = 0.0f;
    controller->has_detected_player = false;
    controller->detection_confidence = 0.0f;
    
    controller->is_active = true;
    controller->is_in_office = false;
    controller->is_attacking = false;
    controller->is_glitched = (type == ANIMATRONIC_WITHERED_MAGNUS);
    
    controller->entity = NULL;
    
    // Initialize behavior based on animatronic type
    switch (type) {
        case ANIMATRONIC_MAGNUS:
            controller->behavior.aggression = 0.4f + (night * 0.1f);
            controller->behavior.intelligence = 0.6f;
            controller->behavior.speed_multiplier = 1.0f;
            controller->behavior.awareness_range = 1500.0f;
            controller->behavior.randomness = 0.3f;
            controller->behavior.can_use_vents = false;
            controller->behavior.can_cut_power = true;
            controller->behavior.can_move_fast = false;
            break;
            
        case ANIMATRONIC_STRIKER: // Most aggressive early
            controller->behavior.aggression = 0.6f + (night * 0.12f);
            controller->behavior.intelligence = 0.5f;
            controller->behavior.speed_multiplier = 1.2f;
            controller->behavior.awareness_range = 1200.0f;
            controller->behavior.randomness = 0.5f;
            controller->behavior.can_use_vents = true;
            controller->behavior.can_cut_power = false;
            controller->behavior.can_move_fast = true;
            break;
            
        case ANIMATRONIC_SCRAPBIRD:
            controller->behavior.aggression = 0.5f + (night * 0.08f);
            controller->behavior.intelligence = 0.55f;
            controller->behavior.speed_multiplier = 1.1f;
            controller->behavior.awareness_range = 1400.0f;
            controller->behavior.randomness = 0.4f;
            controller->behavior.can_use_vents = true;
            controller->behavior.can_cut_power = false;
            controller->behavior.can_move_fast = true;
            break;
            
        case ANIMATRONIC_RAVAGER: // Mangle equivalent - unpredictable
            controller->behavior.aggression = 0.7f + (night * 0.1f);
            controller->behavior.intelligence = 0.4f;
            controller->behavior.speed_multiplier = 1.3f;
            controller->behavior.awareness_range = 800.0f;
            controller->behavior.randomness = 0.8f;
            controller->behavior.can_use_vents = true;
            controller->behavior.can_cut_power = true;
            controller->behavior.can_move_fast = true;
            break;
            
        case ANIMATRONIC_VOLT:
            controller->behavior.aggression = 0.3f + (night * 0.07f);
            controller->behavior.intelligence = 0.7f;
            controller->behavior.speed_multiplier = 1.0f;
            controller->behavior.awareness_range = 1600.0f;
            controller->behavior.randomness = 0.2f;
            controller->behavior.can_use_vents = false;
            controller->behavior.can_cut_power = true;
            controller->behavior.can_move_fast = false;
            break;
            
        case ANIMATRONIC_SOVEREIGN: // Golden Freddy - rare, unpredictable
            controller->behavior.aggression = 0.5f + (night * 0.15f);
            controller->behavior.intelligence = 0.9f;
            controller->behavior.speed_multiplier = 1.5f;
            controller->behavior.awareness_range = 2000.0f;
            controller->behavior.randomness = 0.9f;
            controller->behavior.can_use_vents = true;
            controller->behavior.can_cut_power = true;
            controller->behavior.can_move_fast = true;
            break;
            
        case ANIMATRONIC_WITHERED_MAGNUS: // Corrupted, glitchy
            controller->behavior.aggression = 0.8f + (night * 0.1f);
            controller->behavior.intelligence = 0.4f;
            controller->behavior.speed_multiplier = 0.8f;
            controller->behavior.awareness_range = 900.0f;
            controller->behavior.randomness = 0.95f;
            controller->behavior.can_use_vents = true;
            controller->behavior.can_cut_power = true;
            controller->behavior.can_move_fast = false;
            controller->is_glitched = true;
            break;
            
        case ANIMATRONIC_PUPPET: // Rare, stealth-based
            controller->behavior.aggression = 0.4f + (night * 0.1f);
            controller->behavior.intelligence = 0.95f;
            controller->behavior.speed_multiplier = 0.9f;
            controller->behavior.awareness_range = 1100.0f;
            controller->behavior.randomness = 0.1f;
            controller->behavior.can_use_vents = false;
            controller->behavior.can_cut_power = false;
            controller->behavior.can_move_fast = false;
            break;
            
        default:
            controller->behavior.aggression = 0.5f;
            controller->behavior.intelligence = 0.5f;
            controller->behavior.speed_multiplier = 1.0f;
            controller->behavior.awareness_range = 1200.0f;
            controller->behavior.randomness = 0.5f;
            controller->behavior.can_use_vents = false;
            controller->behavior.can_cut_power = false;
            controller->behavior.can_move_fast = false;
    }
    
    // Initialize waypoints (basic stage layout)
    controller->waypoint_count = 4;
    controller->waypoints = (Waypoint*)malloc(sizeof(Waypoint) * controller->waypoint_count);
    
    controller->waypoints[0] = (Waypoint){LOCATION_STAGE, 400.0f, 300.0f, 2.0f, false};
    controller->waypoints[1] = (Waypoint){LOCATION_PARTS_CORRIDOR, 600.0f, 300.0f, 1.5f, true};
    controller->waypoints[2] = (Waypoint){LOCATION_BACK_ROOM, 800.0f, 400.0f, 2.0f, false};
    controller->waypoints[3] = (Waypoint){LOCATION_OFFICE_DOOR, 1000.0f, 300.0f, 0.5f, true};
    
    controller->current_waypoint = 0;
    
    controller->can_disable_door = controller->behavior.can_cut_power;
    controller->can_disable_camera = false;
    controller->can_cause_blackout = controller->behavior.can_cut_power;
    
    return controller;
}

void ai_controller_destroy(AIController* controller) {
    if (!controller) return;
    
    if (controller->waypoints) {
        free(controller->waypoints);
    }
    
    free(controller);
}

// ============================================================================
// State Management
// ============================================================================

void ai_change_state(AIController* controller, AIState new_state) {
    if (!controller) return;
    
    controller->previous_state = controller->current_state;
    controller->current_state = new_state;
    controller->state_timer = 0.0f;
    
    // State-specific initialization
    switch (new_state) {
        case AI_STATE_HUNTING:
            controller->hunt_timer = 0.0f;
            break;
        case AI_STATE_ATTACKING:
            controller->is_attacking = true;
            controller->action_cooldown = 5.0f;
            break;
        case AI_STATE_OFFICE:
            controller->is_in_office = true;
            break;
        case AI_STATE_GLITCHED:
            controller->is_glitched = true;
            break;
        default:
            break;
    }
}

void ai_set_target_location(AIController* controller, LocationType location) {
    if (!controller) return;
    controller->target_location = location;
}

// ============================================================================
// Behavior Calculation
// ============================================================================

float ai_calculate_aggression(AIController* controller, int night) {
    if (!controller) return 0.5f;
    
    float base_aggression = controller->behavior.aggression;
    float difficulty_scale = 1.0f + (night * 0.15f);
    
    // Withered animatronics are more aggressive and unpredictable
    if (controller->is_glitched) {
        difficulty_scale *= 1.4f;
    }
    
    return fminf(base_aggression * difficulty_scale, 1.0f);
}

float ai_calculate_detection_chance(AIController* controller, AIManager* manager) {
    if (!controller || !manager) return 0.0f;
    
    float detection_chance = 0.0f;
    float base_chance = 0.1f;
    
    // Affected by: activity level, ambient noise, intelligence, awareness range
    detection_chance += manager->ambient_noise * 0.3f;
    detection_chance += controller->activity_level * 0.4f;
    detection_chance += controller->behavior.intelligence * 0.2f;
    
    // Withered animatronics have better/worse detection based on glitch level
    if (controller->is_glitched) {
        detection_chance = fmaxf(0.0f, detection_chance - 0.2f); // Glitchy = less predictable
    }
    
    return fminf(detection_chance, 1.0f);
}

bool ai_should_attack_player(AIController* controller, AIManager* manager) {
    if (!controller || !manager) return false;
    
    if (controller->is_in_office) return true;
    if (controller->current_location == LOCATION_OFFICE_DOOR) return true;
    
    float attack_threshold = 1.0f - controller->behavior.aggression;
    return controller->detection_confidence > attack_threshold;
}

bool ai_can_reach_player(AIController* controller, AIManager* manager) {
    if (!controller || !manager) return false;
    
    return controller->current_location >= LOCATION_OFFICE_DOOR;
}

// ============================================================================
// Special Abilities
// ============================================================================

void ai_trigger_attack(AIController* controller) {
    if (!controller) return;
    
    ai_change_state(controller, AI_STATE_ATTACKING);
    controller->times_attacked++;
}

void ai_trigger_blackout(AIManager* manager, AnimatronicType attacker) {
    if (!manager) return;
    
    manager->blackout_active = true;
    manager->blackout_timer = 5.0f + (rand() % 3);
}

void ai_trigger_glitch(AIController* controller) {
    if (!controller) return;
    
    controller->is_glitched = true;
    controller->behavior.randomness = fminf(controller->behavior.randomness + 0.3f, 1.0f);
    ai_change_state(controller, AI_STATE_GLITCHED);
}

// ============================================================================
// Main Update Loop
// ============================================================================

void ai_manager_update(AIManager* manager, float delta_time) {
    if (!manager) return;
    
    manager->game_time += delta_time;
    manager->difficulty_multiplier = 1.0f + (manager->night * 0.15f);
    
    // Update blackout timer
    if (manager->blackout_active) {
        manager->blackout_timer -= delta_time;
        if (manager->blackout_timer <= 0.0f) {
            manager->blackout_active = false;
        }
    }
    
    // Update all animatronics
    for (int i = 0; i < ANIMATRONIC_COUNT; i++) {
        if (manager->animatronics[i]) {
            ai_controller_update(manager->animatronics[i], manager, delta_time);
        }
    }
}

void ai_controller_update(AIController* controller, AIManager* manager, float delta_time) {
    if (!controller || !manager) return;
    
    // Update timers
    controller->state_timer += delta_time;
    if (controller->action_cooldown > 0.0f) {
        controller->action_cooldown -= delta_time;
    }
    if (controller->hunt_timer > 0.0f) {
        controller->hunt_timer -= delta_time;
    }
    
    // Calculate current activity level based on night
    float base_activity = ai_calculate_aggression(controller, manager->night);
    controller->activity_level = base_activity;
    
    // Update detection
    float detection_chance = ai_calculate_detection_chance(controller, manager);
    if ((rand() % 1000) / 1000.0f < detection_chance * delta_time) {
        controller->detection_confidence += 0.1f;
    } else {
        controller->detection_confidence -= 0.05f * delta_time;
    }
    controller->detection_confidence = fmaxf(0.0f, fminf(1.0f, controller->detection_confidence));
    
    // State machine
    switch (controller->current_state) {
        case AI_STATE_INACTIVE:
            // Check if should activate
            if (controller->activity_level > 0.3f && controller->state_timer > 5.0f) {
                ai_change_state(controller, AI_STATE_IDLE);
            }
            break;
            
        case AI_STATE_IDLE:
            // Move between waypoints
            if (controller->state_timer > controller->waypoints[controller->current_waypoint].pause_duration) {
                controller->current_waypoint = (controller->current_waypoint + 1) % controller->waypoint_count;
                controller->state_timer = 0.0f;
            }
            
            // Check for player detection
            if (controller->detection_confidence > 0.5f) {
                ai_change_state(controller, AI_STATE_ALERT);
            }
            break;
            
        case AI_STATE_ALERT:
            // Investigate potential player activity
            if (controller->state_timer > 3.0f) {
                if (controller->detection_confidence > 0.7f) {
                    ai_change_state(controller, AI_STATE_HUNTING);
                } else {
                    ai_change_state(controller, AI_STATE_IDLE);
                    controller->detection_confidence -= 0.2f;
                }
            }
            break;
            
        case AI_STATE_HUNTING:
            // Actively pursue player
            ai_set_target_location(controller, LOCATION_OFFICE_DOOR);
            controller->current_location = LOCATION_OFFICE_DOOR;
            
            if (ai_should_attack_player(controller, manager)) {
                ai_trigger_attack(controller);
            }
            
            // Can trigger blackouts
            if (controller->can_cause_blackout && (rand() % 100) < 10) {
                ai_trigger_blackout(manager, controller->type);
            }
            break;
            
        case AI_STATE_ATTACKING:
            // At door/window, attacking
            if (controller->action_cooldown <= 0.0f) {
                ai_change_state(controller, AI_STATE_HUNTING);
            }
            break;
            
        case AI_STATE_OFFICE:
            // Inside office - game over scenario
            break;
            
        case AI_STATE_GLITCHED:
            // Withered animatronics - erratic behavior
            if (controller->state_timer > 1.0f + (rand() % 100) / 100.0f) {
                // Random behavior changes
                int random_behavior = rand() % 3;
                switch (random_behavior) {
                    case 0:
                        ai_change_state(controller, AI_STATE_IDLE);
                        break;
                    case 1:
                        ai_change_state(controller, AI_STATE_HUNTING);
                        break;
                    case 2:
                        ai_change_state(controller, AI_STATE_ALERT);
                        break;
                }
            }
            break;
            
        default:
            break;
    }
}

// ============================================================================
// Debug
// ============================================================================

void ai_debug_print_state(AIController* controller) {
    if (!controller) return;
    
    const char* animatronic_names[] = {
        "Magnus", "Striker", "ScrapBird", "Ravager", "Volt",
        "Sovereign", "Withered Magnus", "Puppet"
    };
    
    const char* state_names[] = {
        "INACTIVE", "IDLE", "ALERT", "HUNTING", "ATTACKING",
        "OFFICE", "DESPAWNED", "GLITCHED"
    };
    
    printf("[AI Debug] %s - State: %s | Activity: %.2f | Detection: %.2f | Location: %d\n",
           animatronic_names[controller->type],
           state_names[controller->current_state],
           controller->activity_level,
           controller->detection_confidence,
           controller->current_location);
}
