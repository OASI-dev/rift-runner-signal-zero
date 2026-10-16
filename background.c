/*
 * Procedural space environments.
 * Every 1,000 score, the scene changes its actual landmarks and geometry—not
 * only its colors. The transition is a brief quiet dim-and-reveal, not an
 * explosion or radial burst.
 */
#include "game.h"
#include <math.h>

const char *GetSectorName(void)
{
    static const char *names[5] = {
        "THE CRADLE", "GRAVEYARD BELT", "THE RIFT", "BLOOD MOON", "VOID CATHEDRAL"
    };
    int theme = g.backgroundTheme % 5;
    if (theme < 0) theme += 5;
    return names[theme];
}

static Color FadeColor(Color c, float opacity)
{
    c.a = (unsigned char)(ClampF(opacity, 0.0f, 1.0f) * (float)c.a);
    return c;
}

static void Nebula(Vector2 center, float radius, Color color, float drift)
{
    Vector2 p = V2(center.x + sinf(g.worldTime * 0.035f + drift) * 24.0f,
                   center.y + cosf(g.worldTime * 0.027f + drift) * 15.0f);
    DrawCircleGradient(p, radius, FadeColor(color, 0.22f), FadeColor(color, 0.0f));
    DrawCircleGradient(V2(p.x + radius * 0.18f, p.y - radius * 0.08f), radius * 0.62f,
                       FadeColor(color, 0.13f), FadeColor(color, 0.0f));
}

static void RingedPlanet(Vector2 p, float radius, Color body, Color ring)
{
    DrawCircleGradient(p, radius * 1.48f, FadeColor(ring, 0.12f), FadeColor(ring, 0.0f));
    /* A slanted-looking segmented ring, layered behind and in front of the planet. */
    for (int i = 0; i < 52; ++i) {
        float a0 = (float)i * (2.0f * PI_F / 52.0f);
        float a1 = (float)(i + 1) * (2.0f * PI_F / 52.0f);
        Vector2 a = V2(p.x + cosf(a0) * radius * 1.65f,
                       p.y + sinf(a0) * radius * 0.48f);
        Vector2 b = V2(p.x + cosf(a1) * radius * 1.65f,
                       p.y + sinf(a1) * radius * 0.48f);
        DrawLineEx(a, b, 2.2f, FadeColor(ring, 0.44f));
    }
    DrawCircleGradient(p, radius, body, FadeColor(body, 0.60f));
    DrawCircleV(V2(p.x - radius * 0.23f, p.y - radius * 0.16f), radius * 0.17f,
                FadeColor((Color){ 220, 230, 255, 255 }, 0.14f));
    DrawCircleLines((int)p.x, (int)p.y, radius, FadeColor(ring, 0.60f));
    for (int i = 0; i < 52; ++i) {
        float a0 = (float)i * (2.0f * PI_F / 52.0f);
        float a1 = (float)(i + 1) * (2.0f * PI_F / 52.0f);
        if (sinf((a0 + a1) * 0.5f) < -0.10f) continue;
        Vector2 a = V2(p.x + cosf(a0) * radius * 1.65f,
                       p.y + sinf(a0) * radius * 0.48f);
        Vector2 b = V2(p.x + cosf(a1) * radius * 1.65f,
                       p.y + sinf(a1) * radius * 0.48f);
        DrawLineEx(a, b, 2.0f, FadeColor(ring, 0.72f));
    }
}

static void DrawCraterMoon(Vector2 p, float radius, Color base, Color edge)
{
    DrawCircleGradient(p, radius * 1.30f, FadeColor(edge, 0.20f), FadeColor(edge, 0.0f));
    DrawCircleGradient(p, radius, base, FadeColor(base, 0.78f));
    for (int i = 0; i < 9; ++i) {
        float angle = (float)i * 2.39996f + 0.6f;
        float distance = radius * (0.22f + 0.44f * (float)((i * 7) % 9) / 9.0f);
        Vector2 c = V2(p.x + cosf(angle) * distance, p.y + sinf(angle) * distance);
        float cr = radius * (0.035f + 0.022f * (float)(i % 3));
        DrawCircleV(c, cr * 1.55f, FadeColor(edge, 0.23f));
        DrawCircleV(c, cr, FadeColor((Color){ 9, 8, 20, 255 }, 0.38f));
        DrawCircleLines((int)c.x, (int)c.y, cr, FadeColor(edge, 0.26f));
    }
    DrawCircleLines((int)p.x, (int)p.y, radius, FadeColor(edge, 0.44f));
}

static void DrawAsteroid(float x, float y, float radius, float rotation, int seed)
{
    Color rock = (seed % 2) ? (Color){ 47, 51, 79, 255 } : (Color){ 34, 39, 61, 255 };
    Color edge = (Color){ 105, 120, 157, 255 };
    DrawCircleGradient(V2(x, y), radius * 1.8f, FadeColor(C_PURPLE, 0.07f), FadeColor(C_PURPLE, 0.0f));
    DrawPoly(V2(x, y), 7, radius, rotation, rock);
    DrawPolyLines(V2(x, y), 7, radius, rotation, FadeColor(edge, 0.38f));
    for (int k = 0; k < 3; ++k) {
        float a = rotation * PI_F / 180.0f + (float)k * 2.1f;
        Vector2 c = V2(x + cosf(a) * radius * 0.34f, y + sinf(a) * radius * 0.30f);
        DrawCircleV(c, radius * (0.09f + 0.025f * (float)(k % 2)), FadeColor((Color){ 6, 9, 24, 255 }, 0.6f));
    }
}

static void DrawVoidSpire(float x, float baseY, float width, float height, float pulse)
{
    Vector2 tip = V2(x + sinf(g.worldTime * 0.16f + x) * width * 0.12f, baseY - height);
    Vector2 left = V2(x - width * 0.5f, baseY);
    Vector2 right = V2(x + width * 0.5f, baseY);
    DrawTriangle(tip, left, right, (Color){ 11, 13, 34, 255 });
    DrawTriangle(tip, V2(x, baseY - height * 0.62f), right, FadeColor(C_PURPLE, 0.12f));
    DrawLineEx(tip, left, 1.3f, FadeColor(C_CYAN, 0.15f + pulse * 0.08f));
    DrawLineEx(tip, right, 1.3f, FadeColor(C_PURPLE, 0.19f));
}

static void DrawEnvironment(int theme, int w, int h)
{
    Vector2 center = V2(w * 0.5f, h * 0.48f);
    switch (theme) {
        case 0: /* THE CRADLE — distant ringworld and a soft violet nebula. */
            Nebula(V2(w * 0.28f, h * 0.40f), (float)h * 0.56f, C_PURPLE, 0.4f);
            Nebula(V2(w * 0.60f, h * 0.72f), (float)h * 0.38f, C_CYAN, 2.1f);
            RingedPlanet(V2(w * 0.79f, h * 0.25f), (float)h * 0.105f,
                         (Color){ 43, 50, 91, 255 }, (Color){ 93, 226, 255, 255 });
            DrawCircleLines((int)(w * 0.22f), (int)(h * 0.72f), (float)h * 0.07f, FadeColor(C_PURPLE, 0.16f));
            break;
        case 1: /* GRAVEYARD BELT — huge slow debris crossing the starfield. */
            Nebula(V2(w * 0.52f, h * 0.34f), (float)h * 0.52f, C_BLUE, 1.0f);
            DrawAsteroid(w * 0.13f, h * 0.24f, h * 0.090f, 18.0f + sinf(g.worldTime * 0.05f) * 8.0f, 1);
            DrawAsteroid(w * 0.89f, h * 0.60f, h * 0.125f, -26.0f + sinf(g.worldTime * 0.04f) * 6.0f, 2);
            DrawAsteroid(w * 0.67f, h * 0.11f, h * 0.045f, 42.0f + g.worldTime * 0.25f, 3);
            DrawAsteroid(w * 0.39f, h * 0.86f, h * 0.060f, -9.0f - g.worldTime * 0.19f, 4);
            for (int i = 0; i < 9; ++i) {
                float x = (float)((i * 137 + 51) % 1000) / 1000.0f * w;
                float y = fmodf((float)((i * 193 + 71) % 700) / 700.0f * h + g.worldTime * (3.0f + i % 4), (float)h);
                DrawCircleV(V2(x, y), 1.5f + (float)(i % 3), FadeColor(C_MUTED, 0.25f));
            }
            break;
        case 2: /* THE RIFT — nested dimensional rings around a dark void. */
            Nebula(V2(w * 0.50f, h * 0.48f), (float)h * 0.70f, C_PURPLE, 1.9f);
            Nebula(V2(w * 0.47f, h * 0.54f), (float)h * 0.46f, C_PINK, 0.7f);
            for (int i = 0; i < 7; ++i) {
                float r = (float)h * (0.13f + (float)i * 0.085f);
                float phase = g.worldTime * (i % 2 ? 2.4f : -1.7f) + (float)i * 19.0f;
                DrawCircleLines((int)(center.x + sinf(phase * 0.08f) * 13.0f),
                                (int)(center.y + cosf(phase * 0.06f) * 9.0f), r,
                                FadeColor((i % 2) ? C_CYAN : C_PURPLE, 0.08f + 0.035f * (float)(i % 3)));
            }
            DrawCircleGradient(center, (float)h * 0.115f, (Color){ 2, 3, 15, 255 }, FadeColor(C_PINK, 0.08f));
            DrawCircleLines((int)center.x, (int)center.y, (float)h * 0.115f, FadeColor(C_PINK, 0.25f));
            break;
        case 3: /* BLOOD MOON — cold red moon with craters and drifting haze. */
            Nebula(V2(w * 0.72f, h * 0.65f), (float)h * 0.62f, C_RED, 0.2f);
            Nebula(V2(w * 0.28f, h * 0.24f), (float)h * 0.35f, C_PURPLE, 2.7f);
            DrawCraterMoon(V2(w * 0.78f, h * 0.67f), (float)h * 0.22f,
                           (Color){ 67, 30, 56, 255 }, (Color){ 248, 85, 135, 255 });
            for (int i = 0; i < 8; ++i) {
                float x = (float)i * (float)w / 7.0f;
                float spike = (float)(i % 3 + 1) * h * 0.045f;
                DrawTriangle(V2(x - w * 0.045f, (float)h), V2(x + w * 0.015f, (float)h - spike),
                             V2(x + w * 0.06f, (float)h), (Color){ 10, 10, 27, 255 });
            }
            break;
        default: /* VOID CATHEDRAL — distant obsidian spires and ghost-light. */
            Nebula(V2(w * 0.50f, h * 0.50f), (float)h * 0.66f, C_BLUE, 0.8f);
            Nebula(V2(w * 0.80f, h * 0.29f), (float)h * 0.34f, C_CYAN, 2.4f);
            for (int i = 0; i < 12; ++i) {
                float x = (float)i * (float)w / 11.0f;
                float tall = (float)(2 + (i * 7) % 6) * (float)h * 0.075f;
                DrawVoidSpire(x, (float)h, (float)w * (0.035f + 0.01f * (float)(i % 3)), tall,
                              0.5f + 0.5f * sinf(g.worldTime * 0.7f + (float)i));
            }
            for (int i = 0; i < 5; ++i) {
                float y = h * (0.18f + 0.14f * (float)i) + sinf(g.worldTime * 0.22f + (float)i) * 10.0f;
                DrawLineEx(V2(w * 0.14f, y), V2(w * 0.86f, y - 16.0f), 1.0f, FadeColor(C_CYAN, 0.06f));
            }
            break;
    }
}

void DrawBackground(void)
{
    int w = SW(), h = SH();
    static const Color baseTop[5] = {
        { 16, 14, 42, 255 }, { 9, 19, 36, 255 }, { 19, 8, 38, 255 },
        { 35, 9, 28, 255 }, { 5, 15, 34, 255 }
    };
    static const Color baseBottom[5] = {
        { 4, 6, 19, 255 }, { 3, 7, 19, 255 }, { 4, 3, 17, 255 },
        { 13, 3, 19, 255 }, { 2, 4, 16, 255 }
    };
    int theme = g.backgroundTheme % 5;
    ClearBackground(baseBottom[theme]);
    DrawRectangleGradientV(0, 0, w, h, baseTop[theme], baseBottom[theme]);

    /* Landmark layers make each score band a distinct place in space. */
    DrawEnvironment(theme, w, h);

    /* Three subtle parallax star layers keep the space moving behind combat. */
    for (int i = 0; i < MAX_STARS; ++i) {
        Star *s = &g.stars[i];
        float x = fmodf(s->x + g.worldTime * s->speed * (i % 3 == 0 ? 0.11f : 0.025f), (float)w);
        float y = fmodf(s->y + g.worldTime * s->speed * (i % 3 == 0 ? 0.17f : 0.045f), (float)h);
        float pulse = 0.58f + 0.42f * sinf(g.worldTime * 1.4f + s->phase);
        Color star = (i % 11 == 0) ? C_CYAN : ((i % 13 == 0) ? C_PURPLE : C_TEXT);
        DrawCircleV(V2(x, y), s->size * pulse, FadeColor(star, 0.28f + 0.48f * pulse));
        if (i % 19 == 0) {
            DrawLineEx(V2(x - 3.0f, y), V2(x + 3.0f, y), 0.8f, FadeColor(star, 0.22f));
        }
    }

    /* Every 1,000 score: fade the new scene in through a soft dark veil. */
    if (g.backgroundTransition > 0.0f) {
        float amount = ClampF(g.backgroundTransition / 0.90f, 0.0f, 1.0f);
        float progress = 1.0f - amount;
        float sweepX = progress * (float)w;
        /* The new scene is revealed by a quiet scanning veil—not an explosion. */
        DrawRectangle(0, 0, w, h, (Color){ 2, 2, 12, (unsigned char)(72.0f * amount) });
        DrawRectangle((int)(sweepX - 42.0f), 0, 84, h,
                      (Color){ 65, 96, 190, (unsigned char)(18.0f * amount) });
        DrawLineEx(V2(sweepX, 0.0f), V2(sweepX, (float)h), 2.0f,
                   (Color){ 92, 225, 255, (unsigned char)(58.0f * amount) });
        DrawLineEx(V2(sweepX + 4.0f, 0.0f), V2(sweepX + 4.0f, (float)h), 1.0f,
                   (Color){ 179, 112, 255, (unsigned char)(34.0f * amount) });
    }
}
