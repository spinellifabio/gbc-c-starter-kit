/* Headless utils.c checks (see tests/test_rom.py). */
#include <string.h>

#include "harness.h"
#include "utils.h"

static uint8_t str_eq(uint16_t v, uint8_t min_digits, const char *want) {
    char buf[8];
    int_to_str(v, buf, min_digits);
    return (uint8_t)(strcmp(buf, want) == 0);
}

void main(void) {
    CHECK(str_eq(0u, 1u, "0"));
    CHECK(str_eq(65535u, 1u, "65535"));
    CHECK(str_eq(7u, 3u, "007"));
    CHECK(str_eq(1234u, 2u, "1234"));      /* min_digits below actual digits */
    CHECK(str_eq(100u, 1u, "100"));        /* inner zeros kept */

    text_set_font_base(10u);
    CHECK(text_char_tile(' ') == 10u);
    CHECK(text_char_tile('A') == (uint8_t)(10u + 33u));
    CHECK(text_char_tile('~') == (uint8_t)(10u + 94u));
    CHECK(text_char_tile('\x1F') == 10u);  /* below ' ' -> space */
    CHECK(text_char_tile('') == 10u);     /* DEL, just above '~' -> space */
    CHECK(text_char_tile((char)0x80) == 10u); /* high bit -> space */

    test_finish();
}
