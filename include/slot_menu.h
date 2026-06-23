#ifndef SLOT_MENU_H
#define SLOT_MENU_H

#include <gb/gb.h>
#include <stdint.h>

typedef enum {
    SLOT_ACTION_CANCEL = 0,  /* player backed out to title */
    SLOT_ACTION_NEW,         /* start a fresh game in the chosen slot */
    SLOT_ACTION_CONTINUE     /* resume the saved game in the chosen slot */
} SlotAction;

typedef struct {
    uint8_t    slot;     /* 0..SAVE_SLOT_COUNT-1 (valid when action != CANCEL) */
    SlotAction action;
} SlotChoice;

/**
 * @brief Pre-game slot selection screen. Blocks until the player picks an
 *        action (NEW / CONTINUE) or backs out (CANCEL).
 */
SlotChoice slot_menu(void);

#endif /* SLOT_MENU_H */
