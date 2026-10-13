# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### 🚀 Features

- *(assets)* Add asset manifest and generator pipeline ([`50cd6b1`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/50cd6b1e427008767c45e7bc3df5d31c04d709a6))
- *(build)* Wire visual content layer into CMakeLists and game_system init ([`b374b9d`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/b374b9d6edc210ba5f7d53492daa9731d5132c3b))
- *(visual-content-layer)* Add effects engine with 4 reusable effects ([`00f1a90`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/00f1a907c8ac6fb12a52d328a5791a521d4fd84a))
- *(visual-content-layer)* Add palette allocation with CGB + DMG fallback ([`bb31cd6`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/bb31cd6ad899aeca46eafc9d1b949dcf00d56e1b))
- *(visual-content-layer)* Add background layer system ([`743f121`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/743f121ee0bdb79a890c79bcd472aae3f2eb0f4c))
- *(visual-content-layer)* Add sprite manager with OAM and VRAM tile reuse ([`d63491f`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/d63491fad0d375a1e2af196f0999e5eed3307f24))
- *(save)* Add 3-slot SRAM save system ([`2891512`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/2891512e5522e9d468a9865b4418511593302c3a))
- *(audio)* Add SFX system and VBL BGM sequencer ([`e12eae7`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/e12eae78bbe9f95acd5e598c625474420fe5d977))
- *(npc)* Add NPC system with multi-NPC array and solid collision ([`67f1632`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/67f16320321676221da2f94c4bf078475909b4e6))
- *(game_system)* Wire minimal LCD ISR stub ([`a1cbd5f`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a1cbd5f723fdc459bae07378e2e4113e0ba43a80))
- *(examples)* Add curated GBDK reference examples ([`22f38b2`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/22f38b2b6f822e0fd0b8d564c4411dc2d7b1a0ff))
- *(scenes)* Add fade-to-black transitions and fix scene glitching ([`704f9fa`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/704f9fa4a0e3f3cffe8bc812ed420f362219b518))
- Feat(dialogue): use font tile base for window rendering. ([`c00acfc`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/c00acfcc3b4f2b50cdb6c82edeecc5f0d3d39c3d))
- *(gameplay)* Implement win screen and improve win condition ([`3940dc2`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/3940dc26655336ec71307a29a658beb009c1a9a0))
- *(main)* Game over flow. ([`1fdc488`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/1fdc48817ad008433d650c02647ae9fe9684067e))
- *(gameplay)* Make interations. ([`d4d8441`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/d4d8441f791d647cfd4d0ccbfb8e057ab0081f70))
- Implement game state management and update documentation ([`7f535d6`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/7f535d689a3842b0f81670538e80a4b410d5dbf5))
- Add skippable credits scene ([`1f4b0bf`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/1f4b0bfa9ff14f655da26473769886e6ecdb6ed5))
- *(intro)* Animate cutscene and streamline dialogue ([`88f4f7e`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/88f4f7e26e67586e0aa938d721325add2c4e7df1))
- *(intro)* Add two-character walk-in cutscene ([`25f14b7`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/25f14b7b300cc6c90ca781fbd4a0e4a22896760e))

### 🐛 Bug Fixes

- Stop options/slot-menu sound drone and dead B button ([`00dd17d`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/00dd17dab8f7329924ea33284edd9a7e599ebac0))
- Correct VRAM/sprite/palette usage across all screens ([`f8ffed8`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/f8ffed854d22791b3da029b5f52c200bd1d3cb0d))
- Minor adjustment ([`a40d0ef`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a40d0ef8a1a39bde6dabb9679bb2730211563261))
- *(utils)* Put font ink color at palette index 3 in set_solid_bkg ([`20e247c`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/20e247c1667e01544003f4c1d5a1d6c99520d31b))
- *(scene)* Correct OAM clearing, fade palette, and scroll reset on transition ([`9d59443`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/9d59443ae69ffc6dddafaae5daae6624311c110c))
- *(scene)* Clear OAM to prevent sprite persistence after transitions ([`dfab14a`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/dfab14a68caeaba4d86282e02b7717932e6923b0))
- *(scenes)* Fix sprite/background persistence and make fade animation visible ([`23552e9`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/23552e97fe82c17d15538c2aa3b33841790a65e1))
- *(render)* Clear cgb attr map between scenes ([`13a99b0`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/13a99b0a184586276067833cfb2f92ac371d38ea))
- *(dialogue)* Let b pick no and animate cursor ([`a136f05`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a136f0572aeabf367e71fab27feba18c97ec5eca))
- Minor. ([`0f5d383`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/0f5d383af909a2b1901e8edf5788bc1f318673da))
- *(background)* Resolve CGB detection and tile rendering issues ([`1bbe1c2`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/1bbe1c278a4925e8ad455470ace76b4ac540683c))
- Remove lag if sprite moving. ([`ea63e4a`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/ea63e4a63a94e817b91395f58259e57a5974ba83))
- Tileset. ([`afa0ac0`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/afa0ac023c1a72754b126a0ed0346dbb790f893d))
- Try to simplify the map settings. ([`40254bb`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/40254bbaf39a71877e46839585159804b7109a26))

### 💼 Other

- Update text box sistem. fit in the monitor. ([`cad7b38`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/cad7b38bfd47066e1d75623d0094e8002c78513a))
- Draw map. ([`f840809`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/f8408095c463cfa5c2675eaac3893b0a8990d403))
- Massive refactoring. ([`4ff9d67`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/4ff9d67bc5b477bf8a5829553085a782a9318253))
- Loading palette if is CBG. ([`e127143`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/e12714372dec232bf46f36a652ede96bdeff209c))
- Using tile-set has a background ([`07caf3b`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/07caf3bec9f7abab54c38ad93128e13a36633808))
- Minor. ([`9e67b30`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/9e67b3014367e7d8c7d6000b23955f4808591cfa))
- A new tile set. ([`a4ce76b`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a4ce76bd75e9b317b832520f69870bd9a64c26ea))
- Simplify ([`f1e3cba`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/f1e3cbaa5a6c680a2a61e11262e4d5af23f8e81e))
- First try map! ([`136fcb1`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/136fcb1bed79b60cab6bd3659723e875ae210a3d))
- Map limit. ([`b6da7cf`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/b6da7cf31e2435c4f3db1f3891f1e3589576f909))
- Main configuration. ([`70cb0d8`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/70cb0d84635a54aa521f485692f53bb890ba1299))
- Run animation fixed. ([`2136d38`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/2136d38834b9cf7c7bf4cac15d58dd0a37c1785e))
- Run sprite error. ([`e5ed5f7`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/e5ed5f7ef559c6ca6605fac2cda6d1813b0020cf))
- Sprite move. ([`e597ba2`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/e597ba286e389267e03169f388c25022e8e1f147))
- First sprite. ([`c3ad728`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/c3ad728bccb851072819c544b1db2bb32931d53a))
- README.md ([`a47a566`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a47a566a3f4d55eb383347ece79de184e6b9ad1e))
- Gameplay with tiles. ([`c5f2414`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/c5f2414d47537d017b1db33ba54ee5c894f62278))
- Better language system; update roadmap and readme. ([`4c89675`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/4c89675d0e53a701220d2dd7266415ddfb5a2226))
- Intro cut scene; better language system (partial). ([`1bc681b`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/1bc681b6538f220e40903f8edf77a8f64938fbd4))
- Gameplay prototype. ([`55989bc`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/55989bc43a0ca5584f49d41e9598f141c2930121))
- Better option screen. ([`db7dc27`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/db7dc27c0af4461fa57124441c3b6b9b853eaea4))
- Options prototype. ([`0b8a7c2`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/0b8a7c20daf30656422b974cbef7099729187f08))
- Textual prototype ([`23c830c`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/23c830c0b3973eb4a114152ae525ba076f93f156))
- Title example. ([`065bc61`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/065bc61c15cb684e8fd5e94ec8c875c52e113d80))
- GBC  C Starter Kit README,md ([`f3c8c09`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/f3c8c097f7d9419d9a8ec580b3890694c25c2b9e))
- Bgb64 path. ([`9d59efd`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/9d59efd5f624c7e81a600e4e9af146507660265e))
- Template settings. ([`9bc1c74`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/9bc1c74b46b72604217e17139cabd3826b64b1af))

### 📚 Documentation

- Sync README and roadmap with codebase ([`13f92ec`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/13f92ec9615edbc0adca04845f7b20fd3cbcf4be))
- Minor adjustment ([`93c0ae3`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/93c0ae37c5968430159a5c1d64b5c637e299dcd3))
- *(constitution)* Add scope and context rules ([`976464b`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/976464b78d74165246820e3328319a3158602d90))
- *(spec)* Add VRAM & scene visual polish spec/plan/tasks ([`4539653`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/4539653510928d2642528088bff14e84188c398b))
- Mark visual content layer (section 4) complete ([`c72ee9c`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/c72ee9cb1594718ad7d9590c125fec7f34d64e95))
- Ratify project constitution v1.0.0 ([`f6b82f9`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/f6b82f9934cfbde16b5583bc1153bf9375a29233))
- Agent settings up. ([`1fc46e8`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/1fc46e88a17887d90a91a7c9e7d3c5bebe82f93c))
- Add comprehensive optimization report ([`182da38`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/182da38792ddf8f0d6724528ff193625f58770c5))
- Add yes/no control behavior ([`eb522b0`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/eb522b0a4440d05e19125a032be7de4d8cb78036))
- Record credits flow updates ([`afc5c9a`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/afc5c9a9d1f08484ad2ff2f617efe89449c01bc5))

### ⚡ Performance

- *(memory)* Optimize WRAM usage and fix critical bugs ([`6c5b4d6`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/6c5b4d689b6aa9eb48d8961fb84fec4457d3e56c))

### ⚙️ Miscellaneous Tasks

- *(tests)* Add lives and collision headless checks ([`a6f770d`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/a6f770dde137c28ab56596c4a32fdadaa26c01f5))
- *(tests)* Add utils.c headless checks and share test harness ([`bfac194`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/bfac194bbce3f067673c4ba646b6cf4050ef00c5))
- *(tests)* Add headless BGB test ROM for save system ([`4b412e0`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/4b412e092cb4b8a65301751daec9f82d4eef9f26))
- *(tests)* Add stdlib unittest checks for asset pipeline ([`e158181`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/e158181e5ac743f80d598ea8732015ff9ae038d4))
- *(claude)* Replace vexp with ccc, slim agent docs ([`c6f6e3f`](https://github.com/spinellifabio/gbc-c-starter-kit/commit/c6f6e3f76719e8365e91d0d19cdee52b49b6283c))

## Credits

```text
 $$$$$$\  $$$$$$$\   $$$$$$\         $$$$$$\         $$$$$$\    $$\                          $$\                               $$\   $$\ $$\   $$\
$$  __$$\ $$  __$$\ $$  __$$\       $$  __$$\       $$  __$$\   $$ |                         $$ |                              $$ | $$  |\__|  $$ |
$$ /  \__|$$ |  $$ |$$ /  \__|      $$ /  \__|      $$ /  \__|$$$$$$\    $$$$$$\   $$$$$$\ $$$$$$\    $$$$$$\   $$$$$$\        $$ |$$  / $$\ $$$$$$\
$$ |$$$$\ $$$$$$$\ |$$ |            $$ |            \$$$$$$\  \_$$  _|   \____$$\ $$  __$$\\_$$  _|  $$  __$$\ $$  __$$\       $$$$$  /  $$ |\_$$  _|
$$ |\_$$ |$$  __$$\ $$ |            $$ |             \____$$\   $$ |     $$$$$$$ |$$ |  \__| $$ |    $$$$$$$$ |$$ |  \__|      $$  $$<   $$ |  $$ |
$$ |  $$ |$$ |  $$ |$$ |  $$\       $$ |  $$\       $$\   $$ |  $$ |$$\ $$  __$$ |$$ |       $$ |$$\ $$   ____|$$ |            $$ |\$$\  $$ |  $$ |$$\
\$$$$$$  |$$$$$$$  |\$$$$$$  |      \$$$$$$  |      \$$$$$$  |  \$$$$  |\$$$$$$$ |$$ |       \$$$$  |\$$$$$$$\ $$ |            $$ | \$$\ $$ |  \$$$$  |
 \______/ \_______/  \______/        \______/        \______/    \____/  \_______|\__|        \____/  \_______|\__|            \__|  \__|\__|   \____/

                                                                                                         v0.2.0 - Code assembled in spare time by Fabio
```

