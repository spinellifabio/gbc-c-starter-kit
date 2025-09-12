#include "lang.h"

/* Lingua corrente */
static Language current_lang = LANG_EN;

/* --- Tabelle stringhe UI (append-only).
 * Array [LANG_COUNT][STR__COUNT], tutti i puntatori sono in ROM.
 */
static const char* const STRINGS_EN[STR__COUNT] = {
    /* STR_SPLASH_STUDIO        */ "OPENAI GAMES",
    /* STR_SPLASH_PRESENTS      */ "PRESENTS",
    /* STR_INTRO_HURRY          */ "In a hurry, huh?",

    /* STR_TITLE_HINT_START_PLAY*/ "START=PLAY",
    /* STR_TITLE_HINT_SELECT_OPT*/ "SELECT=OPTIONS",

    /* STR_OPTIONS_TITLE        */ "OPTIONS",
    /* STR_OPTIONS_BUILD        */ "%s %s",
    /* STR_OPTIONS_FOOTER_BACK  */ "PRESS B TO RETURN",

    /* STR_OPT_SOUND            */ "SOUND",
    /* STR_OPT_DIFFICULTY       */ "DIFFICULTY",
    /* STR_OPT_LIVES            */ "LIVES",
    /* STR_OPT_MODE             */ "MODE",
    /* STR_OPT_LANGUAGE         */ "LANGUAGE",

    /* STR_VAL_ON               */ "ON",
    /* STR_VAL_OFF              */ "OFF",
    /* STR_VAL_EASY             */ "EASY",
    /* STR_VAL_NORMAL           */ "NORMAL",
    /* STR_VAL_HARD             */ "HARD",
    /* STR_VAL_MODE_RELEASE     */ "RELEASE",
    /* STR_VAL_MODE_DEBUG       */ "DEBUG",
    /* STR_VAL_LANG_EN          */ "EN",
    /* STR_VAL_LANG_IT          */ "IT",

    /* STR_GAMEPLAY_START       */ "GAMEPLAY START",

    /* STR_GAMEOVER_TITLE       */ "GAME OVER",
    /* STR_GAMEOVER_REASON_HOLE */ "FELL INTO A HOLE",
    /* STR_GAMEOVER_REASON_ENEMY*/ "DEFEATED BY ENEMY"
};

static const char* const STRINGS_IT[STR__COUNT] = {
    /* STR_SPLASH_STUDIO        */ "OPENAI GAMES",
    /* STR_SPLASH_PRESENTS      */ "PRESENTA",
    /* STR_INTRO_HURRY          */ "Hai fretta, eh?",

    /* STR_TITLE_HINT_START_PLAY*/ "START=GIOCA",
    /* STR_TITLE_HINT_SELECT_OPT*/ "SELECT=OPZIONI",

    /* STR_OPTIONS_TITLE        */ "OPZIONI",
    /* STR_OPTIONS_BUILD        */ "%s %s",
    /* STR_OPTIONS_FOOTER_BACK  */ "PREMI B PER TORNARE",

    /* STR_OPT_SOUND            */ "AUDIO",
    /* STR_OPT_DIFFICULTY       */ "DIFFICOLTA'",
    /* STR_OPT_LIVES            */ "VITE",
    /* STR_OPT_MODE             */ "MODALITA'",
    /* STR_OPT_LANGUAGE         */ "LINGUA",

    /* STR_VAL_ON               */ "ON",        /* tenuto per larghezza fissa */
    /* STR_VAL_OFF              */ "OFF",
    /* STR_VAL_EASY             */ "FACILE",
    /* STR_VAL_NORMAL           */ "NORMALE",
    /* STR_VAL_HARD             */ "DIFFICILE",
    /* STR_VAL_MODE_RELEASE     */ "RILASCIO",
    /* STR_VAL_MODE_DEBUG       */ "DEBUG",
    /* STR_VAL_LANG_EN          */ "EN",
    /* STR_VAL_LANG_IT          */ "IT",

    /* STR_GAMEPLAY_START       */ "GAMEPLAY AVVIO",

    /* STR_GAMEOVER_TITLE       */ "GAME OVER",
    /* STR_GAMEOVER_REASON_HOLE */ "CADUTO IN UNA BUCA",
    /* STR_GAMEOVER_REASON_ENEMY*/ "SCONFITTO DAL NEMICO"
};

static const char* const* const STRINGS_ALL[LANG_COUNT] = {
    STRINGS_EN,
    STRINGS_IT
};

/* --- Dialoghi: 2 pagine x max 3 righe --- */
static const char* const DIALOG_EN[LANG_DLG_PAGE_COUNT][LANG_DLG_MAX_LINES_PER_PAGE] = {
    { "Welcome to the prototype!",
      "It's not much yet...",
      "But at least text works." },
    { "Press A to speed up.",
      "Press START to skip.",
      "" }
};

static const char* const DIALOG_IT[LANG_DLG_PAGE_COUNT][LANG_DLG_MAX_LINES_PER_PAGE] = {
    { "Benvenuto nel prototipo!",
      "Non e' molto ancora...",
      "Ma almeno i testi funzionano." },
    { "Premi A per velocizzare.",
      "Premi START per saltare.",
      "" }
};

static const char* const (* const DIALOG_ALL[LANG_COUNT])[LANG_DLG_MAX_LINES_PER_PAGE] = {
    DIALOG_EN, DIALOG_IT
};

/* --- API --- */
void lang_init(Language default_lang) {
    current_lang = (default_lang < LANG_COUNT) ? default_lang : LANG_EN;
}

void lang_set(Language lang) {
    if (lang < LANG_COUNT) current_lang = lang;
}

Language lang_get(void) {
    return current_lang;
}

/* Lookup O(1) con bound check leggero */
const char* lang_str(LangStringId id) {
    if ((uint8_t)id >= (uint8_t)STR__COUNT) return "";
    return STRINGS_ALL[current_lang][(uint8_t)id];
}

/* Dialog helpers */
uint8_t lang_dialog_page_count(void) {
    return LANG_DLG_PAGE_COUNT;
}

uint8_t lang_dialog_line_count(uint8_t page) {
    if (page >= LANG_DLG_PAGE_COUNT) return 0u;
    /* Conta fino al primo "" (stringa vuota) come terminatore opzionale */
    const char* const (*pages)[LANG_DLG_MAX_LINES_PER_PAGE] = DIALOG_ALL[current_lang];
    uint8_t count = 0;
    for (uint8_t i = 0; i < LANG_DLG_MAX_LINES_PER_PAGE; i++) {
        if (pages[page][i][0] == '\0') break;
        count++;
    }
    return count;
}

const char* lang_dialog_line(uint8_t page, uint8_t line) {
    if (page >= LANG_DLG_PAGE_COUNT) return "";
    if (line >= LANG_DLG_MAX_LINES_PER_PAGE) return "";
    return DIALOG_ALL[current_lang][page][line];
}

/* --- Note performance ---
 * - lang_str(): indicizzazione + check → ~<100 cicli.
 * - dialog helpers: loop max 3 elementi → costo trascurabile.
 * Nessuna allocazione dinamica, tutti i dati in ROM.
 */
