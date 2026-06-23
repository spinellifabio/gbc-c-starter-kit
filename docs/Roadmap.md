# GBC Prototype Roadmap

This roadmap defines the steps to complete the text-based prototype on GameBoy Color, ready to evolve into a mini game framework.

---

## 1. Core Template and Menu

- [x] Multiple unskippable splash screens
- [x] Title screen with START and SELECT → options
- [x] Dynamic options menu with cursor, LEFT/RIGHT to change values
- [x] Consolidated global settings: sound, difficulty, lives, mode, language, game_name, version
- [x] Input flush check to prevent stuck keys

## 2. Game Over (Completed)

- [x] Game over screen with delay before it becomes skippable (~3–4s)
- [x] Ability to set different messages/game over variants for future scenarios

## 3. Game State Management (Completed)

- [x] Centralized game state structure (lives, score, level)
- [x] State initialization and reset functionality
- [x] Integration with game over and menu systems

## 4. Sprites and Visual Content

- [ ] Sprite management (OAM, tile reuse)
- [ ] Background layer management separate from UI
- [ ] Dynamic palette and DMG/CGB compatibility
- [ ] Visual effects: blink, fade, scroll, basic animations

## 5. Intro Video / Cutscene

- [x] Animated intro screen (sprite/background)
- [x] Text dialogue synced with frames → speed up with `A` button
- [x] Ability to skip the entire video with one button
- [x] Final link to the title screen

## 6. Advanced Title Screen

- [ ] Display game_name + version + optional logo
- [ ] START → **Slot Selection / Continue** (see Section 6)
- [ ] SELECT → options menu
- [ ] Multilanguage options working also in UI messages

## 7. Save System (3 Slots) and Game Resume

- [x] **SRAM Map**: define `SaveSlot` with header, version, checksum, language, difficulty, lives, progress flags, seed, etc. (`include/save.h`)
  - [x] Header: magic `"GBCP"`, version `u8`, slot_id `u8`
  - [x] Checksum CRC16 or 16-bit sum (16-bit sum over first 30 bytes)
  - [x] Padding for 16/32 B alignment (32 B/slot, `reserved[13]`)
- [x] **3 Slots**: contiguous layout in SRAM (0xA000, 32 B/slot)
- [x] **Pre-Game Slot Menu** (`src/slot_menu.c`):
  - [x] 3 slots with state: `EMPTY` / summary (lives, diff, score); `play_count` as timestamp stand-in
  - [x] Actions: `CONTINUE`, `NEW GAME`, `OVERWRITE` (confirm), `ERASE` (double confirm)
- [x] **Title Flow**: START → slot menu; select/continue/new (`src/main.c`)
- [x] **Save API** (`src/save.c`):
  - [x] `save_init()`, `save_slot_read(i, out)`, `save_slot_write(i, in)`, `save_slot_erase(i)`
  - [x] `save_autosave(i)` at checkpoints/events
- [x] **Compatibility & Safety**:
  - [x] Format versioning; safe invalidation
  - [x] Checksum validation; `CORRUPTED` state
- [x] **Multilingual UI/UX**: localized labels for slot menu (`STR_SLOT_*`, EN/IT)
- [x] **Performance**: SRAM writes bracketed `ENABLE_RAM`/`DISABLE_RAM` + `__critical`; input flushed during I/O

## 8. Gameplay Test / Mini RPG

- [x] Gameplay screen with controllable character (tile-based movement)
- [x] Interaction with map elements (NPC, objects)
- [x] At least two events:
  - [x] leads to `game_over_screen` (hazard depletes lives)
  - [x] leads to completion (credits) (treasure secured → win)
- [x] Update of global state (e.g., lives, score, language)
- [x] **Save integration**: autosave on win; restore state from slot on CONTINUE (`src/main.c`)

## 8. Game Completion / Credits

- [x] Scrolling or static credits screen
- [x] Ability to return to title or full reset
- [ ] Global language respected also in credits

## 9. Audio (BGM + SFX)

### 9.1 Audio Architecture

- [x] **BGM Driver**: lightweight custom VBL note sequencer (`bgm.c`) using Pulse1; no external dependency
- [x] **Tick timing**: `bgm_vbl_update()` registered via `add_VBL()` — fires every VBL (~60 Hz)
- [ ] **Panning/stereo** for CGB and DMG fallback; master volume consistent with `settings.sound_on`
- [ ] **ROM banking management** for music patterns/instruments (banked audio assets)

### 9.2 Background Music (BGM)

- [x] API: `bgm_init()`, `bgm_play(song)`, `bgm_stop()`, `bgm_pause()`, `bgm_resume()`
- [ ] **Transitions**: non-blocking `bgm_fade_in/out(ms)`
- [x] **Channel allocation**: BGM uses Pulse1 (NR10-NR14); leaves Pulse2/Noise free for SFX
- [ ] **Asset pipeline**: richer song format (tracker-style, banked)
- [ ] Minimum tracks:
  - [ ] Short Splash/Intro
  - [x] Title loop (`demo_song` — C-major arpeggio, plays on title screen)
  - [ ] Options ambience
  - [ ] Gameplay loop
  - [ ] Game Over sting
- [x] **Budget**: VBL update < 1% CPU; song data in ROM as `const BgmNote[]`

### 9.3 Sound Effects (SFX)

- [x] API: `sfx_play(id)` — Pulse2-only, no priority queue (Phase 1)
- [ ] Priority queue: `sfx_play(id, prio)` with preemption
- [ ] **Channel policy**:
  - [ ] Noise: hits/steps
  - [x] Pulse2: UI click/confirm, gameplay SFX
  - [ ] Wave (occasional): short special effects
- [ ] **Interaction with BGM**:
  - [ ] Light ducking of BGM during high-priority SFX
- [x] **Preset instruments**: UI (move, select, back), gameplay (step, pickup, error)
- [ ] **SFX table**: alert, more gameplay variants

### 9.4 State/Configuration

- [x] `settings.sound_on` turns APU on/off: fast muting (NR52) + channel state restore
- [ ] **DEBUG mode**: overlay with channel levels, simplified VU, FPS vs audio tick
- [x] **Failsafe**: if driver not initialized → API no-op

### 9.5 Flow Integration

- [ ] Splash: short jingle
- [x] Title: BGM loop (`demo_song`); pauses when entering options, resumes on exit
- [x] Options menu: SFX for move/confirm/back
- [x] Gameplay: SFX on step (throttled), pickup, life lost/error

### 9.6 Performance & Safety

- [ ] Audio update **outside** critical DMA/OAM sections
- [ ] Avoid APU writes during STAT mode 3 (if using LCD ISR)
- [x] No heap usage; `const` tables in ROM; audio state ≤ 64 B in WRAM

## 10. Multilanguage

- [ ] All texts: splash, title, options, dialogue, game over, credits
- [x] Real-time language switch via options menu
- [x] Prepared for more than two languages (scalable)
- [x] **New slot menu entries** (see Section 6) integrated in `lang.h/.c` (`STR_SLOT_*`)

## 11. Refactoring and Optimization

- [ ] Consolidate common functions (flush_input, draw_text_center, etc.)
- [ ] Separate modules: menu, gameplay, splash, audio, settings, **save**
- [ ] Cycle control to maintain 60 FPS
- [ ] RAM/ROM profiling, footprint reduction where possible

## 12. Testing & Debug

- [ ] Test on emulators (BGB, SameBoy) and real hardware
- [ ] Edge cases: simultaneous input, frame_counter overflow, incompatible palettes
- [ ] Verify complete multilanguage behavior
- [ ] **Save tests**:
  - [ ] Create new slot, overwrite, delete
  - [ ] Checksum integrity and `CORRUPTED` handling
  - [ ] Persistence across reboots/emulator save files
  - [ ] Version migration (if `SaveSlot` layout changes)
- [ ] **Audio tests**:
  - [ ] Verify BGM update timing (no stutter) at 60 FPS
  - [ ] SFX priority over BGM (channel rubber-banding) and ducking
  - [ ] Panning/stereo on CGB and DMG fallback
  - [ ] Input→SFX latency < 1 frame in menus
  - [ ] Instant mute/unmute and channel state restore

---

## Extra / Optional

These objectives are **additional**, useful as extra value for future games.

- [ ] **Achievements / RetroAchievements**
  - [ ] Integrate achievements via [RetroAchievements](https://retroachievements.org/)
  - [ ] Practical examples:
    - Quick skip of intro
    - Reached Game Over
    - Completion / The End
    - Speedrun / quick completion
  - [ ] Module to flag achievement and send signals to RetroAchievements API

- [ ] **Results Export**
  - [ ] Encode results in modulated audio
  - [ ] External platform able to read score, progress, achievements
  - [ ] Minimum: success/failure, score, player name

- [ ] **Extra Visual / Gameplay**
  - [ ] Advanced mini visual effects (shake, fade, parallax scroll)
  - [ ] More complex sprite animations (NPC walking cycles, battles)
  - [ ] Additional languages for narrative texts or dialogues
  - [ ] Persistent score tracking via temporary save on RAM/EEPROM

## 13. Art Assets — Sprites & Backgrounds

Tracks which visual assets still need custom artwork to replace placeholders.

### Sprite Assets

| Asset file | Used by | Status | Notes |
|------------|---------|--------|-------|
| `res/sprites/player_idle.h` | Player standing (4 dirs) | Placeholder | Replace with final character art |
| `res/sprites/player_run.h` | Player walking (4 dirs × 6 frames) | Placeholder | Replace with final character art |
| `res/sprites/cat_idle.h` | Cat NPC (idle + sprint AI) | Placeholder (copy of player_idle) | Replace with 16×16 cat sprite (2+ frames) |
| `res/sprites/cutscene_player.h` | Player in cutscenes | Placeholder (copy of player_idle) | Replace with cutscene-specific player art |
| `res/sprites/cutscene_npc.h` | NPC in cutscenes | Placeholder (copy of player_idle) | Replace with cutscene-specific NPC art |

> All sprites: 16×16 px, 8×16 OAM mode, up to 4 CGB palettes. Use `png2asset` to regenerate `.h`/`.c` from source PNG.

**VRAM budget (current):**
```
  0- 19 : player_idle       (20 tiles)
 20- 99 : player_run        (80 tiles)
100-103 : obj_treasure       (4 tiles)
104-107 : obj_hazard         (4 tiles)
108-127 : cat_idle          (20 tiles, placeholder)
128-147 : cutscene_player   (20 tiles, placeholder)
148-167 : cutscene_npc      (20 tiles, placeholder)
```
168 of 256 VRAM sprite tiles used. 88 tiles free for future sprites.

### Background / Tileset Assets

| Asset | Used by | Status | Notes |
|-------|---------|--------|-------|
| `res/tiles/tileset.*` | Gameplay world map (water/sand/grass) | Placeholder | Replace with final tileset |
| *(missing)* | Splash screen | Placeholder (solid black) | Replace with dedicated art when ready |
| *(missing)* | Title screen logo/background | Placeholder (solid black) | Logo/tilemap art needed; high priority |
| *(missing)* | Options screen background | Placeholder (solid black) | Optional decorative art |
| *(missing)* | Game Over screen | Placeholder (solid black) | Optional decorative art |
| *(missing)* | Win / Credits screen | Placeholder (solid black) | Optional decorative art |

### Asset Priority Order

1. **`cat_idle`** — unlock real cat NPC visuals
2. **`cutscene_player` / `cutscene_npc`** — differentiate cutscene characters from gameplay sprites
3. **Title screen logo** — first thing player sees; high visual impact
4. **`player_idle` / `player_run`** — character defines visual identity
5. **`tileset`** — world look defines game feel
6. Splash, Game Over, Credits — polish pass
