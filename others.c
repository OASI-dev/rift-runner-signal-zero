/*
 * RIFT RUNNER: SIGNAL ZERO  -  others.c
 *
 * This file holds the "extra" code: colors, drawing, sound, saving scores and
 * touch reading. You do NOT need to understand every line of it.
 *
 * It is NOT compiled on its own. main.c pulls it in with  #include "others.c"
 * (after the structs and global variables are defined), so keep both files in
 * the same folder and compile only main.c.
 */

/* ==================================================================
 * SECTION 1: COLORS AND TEXT
 * Colors used everywhere. Change a color here and it changes in the whole game.
 * The difficulty names, taglines and colors also live here (index 0 easy, 1 medium, 2 hard).
 * ================================================================== */

#define C_PANEL  (Color){ 15, 21, 45, 238 }
#define C_CYAN   (Color){ 76, 231, 255, 255 }
#define C_BLUE   (Color){ 78, 131, 255, 255 }
#define C_PURPLE (Color){ 179, 91, 255, 255 }
#define C_PINK   (Color){ 255, 83, 170, 255 }
#define C_GREEN  (Color){ 98, 255, 178, 255 }
#define C_GOLD   (Color){ 255, 207, 100, 255 }
#define C_TEXT   (Color){ 236, 243, 255, 255 }
#define C_MUTED  (Color){ 143, 158, 196, 255 }
#define C_RED    (Color){ 255, 83, 99, 255 }

const char *diffName[3]    = { "EASY", "MEDIUM", "HARD" };
const char *diffTagline[3] = { "Learn the arena", "Stay moving", "Overwhelming pursuit" };
Color diffColor[3]         = { { 98, 255, 178, 255 }, { 76, 231, 255, 255 }, { 255, 83, 170, 255 } };


/* ==================================================================
 * SECTION 2: SMALL HELPER FUNCTIONS
 * Tiny tools used by many other functions.
 * ================================================================== */

/* Random decimal number between lo and hi. */
float RandF(float lo, float hi) { return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo); }

/* Build a Vector2 (an x,y pair) from two numbers. */
Vector2 V2(float x, float y) { Vector2 v = { x, y }; return v; }

/* Current window width. */
int SW(void) { return GetScreenWidth(); }

/* Current window height. */
int SH(void) { return GetScreenHeight(); }

/* Is the point p inside the rectangle r? (used for button clicks) */
bool HitRect(Vector2 p, Rectangle r) { return CheckCollisionPointRec(p, r); }

/* Draw text centered horizontally on the screen. */
void DrawCentered(const char *text, int y, int size, Color color)
{
    DrawText(text, (SW() - MeasureText(text, size)) / 2, y, size, color);
}

/* Draw text so that it ends at x = rightX (right aligned). */
void DrawRight(const char *text, int rightX, int y, int size, Color color)
{
    DrawText(text, rightX - MeasureText(text, size), y, size, color);
}

/* A point "fwd" ahead and "side" to the side of center, rotated by heading.
Used to draw rotated shapes (player, enemies, banana). */
Vector2 Local(Vector2 center, float heading, float fwd, float side)
{
    float c = cosf(heading), s = sinf(heading);
    return V2(center.x + fwd * c - side * s, center.y + fwd * s + side * c);
}


/* ==================================================================
 * SECTION 3: INPUT HELPERS
 * Reading touch and mouse presses.
 * ================================================================== */

/* Phone touch: left half of the screen moves the player, right half aims.
   Returns the movement direction. Sets touchAiming and player.facing for aiming. */
Vector2 ReadTouch(void)
{
    Vector2 move = V2(0, 0);
    touchAiming = false;
    for (int i = 0; i < GetTouchPointCount(); i++) {
        Vector2 touch = GetTouchPosition(i);
        if (touch.x < SW() * 0.47f) {
            /* left side: the farther you drag from the circle, the faster you move */
            Vector2 drag = Vector2Scale(Vector2Subtract(touch, V2(105, SH() - 126)), 1.0f / 62);
            if (Vector2Length(drag) > 1) drag = Vector2Normalize(drag);
            move = Vector2Add(move, drag);
        } else {
            /* right side: aim toward the finger */
            Vector2 aim = Vector2Normalize(Vector2Subtract(touch, player.pos));
            if (Vector2LengthSqr(aim) > 0.1f) { player.facing = aim; touchAiming = true; }
        }
    }
    return move;
}

/* Once per frame: remember whether the mouse/touch was just pressed, and where. */
void PollPointer(void)
{
    bool touchDown = GetTouchPointCount() > 0;
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    pointerPressed = mousePressed || (touchDown && !touchWasDown);
    pointerPos = (touchDown && !mousePressed) ? GetTouchPosition(0) : GetMousePosition();
    touchWasDown = touchDown;
}


/* ==================================================================
 * SECTION 4: SOUND AND SAVED SCORES
 * Loading/playing sound effects and saving the best scores to a file.
 * ================================================================== */

/* Start the sound system (browsers only allow sound after a click). */
void StartAudioIfNeeded(void)
{
    if (audioReady) return;
    InitAudioDevice();
    audioReady = IsAudioDeviceReady();
    if (audioReady) {
        const char *files[SFX_COUNT] = { "assets/shot.wav", "assets/impact.wav", "assets/pickup.wav",
                                         "assets/enemy_down.wav", "assets/surge.wav" };
        for (int i = 0; i < SFX_COUNT; i++) sounds[i] = LoadSound(files[i]);
    }
}

/* Play sound effect number id. */
void PlaySfx(int id)
{
    if (audioReady && IsSoundValid(sounds[id])) PlaySound(sounds[id]);
}

#if defined(__EMSCRIPTEN__)
#define SCORE_FILE "/persist/rift_runner_bests.txt"   /* web build */
#else
#define SCORE_FILE "rift_runner_bests.txt"            /* desktop build */
#endif

/* Write the 3 best scores into the score file. */
void SaveBestScores(void)
{
    FILE *f = fopen(SCORE_FILE, "w");
    if (!f) return;
    fprintf(f, "%d %d %d\n", bestScores[0], bestScores[1], bestScores[2]);
    fclose(f);
#if defined(__EMSCRIPTEN__)
    /* In a browser, copy the file into permanent storage. */
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(error) { if (error) console.warn('score save failed', error); });
    }, 0);
#endif
}

/* Read the 3 best scores from the score file (if it exists). */
void LoadBestScores(void)
{
    FILE *f = fopen(SCORE_FILE, "r");
    if (!f) return;
    if (fscanf(f, "%d %d %d", &bestScores[0], &bestScores[1], &bestScores[2]) != 3) {
        for (int i = 0; i < 3; i++) bestScores[i] = -1;
    }
    for (int i = 0; i < 3; i++) if (bestScores[i] < -1) bestScores[i] = -1;
    fclose(f);
}


/* ==================================================================
 * SECTION 5: PARTICLES AND SCREEN EFFECTS
 * Small glowing dots for explosions, plus the timers for shake/flash.
 * ================================================================== */

/* Create one particle in the first free slot. */
void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i] = (Particle){ true, pos, vel, radius, life, life, color };
            return;
        }
    }
}

/* An explosion: "count" particles flying out in random directions. */
void Burst(Vector2 pos, Color color, int count, float speed, float radius)
{
    for (int i = 0; i < count; i++) {
        float angle = RandF(0, 2 * PI);
        float spd = RandF(speed * 0.35f, speed);
        AddParticle(pos, V2(cosf(angle) * spd, sinf(angle) * spd),
                    RandF(radius * 0.55f, radius), RandF(0.24f, 0.72f), color);
    }
}

/* Move particles and count the effect timers (shake, flash, notices) down. */
void UpdateEffects(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].life -= dt;
        if (particles[i].life <= 0) { particles[i].active = false; continue; }
        particles[i].pos = Vector2Add(particles[i].pos, Vector2Scale(particles[i].vel, dt));
        particles[i].vel = Vector2Scale(particles[i].vel, 1.0f - fminf(2.8f * dt, 0.22f));   /* slow down */
        particles[i].radius *= 1.0f - fminf(0.5f * dt, 0.06f);                     /* shrink */
    }
    /* Timers that count down to zero. */
    screenShake = fmaxf(0, screenShake - dt);
    flashAlpha = fmaxf(0, flashAlpha - 1.75f * dt);
    stageNotice = fmaxf(0, stageNotice - dt);
    backgroundTransition = fmaxf(0, backgroundTransition - dt);
}


/* ==================================================================
 * SECTION 6: BACKGROUND
 * The space scene. There are 5 different scenes (one per 1000 score).
 * ================================================================== */

/* Name of the current scene (shown on the HUD). */
const char *SectorName(void)
{
    const char *names[5] = { "THE CRADLE", "GRAVEYARD BELT", "THE RIFT", "BLOOD MOON", "VOID CATHEDRAL" };
    return names[backgroundTheme % 5];
}

/* A soft glowing, slowly moving cloud. */
void Nebula(float x, float y, float radius, Color color, float drift)
{
    Vector2 p = V2(x + sinf(worldTime * 0.035f + drift) * 24, y + cosf(worldTime * 0.027f + drift) * 15);
    DrawCircleGradient(p, radius, Fade(color, 0.22f), Fade(color, 0));
}

/* Draw the special objects of the current scene (planet, asteroids, rings, moon, spires). */
void DrawEnvironment(int theme, int w, int h)
{
    switch (theme) {
    case 0: /* THE CRADLE: violet nebula and a ringed planet */
        Nebula(w * 0.28f, h * 0.40f, h * 0.56f, C_PURPLE, 0.4f);
        Nebula(w * 0.60f, h * 0.72f, h * 0.38f, C_CYAN, 2.1f);
        DrawCircleGradient(V2(w * 0.79f, h * 0.25f), h * 0.105f, (Color){ 43, 50, 91, 255 }, (Color){ 26, 30, 55, 255 });
        DrawEllipseLines((int)(w * 0.79f), (int)(h * 0.25f), h * 0.17f, h * 0.05f, Fade(C_CYAN, 0.7f));
        break;
    case 1: /* GRAVEYARD BELT: big slow asteroids */
        Nebula(w * 0.52f, h * 0.34f, h * 0.52f, C_BLUE, 1.0f);
        DrawPoly(V2(w * 0.13f, h * 0.24f), 7, h * 0.09f, 18 + worldTime, (Color){ 47, 51, 79, 255 });
        DrawPoly(V2(w * 0.89f, h * 0.60f), 7, h * 0.125f, -26 + worldTime, (Color){ 34, 39, 61, 255 });
        DrawPoly(V2(w * 0.67f, h * 0.11f), 7, h * 0.045f, 42 + worldTime * 0.25f, (Color){ 47, 51, 79, 255 });
        DrawPoly(V2(w * 0.39f, h * 0.86f), 7, h * 0.06f, -9 - worldTime * 0.19f, (Color){ 34, 39, 61, 255 });
        break;
    case 2: /* THE RIFT: rings around a dark void */
        Nebula(w * 0.50f, h * 0.48f, h * 0.70f, C_PURPLE, 1.9f);
        Nebula(w * 0.47f, h * 0.54f, h * 0.46f, C_PINK, 0.7f);
        for (int i = 0; i < 7; i++) {
            float r = h * (0.13f + i * 0.085f);
            DrawCircleLines((int)(w * 0.5f + sinf(worldTime * 0.1f + i) * 13), (int)(h * 0.48f), r,
                            Fade(i % 2 ? C_CYAN : C_PURPLE, 0.08f + 0.035f * (i % 3)));
        }
        DrawCircle((int)(w * 0.5f), (int)(h * 0.48f), h * 0.115f, (Color){ 2, 3, 15, 255 });
        DrawCircleLines((int)(w * 0.5f), (int)(h * 0.48f), h * 0.115f, Fade(C_PINK, 0.25f));
        break;
    case 3: /* BLOOD MOON: red cratered moon and dark spikes along the bottom */
        Nebula(w * 0.72f, h * 0.65f, h * 0.62f, C_RED, 0.2f);
        Nebula(w * 0.28f, h * 0.24f, h * 0.35f, C_PURPLE, 2.7f);
        DrawCircle((int)(w * 0.78f), (int)(h * 0.67f), h * 0.22f, (Color){ 67, 30, 56, 255 });
        for (int i = 0; i < 9; i++) {
            float a = i * 2.4f + 0.6f, d = h * 0.22f * (0.22f + 0.05f * (i % 9));
            DrawCircle((int)(w * 0.78f + cosf(a) * d), (int)(h * 0.67f + sinf(a) * d),
                       h * 0.22f * (0.035f + 0.022f * (i % 3)), (Color){ 40, 15, 35, 255 });
        }
        DrawCircleLines((int)(w * 0.78f), (int)(h * 0.67f), h * 0.22f, Fade((Color){ 248, 85, 135, 255 }, 0.44f));
        for (int i = 0; i < 8; i++) {
            float x = i * (float)w / 7;
            DrawTriangle(V2(x - w * 0.045f, h), V2(x + w * 0.015f, h - (i % 3 + 1) * h * 0.045f),
                         V2(x + w * 0.06f, h), (Color){ 10, 10, 27, 255 });
        }
        break;
    default: /* VOID CATHEDRAL: dark spires */
        Nebula(w * 0.50f, h * 0.50f, h * 0.66f, C_BLUE, 0.8f);
        Nebula(w * 0.80f, h * 0.29f, h * 0.34f, C_CYAN, 2.4f);
        for (int i = 0; i < 12; i++) {
            float x = i * (float)w / 11, tall = (2 + (i * 7) % 6) * h * 0.075f, half = w * 0.02f;
            DrawTriangle(V2(x, h - tall), V2(x - half, h), V2(x + half, h), (Color){ 11, 13, 34, 255 });
            DrawLineEx(V2(x, h - tall), V2(x - half, h), 1.3f, Fade(C_CYAN, 0.2f));
            DrawLineEx(V2(x, h - tall), V2(x + half, h), 1.3f, Fade(C_PURPLE, 0.2f));
        }
        break;
    }
}

/* Draw the sky gradient, the scene, the twinkling stars and the scene-change effect. */
void DrawBackground(void)
{
    int w = SW(), h = SH();
    Color top[5] = { { 16, 14, 42, 255 }, { 9, 19, 36, 255 }, { 19, 8, 38, 255 }, { 35, 9, 28, 255 }, { 5, 15, 34, 255 } };
    Color bottom[5] = { { 4, 6, 19, 255 }, { 3, 7, 19, 255 }, { 4, 3, 17, 255 }, { 13, 3, 19, 255 }, { 2, 4, 16, 255 } };
    int theme = backgroundTheme % 5;

    ClearBackground(bottom[theme]);
    DrawRectangleGradientV(0, 0, w, h, top[theme], bottom[theme]);
    DrawEnvironment(theme, w, h);

    /* Twinkling stars that drift slowly. */
    for (int i = 0; i < MAX_STARS; i++) {
        float x = fmodf(stars[i].x + worldTime * stars[i].speed * (i % 3 == 0 ? 0.11f : 0.025f), (float)w);
        float y = fmodf(stars[i].y + worldTime * stars[i].speed * (i % 3 == 0 ? 0.17f : 0.045f), (float)h);
        float pulse = 0.58f + 0.42f * sinf(worldTime * 1.4f + stars[i].phase);
        Color c = (i % 11 == 0) ? C_CYAN : ((i % 13 == 0) ? C_PURPLE : C_TEXT);
        DrawCircleV(V2(x, y), stars[i].size * pulse, Fade(c, 0.28f + 0.48f * pulse));
    }

    /* Scene-change effect: a dark veil and a scanning line sweeping across. */
    if (backgroundTransition > 0) {
        float amount = Clamp(backgroundTransition / 0.9f, 0, 1);
        float sweepX = (1 - amount) * w;
        DrawRectangle(0, 0, w, h, (Color){ 2, 2, 12, (unsigned char)(72 * amount) });
        DrawLineEx(V2(sweepX, 0), V2(sweepX, (float)h), 2, (Color){ 92, 225, 255, (unsigned char)(58 * amount) });
    }
}


/* ==================================================================
 * SECTION 7: DRAWING THE PLAYER, ENEMIES AND PICKUPS
 * Everything in the arena is drawn from simple shapes (circles, triangles, lines).
 * ================================================================== */

/* Draw the player: body, legs, arms, blaster, head and the shield rings. */
void DrawRunner(void)
{
    float a = atan2f(player.facing.y, player.facing.x);   /* direction the runner faces */
    bool blinking = player.invulnerable > 0 && ((int)(worldTime * 22) % 2 == 0);
    Color armor = (player.shieldTime > 0) ? C_GOLD : C_TEXT;
    if (blinking) armor = Fade(armor, 0.35f);
    float stride = (Vector2LengthSqr(player.velocity) > 80) ? sinf(player.animation) * 3 : 0;

    /* Key body points, all calculated relative to position + facing. */
    Vector2 head = Local(player.pos, a, 9, 0);
    Vector2 shoulderL = Local(player.pos, a, 3, 8), shoulderR = Local(player.pos, a, 3, -8);
    Vector2 hipL = Local(player.pos, a, -8, 5.5f), hipR = Local(player.pos, a, -8, -5.5f);
    Vector2 footL = Local(player.pos, a, -17 + stride, 9), footR = Local(player.pos, a, -17 - stride, -9);
    Vector2 gunBase = Local(player.pos, a, 2.5f, -8), gunTip = Local(player.pos, a, 19.5f, -8);

    DrawCircleGradient(player.pos, 36, (Color){ 63, 220, 255, 49 }, (Color){ 63, 220, 255, 0 });

    /* Shield rings. */
    if (player.shieldTime > 0) {
        DrawCircleLines((int)player.pos.x, (int)player.pos.y, 27 + sinf(worldTime * 8) * 2, C_GOLD);
        DrawCircleLines((int)player.pos.x, (int)player.pos.y, 31, Fade(C_GOLD, 0.5f));
    }
    /* Legs and boots. */
    DrawLineEx(hipL, footL, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(hipR, footR, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(footL, Local(footL, a, 4, 0), 5.2f, C_PURPLE);
    DrawLineEx(footR, Local(footR, a, 4, 0), 5.2f, C_PURPLE);
    /* Chest armour (two triangles). */
    DrawTriangle(shoulderL, shoulderR, hipR, armor);
    DrawTriangle(shoulderL, hipR, hipL, (Color){ 44, 77, 133, 255 });
    DrawLineEx(shoulderL, shoulderR, 2, C_CYAN);
    /* Arms and blaster. */
    DrawLineEx(Local(player.pos, a, 0, 6.5f), Local(player.pos, a, -1, 14), 3.4f, C_BLUE);
    DrawLineEx(Local(player.pos, a, 0, -6.5f), gunBase, 4.1f, (Color){ 108, 165, 255, 255 });
    DrawLineEx(gunBase, gunTip, 4, C_CYAN);
    DrawCircleV(gunTip, 3, C_TEXT);
    /* Head with visor. */
    DrawCircleV(head, 6.4f, armor);
    DrawCircleLines((int)head.x, (int)head.y, 6.4f, C_CYAN);
    DrawLineEx(Local(head, a, 1, 0), Local(head, a, 5, 0), 3.1f, (Color){ 21, 44, 80, 255 });
}

/* Draw one enemy (red or blue): glow, legs, body, eyes and health bar. */
void DrawEnemy(Enemy e)
{
    float r = e.radius * (1 + 0.045f * sinf(worldTime * (e.blue ? 5.6f : 3.2f) + e.pos.x * 0.02f));
    float h = e.heading;
    Color base = e.blue ? (Color){ 34, 113, 175, 255 } : (Color){ 123, 27, 48, 255 };
    Color glow = e.blue ? C_CYAN : (Color){ 255, 62, 103, 255 };
    Color dark = e.blue ? (Color){ 11, 27, 59, 255 } : (Color){ 40, 14, 35, 255 };
    Color body = (e.flash > 0) ? C_GOLD : base;   /* flashes gold when hit */

    DrawCircleGradient(e.pos, r * 1.9f, Fade(glow, 0.25f), Fade(glow, 0));

    /* Wavy legs ("tendrils") on both sides: red has 3 per side, blue has 2. */
    int legs = e.blue ? 2 : 3;
    for (int i = 0; i < legs; i++) {
        for (int side = -1; side <= 1; side += 2) {
            float sway = sinf(worldTime * (e.blue ? 4.7f : 3.1f) + i * 1.4f + side + e.pos.x * 0.01f);
            Vector2 root = Local(e.pos, h, -r * (0.18f + 0.16f * i), side * r * 0.3f);
            Vector2 mid = Local(e.pos, h, -r * (0.4f + 0.16f * i), side * r * 0.9f);
            Vector2 tip = Local(e.pos, h, -r * (0.9f + 0.1f * i), side * (r * (1 + 0.2f * i) + sway * r * 0.18f));
            DrawLineEx(root, mid, e.blue ? 3 : 4.5f, dark);
            DrawLineEx(mid, tip, e.blue ? 1.8f : 2.7f, dark);
            DrawCircleV(tip, 2, glow);
        }
    }

    /* Pointed body made of triangles: nose, top, bottom, tail, around a core point. */
    Vector2 nose = Local(e.pos, h, r * 1.1f, 0);
    Vector2 top = Local(e.pos, h, r * 0.15f, -r * 0.55f);
    Vector2 bottom = Local(e.pos, h, r * 0.15f, r * 0.55f);
    Vector2 tail = Local(e.pos, h, -r * 0.85f, 0);
    Vector2 core = Local(e.pos, h, -r * 0.15f, 0);
    DrawTriangle(nose, top, core, body);
    DrawTriangle(nose, core, bottom, body);
    DrawTriangle(top, tail, core, dark);
    DrawTriangle(bottom, core, tail, dark);
    DrawLineEx(Local(e.pos, h, r * 0.25f, 0), Local(e.pos, h, r * 0.8f, 0), r * 0.1f, glow);

    /* Eyes: red has two, blue has one. */
    if (e.blue) {
        DrawCircleV(Local(e.pos, h, r * 0.3f, 0), r * 0.11f, glow);
    } else {
        DrawCircleV(Local(e.pos, h, r * 0.23f, -r * 0.23f), r * 0.11f, (Color){ 255, 83, 104, 255 });
        DrawCircleV(Local(e.pos, h, r * 0.23f, r * 0.23f), r * 0.11f, (Color){ 255, 83, 104, 255 });
    }

    /* Health bar, only after the enemy has been damaged. */
    if (e.hp < e.maxHp) {
        float barW = r * 2.2f;
        DrawRectangleRounded((Rectangle){ e.pos.x - barW / 2, e.pos.y - r - 9, barW, 4 }, 0.5f, 4, (Color){ 20, 25, 46, 230 });
        DrawRectangleRounded((Rectangle){ e.pos.x - barW / 2, e.pos.y - r - 9, barW * Clamp(e.hp / e.maxHp, 0, 1), 4 }, 0.5f, 4, C_GOLD);
    }
}

/* Draw the banana pickup as a golden curved shape. */
void DrawBanana(Pickup p, float r)
{
    float angle = -0.48f + 0.10f * sinf(p.phase * 0.5f);
    Vector2 outer[13], inner[13];
    for (int i = 0; i <= 12; i++) {
        float t = i / 12.0f, curve = sinf(PI * t);
        float x = (t - 0.5f) * 2 * r;
        float yOuter = -0.10f * r - 0.52f * r * curve;
        outer[i] = Local(p.pos, angle, x, yOuter);
        inner[i] = Local(p.pos, angle, x, yOuter + 0.34f * r * curve);
    }
    DrawCircleGradient(p.pos, r * 2.5f, Fade(C_GOLD, 0.32f), Fade(C_GOLD, 0));
    for (int i = 0; i < 12; i++) {
        DrawTriangle(outer[i], outer[i + 1], inner[i + 1], (Color){ 255, 191, 54, 255 });
        DrawTriangle(outer[i], inner[i + 1], inner[i], (Color){ 255, 226, 104, 255 });
        DrawLineEx(outer[i], outer[i + 1], 1.7f, (Color){ 255, 246, 173, 255 });
    }
}

/* Draw all bananas, bullets, enemies, particles and the player. */
void DrawWorld(void)
{
    for (int i = 0; i < MAX_PICKUPS; i++)
        if (pickups[i].active) DrawBanana(pickups[i], 12 * (1 + sinf(pickups[i].phase * 1.6f) * 0.08f));

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        Vector2 tail = Vector2Subtract(bullets[i].pos, Vector2Scale(Vector2Normalize(bullets[i].vel), 16));
        DrawLineEx(tail, bullets[i].pos, 3, Fade(C_CYAN, 0.45f));
        DrawCircleGradient(bullets[i].pos, 10, Fade(C_CYAN, 0.56f), Fade(C_CYAN, 0));
        DrawCircleV(bullets[i].pos, bullets[i].radius, C_CYAN);
    }

    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active) DrawEnemy(enemies[i]);

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        float a = Clamp(particles[i].life / particles[i].fullLife, 0, 1);   /* 1 = fresh, 0 = gone */
        DrawCircleV(particles[i].pos, particles[i].radius * (0.36f + 0.64f * a), Fade(particles[i].color, a));
    }

    DrawRunner();
}


/* ==================================================================
 * SECTION 8: BUTTON AND PANEL POSITIONS
 * Rectangles for every button. They are used both to draw the button
 * and to check if the mouse/touch clicked it.
 * ================================================================== */

/* Draw a dark rounded panel with a colored border. */
void DrawPanel(Rectangle r, Color accent)
{
    DrawRectangleRounded(r, 0.12f, 12, C_PANEL);
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 1, Fade(accent, 0.48f));
}

/* Draw a clickable button; it lights up when selected or hovered. */
void DrawButton(Rectangle r, const char *label, Color accent, bool selected)
{
    bool hover = HitRect(GetMousePosition(), r);
    DrawRectangleRounded(r, 0.16f, 10, (selected || hover) ? Fade(accent, 0.16f) : C_PANEL);
    DrawRectangleRoundedLinesEx(r, 0.16f, 10, (selected || hover) ? 2 : 1,
                                (selected || hover) ? accent : (Color){ 67, 82, 126, 220 });
    int size = 18;
    while (size > 10 && MeasureText(label, size) > (int)r.width - 22) size--;   /* shrink to fit */
    DrawText(label, (int)(r.x + (r.width - MeasureText(label, size)) / 2),
             (int)(r.y + (r.height - size) / 2 - 1), size, (selected || hover) ? C_TEXT : C_MUTED);
}

/* Position of difficulty card number i on the title screen. */
Rectangle DifficultyCard(int i)
{
    float gap = 16, cardW = fminf(218, (SW() - 40 - gap * 2) / 3);
    float start = (SW() - (cardW * 3 + gap * 2)) / 2;
    return (Rectangle){ start + i * (cardW + gap), SH() * 0.455f, cardW, 104 };
}

/* Position of the "ENTER THE RIFT" button. */
Rectangle StartBounds(void)
{
    float w = fminf(300, SW() - 36.0f);
    return (Rectangle){ SW() / 2 - w / 2, SH() * 0.635f, w, 57 };
}

/* Position of the "VIEW PERSONAL BESTS" text on the title screen. */
Rectangle TitleScoresLink(void) { return (Rectangle){ SW() / 2 - 110.0f, SH() * 0.715f, 220, 20 }; }

/* Position of the back button on the scores screen. */
Rectangle ScoresBack(void)
{
    float w = fminf(220, SW() - 36.0f);
    return (Rectangle){ SW() / 2 - w / 2, SH() * 0.80f, w, 42 };
}

/* Position of the resume button on the pause screen. */
Rectangle PauseResume(void)
{
    float w = fminf(300, SW() - 36.0f);
    return (Rectangle){ SW() / 2 - w / 2, SH() * 0.48f, w, 52 };
}

/* Position of the "return to flight deck" button on the pause screen. */
Rectangle PauseTitle(void)
{
    float w = fminf(300, SW() - 36.0f);
    return (Rectangle){ SW() / 2 - w / 2, SH() * 0.57f, w, 42 };
}

/* Position of the personal-bests text on the pause screen. */
Rectangle PauseScoresLink(void) { return (Rectangle){ SW() / 2 - 110.0f, SH() * 0.64f, 220, 20 }; }

/* Position of the whole game-over box. */
Rectangle GameOverPanel(void)
{
    float w = fminf(476, SW() - 24.0f), h = fminf(410, SH() - 24.0f);
    return (Rectangle){ SW() / 2 - w / 2, SH() / 2 - h / 2, w, h };
}

/* Position of the "RUN AGAIN" button. */
Rectangle GameOverAgain(void)
{
    Rectangle p = GameOverPanel();
    return (Rectangle){ p.x + 24, p.y + p.height * 0.64f, p.width - 48, 50 };
}

/* Position of the personal-bests text on the game-over box. */
Rectangle GameOverScores(void)
{
    Rectangle p = GameOverPanel();
    return (Rectangle){ p.x + 24, p.y + p.height * 0.80f, p.width - 48, 20 };
}

/* Position of the "return to flight deck" text on the game-over box. */
Rectangle GameOverTitle(void)
{
    Rectangle p = GameOverPanel();
    return (Rectangle){ p.x + 40, p.y + p.height * 0.90f, p.width - 80, 20 };
}

/* Draw small clickable text that turns cyan when the mouse is over it. */
void DrawLink(const char *text, Rectangle r, int size)
{
    DrawCentered(text, (int)r.y + 3, size, HitRect(GetMousePosition(), r) ? C_CYAN : C_MUTED);
}


/* ==================================================================
 * SECTION 9: SCREENS
 * Drawing the HUD (during play) and the title, scores, game-over and pause screens.
 * ================================================================== */

/* The top bar while playing: score, level/sector progress, health bar, messages, shield timer. */
void DrawHUD(void)
{
    char buf[112];
    int w = SW(), h = SH();
    float healthRatio = (player.maxHealth > 0) ? Clamp(player.health / player.maxHealth, 0, 1) : 0;
    Color healthColor = (healthRatio > 0.5f) ? C_GREEN : ((healthRatio > 0.25f) ? C_GOLD : C_RED);
    float sectorProgress = (score % 1000) / 1000.0f;

    /* Left: score. */
    DrawPanel((Rectangle){ 18, 16, 248, 90 }, C_CYAN);
    DrawText("SCORE", 34, 27, 12, C_MUTED);
    snprintf(buf, sizeof(buf), "%07d", score);
    DrawText(buf, 31, 43, 31, C_TEXT);
    DrawText("SURVIVE THE RIFT", 34, 80, 10, C_CYAN);

    /* Middle: level, sector name and progress bar. */
    float sx = w * 0.5f - 144;
    DrawPanel((Rectangle){ sx, 16, 288, 90 }, C_PURPLE);
    snprintf(buf, sizeof(buf), "RIFT LEVEL  %02d", stage);
    DrawCentered(buf, 25, 15, C_TEXT);
    snprintf(buf, sizeof(buf), "SECTOR %02d  //  %s", backgroundTheme + 1, SectorName());
    DrawCentered(buf, 47, 10, C_CYAN);
    DrawRectangleRounded((Rectangle){ sx + 24, 70, 240, 6 }, 0.5f, 6, (Color){ 35, 45, 75, 235 });
    if (sectorProgress > 0) DrawRectangleRounded((Rectangle){ sx + 24, 70, 240 * sectorProgress, 6 }, 0.5f, 6, C_CYAN);
    snprintf(buf, sizeof(buf), "%d%% TO NEXT ENVIRONMENT", (int)(sectorProgress * 100));
    DrawCentered(buf, 82, 9, C_MUTED);

    /* Right: health bar. */
    DrawPanel((Rectangle){ w - 242.0f, 16, 224, 90 }, diffColor[difficulty]);
    DrawText(diffName[difficulty], w - 224, 27, 12, diffColor[difficulty]);
    DrawText("INTEGRITY", w - 224, 47, 10, C_MUTED);
    DrawRectangleRounded((Rectangle){ w - 224.0f, 63, 188, 10 }, 0.5f, 6, (Color){ 38, 47, 74, 255 });
    if (healthRatio > 0) DrawRectangleRounded((Rectangle){ w - 224.0f, 63, 188 * healthRatio, 10 }, 0.5f, 6, healthColor);
    snprintf(buf, sizeof(buf), "%d%%", (int)(healthRatio * 100));
    DrawRight(buf, w - 36, 79, 11, C_TEXT);

    /* Event messages (sector shift has priority over level-up). */
    if (backgroundTransition > 0.02f) {
        snprintf(buf, sizeof(buf), "SECTOR SHIFT  //  %s", SectorName());
        DrawCentered(buf, (int)(0.205f * h), 18, Fade(C_CYAN, Clamp(backgroundTransition / 0.55f, 0, 1)));
    } else if (stageNotice > 0) {
        DrawCentered("RIFT LEVEL INCREASED  //  PRESSURE RISING", (int)(0.205f * h), 15,
                     Fade(C_GOLD, Clamp(stageNotice * 1.7f, 0, 1)));
    }

    if (player.shieldTime > 0) {
        snprintf(buf, sizeof(buf), "SHIELD  %.1fs", player.shieldTime);
        DrawText(buf, 24, h - 30, 13, C_GOLD);
    }
}

/* The title screen: name, three difficulty cards, start button and help text. */
void DrawTitle(void)
{
    int w = SW(), h = SH();
    int titleSize = (w < 440) ? 32 : ((w < 720) ? 40 : 57);

    DrawCentered("RIFT NETWORK  //  FLIGHT SYSTEMS ONLINE", (int)(h * 0.105f), 11, C_CYAN);
    DrawCentered("RIFT RUNNER", (int)(h * 0.165f) + 3, titleSize, (Color){ 65, 97, 150, 110 });   /* shadow */
    DrawCentered("RIFT RUNNER", (int)(h * 0.165f), titleSize, C_TEXT);
    DrawCentered("SIGNAL ZERO  *  SURVIVE THE SIGNAL. OUTLAST THE VOID.", (int)(h * 0.265f), 12, C_MUTED);
    DrawCentered("SELECT YOUR PRESSURE LEVEL", (int)(h * 0.395f), 12, C_CYAN);

    /* Three difficulty cards. */
    for (int i = 0; i < 3; i++) {
        Rectangle r = DifficultyCard(i);
        bool selected = (menuSelection == i);
        DrawRectangleRounded(r, 0.12f, 10, selected ? (Color){ 25, 34, 69, 245 } : C_PANEL);
        DrawRectangleRoundedLinesEx(r, 0.12f, 10, selected ? 2 : 1, selected ? diffColor[i] : (Color){ 60, 73, 112, 220 });
        DrawText(diffName[i], (int)(r.x + (r.width - MeasureText(diffName[i], 19)) / 2), (int)r.y + 18, 19, diffColor[i]);
        DrawText(diffTagline[i], (int)r.x + 12, (int)r.y + 54, 11, C_MUTED);
        DrawText(TextFormat("%d", i + 1), (int)(r.x + r.width - 25), (int)r.y + 10, 11, selected ? C_TEXT : C_MUTED);
        if (selected) DrawRectangleRounded((Rectangle){ r.x + 16, r.y + r.height - 12, r.width - 32, 3 }, 0.5f, 5, diffColor[i]);
    }

    DrawButton(StartBounds(), "ENTER THE RIFT  [ENTER]", C_CYAN, true);
    DrawLink("VIEW PERSONAL BESTS  [B]", TitleScoresLink(), 10);

    DrawCentered("WASD / ARROWS MOVE     *     MOUSE TO AIM     *     AUTO-FIRE", (int)(h * 0.78f), 12, C_TEXT);
    DrawCentered("P / ESC  PAUSE DURING A RUN     *     HOLD MOUSE TO AIM", (int)(h * 0.825f), 11, C_MUTED);
#if defined(__EMSCRIPTEN__)
    DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE", (int)(h * 0.865f), 10, (Color){ 111, 128, 176, 255 });
#else
    DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE     *     ESC QUIT", (int)(h * 0.865f), 10, (Color){ 111, 128, 176, 255 });
#endif
    DrawCentered("PERSONAL BESTS ARE SAVED ON THIS DEVICE", (int)(h * 0.91f), 9, (Color){ 85, 103, 149, 255 });
}

/* The personal-bests screen. */
void DrawScores(void)
{
    int w = SW(), h = SH();
    DrawCentered("PERSONAL BESTS", (int)(h * 0.10f), (w < 440) ? 21 : 30, C_TEXT);
    DrawCentered("ONE HIGH SCORE PER MODE  -  SAVED ON THIS DEVICE", (int)(h * 0.16f), 11, C_MUTED);

    float panelW = fminf(700, w * 0.9f), rowH = fminf(76, h * 0.105f), panelH = rowH * 3 + 54;
    Rectangle panel = { (w - panelW) / 2, (h - panelH) * 0.51f, panelW, panelH };
    DrawPanel(panel, C_PURPLE);

    for (int i = 0; i < 3; i++) {
        Rectangle row = { panel.x + 12, panel.y + 18 + i * rowH, panel.width - 24, rowH - 7 };
        DrawRectangleRounded(row, 0.16f, 8, (Color){ 21, 29, 59, 235 });
        DrawRectangleRoundedLinesEx(row, 0.16f, 8, 1.2f, Fade(diffColor[i], 0.65f));
        DrawText(diffName[i], (int)row.x + 16, (int)(row.y + row.height / 2 - 10), 16, diffColor[i]);
        DrawText("HIGH SCORE", (int)row.x + 170, (int)row.y + 9, 9, C_MUTED);
        if (bestScores[i] >= 0) {
            DrawRight(TextFormat("%07d", bestScores[i]), (int)(row.x + row.width - 16), (int)(row.y + row.height / 2 - 12), 24, C_TEXT);
        } else {
            DrawRight("N/A", (int)(row.x + row.width - 16), (int)(row.y + row.height / 2 - 12), 24, C_MUTED);
            DrawText("PLAY THIS MODE TO SET A RECORD", (int)row.x + 170, (int)row.y + 24, 9, C_MUTED);
        }
    }
    const char *label = "BACK TO FLIGHT DECK";
    if (scoresReturnState == STATE_PAUSED) label = "RETURN TO RUN";
    if (scoresReturnState == STATE_GAME_OVER) label = "RETURN TO RUN SUMMARY";
    DrawButton(ScoresBack(), label, C_CYAN, true);
    DrawCentered("B / ESC  BACK", (int)(h * 0.92f), 10, C_MUTED);
}

/* The game-over box: final score and buttons. */
void DrawGameOver(void)
{
    int w = SW(), h = SH();
    Rectangle p = GameOverPanel();
    float k = p.height / 410;     /* scale the layout if the window is short */
    char buf[100];

    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 190 });
    DrawPanel(p, C_PINK);
    DrawCentered("RUN TERMINATED", (int)(p.y + 31 * k), 25, C_PINK);
    DrawCentered("FINAL SCORE", (int)(p.y + 83 * k), 12, C_MUTED);
    snprintf(buf, sizeof(buf), "%07d", score);
    DrawCentered(buf, (int)(p.y + 105 * k), (w < 440) ? 38 : 51, C_TEXT);
    snprintf(buf, sizeof(buf), "%s    /    RIFT LEVEL %02d", diffName[difficulty], stage);
    DrawCentered(buf, (int)(p.y + 167 * k), 12, diffColor[difficulty]);
    if (newPersonalBest) DrawCentered("NEW PERSONAL BEST FOR THIS MODE!", (int)(p.y + 207 * k), 15, C_GOLD);
    else DrawCentered("MODE BEST: SCORE NOT BEATEN", (int)(p.y + 207 * k), 11, C_MUTED);

    DrawButton(GameOverAgain(), "RUN AGAIN  [ENTER]", C_CYAN, true);
    DrawLink("VIEW PERSONAL BESTS  [B]", GameOverScores(), 10);
    DrawLink("RETURN TO FLIGHT DECK  [ESC]", GameOverTitle(), 9);
}

/* The pause box. */
void DrawPause(void)
{
    int w = SW(), h = SH();
    float panelW = fminf(420, w - 32.0f);
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 195 });
    DrawPanel((Rectangle){ w / 2 - panelW / 2, h * 0.32f, panelW, h * 0.42f }, C_CYAN);
    DrawCentered("PAUSED", (int)(h * 0.365f), 36, C_TEXT);
    DrawCentered("YOUR RUN IS ON HOLD", (int)(h * 0.43f), 10, C_MUTED);
    DrawButton(PauseResume(), "RESUME RUN  [P / ENTER]", C_CYAN, true);
    DrawButton(PauseTitle(), "RETURN TO FLIGHT DECK", C_PURPLE, false);
    DrawLink("VIEW PERSONAL BESTS  [B]", PauseScoresLink(), 10);
    DrawCentered("ESC ALSO RESUMES", (int)(h * 0.715f), 9, C_MUTED);
}

/* The on-screen circles that show the touch controls (phones/small windows). */
void DrawTouchControls(void)
{
    if (SW() > 820 && GetTouchPointCount() <= 0) return;
    Vector2 anchor = V2(105, SH() - 126.0f);
    DrawCircleLines((int)anchor.x, (int)anchor.y, 55, (Color){ 126, 206, 255, 95 });
    DrawCircleV(anchor, 22, (Color){ 77, 123, 195, 50 });
    DrawCircleLines(SW() - 84, SH() - 92, 39, (Color){ 126, 206, 255, 95 });
    DrawText("AIM", SW() - 111, SH() - 98, 11, C_CYAN);
}

/* The little crosshair that appears while the mouse button is held. */
void DrawAimingReticle(void)
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) return;
    if (GetTouchPointCount() > 0) return;
    Vector2 p = GetMousePosition();
    Color c = Fade(C_CYAN, 0.78f);
    DrawCircleLines((int)p.x, (int)p.y, 7, c);
    DrawLineEx(V2(p.x - 15, p.y), V2(p.x - 9, p.y), 1.2f, c);
    DrawLineEx(V2(p.x + 9, p.y), V2(p.x + 15, p.y), 1.2f, c);
    DrawLineEx(V2(p.x, p.y - 15), V2(p.x, p.y - 9), 1.2f, c);
    DrawLineEx(V2(p.x, p.y + 9), V2(p.x, p.y + 15), 1.2f, c);
    DrawCircleV(p, 1.5f, C_TEXT);
}

/* Draw one whole frame: background, world, the active screen, and the white flash. */
void DrawFrame(void)
{
    int w = SW(), h = SH();

    /* Screen shake: slide the camera a tiny bit using sine waves. */
    float sx = 0, sy = 0;
    if (screenShake > 0) {
        float amount = screenShake * 19;
        sx = (sinf(worldTime * 61) + 0.32f * sinf(worldTime * 103)) * amount * 0.62f;
        sy = (cosf(worldTime * 67) + 0.28f * sinf(worldTime * 89)) * amount * 0.62f;
    }
    Camera2D camera = { 0 };
    camera.offset = V2(w * 0.5f + sx, h * 0.5f + sy);
    camera.target = V2(w * 0.5f, h * 0.5f);
    camera.zoom = 1;

    BeginDrawing();
    BeginMode2D(camera);
    DrawBackground();
    if (state == STATE_PLAYING || state == STATE_PAUSED || state == STATE_GAME_OVER) {
        DrawWorld();
    } else {
        /* Decorative glowing rings behind the menus. */
        Vector2 c = V2(w * 0.5f, h * 0.52f);
        float pulse = 1 + sinf(worldTime * 1.2f) * 0.025f;
        DrawCircleGradient(c, 118 * pulse, (Color){ 75, 44, 194, 26 }, (Color){ 75, 44, 194, 0 });
        DrawCircleLines((int)c.x, (int)c.y, 80 * pulse, (Color){ 89, 222, 255, 90 });
        DrawCircleLines((int)c.x, (int)c.y, 100 * pulse, (Color){ 175, 91, 255, 68 });
        for (int i = 0; i < 10; i++) {
            float angle = worldTime * 0.55f + i * (2 * PI / 10);
            DrawCircleGradient(V2(c.x + cosf(angle) * 90, c.y + sinf(angle) * 90), 9, Fade(C_CYAN, 0.35f), (Color){ 76, 231, 255, 0 });
        }
    }
    EndMode2D();

    /* Screens drawn on top (not affected by shake). */
    switch (state) {
    case STATE_TITLE:     DrawTitle(); break;
    case STATE_PLAYING:   DrawHUD(); DrawTouchControls(); DrawAimingReticle(); break;
    case STATE_PAUSED:    DrawHUD(); DrawPause(); break;
    case STATE_GAME_OVER: DrawGameOver(); break;
    case STATE_SCORES:    DrawScores(); break;
    }

    /* White flash on big hits. */
    if (flashAlpha > 0)
        DrawRectangle(0, 0, w, h, (Color){ 188, 226, 255, (unsigned char)(Clamp(flashAlpha, 0, 0.4f) * 255) });
    EndDrawing();
}

