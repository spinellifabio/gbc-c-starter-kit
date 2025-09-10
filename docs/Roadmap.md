# GBC Prototype Roadmap

Questa roadmap definisce i passi per completare il prototipo testuale su GameBoy Color, pronto a diventare un mini-framework di gioco.

---

## 1. Core Template e Menu

- [x] Gestione splash screen multipla non skippabile
- [x] Schermata titolo con START e SELECT → opzioni
- [x] Menu opzioni dinamico con cursore, LEFT/RIGHT per cambiare valori
- [x] Global settings consolidati: sound, difficulty, lives, mode, language, game_name, version
- [x] Verifica input flush per evitare tasti appiccicati

## 2. Game Over

- [x] Game over screen con delay prima di essere skippabile (~3-4s)
- [ ] Possibilità di impostare differenti messaggi/game over per scenari futuri

## 3. Sprites e Visual Content

- [ ] Gestione sprite (OAM, tile reuse)
- [ ] Gestione background layer separato da UI
- [ ] Palette dinamica e compatibilità DMG/CGB
- [ ] Effetti visivi: blink, fade, scroll, animazioni base

## 4. Intro Video / Cutscene

- [ ] Schermata introduttiva animata (sprite/background)
- [ ] Dialogo testuale sincronizzato con frame → velocizzabile con tasto `A`
- [ ] Possibilità di skip completo del video con un tasto
- [ ] Collegamento finale alla schermata titolo

## 5. Schermata Titolo Avanzata

- [ ] Mostrare game_name + versione + eventuale logo
- [ ] START → gameplay
- [ ] SELECT → menu opzioni
- [ ] Opzioni multilingua funzionanti anche nei messaggi UI

## 6. Gameplay Test / Mini RPG

- [ ] Schermata gioco con personaggio controllabile (tile-based movement)
- [ ] Interazione con elementi della mappa (NPC, oggetti)
- [ ] Almeno due eventi:
  - [ ] porta al `game_over_screen`
  - [ ] porta al completamento (credits)
- [ ] Aggiornamento dello stato globale (es. vite, punteggio, lingua)

## 7. Game Completion / Credits

- [ ] Schermata credits scorrevoli o statici
- [ ] Possibilità di tornare al titolo o reset completo
- [ ] Lingua globale rispettata anche nei credits

## 8. Audio Minimale

- [ ] SFX base: button press, step movimento, interazioni
- [ ] Musica di sottofondo opzionale per splash/title/gameplay
- [ ] Integrazione con settings.sound_on

## 9. Multilingua

- [ ] Tutti i testi: splash, titolo, opzioni, dialogo, game over, credits
- [ ] Switch lingua in tempo reale tramite menu opzioni
- [ ] Preparazione per più di due lingue (scalabile)

## 10. Refactoring e Ottimizzazione

- [ ] Consolidare funzioni comuni (flush_input, draw_text_center, etc.)
- [ ] Separazione moduli: menu, gameplay, splash, audio, settings
- [ ] Controllo cicli per mantenere 60 FPS
- [ ] Profilazione RAM/ROM, riduzione footprint ove possibile

## 11. Testing & Debug

- [ ] Testare su emulatori (BGB, SameBoy) e hardware reale
- [ ] Edge case: input simultanei, overflow di frame_counter, palette non compatibili
- [ ] Verificare comportamento multilingua completo

---

## Extra / Optional

Questi obiettivi sono **aggiuntivi**, utili come valore extra per giochi futuri.

- [ ] **Achievements / RetroAchievements**
  - [ ] Integrare sistemi di achievements tramite [RetroAchievements](https://retroachievements.org/)
  - [ ] Esempi pratici:
    - Skip rapido dell’intro
    - Game Over raggiunto
    - Completion / The End
    - Speedrun / completamento rapido
  - [ ] Modulo per flaggare achievement e inviare segnali all’API RetroAchievements

- [ ] **Esportazione risultati**
  - [ ] Codifica dei risultati in audio modulato
  - [ ] Piattaforma esterna capace di leggere punteggio, progressi, successi
  - [ ] Minimo: successo/fallimento, punteggio, nome giocatore

- [ ] **Extra Visual / Gameplay**
  - [ ] Mini-effetti visivi avanzati (shake, fade, scroll parallax)
  - [ ] Animazioni sprite più complesse (NPC walking cycles, battaglie)
  - [ ] Ulteriore multilingua per testi narrativi o dialoghi
  - [ ] Score tracking persistente tramite save temporaneo su RAM/EEPROM
