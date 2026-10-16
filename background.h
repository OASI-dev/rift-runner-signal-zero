#ifndef RR_BACKGROUND_H
#define RR_BACKGROUND_H
/* Star position and animation data used by the background renderer. */
typedef struct Star {
    float x, y, speed, size, phase;
} Star;

/* Friendly sector names shared by the background and HUD. */
const char *GetSectorName(void);
#endif
