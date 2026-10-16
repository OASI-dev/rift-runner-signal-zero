/* Menus, HUD, screen rendering and menu input. */

#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Poll once per frame so touch taps work for menus as reliably as mouse clicks. */
static bool uiPointerPressed = false;
static bool uiTouchWasDown = false;
static Vector2 uiPointerPosition = { 0.0f, 0.0f };

void PollUIPointer(void)
{
    int touchCount = GetTouchPointCount();
    bool touchDown = touchCount > 0;
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    uiPointerPressed = mousePressed || (touchDown && !uiTouchWasDown);
    if (touchDown && !mousePressed) uiPointerPosition = GetTouchPosition(0);
    else uiPointerPosition = GetMousePosition();
    uiTouchWasDown = touchDown;
}

bool GetUIPointerPress(Vector2 *position)
{
    if (!uiPointerPressed) return false;
    if (position) *position = uiPointerPosition;
    uiPointerPressed = false;
    return true;
}

void DrawPanel(Rectangle r, Color accent)
{
    DrawRectangleRounded(r, 0.12f, 12, C_PANEL);
    /* A restrained top-edge glint gives the HUD a layered glass-panel finish. */
    DrawLineEx(V2(r.x + 12.0f, r.y + 1.0f),
               V2(r.x + r.width - 12.0f, r.y + 1.0f), 1.0f,
               WithOpacity(C_TEXT, 0.055f));
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 1.0f, WithOpacity(accent, 0.48f));
}

static Color GetHealthColor(void)
{
    float ratio = (g.player.maxHealth > 0.0f) ? g.player.health / g.player.maxHealth : 0.0f;
    if (ratio > 0.50f) return C_GREEN;
    if (ratio > 0.25f) return C_GOLD;
    return C_RED;
}

static void DrawSectorProgress(float x, float y, float width, float height)
{
    float progress = (float)(g.score % 1000) / 1000.0f;
    DrawRectangleRounded((Rectangle){ x, y, width, height }, 0.5f, 6,
                         (Color){ 35, 45, 75, 235 });
    if (progress > 0.0f) {
        DrawRectangleRounded((Rectangle){ x, y, width * progress, height }, 0.5f, 6, C_CYAN);
    }
}

void DrawHUD(void)
{
    char buf[112];
    const int w = SW(), h = SH();
    const DifficultyInfo *d = GetDifficulty();
    const bool compactHud = w < 900 || h < 560;
    const float healthRatio = (g.player.maxHealth > 0.0f)
        ? ClampF(g.player.health / g.player.maxHealth, 0.0f, 1.0f) : 0.0f;
    const Color healthColor = GetHealthColor();
    const float environmentProgress = (float)(g.score % 1000) / 1000.0f;

    if (compactHud) {
        float sideW = MinF(180.0f, (float)w * 0.40f);
        float healthW = MinF(170.0f, (float)w * 0.39f);
        float healthX = (float)w - healthW - 8.0f;
        DrawPanel((Rectangle){ 8, 8, sideW, 66 }, C_CYAN);
        DrawText("SCORE", 18, 16, 9, C_MUTED);
        snprintf(buf, sizeof(buf), "%d", g.score);
        DrawText(buf, 16, 29, 23, C_TEXT);
        snprintf(buf, sizeof(buf), "RIFT %02d  /  %d K", g.stage, g.kills);
        DrawText(buf, 18, 56, 9, C_CYAN);

        DrawPanel((Rectangle){ healthX, 8, healthW, 66 }, d->color);
        DrawText(d->name, (int)healthX + 10, 16, 9, d->color);
        DrawText("INTEGRITY", (int)healthX + 10, 32, 9, C_MUTED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, healthW - 20, 8 }, 0.5f, 6,
                             (Color){ 38, 47, 74, 255 });
        if (healthRatio > 0.0f) {
            DrawRectangleRounded((Rectangle){ healthX + 10, 48, (healthW - 20) * healthRatio, 8 },
                                 0.5f, 6, healthColor);
        }

        /* Full-width micro progress bar stays clear of the mobile touch zones. */
        DrawSectorProgress(8.0f, 80.0f, (float)w - 16.0f, 3.0f);
        snprintf(buf, sizeof(buf), "SECTOR %02d  //  %s", g.backgroundTheme + 1, GetSectorName());
        DrawText(buf, 10, 87, 9, WithOpacity(C_TEXT, 0.78f));
        snprintf(buf, sizeof(buf), "%d%%", (int)(environmentProgress * 100.0f));
        DrawRight(buf, w - 10, 87, 9, C_CYAN);
    } else {
        /* Score and combat stats. */
        DrawPanel((Rectangle){ 18, 16, 248, 90 }, C_CYAN);
        DrawText("SCORE", 34, 27, 12, C_MUTED);
        snprintf(buf, sizeof(buf), "%07d", g.score);
        DrawText(buf, 31, 43, 31, C_TEXT);
        snprintf(buf, sizeof(buf), "%d KILLS", g.kills);
        DrawText(buf, 34, 80, 11, C_CYAN);

        /* Center sector card: level, named environment and progress to next shift. */
        float sectorX = (float)w * 0.5f - 144.0f;
        DrawPanel((Rectangle){ sectorX, 16, 288, 90 }, C_PURPLE);
        snprintf(buf, sizeof(buf), "RIFT LEVEL  %02d", g.stage);
        DrawCentered(buf, 25, 15, C_TEXT);
        snprintf(buf, sizeof(buf), "SECTOR %02d  //  %s", g.backgroundTheme + 1, GetSectorName());
        DrawCentered(buf, 47, 10, C_CYAN);
        DrawSectorProgress(sectorX + 24.0f, 70.0f, 240.0f, 6.0f);
        snprintf(buf, sizeof(buf), "%d%% TO NEXT ENVIRONMENT", (int)(environmentProgress * 100.0f));
        DrawCentered(buf, 82, 9, C_MUTED);

        /* Integrity, with explicit percentage and color changing at danger thresholds. */
        DrawPanel((Rectangle){ (float)w - 242.0f, 16, 224, 90 }, d->color);
        DrawText(d->name, w - 224, 27, 12, d->color);
        DrawText("INTEGRITY", w - 224, 47, 10, C_MUTED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188, 10 }, 0.5f, 6,
                             (Color){ 38, 47, 74, 255 });
        if (healthRatio > 0.0f) {
            DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188.0f * healthRatio, 10 },
                                 0.5f, 6, healthColor);
        }
        snprintf(buf, sizeof(buf), "%d%%", (int)(healthRatio * 100.0f));
        DrawRight(buf, w - 36, 79, 11, C_TEXT);
    }

    /* Context-sensitive event notices: sector shifts take priority over level-ups. */
    if (g.backgroundTransition > 0.02f) {
        float alpha = ClampF(g.backgroundTransition / 0.55f, 0.0f, 1.0f);
        snprintf(buf, sizeof(buf), "SECTOR SHIFT  //  %s", GetSectorName());
        DrawCentered(buf, (int)((compactHud ? 0.245f : 0.205f) * (float)h), compactHud ? 13 : 18,
                     WithOpacity(C_CYAN, alpha));
    } else if (g.stageNotice > 0.0f) {
        float alpha = ClampF(g.stageNotice * 1.7f, 0.0f, 1.0f);
        DrawCentered("RIFT LEVEL INCREASED  //  PRESSURE RISING",
                     (int)((compactHud ? 0.245f : 0.205f) * (float)h), compactHud ? 11 : 15,
                     WithOpacity(C_GOLD, alpha));
    }

    if (g.player.shieldTime > 0.0f) {
        snprintf(buf, sizeof(buf), "SHIELD  %.1fs", g.player.shieldTime);
        DrawText(buf, 24, h - 30, 13, C_GOLD);
    }
    if (g.combo >= 2 && g.comboClock > 0.0f) {
        snprintf(buf, sizeof(buf), "COMBO  x%d", g.combo);
        DrawRight(buf, w - 26, h - 42, 15, C_GOLD);
    }
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g.floaters[i];
        if (!f->active) continue;
        snprintf(buf, sizeof(buf), "+%d", f->points);
        int alpha = (int)(255.0f * ClampF(f->life / 0.88f, 0.0f, 1.0f));
        DrawText(buf, (int)f->pos.x - MeasureText(buf, 15) / 2, (int)f->pos.y, 15,
                 Alpha(f->color, (float)alpha));
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
    int fontSize = 18;
    while (fontSize > 10 && MeasureText(label, fontSize) > (int)r.width - 22) --fontSize;
    DrawText(label, (int)(r.x + (r.width - (float)MeasureText(label, fontSize)) * 0.5f),
             (int)(r.y + (r.height - (float)fontSize) * 0.5f - 1.0f), fontSize,
             selected || hover ? C_TEXT : C_MUTED);
}

Rectangle DifficultyCard(int i)
{
    float gap = (SW() < 600) ? 6.0f : 16.0f;
    float cardW = MinF(218.0f, ((float)SW() - 40.0f - gap * 2.0f) / 3.0f);
    float total = cardW * 3.0f + gap * 2.0f;
    float start = ((float)SW() - total) * 0.5f;
    float h = (float)SH();
    float cardH = h < 400.0f ? MinF(84.0f, h * 0.20f) :
                  (h < 590.0f ? MinF(104.0f, h * 0.22f) : 104.0f);
    float y = h < 400.0f ? h * 0.34f : (h < 590.0f ? h * 0.36f : h * 0.455f);
    return (Rectangle){ start + i * (cardW + gap), y, cardW, cardH };
}

static void OpenScores(GameState returnState)
{
    g.scoresReturnState = returnState;
    g.state = STATE_SCORES;
}

static Rectangle PauseScoresBounds(void)
{
    float h = (float)SH();
    float y = h < 400.0f ? h * 0.62f : (h < 560.0f ? h * 0.655f : h * 0.64f);
    return (Rectangle){ (float)SW() * 0.5f - 110.0f, y, 220.0f, 20.0f };
}

static Rectangle StartRunBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    bool shortView = h < 590.0f;
    float buttonY = h * (shortView ? (h < 400.0f ? 0.57f : 0.585f) : 0.635f);
    float buttonH = shortView ? 45.0f : 57.0f;
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, buttonY, w, buttonH };
}

static Rectangle ScoresLinkBounds(void)
{
    float h = (float)SH();
    float y;
    if (h < 590.0f) {
        Rectangle start = StartRunBounds();
        y = start.y + start.height + 2.0f;
    } else {
        y = h * 0.715f;
    }
    return (Rectangle){ (float)SW() * 0.5f - 110.0f, y, 220.0f, 20.0f };
}

static Rectangle ScoresBackBounds(void)
{
    float w = (float)SW(), h = (float)SH();
    float y = h < 400.0f ? h * 0.75f : h * 0.80f;
    return (Rectangle){ w * 0.5f - MinF(220.0f, w - 36.0f) * 0.5f, y,
                        MinF(220.0f, w - 36.0f), 42.0f };
}

static Rectangle PauseResumeBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    float buttonH = h < 400.0f ? 34.0f : (h < 560.0f ? 42.0f : 52.0f);
    float y = h < 400.0f ? h * 0.405f : (h < 560.0f ? h * 0.47f : h * 0.48f);
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, y, w, buttonH };
}

static Rectangle PauseTitleBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    float y = h < 400.0f ? h * 0.52f : h * 0.57f;
    float height = h < 400.0f ? 32.0f : (h < 560.0f ? 36.0f : 42.0f);
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, y, w, height };
}

static Rectangle GameOverScoresBounds(Rectangle panel)
{
    return (Rectangle){ panel.x + 24.0f, panel.y + panel.height * 0.80f, panel.width - 48.0f, 20.0f };
}

static Rectangle GameOverTitleBounds(Rectangle panel)
{
    return (Rectangle){ panel.x + 40.0f, panel.y + panel.height * 0.90f, panel.width - 80.0f, 20.0f };
}

void DrawTitle(void)
{
    const int h = SH(), w = SW();
    const bool shortView = h < 590;
    const int titleSize = (w < 440) ? 32 : ((w < 720) ? 40 : 57);
    const int eyebrowY = (int)(h * (shortView ? 0.055f : 0.105f));
    const int titleY = (int)(h * (shortView ? 0.095f : 0.165f));
    const int subtitleY = (int)(h * (shortView ? 0.205f : 0.265f));
    const int chooseY = (int)(h * (h < 400 ? 0.285f : (shortView ? 0.305f : 0.395f)));

    /* Small framing rules help the title feel like a designed interface, not plain text. */
    DrawLineEx(V2((float)w * 0.18f, (float)eyebrowY + 8.0f),
               V2((float)w * 0.32f, (float)eyebrowY + 8.0f), 1.0f,
               WithOpacity(C_CYAN, 0.50f));
    DrawLineEx(V2((float)w * 0.68f, (float)eyebrowY + 8.0f),
               V2((float)w * 0.82f, (float)eyebrowY + 8.0f), 1.0f,
               WithOpacity(C_CYAN, 0.50f));
    DrawCentered("RIFT NETWORK  //  FLIGHT SYSTEMS ONLINE", eyebrowY, 11, C_CYAN);
    DrawCentered("RIFT RUNNER", titleY + 3, titleSize, (Color){ 65, 97, 150, 110 });
    DrawCentered("RIFT RUNNER", titleY, titleSize, C_TEXT);
    if (w < 440) {
        DrawCentered("SIGNAL ZERO  •  SURVIVE THE VOID", subtitleY, 10, C_MUTED);
    } else {
        DrawCentered("SIGNAL ZERO  •  SURVIVE THE SIGNAL. OUTLAST THE VOID.", subtitleY, 12, C_MUTED);
    }
    DrawCentered("SELECT YOUR PRESSURE LEVEL", chooseY, 12, C_CYAN);

    for (int i = 0; i < 3; ++i) {
        Rectangle r = DifficultyCard(i);
        bool selected = g.menuSelection == i;
        const DifficultyInfo *d = &DIFFICULTIES[i];
        DrawRectangleRounded(r, 0.12f, 10, selected ? (Color){ 25, 34, 69, 245 } : C_PANEL);
        DrawRectangleRoundedLinesEx(r, 0.12f, 10, selected ? 2.0f : 1.0f,
                                    selected ? d->color : (Color){ 60, 73, 112, 220 });
        DrawLineEx(V2(r.x + 12.0f, r.y + 1.0f),
                   V2(r.x + r.width - 12.0f, r.y + 1.0f), 1.0f,
                   WithOpacity(C_TEXT, selected ? 0.12f : 0.04f));
        int nameSize = r.width < 120.0f ? 15 : 19;
        DrawText(d->name, (int)(r.x + (r.width - (float)MeasureText(d->name, nameSize)) * 0.5f),
                 (int)r.y + (shortView ? 10 : 18), nameSize, d->color);
        if (r.width > 150.0f) {
            DrawText(d->tagline, (int)(r.x + 12), (int)r.y + (shortView ? 37 : 54), 11, C_MUTED);
        }
        DrawText(i == 0 ? "1" : (i == 1 ? "2" : "3"),
                 (int)(r.x + r.width - 25), (int)r.y + 10, 11, selected ? C_TEXT : C_MUTED);
        if (selected) {
            DrawRectangleRounded((Rectangle){ r.x + 16, r.y + r.height - 12,
                                               r.width - 32, 3 }, 0.5f, 5, d->color);
        }
    }

    Rectangle startBtn = StartRunBounds();
    DrawButton(startBtn, "ENTER THE RIFT  [ENTER]", C_CYAN, true);
    Rectangle scoresLink = ScoresLinkBounds();
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 3, 10,
                 scoresHover ? C_CYAN : C_MUTED);

    if (!shortView) {
        DrawCentered("WASD / ARROWS MOVE     •     MOUSE TO AIM     •     AUTO-FIRE", (int)(h * 0.78f), 12, C_TEXT);
        DrawCentered("P / ESC  PAUSE DURING A RUN     •     HOLD MOUSE TO AIM", (int)(h * 0.825f), 11, C_MUTED);
#if defined(__EMSCRIPTEN__)
        DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE", (int)(h * 0.865f), 10,
                     (Color){ 111, 128, 176, 255 });
#else
        DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE     •     ESC QUIT", (int)(h * 0.865f), 10,
                     (Color){ 111, 128, 176, 255 });
#endif
        DrawCentered("PERSONAL BESTS ARE SAVED ON THIS DEVICE", (int)(h * 0.91f), 9,
                     (Color){ 85, 103, 149, 255 });
    } else if (h >= 400) {
        DrawCentered("MOVE: WASD / ARROWS     •     AIM: MOUSE",
                     (int)(h * (h < 480 ? 0.765f : 0.75f)), 10, C_TEXT);
        DrawCentered("NEW SECTOR EVERY 1,000 SCORE", (int)(h * 0.80f), 9, C_MUTED);
    } else {
        DrawCentered("NEW SECTOR EVERY 1,000 SCORE", (int)(h * 0.84f), 9, C_MUTED);
    }
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
    Rectangle back = ScoresBackBounds();
    const char *backLabel = g.scoresReturnState == STATE_PAUSED ? "RETURN TO RUN" :
                            (g.scoresReturnState == STATE_GAME_OVER ? "RETURN TO RUN SUMMARY" : "BACK TO FLIGHT DECK");
    DrawButton(back, backLabel, C_CYAN, true);
    DrawCentered("B / ESC  BACK", (int)(h * 0.92f), 10, C_MUTED);
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
    Rectangle scoresLink = GameOverScoresBounds(p);
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 2, 10,
                 scoresHover ? C_CYAN : C_MUTED);
    Rectangle titleLink = GameOverTitleBounds(p);
    bool titleHover = HitRect(GetMousePosition(), titleLink);
    DrawCentered("RETURN TO FLIGHT DECK  [ESC]", (int)titleLink.y + 2, 9,
                 titleHover ? C_CYAN : C_MUTED);
}

void DrawPause(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 195 });
    float panelW = MinF(420.0f, (float)w - 32.0f);
    bool veryShort = h < 400;
    float panelY = veryShort ? h * 0.24f : h * 0.32f;
    float panelH = veryShort ? h * 0.54f : h * 0.42f;
    Rectangle panel = { (float)w * 0.5f - panelW * 0.5f, panelY, panelW, panelH };
    DrawPanel(panel, C_CYAN);
    DrawCentered("PAUSED", (int)(h * (veryShort ? 0.275f : 0.365f)), veryShort ? 27 : 36, C_TEXT);
    DrawCentered("YOUR RUN IS ON HOLD", (int)(h * (veryShort ? 0.355f : 0.43f)), 10, C_MUTED);
    DrawButton(PauseResumeBounds(), "RESUME RUN  [P / ENTER]", C_CYAN, true);
    DrawButton(PauseTitleBounds(), "RETURN TO FLIGHT DECK", C_PURPLE, false);
    Rectangle scoresLink = PauseScoresBounds();
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 3, 10,
                 scoresHover ? C_CYAN : C_MUTED);
    DrawCentered("ESC ALSO RESUMES", (int)(h * (veryShort ? 0.70f : 0.715f)), 9, C_MUTED);
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

static void DrawAimingReticle(void)
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) return;
    if (GetTouchPointCount() > 0) return;
    Vector2 p = GetMousePosition();
    Color faint = WithOpacity(C_CYAN, 0.78f);
    DrawCircleLines((int)p.x, (int)p.y, 7.0f, faint);
    DrawLineEx(V2(p.x - 15.0f, p.y), V2(p.x - 9.0f, p.y), 1.2f, faint);
    DrawLineEx(V2(p.x + 9.0f, p.y), V2(p.x + 15.0f, p.y), 1.2f, faint);
    DrawLineEx(V2(p.x, p.y - 15.0f), V2(p.x, p.y - 9.0f), 1.2f, faint);
    DrawLineEx(V2(p.x, p.y + 9.0f), V2(p.x, p.y + 15.0f), 1.2f, faint);
    DrawCircleV(p, 1.5f, C_TEXT);
}

void DrawFrame(void)
{
    int w = SW(), h = SH();
    Camera2D camera = { 0 };
    float sx = 0.0f, sy = 0.0f;
    if (g.screenShake > 0.0f) {
        float amount = g.screenShake * 19.0f;
        /* Deterministic oscillation feels controlled and does not consume gameplay RNG. */
        sx = (sinf(g.worldTime * 61.0f) + 0.32f * sinf(g.worldTime * 103.0f)) * amount * 0.62f;
        sy = (cosf(g.worldTime * 67.0f) + 0.28f * sinf(g.worldTime * 89.0f)) * amount * 0.62f;
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
    if (g.state == STATE_PLAYING) DrawAimingReticle();
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
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_TITLE); return; }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }

    Vector2 m;
    if (GetUIPointerPress(&m)) {
        for (int i = 0; i < 3; ++i) if (HitRect(m, DifficultyCard(i))) SetDifficulty(i);
        if (HitRect(m, ScoresLinkBounds())) { OpenScores(STATE_TITLE); return; }
        if (HitRect(m, StartRunBounds())) { StartRun(); return; }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
#if !defined(__EMSCRIPTEN__)
        g.quitRequested = true;
#endif
    }
}

void UpdatePaused(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_PLAYING; return; }
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_PAUSED); return; }
    Vector2 m;
    if (GetUIPointerPress(&m)) {
        if (HitRect(m, PauseResumeBounds())) { g.state = STATE_PLAYING; return; }
        if (HitRect(m, PauseTitleBounds())) { g.state = STATE_TITLE; return; }
        if (HitRect(m, PauseScoresBounds())) { OpenScores(STATE_PAUSED); return; }
    }
}

void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) StartRun();
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_GAME_OVER); return; }
    if (IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_TITLE; }
    Vector2 m;
    if (GetUIPointerPress(&m)) {
        float panelW = MinF(476.0f, (float)SW() - 24.0f);
        float panelH = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle again = { (float)SW() * 0.5f - panelW * 0.5f + 24.0f, (float)SH() * 0.5f - panelH * 0.5f + panelH * 0.64f, panelW - 48.0f, 50.0f };
        if (HitRect(m, again)) { StartRun(); return; }
        float panelW2 = MinF(476.0f, (float)SW() - 24.0f);
        float panelH2 = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle panel = { (float)SW() * 0.5f - panelW2 * 0.5f,
                            (float)SH() * 0.5f - panelH2 * 0.5f, panelW2, panelH2 };
        if (HitRect(m, GameOverScoresBounds(panel))) { OpenScores(STATE_GAME_OVER); return; }
        if (HitRect(m, GameOverTitleBounds(panel))) { g.state = STATE_TITLE; return; }
    }
}

void UpdateScores(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) {
        g.state = g.scoresReturnState;
        return;
    }
    Vector2 m;
    if (GetUIPointerPress(&m) && HitRect(m, ScoresBackBounds())) {
        g.state = g.scoresReturnState;
    }
}
