/* Shared math, screen and text helpers. */

#include "game.h"
#include <math.h>
#include <stdlib.h>

float ClampF(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }

float MaxF(float a, float b) { return (a > b) ? a : b; }

float MinF(float a, float b) { return (a < b) ? a : b; }

float RandF(float lo, float hi) { return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo); }

float LengthSq(Vector2 v) { return v.x * v.x + v.y * v.y; }

float Length(Vector2 v) { return sqrtf(LengthSq(v)); }

Vector2 V2(float x, float y) { Vector2 v = { x, y }; return v; }

Vector2 Add(Vector2 a, Vector2 b) { return V2(a.x + b.x, a.y + b.y); }

Vector2 Sub(Vector2 a, Vector2 b) { return V2(a.x - b.x, a.y - b.y); }

Vector2 Scale(Vector2 a, float s) { return V2(a.x * s, a.y * s); }

Vector2 Normalize(Vector2 v) { float len = Length(v); return (len > 0.0001f) ? Scale(v, 1.0f / len) : V2(0.0f, 0.0f); }

Vector2 Perp(Vector2 v) { return V2(-v.y, v.x); }

float DistSq(Vector2 a, Vector2 b) { return LengthSq(Sub(a, b)); }

Color Alpha(Color c, float a) { c.a = (unsigned char)ClampF(a, 0.0f, 255.0f); return c; }

Color WithOpacity(Color c, float opacity) { return Alpha(c, (float)c.a * ClampF(opacity, 0.0f, 1.0f)); }

int SW(void) { return GetScreenWidth(); }

int SH(void) { return GetScreenHeight(); }

bool HitRect(Vector2 p, Rectangle r) { return p.x >= r.x && p.y >= r.y && p.x <= r.x + r.width && p.y <= r.y + r.height; }

void DrawCentered(const char *text, int y, int size, Color color) { DrawText(text, (SW() - MeasureText(text, size)) / 2, y, size, color); }

void DrawRight(const char *text, int rightX, int y, int size, Color color) { DrawText(text, rightX - MeasureText(text, size), y, size, color); }

const DifficultyInfo *GetDifficulty(void) { return &DIFFICULTIES[(int)g.difficulty]; }

int CurrentStageForScore(int score) { return 1 + score / SCORE_STEP; }
