/* Menus, HUD, screen rendering and menu input. */

#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void DrawPanel(Rectangle r, Color accent)
{
    DrawRectangleRounded(r, 0.12f, 12, C_PANEL);
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 1.0f, WithOpacity(accent, 0.48f));
}

void DrawHUD(void)
{
    char buf[96];
    int w = SW();
    const DifficultyInfo *d = GetDifficulty();
    bool compactHud = w < 760;
    if (compactHud) {
        float sideW = MinF(180.0f, (float)w * 0.40f);
        DrawPanel((Rectangle){ 8, 8, sideW, 66 }, C_CYAN);
        DrawText("SCORE", 18, 16, 9, C_MUTED);
        snprintf(buf, sizeof(buf), "%d", g.score);
        DrawText(buf, 16, 30, 23, C_TEXT);
        snprintf(buf, sizeof(buf), "STAGE %02d / %d K", g.stage, g.kills);
        DrawText(buf, 18, 56, 9, C_CYAN);

        float healthW = MinF(170.0f, (float)w * 0.39f);
        float healthX = (float)w - healthW - 8.0f;
        DrawPanel((Rectangle){ healthX, 8, healthW, 66 }, d->color);
        DrawText(d->name, (int)healthX + 10, 16, 9, d->color);
        DrawText("INTEGRITY", (int)healthX + 10, 33, 9, C_MUTED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, healthW - 20, 8 }, 0.5f, 6, (Color){ 38, 47, 74, 255 });
        Color healthColor = (g.player.health > g.player.maxHealth * 0.5f) ? C_GREEN : (g.player.health > g.player.maxHealth * 0.25f ? C_GOLD : C_RED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, (healthW - 20) * ClampF(g.player.health / g.player.maxHealth, 0, 1), 8 }, 0.5f, 6, healthColor);
    } else {
        DrawPanel((Rectangle){ 18, 16, 248, 90 }, C_CYAN);
        DrawText("SCORE", 34, 27, 12, C_MUTED);
        snprintf(buf, sizeof(buf), "%07d", g.score);
        DrawText(buf, 31, 43, 31, C_TEXT);
        snprintf(buf, sizeof(buf), "%d KILLS", g.kills);
        DrawText(buf, 34, 80, 11, C_CYAN);

        DrawPanel((Rectangle){ (float)w - 242.0f, 16, 224, 90 }, d->color);
        DrawText(d->name, w - 224, 27, 12, d->color);
        DrawText("INTEGRITY", w - 224, 47, 10, C_MUTED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188, 10 }, 0.5f, 6, (Color){ 38, 47, 74, 255 });
        Color healthColor = (g.player.health > g.player.maxHealth * 0.5f) ? C_GREEN : (g.player.health > g.player.maxHealth * 0.25f ? C_GOLD : C_RED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188.0f * ClampF(g.player.health / g.player.maxHealth, 0, 1), 10 }, 0.5f, 6, healthColor);
        snprintf(buf, sizeof(buf), "%d%%", (int)(g.player.health / g.player.maxHealth * 100.0f));
        DrawRight(buf, w - 36, 79, 11, C_TEXT);

        DrawPanel((Rectangle){ (float)w * 0.5f - 125.0f, 18, 250, 43 }, C_PURPLE);
        snprintf(buf, sizeof(buf), "RIFT %02d", g.stage);
        DrawCentered(buf, 31, 17, C_TEXT);
    }
    if (g.player.shieldTime > 0.0f) DrawText(TextFormat("SHIELD %.1fs", g.player.shieldTime), 24, SH() - 30, 13, C_GOLD);
    if (g.combo >= 2 && g.comboClock > 0.0f) {
        snprintf(buf, sizeof(buf), "COMBO x%d", g.combo);
        DrawRight(buf, w - 26, SH() - 42, 15, C_GOLD);
    }
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g.floaters[i];
        if (!f->active) continue;
        snprintf(buf, sizeof(buf), "+%d", f->points);
        int alpha = (int)(255.0f * ClampF(f->life / 0.88f, 0, 1));
        DrawText(buf, (int)f->pos.x - MeasureText(buf, 15) / 2, (int)f->pos.y, 15, Alpha(f->color, (float)alpha));
    }
    if (g.stageNotice > 0.0f) {
        float a = ClampF(g.stageNotice * 2.0f, 0, 1);
        snprintf(buf, sizeof(buf), "NEW RIFT ENVIRONMENT  //  PRESSURE INCREASED");
        int size = (SW() < 760) ? 16 : 21;
        DrawCentered(buf, (int)(SH() * 0.23f), size, Alpha(C_GOLD, a * 255.0f));
    }
}

void DrawButton(Rectangle r, const char *label, Color accent, bool selected)
{
    Vector2 mouse = GetMousePosition();
    bool hover = HitRect(mouse, r);
    Color fill = selected || hover ? WithOpacity(accent, 0.16f) : C_PANEL;
    Color border = selected || hover ? accent : (Color){ 67, 82, 126, 220 };
    DrawRectangleRounded(r, 0.16f, 10, fill);
    DrawRectangleRoundedLinesEx(r, 0.16f, 10, selected || hover ? 2.0f : 1.0f, border);
    DrawText(label, (int)(r.x + (r.width - MeasureText(label, 18)) * 0.5f), (int)(r.y + (r.height - 18) * 0.5f - 1), 18, selected || hover ? C_TEXT : C_MUTED);
}

Rectangle DifficultyCard(int i)
{
    float gap = (SW() < 600) ? 6.0f : 16.0f;
    float cardW = MinF(218.0f, ((float)SW() - 40.0f - gap * 2.0f) / 3.0f);
    float total = cardW * 3.0f + gap * 2.0f;
    float start = ((float)SW() - total) * 0.5f;
    return (Rectangle){ start + i * (cardW + gap), (float)SH() * 0.455f, cardW, 104.0f };
}

void DrawTitle(void)
{
    int h = SH(), w = SW();
    DrawCentered("SIGNAL ZERO", (int)(h * 0.135f), 17, C_CYAN);
    DrawCentered("RIFT RUNNER", (int)(h * 0.17f), (w < 440) ? 32 : ((w < 720) ? 40 : 57), C_TEXT);
    DrawCentered("A SCORE-DRIVEN NEON SURVIVAL RUN", (int)(h * 0.265f), 13, C_MUTED);
    DrawCentered("CHOOSE YOUR PRESSURE LEVEL", (int)(h * 0.395f), 13, C_CYAN);

    for (int i = 0; i < 3; ++i) {
        Rectangle r = DifficultyCard(i);
        bool selected = g.menuSelection == i;
        const DifficultyInfo *d = &DIFFICULTIES[i];
        DrawRectangleRounded(r, 0.12f, 10, selected ? (Color){ 25, 34, 69, 245 } : C_PANEL);
        DrawRectangleRoundedLinesEx(r, 0.12f, 10, selected ? 2.0f : 1.0f, selected ? d->color : (Color){ 60, 73, 112, 220 });
        DrawText(d->name, (int)(r.x + (r.width - MeasureText(d->name, 19)) * 0.5f), (int)r.y + 18, 19, d->color);
        if (r.width > 150.0f) DrawText(d->tagline, (int)(r.x + 12), (int)r.y + 54, 11, C_MUTED);
        DrawText(i == 0 ? "1" : (i == 1 ? "2" : "3"), (int)(r.x + r.width - 25), (int)r.y + 12, 11, selected ? C_TEXT : C_MUTED);
        if (selected) DrawRectangleRounded((Rectangle){ r.x + 16, r.y + r.height - 13, r.width - 32, 3 }, 0.5f, 5, d->color);
    }
    float startW = MinF(300.0f, (float)w - 36.0f);
    Rectangle startBtn = { (float)w * 0.5f - startW * 0.5f, (float)h * 0.635f, startW, 57.0f };
    DrawButton(startBtn, "ENTER THE RIFT  [ENTER]", C_CYAN, true);
    DrawCentered("WASD / ARROWS MOVE  ·  AUTO-FIRE FOLLOWS FACING", (int)(h * 0.755f), 12, C_TEXT);
    DrawCentered("HOLD LEFT MOUSE TO AIM  ·  WASD / ARROWS TO MOVE", (int)(h * 0.79f), 11, C_MUTED);
    DrawCentered("B  PERSONAL BESTS       ESC  QUIT", (int)(h * 0.85f), 11, C_MUTED);
    DrawCentered("EVERY STAGE ADVANCES AT SCORE MILESTONES — NOT BY ELAPSED TIME", (int)(h * 0.91f), 10, (Color){ 111, 128, 176, 255 });
}

void DrawScores(void)
{
    int w = SW(), h = SH();
    int titleSize = (w < 440) ? 21 : 30;
    DrawCentered("PERSONAL BESTS", (int)(h * 0.10f), titleSize, C_TEXT);
    DrawCentered("ONE HIGH SCORE PER MODE  ·  SAVED ON THIS DEVICE", (int)(h * 0.16f), 11, C_MUTED);

    float panelW = MinF(700.0f, (float)w * 0.90f);
    float rowH = MinF(76.0f, (float)h * 0.105f);
    float panelH = rowH * 3.0f + 54.0f;
    Rectangle panel = { ((float)w - panelW) * 0.5f, ((float)h - panelH) * 0.51f, panelW, panelH };
    DrawPanel(panel, C_PURPLE);

    for (int i = 0; i < 3; ++i) {
        const DifficultyInfo *d = &DIFFICULTIES[i];
        float y = panel.y + 18.0f + i * rowH;
        Rectangle row = { panel.x + 12.0f, y, panel.width - 24.0f, rowH - 7.0f };
        DrawRectangleRounded(row, 0.16f, 8, (Color){ 21, 29, 59, 235 });
        DrawRectangleRoundedLinesEx(row, 0.16f, 8, 1.2f, WithOpacity(d->color, 0.65f));
        DrawText(d->name, (int)row.x + 16, (int)(row.y + row.height * 0.5f - 10), 16, d->color);
        DrawText("HIGH SCORE", (int)row.x + (w < 480 ? 90 : 170), (int)row.y + 9, 9, C_MUTED);
        char scoreText[32];
        if (g.bestScores[i] >= 0) snprintf(scoreText, sizeof(scoreText), "%07d", g.bestScores[i]);
        else snprintf(scoreText, sizeof(scoreText), "N/A");
        int scoreSize = (w < 480) ? 19 : 24;
        DrawRight(scoreText, (int)(row.x + row.width - 16.0f), (int)(row.y + row.height * 0.5f - scoreSize * 0.5f), scoreSize,
                  g.bestScores[i] >= 0 ? C_TEXT : C_MUTED);
        if (g.bestScores[i] < 0) DrawText("PLAY THIS MODE TO SET A RECORD", (int)row.x + (w < 480 ? 90 : 170), (int)row.y + 24, 9, C_MUTED);
    }
    DrawCentered("B / ESC  BACK", (int)(h * 0.88f), 13, C_CYAN);
}

void DrawGameOver(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 190 });
    float panelW = MinF(476.0f, (float)w - 24.0f);
    float panelH = MinF(410.0f, (float)h - 24.0f);
    Rectangle p = { (float)w * 0.5f - panelW * 0.5f, (float)h * 0.5f - panelH * 0.5f, panelW, panelH };
    DrawPanel(p, C_PINK);
    float scaleY = panelH / 410.0f;
    DrawCentered("RUN TERMINATED", (int)p.y + (int)(31 * scaleY), 25, C_PINK);
    DrawCentered("FINAL SCORE", (int)p.y + (int)(83 * scaleY), 12, C_MUTED);
    char buf[100];
    snprintf(buf, sizeof(buf), "%07d", g.score);
    DrawCentered(buf, (int)p.y + (int)(105 * scaleY), (w < 440) ? 38 : 51, C_TEXT);
    snprintf(buf, sizeof(buf), "%s    /    RIFT %02d    /    %d KILLS", GetDifficulty()->name, g.stage, g.kills);
    DrawCentered(buf, (int)p.y + (int)(167 * scaleY), (w < 440) ? 9 : 12, GetDifficulty()->color);
    if (g.newPersonalBest) DrawCentered("NEW PERSONAL BEST FOR THIS MODE!", (int)p.y + (int)(207 * scaleY), (w < 440) ? 11 : 15, C_GOLD);
    else DrawCentered("MODE BEST: SCORE NOT BEATEN", (int)p.y + (int)(207 * scaleY), (w < 440) ? 9 : 11, C_MUTED);
    Rectangle again = { p.x + 24, p.y + panelH * 0.64f, p.width - 48, 50 };
    DrawButton(again, "RUN AGAIN  [ENTER]", C_CYAN, true);
    DrawCentered("ESC  TITLE     B  PERSONAL BESTS", (int)p.y + (int)(348 * scaleY), 11, C_MUTED);
}

void DrawPause(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 178 });
    DrawCentered("PAUSED", (int)(h * 0.40f), 42, C_TEXT);
    DrawCentered("P / ENTER  RESUME", (int)(h * 0.50f), 15, C_CYAN);
    DrawCentered("B  PERSONAL BESTS       ENTER / P  RESUME", (int)(h * 0.55f), 12, C_MUTED);
}

void DrawTouchControls(void)
{
    if (g.state != STATE_PLAYING) return;
    if (SW() > 820 && GetTouchPointCount() <= 0) return;
    Vector2 anchor = V2(105.0f, (float)SH() - 126.0f);
    DrawCircleLines((int)anchor.x, (int)anchor.y, 55.0f, (Color){ 126, 206, 255, 95 });
    DrawCircleV(anchor, 22.0f, (Color){ 77, 123, 195, 50 });
    DrawCircleLines(SW() - 84, SH() - 92, 39.0f, (Color){ 126, 206, 255, 95 });
    DrawText("AIM", SW() - 111, SH() - 98, 11, C_CYAN);
}

void DrawFrame(void)
{
    int w = SW(), h = SH();
    Camera2D camera = { 0 };
    float sx = 0.0f, sy = 0.0f;
    if (g.screenShake > 0.0f) {
        float amount = g.screenShake * 19.0f;
        sx = RandF(-amount, amount);
        sy = RandF(-amount, amount);
    }
    camera.offset = V2(w * 0.5f + sx, h * 0.5f + sy);
    camera.target = V2(w * 0.5f, h * 0.5f);
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    BeginDrawing();
    BeginMode2D(camera);
    DrawBackground();
    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawWorld();
    else {
        /* Decorative rift rings and sparks on menus, without starting gameplay. */
        Vector2 c = V2(w * 0.5f, h * 0.52f);
        float pulse = 1.0f + sinf(g.worldTime * 1.2f) * 0.025f;
        DrawCircleGradient(c, 118.0f * pulse, (Color){ 75, 44, 194, 26 }, (Color){ 75, 44, 194, 0 });
        DrawCircleLines((int)c.x, (int)c.y, 80.0f * pulse, (Color){ 89, 222, 255, 90 });
        DrawCircleLines((int)c.x, (int)c.y, 100.0f * pulse, (Color){ 175, 91, 255, 68 });
        for (int i = 0; i < 10; ++i) {
            float angle = g.worldTime * 0.55f + (float)i * (2.0f * PI_F / 10.0f);
            Vector2 p = Add(c, V2(cosf(angle) * 90.0f, sinf(angle) * 90.0f));
            DrawCircleGradient(p, 9.0f, WithOpacity(C_CYAN, 0.35f), (Color){ 76, 231, 255, 0 });
        }
    }
    EndMode2D();

    if (g.state == STATE_TITLE) DrawTitle();
    if (g.state == STATE_PLAYING) { DrawHUD(); DrawTouchControls(); }
    if (g.state == STATE_PAUSED) { DrawHUD(); DrawPause(); }
    if (g.state == STATE_GAME_OVER) DrawGameOver();
    if (g.state == STATE_SCORES) DrawScores();
    if (g.flashAlpha > 0.0f) DrawRectangle(0, 0, w, h, (Color){ 188, 226, 255, (unsigned char)(ClampF(g.flashAlpha, 0, 0.4f) * 255.0f) });
    EndDrawing();
}





void UpdateTitle(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) SetDifficulty(g.menuSelection - 1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) SetDifficulty(g.menuSelection + 1);
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) SetDifficulty(DIFF_EASY);
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) SetDifficulty(DIFF_MEDIUM);
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetDifficulty(DIFF_HARD);
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 3; ++i) if (HitRect(m, DifficultyCard(i))) SetDifficulty(i);
        float startW = MinF(300.0f, (float)SW() - 36.0f);
        Rectangle startBtn = { (float)SW() * 0.5f - startW * 0.5f, (float)SH() * 0.635f, startW, 57.0f };
        if (HitRect(m, startBtn)) StartRun();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
#if !defined(__EMSCRIPTEN__)
        g.quitRequested = true;
#endif
    }
}

void UpdatePaused(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) g.state = STATE_PLAYING;
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
}

void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) StartRun();
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
    if (IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_TITLE; }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float panelW = MinF(476.0f, (float)SW() - 24.0f);
        float panelH = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle again = { (float)SW() * 0.5f - panelW * 0.5f + 24.0f, (float)SH() * 0.5f - panelH * 0.5f + panelH * 0.64f, panelW - 48.0f, 50.0f };
        if (HitRect(GetMousePosition(), again)) StartRun();
    }
}

void UpdateScores(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) g.state = STATE_TITLE;
}
