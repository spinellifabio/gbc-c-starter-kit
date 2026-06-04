#ifndef BGM_H
#define BGM_H

#include <stdint.h>

/* Note entry: freq register value (11-bit, 0 = rest), duration in VBL frames */
typedef struct {
    uint16_t freq;
    uint8_t  duration;
} BgmNote;

#define BGM_NOTE_END { 0xFFFFu, 0u }  /* sentinel — player loops back to note 0 */

void bgm_play(const BgmNote *song);
void bgm_play_title_theme(void);
void bgm_stop(void);
void bgm_pause(void);
void bgm_resume(void);
void bgm_vbl_update(void);  /* called from add_VBL() — do not call directly */

#endif /* BGM_H */
