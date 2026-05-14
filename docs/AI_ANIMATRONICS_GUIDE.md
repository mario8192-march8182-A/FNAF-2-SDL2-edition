# Animatronic AI System Guide

## Overview

The AI system controls 8 unique animatronics, each with distinct personalities, behaviors, and hunting strategies. The system uses a **state machine** architecture for realistic and unpredictable behavior.

---

## 🤖 Animatronics & Behaviors

### 1. **MAGNUS** (Balanced Threat)
- **Role**: Main antagonist (Toy Freddy replacement)
- **Aggression**: 50% → scales to ~65% by Night 5
- **Intelligence**: 60% (strategic)
- **Speed**: Normal (1.0x)
- **Special Abilities**: 
  - Can cause blackouts
  - Moderate threat level
- **Hunting Pattern**: Deliberate, methodical approach
- **Voice**: Deep, mechanical growl

### 2. **STRIKER** (Fast & Aggressive)
- **Role**: Quick attacker (Toy Bonnie replacement)
- **Aggression**: 65%
- **Intelligence**: 50% (reactive)
- **Speed**: Very Fast (1.4x)
- **Special Abilities**:
  - Can use ventilation system
  - Sprint capability
- **Hunting Pattern**: Rapid room-to-room movement
- **Voice**: Sharp metallic screeches

### 3. **SCRAPBIRD** (Clever & Agile)
- **Role**: Unpredictable (Toy Chica replacement)
- **Aggression**: 55%
- **Intelligence**: 55%
- **Speed**: Fast (1.2x)
- **Special Abilities**:
  - Can use vents
  - Erratic movement patterns
- **Hunting Pattern**: Zigzag paths, sudden direction changes
- **Voice**: Chirping electronic sounds

### 4. **RAVAGER** (Most Dangerous - Normal)
- **Role**: Apex predator (Mangle replacement)
- **Aggression**: 80% → ~95% by Night 5 ⚠️
- **Intelligence**: 40% (pure instinct)
- **Speed**: Very Fast (1.5x)
- **Special Abilities**:
  - Can use vents
  - Extreme speed
  - High stamina
- **Hunting Pattern**: Relentless charging attacks
- **Voice**: Brutal metallic screeching & snarling
- **Warning**: Encounters with this animatronic are EXTREMELY DANGEROUS

### 5. **VOLT** (Intelligent Strategist)
- **Role**: Tactician (Custom - unique)
- **Aggression**: 40% (patient)
- **Intelligence**: 70% (HIGHEST non-special)
- **Speed**: Normal (1.1x)
- **Special Abilities**:
  - Can cause blackouts
  - Predicts player movements
- **Hunting Pattern**: Strategic position-taking, waits for opportunities
- **Voice**: Subtle electrical hums & calculating beeps

### 6. **SOVEREIGN** (Rare & Mysterious)
- **Role**: Golden anomaly (Golden Freddy replacement)
- **Aggression**: 60%
- **Intelligence**: 90% (Very high)
- **Speed**: Fast (1.3x)
- **Special Abilities**:
  - Can use vents
  - Can cause blackouts
  - Extremely unpredictable (+80% randomness)
  - Massive detection range (400 units)
- **Hunting Pattern**: Appears and disappears unpredictably
- **Voice**: Distorted, haunting tones
- **Warning**: Spawns randomly, encounters are terrifying

### 7. **WITHERED MAGNUS** 🔥 (CORRUPTED CHAOS)
- **Role**: Apex threat (Withered Animatronic)
- **Aggression**: 95% → 100%+ by Night 5 💀
- **Intelligence**: 35% (BROKEN, UNSTABLE)
- **Speed**: Extremely Fast (1.6x) - FASTEST
- **Special Abilities**:
  - Can use vents
  - Can cause blackouts
  - Glitches unpredictably
  - Has 70% attack chance when hunting
- **Hunting Pattern**: CHAOTIC - Random state changes, teleporting effects
- **Voice**: Corrupted screaming, distorted metal crunching
- **Warning**: ⚠️ EXTREME THREAT - Appears in later nights only
- **Behavior**: 
  - Enters GLITCHED state randomly
  - Attacks with 70% probability
  - Triggers glitches every 1-2 seconds while hunting
  - Extremely difficult to predict or defend against

### 8. **PUPPET** (Master Tactician)
- **Role**: Shadow hunter (Marionette)
- **Aggression**: 50%
- **Intelligence**: 95% (HIGHEST - EXTREMELY SMART) 🧠
- **Speed**: Fast (1.25x)
- **Special Abilities**:
  - Can use vents
  - Can cause blackouts
  - Massive detection range (450 units)
  - Silent/invisible movement
  - Has 60% attack chance (but always strategic)
- **Hunting Pattern**: Perfectly calculated movements, always one step ahead
- **Voice**: Eerie wind-like sounds, minimal vocalizations
- **Warning**: ⚠️ MOST INTELLIGENT - Prediction is nearly impossible
- **Behavior**:
  - Silently tracks player movements
  - Rarely makes mistakes
  - Can predict defensive patterns
  - Extremely dangerous despite lower aggression

---

## 🧠 AI State Machine

```
        ┌─────────────────────────────────────┐
        │       INACTIVE (Charging)           │
        └──────────────┬──────────────────────┘
                       │ (ambient noise detected)
                       ▼
        ┌─────────────────────────────────────┐
        │         IDLE (Wandering)            │
        └──────────────┬──────────────────────┘
                       │ (player activity)
                       ▼
        ┌─────────────────────────────────────┐
        │      ALERT (Investigating)          │
        └──────────────┬──────────────────────┘
                       │ (confirmed activity)
                       ▼
        ┌─────────────────────────────────────┐
        │      HUNTING (Pursuing Player)      │
        └─────────┬──────────────┬────────────┘
                  │              │
         (reached │              │ (timeout)
          player) │              ▼
                  ▼       ┌─────────────────────┐
        ┌─────────────────┤   DESPAWNED        │
        │                 │ (Waiting for reset)│
        │  ATTACKING      └─────────────────────┘
        │ (At door/window)        │
        │                         │ (reset timer)
        └────────────────────────┬─┘
                                 │
                    (loop back to IDLE/INACTIVE)
```

**Special Case: WITHERED MAGNUS**
- In GLITCHED state: Random state changes, becomes unpredictable
- May teleport between states
- Attacks appear "stuttering" or out of sync

---

## 📊 Difficulty Scaling

Each night increases animatronic threat:

| Night | Aggression Multiplier | Examples |
|-------|----------------------|----------|
| 1 | 1.0x | Magnus: 50%, Ravager: 80% |
| 2 | 1.15x | Magnus: 57.5%, Ravager: 92% |
| 3 | 1.30x | Magnus: 65%, Ravager: 104% (capped) |
| 4 | 1.45x | Magnus: 72.5%, Ravager: 116% (capped) |
| 5 | 1.60x | Magnus: 80%, Ravager: 128% (capped) |
| 6 | 1.75x | Magnus: 87.5%, Withered Magnus: 166% (capped) |

---

## 🎮 Example Usage in Game Loop

```c
// Initialize
AIManager* ai_manager = ai_manager_create();

// Main game loop
while (game_running) {
    float delta_time = 0.016f; // ~60 FPS
    
    // Update AI
    ai_manager_update(ai_manager, delta_time);
    
    // Check for attacks
    for (int i = 0; i < ANIMATRONIC_COUNT; i++) {
        AIController* anim = ai_manager->animatronics[i];
        
        if (ai_should_attack_player(anim, ai_manager)) {
            // Trigger defense animation
            game_player_defend(i);
        }
        
        if (ai_can_reach_player(anim, ai_manager)) {
            // Game Over
            game_end_failure();
        }
    }
    
    // Debug (optional)
    if (DEBUG_MODE) {
        ai_debug_print_state(ai_manager->animatronics[ANIMATRONIC_WITHERED_MAGNUS]);
    }
}

// Cleanup
ai_manager_destroy(ai_manager);
```

---

## 🔊 Audio Integration

Each animatronic should have distinct audio cues:

- **Idle sounds**: Quiet mechanical noises, humming
- **Alert sounds**: Attention-grabbing growls/screeches
- **Hunting sounds**: Aggressive vocalizations, warning roars
- **Attack sounds**: Violent crashes, door impacts
- **Glitch sounds** (Withered Magnus): Distorted, stuttering audio

---

## 🎯 Detection & Awareness

**Factors affecting detection:**

1. **Intelligence**: Base awareness capability
2. **Ambient Noise**: Player movement sounds, door usage
3. **Power Usage**: Cameras/doors consume power, alerting animatronics
4. **Difficulty**: Higher nights = better senses

**Detection Formula:**
```
detection_chance = (intelligence × 0.3) + 
                  (ambient_noise × 0.3) + 
                  (power_usage × 0.2) × 
                  difficulty_multiplier
```

**Special Cases:**
- **Withered Magnus**: +30% base detection (chaotic awareness)
- **Puppet**: +40% base detection (omniscient hunter)

---

## 💡 Design Notes

### Animatronic Roles in Gameplay:

1. **Magnus**: Tutorial-level threat, teaches player basics
2. **Striker**: Mobile threat, requires active defense
3. **ScrapBird**: Psychological threat (erratic patterns)
4. **Ravager**: Skill check, extreme danger
5. **Volt**: Strategic threat (predicts player)
6. **Sovereign**: Horror element (rare, mysterious)
7. **Withered Magnus**: Endgame difficulty spike
8. **Puppet**: Masterclass in AI (nearly unbeatable)

### Progression:
- **Night 1-2**: Magnus, Striker, ScrapBird introduce mechanics
- **Night 3-4**: Ravager and Volt enter; difficulty ramps
- **Night 5**: Withered Magnus appears (chaos element)
- **Night 6+**: Puppet becomes threat; extreme difficulty

---

## 🐛 Debugging Commands

```c
// Print state of any animatronic
ai_debug_print_state(ai_manager->animatronics[ANIMATRONIC_WITHERED_MAGNUS]);

// Trigger events for testing
ai_trigger_blackout(ai_manager, ANIMATRONIC_VOLT);
ai_trigger_glitch(ai_manager->animatronics[ANIMATRONIC_WITHERED_MAGNUS]);

// Monitor overall system
printf("Game Time: %.2f | Night: %d | Blackout: %d\n",
       ai_manager->game_time, ai_manager->night, ai_manager->blackout_active);
```

---

## 📝 Future Enhancements

- [ ] Advanced pathfinding (A*)
- [ ] Dynamic learning (animatronics remember player strategies)
- [ ] Coordinated attacks (multiple animatronics working together)
- [ ] Environmental interactions (breaking cameras, cutting vents)
- [ ] Personality quirks (unique sound effects, animation styles)
- [ ] Nightmare modes (alternative behaviors)

---

**This system provides a challenging, diverse, and engaging antagonist roster for your FNaF 2 SDL2 game!** 🎮🤖
