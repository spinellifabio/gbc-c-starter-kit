# Feature Specification: VRAM & Scene Visual Polish

**Feature Branch**: `002-vram-scene-polish`

**Created**: 2026-08-15

**Status**: Draft

**Input**: User description: "considera il contenuto di @docs/screenshots per sistemare l'utilizzo della vram. Ad esempio rifina il text box usando l'apposito bordo fornito dal font. aggiusta l'utilizzo degli sprite e delle palette per tutte le scene impostate fino ad ora. alcune scene sono molto compromesse. soprattutto dalla fase di gioco in poi."

## Clarifications

### Session 2026-08-15

- Q: Title screen fix scope — logo art asset, or tile-mapping bug fix only? → A: Include creating a new dedicated logo graphic asset (bigger scope, new ROM art).
- Q: Placeholder sprites (`cat_idle`, `cutscene_player`, `cutscene_npc`) are visual copies of `player_idle` — in scope? → A: Include creating distinct placeholder-replacement art so they look visually distinct from the player.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Dialogue box uses the real font border (Priority: P1)

A player reading dialogue text sees a text box framed with the font's
dedicated border tiles (corners, edges) instead of plain punctuation
characters (`+`, `-`, `|`) standing in for a border.

**Why this priority**: The dialogue box is the most frequently seen UI
element (every dialogue-driven screen). It currently reads as a programmer
placeholder rather than a finished box, which is the most visible polish gap.

**Independent Test**: Trigger any dialogue screen (e.g. intro) and confirm in
the emulator that the box frame renders with proper border graphics, not
literal `+`/`-`/`|` glyphs, while text content and typewriter effect keep
working.

**Acceptance Scenarios**:

1. **Given** the dialogue box is opened, **When** it slides into view, **Then**
   its frame renders using the font's border tile set (distinct corner,
   horizontal-edge, and vertical-edge tiles) rather than character glyphs.
2. **Given** the dialogue box is open, **When** text types out inside it,
   **Then** the border remains stable and unaffected by the typewriter effect.
3. **Given** DMG hardware (no color), **When** the dialogue box is shown,
   **Then** the border still renders correctly using the DMG-safe palette.

---

### User Story 2 - Gameplay-and-later scenes render sprites/palettes correctly (Priority: P1)

A player progressing from gameplay through game over/win and credits sees
each actor (player, hazards, treasure, NPCs) drawn with its intended graphic
and color palette — no missing sprites, no palette bleed between actors, and
no corrupted/garbled tiles on background screens (e.g. the win screen's
border pattern).

**Why this priority**: This is the segment the user flags as "most
compromised" (`soprattutto dalla fase di gioco in poi`) — gameplay, win/game
over, and subsequent screens directly affect whether the game reads as
functional.

**Independent Test**: Play from gameplay start through to the win screen and
credits in the emulator; at each screen, confirm in the VRAM/OAM viewer that
sprite tiles map to the correct graphic and palette per actor, and that
background tile/attribute writes produce the intended pattern (no stray or
mismatched tiles).

**Acceptance Scenarios**:

1. **Given** gameplay is active, **When** the player, hazards, and treasure
   are all on screen simultaneously, **Then** each renders with its own
   correct graphic and none overwrite each other's OAM slot or VRAM tile.
2. **Given** the player and an in-scene NPC/enemy are both visible, **When**
   they use different palettes, **Then** each actor shows its assigned colors
   without bleeding into the other's palette slot.
3. **Given** the game reaches the win/game-over screen, **When** the
   background border/frame is drawn, **Then** the pattern renders as a clean,
   consistent tile (no visibly corrupted or mismatched glyph in the border).
4. **Given** the game proceeds to credits, **When** the screen is shown,
   **Then** sprite and palette usage remains consistent with the rest of the
   flow (regression-free baseline, already closest to correct).

---

### User Story 3 - Title screen shows a real logo, options screen fits on screen (Priority: P2)

A player at the title screen sees a dedicated logo graphic (not a garbled
tile-mapping of the game name), and at the options screen sees every menu
line and value fully visible within the 20-column screen width, with a
consistent palette.

**Why this priority**: Title and options are seen before gameplay and set
first impressions, but they are static/menu screens with lower visual
complexity than the sprite-heavy gameplay screens in User Story 2.

**Independent Test**: Open the title screen and confirm a dedicated logo
graphic renders correctly in place of the previously garbled name tiles; open
the options screen and confirm every label and value (e.g. "DIFFICULTY:
NORMAL") stays within the visible screen area with no line wrapping
off-screen.

**Acceptance Scenarios**:

1. **Given** the title screen loads, **When** the screen is drawn, **Then** a
   dedicated logo graphic renders correctly, replacing the previously garbled
   game-name tile mapping.
2. **Given** the options screen is open, **When** any menu row's label and
   value are combined, **Then** the full row fits within the visible screen
   width without being cut off or wrapping past the right edge.
3. **Given** the options screen is open, **When** rendered on CGB, **Then**
   text uses a single consistent, readable palette across all rows.

---

### User Story 4 - Cutscene and future NPC sprites look distinct from the player (Priority: P3)

A player watching the intro cutscene, or seeing the placeholder `cat_idle`
actor, sees graphics visually distinct from the main player character,
instead of the current placeholder copies of the player sprite.

**Why this priority**: These actors are not currently shown as
"corrupted" — they are functional placeholders — so this is lower-impact
polish than fixing broken/colliding scenes, but was confirmed in scope
alongside the title logo fix.

**Independent Test**: Play the intro cutscene and confirm the cutscene player
and NPC render with graphics distinct from each other and from the main
gameplay player sprite; if `cat_idle` is shown anywhere reachable in the
current build, confirm it is visually distinct from the player.

**Acceptance Scenarios**:

1. **Given** the intro cutscene is playing, **When** the cutscene player and
   cutscene NPC are both on screen, **Then** each renders with graphics
   visually distinct from the other and from the main gameplay player.
2. **Given** the `cat_idle` placeholder actor is instantiated, **When** it is
   shown, **Then** it renders with a graphic visually distinct from the
   player instead of a reused copy.

---

### Edge Cases

- What happens when a screen's palette budget (8 slots) is already fully
  used by prior scene state that was not cleared on transition? Palette
  slots MUST be reset/reassigned per screen so stale colors from a previous
  screen never bleed into the next.
- How does the system handle a dialogue box opened over a background that
  uses the same tile range as the border graphics? Border and background
  tile ranges MUST stay disjoint so the border never displays background
  content by accident.
- What happens when an options value string (e.g. a longer localized
  difficulty label) is long enough to have overflowed the row today? Row
  layout MUST truncate or reflow rather than exceeding the 20-column screen.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The dialogue box MUST render its frame using the font's
  dedicated border tiles (distinct corner and edge graphics) instead of
  literal character glyphs (`+`, `-`, `|`).
- **FR-002**: The dialogue box border rendering MUST work identically on CGB
  (with palette) and DMG (grayscale-safe) hardware.
- **FR-003**: Every gameplay actor (player, hazard, treasure, NPC) MUST
  display its intended sprite graphic with no OAM-slot or VRAM-tile
  collisions with any other simultaneously visible actor.
- **FR-004**: Every gameplay actor MUST display its intended CGB palette with
  no cross-actor palette bleed (one actor's colors must not leak into
  another's assigned palette slot).
- **FR-005**: The win/game-over screen's background border pattern MUST
  render as a single consistent, non-corrupted tile repeated across the
  frame.
- **FR-006**: The credits screen MUST continue to render correctly (border,
  text, palette) as a non-regression baseline after other fixes are applied.
- **FR-007**: The title screen MUST display a dedicated logo graphic asset
  (new ROM art) in place of the currently garbled game-name tile mapping.
- **FR-008**: The options screen MUST keep every label+value row fully within
  the visible screen width, with no text wrapping or being cut off past the
  right edge.
- **FR-009**: The options screen MUST use one consistent, readable text
  palette across all rows on CGB.
- **FR-010**: Screen transitions (splash → intro → title → options →
  gameplay → game over/win → credits → title) MUST reset or reassign
  palette slots and sprite/VRAM allocations so no stale state from a
  previous screen is visible on the next screen.
- **FR-011**: All fixes MUST stay within the existing VRAM sprite-tile
  budget and OAM index range documented in the project (no expansion of the
  168-tile sprite layout or the sub-40 OAM range) unless a screen's fix is
  provably impossible within that budget, in which case the added cost MUST
  be stated.
- **FR-012**: The `cat_idle`, `cutscene_player`, and `cutscene_npc` sprite
  graphics MUST be replaced with art visually distinct from `player_idle`
  (they currently are byte-for-byte placeholder copies of the player
  graphic), within the same VRAM tile budget those placeholders already
  occupy.

### Key Entities

- **Dialogue Box Border**: The visual frame (corners + edges) drawn around
  dialogue text; currently ASCII-glyph-based, must become font-border-tile
  based.
- **Scene Sprite Set**: The group of sprites (player, hazards, treasure,
  NPCs) active on a given screen, each with a graphic and palette
  assignment that must not collide with others on the same screen.
- **Scene Palette Assignment**: The mapping of a screen's visible
  sprites/background text to one of the 8 available CGB palette slots.
- **Title Logo Graphic**: New dedicated ROM art asset for the title screen,
  replacing the current garbled game-name tile rendering.
- **Placeholder Actor Graphics**: The `cat_idle`, `cutscene_player`, and
  `cutscene_npc` sprite assets, currently reused copies of `player_idle`,
  to be replaced with visually distinct art.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of dialogue-driven screens show a bordered text box using
  proper border graphics (visually confirmed in emulator VRAM viewer),
  with zero occurrences of literal `+`/`-`/`|` box-drawing characters.
- **SC-002**: Across a full playthrough (gameplay → game over/win →
  credits → title), zero visually corrupted or mismatched sprite/background
  tiles are observed in the emulator, versus at least 3 distinct
  corruption/mismatch points observed today (splash sprite placement, title
  logo garble, win-screen border corruption).
- **SC-003**: 100% of options-screen rows are fully visible within the
  20-column screen width at default settings, with zero characters
  rendered off-screen (versus rows currently wrapping past the right edge).
- **SC-004**: Every actor visible simultaneously in gameplay renders with a
  visually distinct, correct color set with zero observed palette bleed
  between actors.
- **SC-005**: The title screen shows a dedicated logo graphic (0 instances of
  garbled/mismatched name-tile rendering, versus the garbled rendering
  observed today).
- **SC-006**: 100% of the `cat_idle`, `cutscene_player`, and `cutscene_npc`
  actors render with graphics visually distinct from `player_idle` (versus
  100% reused-copy today).

## Assumptions

- The GBDK font asset already ships dedicated border/frame tiles (as
  referenced by the user as "l'apposito bordo fornito dal font"); this
  feature wires them into the dialogue box rather than creating new border
  art.
- Visual correctness is judged by manual inspection in BGB/SameBoy
  (including the VRAM/OAM/Palette viewers), consistent with the project's
  no-automated-test constraint.
- The splash-screen small floating sprite artifact is an instance of
  incorrect tile-index/graphic mapping, in scope for fixing even though the
  user's examples focus on gameplay-and-later.
- No new screens or gameplay mechanics are introduced by this feature.
  New sprite/logo *art assets* are in scope only where clarified above
  (title logo, `cat_idle`/`cutscene_player`/`cutscene_npc`); all other work
  is strictly correcting existing VRAM/sprite/palette usage on
  already-implemented screens.
- The credits screen is treated as the closest-to-correct reference example
  and is preserved, not reworked.
- New art assets (title logo, placeholder-replacement sprites) are simple
  pixel art consistent with the existing 16×16 sprite style and GBC color
  constraints; exact visual design is a planning-phase decision, not
  specified here.
