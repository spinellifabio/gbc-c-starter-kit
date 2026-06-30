# Feature Specification: Visual Content Layer

**Feature Branch**: `001-visual-content-layer`

**Created**: 2026-06-29

**Status**: Draft

**Input**: User description: "Visual content layer for the GBC starter kit: a sprite manager with OAM slot allocation and VRAM tile reuse, a background layer kept separate from the UI/text layer, dynamic CGB palette assignment with graceful DMG fallback, and reusable visual effects (blink, fade, scroll, basic frame animation). Must respect the existing VRAM tile budget (168/256 used, 88 free), reuse existing sprite/OAM slots before allocating, and follow the examples/ samples (galaxy=scroll, hblank_copy=VRAM timing, colorbar/dscan=palettes). Effects API drives no net WRAM growth beyond a small fixed effect-state struct."

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Managed sprite placement without manual slot bookkeeping (Priority: P1)

A kit user adding a new on-screen actor (player, enemy, item) asks the visual
layer for a sprite and gets back a handle, without manually tracking which OAM
hardware slots or VRAM tiles are free. When the same graphic is requested again,
the layer reuses the already-loaded tiles instead of consuming more of the tile
budget. When an actor is removed, its OAM slot and any tiles it exclusively used
become available for reuse.

**Why this priority**: Sprite management is the foundation the other visual
features build on (effects animate sprites, the background coexists with them).
Without disciplined slot/tile allocation the kit silently exceeds the 40-OAM and
256-tile hardware limits, which is the single most common GBC failure mode this
feature exists to prevent. It delivers standalone value: a developer can place,
reuse, and free sprites correctly even if no other story ships.

**Independent Test**: Allocate several sprites referencing both shared and
unique graphics, confirm in BGB's VRAM/OAM viewer that duplicate graphics share
tiles, that allocation refuses to exceed the free-tile and free-OAM budget, and
that freeing a sprite returns its resources to the pool.

**Acceptance Scenarios**:

1. **Given** an actor whose graphic is not yet in VRAM, **When** the developer
   requests a sprite for it, **Then** the layer loads the tiles into free VRAM
   (within the 88-tile budget), assigns a free OAM slot, and returns a usable
   handle.
2. **Given** a graphic already loaded by an earlier sprite, **When** a second
   sprite requests the same graphic, **Then** no additional VRAM tiles are
   consumed and both sprites display correctly.
3. **Given** the free OAM slots or free VRAM tiles are exhausted, **When** a new
   allocation is requested, **Then** the layer reports the failure to the caller
   rather than overwriting in-use slots or tiles.
4. **Given** an allocated sprite, **When** it is freed, **Then** its OAM slot is
   released and tiles it alone referenced become available again.

---

### User Story 2 - Background layer independent of the UI/text layer (Priority: P2)

A kit user composes a scene with a scrolling/decorative background while the
existing UI and dialogue/text rendering continues to work on top, with neither
layer corrupting the other's tiles or palettes.

**Why this priority**: The kit already renders UI/text via shared helpers
(`print_centered`, dialogue typewriter, `FONT_OFFSET`). A background layer is
high value but only safe once sprite/tile budgeting (P1) exists; keeping it
separate from text is what makes the visual layer composable rather than a
source of clobbering bugs.

**Independent Test**: Display a background scene and a dialogue box
simultaneously; confirm the text remains legible and correctly positioned, the
background renders without overwriting font tiles, and dismissing the dialogue
leaves the background intact.

**Acceptance Scenarios**:

1. **Given** the UI/text layer is active, **When** a background scene is shown,
   **Then** background tiles occupy a tile range distinct from the font/UI tiles
   and neither overwrites the other.
2. **Given** a visible background, **When** the dialogue box slides in and runs
   the typewriter effect, **Then** text renders correctly on top and the
   background is unaffected.
3. **Given** a background and UI on screen, **When** the background is cleared,
   **Then** the UI/text layer remains intact.

---

### User Story 3 - Dynamic CGB palettes with graceful DMG fallback (Priority: P2)

A kit user assigns colors to backgrounds and sprites at runtime on CGB hardware,
and the same content remains correct (in shades of gray) when run on an original
DMG Game Boy.

**Why this priority**: Color is a defining CGB feature and several effects (fade)
depend on palette control, but it is meaningless without content (sprites P1,
background P2) to color. The DMG fallback is a hard project requirement
(`_cpu == CGB_TYPE` guards) so it is specified alongside.

**Independent Test**: On CGB, assign palettes to sprite and background content
and confirm correct colors in BGB; switch the emulator to DMG mode and confirm
the same content renders in a sensible grayscale without crashing or garbled
tiles.

**Acceptance Scenarios**:

1. **Given** CGB hardware, **When** content is assigned a palette at runtime,
   **Then** the requested colors display and palette indices stay within the
   valid 0–7 range for both background and sprite palettes.
2. **Given** DMG hardware, **When** the same content is shown, **Then** palette
   assignment is skipped gracefully and content renders in grayscale without
   error.
3. **Given** more distinct color sets are requested than there are hardware
   palettes, **When** assignment occurs, **Then** the layer reports the limit
   rather than silently corrupting an in-use palette.

---

### User Story 4 - Reusable visual effects with bounded memory (Priority: P3)

A kit user applies common visual effects — blink, fade, scroll, and basic
frame-by-frame animation — to existing content through a shared effects API,
without each effect inventing its own state storage or growing RAM use.

**Why this priority**: Effects are the polish layer and depend on all prior
stories (something to blink/fade/scroll/animate). They are valuable but optional
for a minimum viable visual layer, hence lowest priority. The fixed effect-state
budget is an explicit constraint.

**Independent Test**: Trigger each effect on sprite and/or background content,
confirm in BGB the visible behavior (blink toggles visibility, fade ramps
brightness, scroll moves the background, animation cycles frames) and confirm
via the build map that total WRAM growth is limited to the single fixed
effect-state structure.

**Acceptance Scenarios**:

1. **Given** visible content, **When** the blink effect is applied, **Then** the
   content toggles between shown and hidden at a steady cadence and stops
   cleanly when the effect ends.
2. **Given** a scene, **When** the fade effect runs, **Then** brightness ramps
   smoothly to/from black on CGB and degrades gracefully on DMG.
3. **Given** a background, **When** the scroll effect runs, **Then** the
   background moves smoothly without tearing (VRAM/scroll updates respect LCD
   timing).
4. **Given** a multi-frame graphic, **When** the animation effect runs, **Then**
   frames cycle at the configured rate and can loop or play once.
5. **Given** several effects active at once, **When** they run, **Then** total
   WRAM use stays within the single fixed effect-state structure with no
   per-effect heap or growing allocation.

---

### Edge Cases

- What happens when a sprite is freed while an effect (blink/animation) is still
  targeting it? The effect must stop or be detached without writing to a freed
  slot.
- How does the layer handle a request that would push VRAM past the 88 free
  tiles or OAM past index 39? It must refuse and report, never overwrite in-use
  resources.
- What happens to background scroll and palette effects on DMG, where color and
  some timing assumptions differ? Effects must degrade to a sensible no-op or
  grayscale equivalent rather than corrupt the screen.
- How are tiles handled when two sprites share a graphic and only one is freed?
  Shared tiles must remain until the last referencing sprite is freed.
- What happens when an effect is asked to run with a zero or invalid duration/
  rate? It must clamp or report rather than divide-by-zero or loop forever.
- How does VRAM updating behave during active LCD rendering (Mode 3)? Writes must
  be deferred to V-Blank or H-Blank to avoid visual glitches.

## Requirements *(mandatory)*

### Functional Requirements

#### Sprite Manager

- **FR-001**: The visual layer MUST allocate sprites on request and return a
  handle the caller uses for later updates and freeing, without the caller
  managing raw OAM indices.
- **FR-002**: The sprite manager MUST reuse already-loaded VRAM tiles when a
  requested graphic is already resident, consuming no additional tiles.
- **FR-003**: The sprite manager MUST reuse existing/free OAM slots before
  treating allocation as growth, and MUST keep all OAM indices below 40.
- **FR-004**: The sprite manager MUST keep total sprite + background tile usage
  within the available VRAM budget (88 free of 256), and MUST refuse allocations
  that would exceed it, reporting failure to the caller.
- **FR-005**: Freeing a sprite MUST release its OAM slot and release any VRAM
  tiles no longer referenced by any remaining sprite.

#### Background Layer

- **FR-006**: The visual layer MUST provide a background layer whose tiles
  occupy a range distinct from the UI/text/font tiles so the two never overwrite
  each other.
- **FR-007**: The background layer MUST coexist with the existing UI and
  dialogue/text rendering such that activating, updating, or clearing one does
  not corrupt the other.
- **FR-008**: Background tile and map updates MUST occur only outside active LCD
  rendering (V-Blank or H-Blank) to avoid visible glitches.

#### Palettes

- **FR-009**: On CGB hardware, the visual layer MUST assign background and sprite
  palettes at runtime, keeping palette indices within 0–7.
- **FR-010**: On DMG hardware, the layer MUST skip color palette assignment
  gracefully and render content in grayscale without error (`_cpu == CGB_TYPE`
  detection).
- **FR-011**: The layer MUST report (not silently corrupt) when more distinct
  color sets are requested than available hardware palettes.

#### Effects

- **FR-012**: The layer MUST provide a blink effect that toggles target content
  visibility at a configurable cadence and stops cleanly.
- **FR-013**: The layer MUST provide a fade effect that ramps scene brightness
  to/from black on CGB and degrades gracefully on DMG.
- **FR-014**: The layer MUST provide a scroll effect that moves the background
  smoothly while respecting LCD timing (no tearing).
- **FR-015**: The layer MUST provide a basic frame animation effect that cycles a
  multi-frame graphic at a configurable rate, with loop-once or repeat modes.
- **FR-016**: All effects MUST share a single fixed-size effect-state structure
  and MUST introduce no net WRAM growth beyond it (no heap, no per-effect growing
  allocation).
- **FR-017**: An effect targeting content that is freed or cleared MUST detach
  or stop without writing to released OAM slots or tiles.
- **FR-018**: Effects MUST validate or clamp invalid timing/rate inputs rather
  than producing undefined or non-terminating behavior.

#### Cross-cutting

- **FR-019**: The implementation MUST follow the canonical `examples/` samples
  for each technique — `galaxy` for scrolling, `hblank_copy` for VRAM-write
  timing, and `colorbar`/`dscan` for palettes.
- **FR-020**: All player-visible text rendering MUST continue to use the existing
  shared helpers and font tiles; the visual layer MUST NOT replace or bypass the
  UI/text layer.

### Key Entities *(include if feature involves data)*

- **Sprite handle**: A reference returned by the sprite manager identifying an
  allocated on-screen actor — its assigned OAM slot, the VRAM tile range it
  uses, and current position/visibility. Used to update, animate, or free it.
- **VRAM tile allocation record**: Tracks which tile ranges are in use, which
  graphic they hold, and how many sprites reference them, enabling reuse and
  reference-counted freeing within the 88-tile budget.
- **Palette set**: A named group of colors assigned to a hardware palette slot
  (0–7) for background or sprite use on CGB, with a grayscale interpretation on
  DMG.
- **Effect state**: A single fixed-size structure holding the active effects'
  parameters (type, target, cadence/rate, progress, mode) — the only RAM the
  effects API is permitted to grow.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A developer can place and reuse sprites referencing both shared and
  unique graphics with zero manual OAM-index or tile-address bookkeeping.
- **SC-002**: Duplicate graphics consume tiles only once — verifiable in the
  emulator VRAM viewer — and total VRAM never exceeds the 256-tile capacity (88
  free over the existing 168).
- **SC-003**: No allocation request ever overwrites an in-use OAM slot or VRAM
  tile; over-budget requests are reported as failures 100% of the time.
- **SC-004**: A background and the dialogue/text box display simultaneously with
  fully legible text and an intact background, on a booting ROM verified in BGB.
- **SC-005**: The same color content renders correctly on CGB and in sensible
  grayscale on DMG, with no crash or garbled tiles in either mode.
- **SC-006**: Each of the four effects (blink, fade, scroll, animation) produces
  its expected visible behavior in the emulator and stops/loops as configured.
- **SC-007**: Enabling the full effects API adds no more WRAM than the single
  fixed effect-state structure, confirmed in the build memory map.
- **SC-008**: Scroll and background updates show no tearing during continuous
  motion at the kit's target frame rate.

## Assumptions

- The existing 168/256 VRAM tile usage and 88 free tiles are the working budget;
  font and UI tiles are part of the 168 and must not be displaced.
- The existing OAM/sprite reservations documented for the kit (Alex idle/run,
  treasure, hazard) remain valid; the manager reuses free slots within the
  sub-40 range around them.
- "Background layer separate from UI/text" means a tile-range and palette
  separation on the single hardware background map, not a second hardware map.
- "No net WRAM growth beyond a small fixed effect-state struct" means the effects
  subsystem's only new RAM is one statically-sized structure, sized to hold the
  maximum simultaneous effects the kit supports.
- Manual verification in BGB/SameBoy is the acceptance gate; there is no
  automated test suite.
- The dead modules `sprite.c` / `background.c` are reference material only and
  stay excluded from the build unless a story explicitly revives and budgets
  them.
- Scope is the reusable visual-layer infrastructure (managers, palette
  assignment, effects API) — not authoring specific game art or scenes.
