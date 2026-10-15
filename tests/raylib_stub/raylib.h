#ifndef RAYLIB_H
#define RAYLIB_H
#include <stdbool.h>
typedef struct Vector2 { float x,y; } Vector2;
typedef struct Rectangle { float x,y,width,height; } Rectangle;
typedef struct Color { unsigned char r,g,b,a; } Color;
typedef struct Camera2D { Vector2 offset; Vector2 target; float rotation; float zoom; } Camera2D;
typedef struct AudioStream { void *buffer; } AudioStream;
typedef struct Music { AudioStream stream; bool looping; } Music;
typedef struct Sound { AudioStream stream; } Sound;
enum { KEY_NULL=0, KEY_A,KEY_D,KEY_DOWN,KEY_ENTER,KEY_ESCAPE,KEY_F,KEY_KP_1,KEY_KP_2,KEY_KP_3,KEY_LEFT,KEY_LEFT_SHIFT,KEY_ONE,KEY_P,KEY_RIGHT,KEY_S,KEY_SPACE,KEY_THREE,KEY_TWO,KEY_UP,KEY_W };
enum { FLAG_MSAA_4X_HINT=1, FLAG_VSYNC_HINT=2, FLAG_WINDOW_RESIZABLE=4 };
void SetConfigFlags(unsigned int); void InitWindow(int,int,const char*); bool IsWindowReady(void); void SetWindowMinSize(int,int); void SetTargetFPS(int); void SetExitKey(int); bool WindowShouldClose(void); float GetFrameTime(void); int GetScreenWidth(void); int GetScreenHeight(void); bool IsKeyDown(int); bool IsKeyPressed(int); void ToggleFullscreen(void); int GetRandomValue(int,int); void CloseWindow(void);
void BeginDrawing(void); void EndDrawing(void); void BeginMode2D(Camera2D); void EndMode2D(void); void ClearBackground(Color); int MeasureText(const char*,int); const char *TextFormat(const char*,...);
void InitAudioDevice(void); bool IsAudioDeviceReady(void); const char *GetApplicationDirectory(void); Music LoadMusicStream(const char*); Sound LoadSound(const char*); void SetMusicVolume(Music,float); void UpdateMusicStream(Music); bool IsMusicStreamPlaying(Music); void PlayMusicStream(Music); void StopMusicStream(Music); void UnloadMusicStream(Music); void SetSoundVolume(Sound,float); void PlaySound(Sound); void UnloadSound(Sound); void CloseAudioDevice(void);
void DrawRectangleGradientV(int,int,int,int,Color,Color); void DrawCircleGradient(Vector2,float,Color,Color); void DrawCircleV(Vector2,float,Color); void DrawCircleLines(int,int,float,Color); void DrawLineEx(Vector2,Vector2,float,Color); void DrawLine(int,int,int,int,Color); void DrawPoly(Vector2,int,float,float,Color); void DrawRectangle(int,int,int,int,Color); void DrawRectangleLines(int,int,int,int,Color); void DrawRectangleLinesEx(Rectangle,float,Color); void DrawRectangleRounded(Rectangle,float,int,Color); void DrawRectangleRoundedLinesEx(Rectangle,float,int,float,Color); void DrawText(const char*,int,int,int,Color);
#endif
