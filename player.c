/* Player movement, aiming, auto-fire timing and player rendering. */

#include "game.h"
#include <math.h>

void UpdatePlayer(float dt)
{
    Player *p = &g.player;
    const DifficultyInfo *d = GetDifficulty();
    Vector2 input = V2(0.0f, 0.0f);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1.0f;

    /* Optional touch controls: drag in the left half to move; touch the right half to aim. */
    bool touchAiming = false;
    int touchCount = GetTouchPointCount();
    if (touchCount > 0) {
        for (int i = 0; i < touchCount; ++i) {
            Vector2 touch = GetTouchPosition(i);
            if (touch.x < (float)SW() * 0.47f) {
                Vector2 anchor = V2(105.0f, (float)SH() - 126.0f);
                Vector2 drag = Scale(Sub(touch, anchor), 1.0f / 62.0f);
                float dragLen = Length(drag);
                if (dragLen > 1.0f) drag = Scale(drag, 1.0f / dragLen);
                input = Add(input, drag);
            } else {
                Vector2 aim = RRNormalizeVector2(Sub(touch, p->pos));
                if (LengthSq(aim) > 0.1f) {
                    p->facing = aim;
                    touchAiming = true;
                }
            }
        }
    }
    float inLen = Length(input);
    if (inLen > 1.0f) input = Scale(input, 1.0f / inLen);
    if (LengthSq(input) > 0.02f) {
        /* Movement sets facing unless the player is explicitly aiming by touch. */
        if (!touchAiming) p->facing = RRNormalizeVector2(input);
        p->animation += dt * 14.0f;
    }

    Vector2 mouse = GetMousePosition();
    bool mouseAiming = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    if (mouseAiming) {
        Vector2 mouseAim = RRNormalizeVector2(Sub(mouse, p->pos));
        if (LengthSq(mouseAim) > 0.1f) p->facing = mouseAim;
    }
    if (LengthSq(input) > 0.02f) p->velocity = Scale(input, d->playerSpeed);
    else p->velocity = Scale(p->velocity, MaxF(0.0f, 1.0f - 7.5f * dt));

    p->shieldTime = MaxF(0.0f, p->shieldTime - dt);
    p->invulnerable = MaxF(0.0f, p->invulnerable - dt);
    p->fireClock -= dt;
    p->pos = Add(p->pos, Scale(p->velocity, dt));
    p->pos.x = ClampF(p->pos.x, 28.0f, (float)SW() - 28.0f);
    p->pos.y = ClampF(p->pos.y, 34.0f, (float)SH() - 30.0f);

    /* Automatic blaster fire follows the runner facing; hold mouse to aim. */
    if (p->fireClock <= 0.0f) {
        AddBullet(p->pos, p->facing);
        p->fireClock = d->fireInterval;
    }
}

void DrawRunner(void)
{
    Player *p = &g.player;
    Vector2 f = RRNormalizeVector2(p->facing);
    Vector2 side = Perp(f);
    bool blinking = p->invulnerable > 0.0f && ((int)(g.worldTime * 22.0f) % 2 == 0);
    Color armor = (p->shieldTime > 0.0f) ? C_GOLD : C_TEXT;
    if (blinking) armor = WithOpacity(armor, 0.35f);
    float stride = (LengthSq(p->velocity) > 80.0f) ? sinf(p->animation) * 3.0f : 0.0f;

    Vector2 head = Add(p->pos, Scale(f, 9.0f));
    Vector2 shoulderL = Add(Add(p->pos, Scale(f, 3.0f)), Scale(side, 8.0f));
    Vector2 shoulderR = Add(Add(p->pos, Scale(f, 3.0f)), Scale(side, -8.0f));
    Vector2 hipL = Add(Add(p->pos, Scale(f, -8.0f)), Scale(side, 5.5f));
    Vector2 hipR = Add(Add(p->pos, Scale(f, -8.0f)), Scale(side, -5.5f));
    Vector2 leftFoot = Add(Add(p->pos, Scale(f, -17.0f + stride)), Scale(side, 9.0f));
    Vector2 rightFoot = Add(Add(p->pos, Scale(f, -17.0f - stride)), Scale(side, -9.0f));
    Vector2 blasterBase = Add(Add(p->pos, Scale(f, 2.5f)), Scale(side, -8.0f));
    Vector2 blasterTip = Add(Add(p->pos, Scale(f, 19.5f)), Scale(side, -8.0f));

    DrawCircleGradient(p->pos, 36.0f, (Color){ 63, 220, 255, 49 }, (Color){ 63, 220, 255, 0 });

    /* Three short wake lines make movement readable without filling the screen with particles. */
    float movement = ClampF(Length(p->velocity) / GetDifficulty()->playerSpeed, 0.0f, 1.0f);
    if (movement > 0.08f) {
        for (int i = 0; i < 3; ++i) {
            float back = 21.0f + (float)i * 5.0f;
            float sideOffset = sinf(g.worldTime * 8.0f + (float)i * 1.9f) * 2.2f;
            Vector2 a = Add(Add(p->pos, Scale(f, -back)), Scale(side, sideOffset));
            Vector2 b = Add(a, Scale(f, -(6.0f + movement * 8.0f)));
            DrawLineEx(a, b, 1.0f + movement, WithOpacity(i == 1 ? C_CYAN : C_BLUE, movement * 0.34f));
        }
    }
    if (p->shieldTime > 0.0f) {
        DrawCircleLines((int)p->pos.x, (int)p->pos.y, 27.0f + sinf(g.worldTime * 8.0f) * 2.0f, C_GOLD);
        DrawCircleLines((int)p->pos.x, (int)p->pos.y, 31.0f, WithOpacity(C_GOLD, 0.5f));
    }
    DrawCircleV(Add(p->pos, V2(0, 8)), 13.0f, (Color){ 1, 3, 15, 190 });

    /* Back legs and boots make the avatar read as a small armored runner, not an orb. */
    DrawLineEx(hipL, leftFoot, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(hipR, rightFoot, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(leftFoot, Add(leftFoot, Scale(f, 4.0f)), 5.2f, C_PURPLE);
    DrawLineEx(rightFoot, Add(rightFoot, Scale(f, 4.0f)), 5.2f, C_PURPLE);

    /* Chest armour is drawn as two triangles so it rotates naturally with facing. */
    DrawTriangle(shoulderL, shoulderR, hipR, armor);
    DrawTriangle(shoulderL, hipR, hipL, (Color){ 44, 77, 133, 255 });
    DrawLineEx(shoulderL, shoulderR, 2.0f, C_CYAN);
    DrawLineEx(shoulderL, hipL, 1.5f, (Color){ 96, 163, 255, 255 });
    DrawLineEx(shoulderR, hipR, 1.5f, (Color){ 96, 163, 255, 255 });

    /* One arm braces the blaster; the other keeps a visible shoulder silhouette. */
    DrawLineEx(Add(p->pos, Scale(side, 6.5f)), Add(Add(p->pos, Scale(f, -1.0f)), Scale(side, 14.0f)), 3.4f, C_BLUE);
    DrawLineEx(Add(p->pos, Scale(side, -6.5f)), blasterBase, 4.1f, (Color){ 108, 165, 255, 255 });
    DrawLineEx(blasterBase, blasterTip, 4.0f, C_CYAN);
    DrawCircleV(blasterTip, 3.0f, C_TEXT);

    DrawCircleGradient(head, 14.0f, (Color){ 76, 231, 255, 52 }, (Color){ 76, 231, 255, 0 });
    DrawCircleV(head, 6.4f, armor);
    DrawCircleLines((int)head.x, (int)head.y, 6.4f, C_CYAN);
    DrawLineEx(Add(head, Scale(f, 1.0f)), Add(head, Scale(f, 5.0f)), 3.1f, (Color){ 21, 44, 80, 255 });
    DrawCircleV(Add(head, Scale(f, 3.2f)), 1.6f, C_TEXT);
    DrawCircleV(Sub(p->pos, Scale(f, 10.5f)), 2.6f, C_GOLD);
}
