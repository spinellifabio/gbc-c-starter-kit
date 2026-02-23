#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>

// Dialogue result codes
#define DIALOGUE_RESULT_NONE   0
#define DIALOGUE_RESULT_YES    1
#define DIALOGUE_RESULT_NO     2

void dialogue_set_window_base_tile(uint8_t base_tile);
void dialogue_show_text(const char *text);
uint8_t dialogue_show_yes_no(const char *question);
void play_dialogue_sequence(void);

#endif /* DIALOGUE_H */
