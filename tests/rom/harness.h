#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

/* Shared by the headless test ROMs (see tests/test_rom.py). Include from one .c only.
 * Results go to SRAM at RESULT_ADDR (past the 3 save slots):
 *   [0] = 0xA5 done marker, [1] = check count, [2..] = 1 pass / 0 fail per check. */
#include <gb/gb.h>
#include <stdint.h>

#define SRAM          ((volatile uint8_t *)0xA000)
#define RESULT_ADDR   0x100u
#define MAX_CHECKS    16u

static uint8_t results[MAX_CHECKS];
static uint8_t n_checks;

#define CHECK(cond) do { results[n_checks++] = (uint8_t)((cond) ? 1u : 0u); } while (0)

/* Runner sets a BGB breakpoint on this symbol (address read from the .map). */
void test_done(void) {
    while (1) { }
}

static void test_finish(void) {
    uint8_t i;
    ENABLE_RAM; SWITCH_RAM(0);
    SRAM[RESULT_ADDR + 1u] = n_checks;
    for (i = 0u; i < n_checks; i++) SRAM[RESULT_ADDR + 2u + i] = results[i];
    SRAM[RESULT_ADDR] = 0xA5u;
    DISABLE_RAM;
    test_done();
}

#endif /* TEST_HARNESS_H */
