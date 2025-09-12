#ifndef LANG_H
#define LANG_H

#include <gb/gb.h>
#include <stdint.h>

typedef enum {
    LANG_EN = 0,
    LANG_IT,
    LANG_COUNT
} Language;

typedef enum {
    /* Splash / Intro */
    STR_SPLASH_STUDIO,          /* "OPENAI GAMES" / "OPENAI GAMES" */
    STR_SPLASH_PRESENTS,        /* "PRESENTS" / "PRESENTA" */
    STR_INTRO_HURRY,            /* "In a hurry, huh?" / "Hai fretta, eh?" */

    /* Title */
    STR_TITLE_HINT_START_PLAY,  /* "START=PLAY" / "START=GIOCA" */
    STR_TITLE_HINT_SELECT_OPT,  /* "SELECT=OPTIONS" / "SELECT=OPZIONI" */

    /* Options: header & footer */
    STR_OPTIONS_TITLE,          /* "OPTIONS" / "OPZIONI" */
    STR_OPTIONS_BUILD,          /* "%s %s" (formatted: game_name + version) */
    STR_OPTIONS_FOOTER_BACK,    /* "PRESS B TO RETURN" / "PREMI B PER TORNARE" */

    /* Option item labels */
    STR_OPT_SOUND,              /* "SOUND" / "AUDIO" */
    STR_OPT_DIFFICULTY,         /* "DIFFICULTY" / "DIFFICOLTA'" */
    STR_OPT_LIVES,              /* "LIVES" / "VITE" */
    STR_OPT_MODE,               /* "MODE" / "MODALITA'" */
    STR_OPT_LANGUAGE,           /* "LANGUAGE" / "LINGUA" */

    /* Option values */
    STR_VAL_ON,                 /* "ON" / "ON"  (lasciato invariato per larghezza) */
    STR_VAL_OFF,                /* "OFF" / "OFF" */
    STR_VAL_EASY,               /* "EASY" / "FACILE" */
    STR_VAL_NORMAL,             /* "NORMAL" / "NORMALE" */
    STR_VAL_HARD,               /* "HARD" / "DIFFICILE" */
    STR_VAL_MODE_RELEASE,       /* "RELEASE" / "RILASCIO" */
    STR_VAL_MODE_DEBUG,         /* "DEBUG" / "DEBUG" */
    STR_VAL_LANG_EN,            /* "EN" / "EN" */
    STR_VAL_LANG_IT,            /* "IT" / "IT" */

    /* Gameplay */
    STR_GAMEPLAY_START,         /* "GAMEPLAY START" / "GAMEPLAY AVVIO" */

    /* Game Over */
    STR_GAMEOVER_TITLE,         /* "GAME OVER" / "GAME OVER" */
    STR_GAMEOVER_REASON_HOLE,   /* "FELL INTO A HOLE" / "CADUTO IN UNA BUCA" */
    STR_GAMEOVER_REASON_ENEMY,  /* "DEFEATED BY ENEMY" / "SCONFITTO DAL NEMICO" */

    STR__COUNT                  /* sempre ultimo */
} LangStringId;

/* --- Dialoghi: layout fisso (2 pagine x 3 righe max) --- */
#define LANG_DLG_PAGE_COUNT          2u
#define LANG_DLG_MAX_LINES_PER_PAGE  3u

/* Inizializza il sistema (default_lang se valido, altrimenti EN) */
void lang_init(Language default_lang);

/* Cambia lingua attiva */
void lang_set(Language lang);

/* Ottieni lingua attiva */
Language lang_get(void);

/* Recupera una stringa UI per ID (ritorna "" se ID out-of-range) */
const char* lang_str(LangStringId id);

/* Dialoghi:
 * - lang_dialog_line_count(page): quante righe valide ha la pagina (0..3)
 * - lang_dialog_line(page, line): puntatore alla riga ("" se out-of-range)
 * Le pagine vanno da 0 a LANG_DLG_PAGE_COUNT-1; linee 0..2.
 */
uint8_t    lang_dialog_page_count(void);
uint8_t    lang_dialog_line_count(uint8_t page);
const char* lang_dialog_line(uint8_t page, uint8_t line);

#endif /* LANG_H */
