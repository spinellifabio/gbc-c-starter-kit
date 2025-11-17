#ifndef UTILS_H
#define UTILS_H

#include <gb/gb.h>
#include <gbdk/console.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Waits for any button press and clears the input buffer
 */
static inline void wait_any_key(void) {
    waitpad(J_START | J_A | J_B | J_UP | J_DOWN | J_LEFT | J_RIGHT);
    waitpadup();
}

/**
 * @brief Clears the screen by filling it with space characters
 */
static inline void clear_screen(void) {
    uint8_t space = ' ';
    for (uint8_t y = 0; y < 18; y++) {
        for (uint8_t x = 0; x < 20; x++) {
            set_bkg_tiles(x, y, 1, 1, &space);
        }
    }
}

/**
 * @brief Converts a number to a string
 *
 * @param value The number to convert
 * @param buffer Buffer to store the result (must be large enough)
 * @param min_digits Minimum number of digits (will be padded with zeros)
 */
static inline void int_to_str(uint16_t value, char *buffer, uint8_t min_digits) {
    uint8_t i = 0;
    uint8_t digits = 1;
    uint16_t temp = value / 10;

    // Count digits
    while (temp > 0) {
        temp /= 10;
        digits++;
    }

    // Add leading zeros if needed
    while (digits < min_digits) {
        buffer[i++] = '0';
        min_digits--;
    }

    // Convert number to string (backwards)
    uint16_t divisor = 1;
    for (uint8_t j = 1; j < digits; j++) {
        divisor *= 10;
    }

    while (divisor > 0) {
        buffer[i++] = '0' + (value / divisor) % 10;
        divisor /= 10;
    }
    buffer[i] = '\0';
}

/**
 * @brief Prints text centered on a specific row
 *
 * @param text The text to print
 * @param row The row to print on (0-17)
 */
static inline void print_centered(const char *text, uint8_t row) {
    uint8_t len = 0;
    const char *p = text;
    while (*p++) len++;

    uint8_t x = (20 - len) / 2;
    gotoxy(x, row);  // Position cursor at calculated x and given row
    while (*text) {
        printf("%c", *text++);
    }
}

#endif // UTILS_H
