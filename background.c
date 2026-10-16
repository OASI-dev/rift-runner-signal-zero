#include "game.h"

#include "game.h"
#include <math.h>

void DrawBackground(void)
{
    int w = SW(), h = SH();
    static const Color skies[][2] = {
        { { 15, 17, 47, 255 }, { 6, 8, 22, 255 } },
        { { 28, 13, 57, 255 }, { 9, 5, 25, 255 } },
        { { 7, 34, 55, 255 }, { 3, 10, 28, 255 } },
        { { 52, 18, 37, 255 }, { 16, 5, 24, 255 } },
        { { 20, 39, 49, 255 }, { 4, 12, 22, 255 } }
    };
    int theme = g.backgroundTheme % 5;
    ClearBackground(skies[theme][1]);
    DrawRectangleGradientV(0, 0, w, h, skies[theme][0], skies[theme][1]);
    Vector2 center = V2(w * 0.5f + sinf(g.worldTime * 0.11f) * 70.0f, h * 0.48f + cosf(g.worldTime * 0.09f) * 25.0f);
    DrawCircleGradient(center, (float)MinF(w, h) * 0.54f, (Color){ 53, 44, 128, 42 }, (Color){ 11, 11, 28, 0 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.19f, (Color){ 90, 108, 228, 33 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.28f, (Color){ 69, 223, 255, 22 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.37f, (Color){ 168, 88, 255, 18 });

    if (g.backgroundBurst > 0.0f) {
        float t = 1.0f - g.backgroundBurst / 1.2f;
        float radius = t * (float)MaxF((float)w, (float)h) * 0.82f;
        for (int i = 0; i < 28; ++i) {
            float a = (float)i * (2.0f * PI_F / 28.0f);
            Vector2 end = Add(V2(w * 0.5f, h * 0.5f), V2(cosf(a) * radius, sinf(a) * radius));
            DrawLineEx(V2(w * 0.5f, h * 0.5f), end, 2.0f + 3.0f * (1.0f - t), WithOpacity((i % 2) ? C_CYAN : C_PURPLE, 1.0f - t));
        }
    }
    float gridShift = fmodf(g.worldTime * 9.0f, 54.0f);
    for (int x = -54; x < w + 54; x += 54) DrawLine((int)(x + gridShift), 0, (int)(x + gridShift), h, (Color){ 86, 116, 202, 15 });
    for (int y = -54; y < h + 54; y += 54) DrawLine(0, (int)(y + gridShift), w, (int)(y + gridShift), (Color){ 86, 116, 202, 13 });

    for (int i = 0; i < MAX_STARS; ++i) {
        Star *s = &g.stars[i];
        float x = fmodf(s->x + g.worldTime * s->speed * 0.16f, (float)w);
        float y = fmodf(s->y + g.worldTime * s->speed, (float)h);
        float pulse = 0.58f + 0.42f * sinf(g.worldTime * 1.6f + s->phase);
        Color starColor = (i % 7 == 0) ? C_CYAN : (i % 9 == 0 ? C_PURPLE : C_TEXT);
        DrawCircleV(V2(x, y), s->size * pulse, WithOpacity(starColor, 0.52f + 0.42f * pulse));
    }
}
