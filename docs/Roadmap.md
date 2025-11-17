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

- [ ] **SRAM Map**: define `SaveSlot` with header, version, checksum, language, difficulty, lives, progress flags, seed, etc.
  - [ ] Header: magic `"GBCP"`, version `u8`, slot_id `u8`
  - [ ] Checksum CRC16 or 16-bit sum
  - [ ] Padding for 16/32 B alignment
- [ ] **3 Slots**: contiguous layout in SRAM (e.g., 0xA000–...) with 256–1024 B/slot
- [ ] **Pre-Game Slot Menu**:
  - [ ] 3 slots with state: `EMPTY` / timestamp / summary (lives, diff, %)
  - [ ] Actions: `CONTINUE`, `NEW GAME`, `OVERWRITE` (confirm), `ERASE` (double confirm)
- [ ] **Title Flow**: START → slot menu; select/continue/new
- [ ] **Save API**:
  - [ ] `save_init()`, `save_slot_read(i, out)`, `save_slot_write(i, in)`, `save_slot_erase(i)`
  - [ ] `save_autosave(i)` at checkpoints/events
- [ ] **Compatibility & Safety**:
  - [ ] Format versioning; safe invalidation
  - [ ] Checksum validation; `CORRUPTED` state
- [ ] **Multilingual UI/UX**: localized labels for slot menu
- [ ] **Performance**: SRAM writes outside vblank; input blocked during I/O

## 8. Gameplay Test / Mini RPG

- [ ] Gameplay screen with controllable character (tile-based movement)
- [ ] Interaction with map elements (NPC, objects)
- [ ] At least two events:
  - [ ] leads to `game_over_screen`
  - [ ] leads to completion (credits)
- [ ] Update of global state (e.g., lives, score, language)
- [ ] **Save integration**: optional checkpoint/autosave, restore state from slot

## 8. Game Completion / Credits

- [x] Scrolling or static credits screen
- [x] Ability to return to title or full reset
- [ ] Global language respected also in credits

## 9. Audio (BGM + SFX)

### 9.1 Audio Architecture

- [ ] **BGM Driver**: choose and integrate (e.g., hUGEDriver/GBT Player) or lightweight custom driver
- [ ] **Tick timing**: update in VBlank or timer; target 50/60 Hz
- [ ] **Panning/stereo** for CGB and DMG fallback; master volume consistent with `settings.sound_on`
- [ ] **ROM banking management** for music patterns/instruments (banked audio assets)

### 9.2 Background Music (BGM)

- [ ] API: `bgm_init()`, `bgm_play(track_id, loop)`, `bgm_stop()`, `bgm_pause/resume()`
- [ ] **Transitions**: non-blocking `bgm_fade_in/out(ms)`
- [ ] **Channel allocation**: BGM primarily uses Pulse1+Wave; leaves Pulse2/Noise free for SFX
- [ ] **Asset pipeline**: conversion from tracker (DefleMask/Famitracker-like) → driver format
- [ ] Minimum tracks:
  - [ ] Short Splash/Intro
  - [ ] Title loop
  - [ ] Options ambience
  - [ ] Gameplay loop
  - [ ] Game Over sting
- [ ] **Budget**: ≤ ~6% CPU for driver update; ROM for tracks 8–24 KB banked

### 9.3 Sound Effects (SFX)

- [ ] API: `sfx_play(id, prio)` with small queue and priority
- [ ] **Channel policy**:
  - [ ] Noise: hits/steps
  - [ ] Pulse2: UI click/confirm
  - [ ] Wave (occasional): short special effects; requires saving/restoring Wave RAM if BGM uses it
- [ ] **Interaction with BGM**:
  - [ ] Light ducking of BGM during high-priority SFX
  - [ ] Temporary lock of stolen channel (e.g., Pulse2) with ADSR restore
- [ ] **Preset instruments** (ADSR, duty, sweep) for common SFX
- [ ] **SFX table**: UI click, step, pickup, error/buzz, confirm, alert

### 9.4 State/Configuration

- [ ] `settings.sound_on` turns APU on/off: fast muting (NR52) + channel state restore
- [ ] **DEBUG mode**: overlay with channel levels, simplified VU, FPS vs audio tick
- [ ] **Failsafe**: if driver not initialized → API no-op

### 9.5 Flow Integration

- [ ] Splash: short jingle
- [ ] Title: BGM loop; pause when entering options, resume on exit (or soft track change)
- [ ] Options menu: SFX for move/confirm/back
- [ ] Gameplay: main BGM; cue SFX on events; sting on Game Over

### 9.6 Performance & Safety

- [ ] Audio update **outside** critical DMA/OAM sections
- [ ] Avoid APU writes during STAT mode 3 (if using LCD ISR)
- [ ] No heap usage; `const` tables in ROM; audio state ≤ 64 B in WRAM

## 10. Multilanguage

- [ ] All texts: splash, title, options, dialogue, game over, credits
- [x] Real-time language switch via options menu
- [x] Prepared for more than two languages (scalable)
- [ ] **New slot menu entries** (see Section 6) integrated in `lang.h/.c`

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
