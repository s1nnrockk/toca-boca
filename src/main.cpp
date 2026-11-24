#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {
constexpr int DESIGN_W = 1280;
constexpr int DESIGN_H = 720;

struct Place {
    const char* name;
    const char* subtitle;
    Vector2 mapPos;
    Color ground;
    Color accent;
    int kind;
};

const std::array<Place, 6> places{{
    {"Sunny City", "Apartments, shops and a busy plaza", {350, 315}, {255, 220, 153, 255}, {245, 139, 91, 255}, 0},
    {"Green Valley", "A quiet village among rolling hills", {565, 245}, {185, 224, 154, 255}, {95, 174, 111, 255}, 1},
    {"Autumn Woods", "A forest path and cozy cabins", {770, 330}, {244, 191, 132, 255}, {194, 111, 77, 255}, 2},
    {"Snowy Town", "A little town beneath the mountains", {970, 225}, {220, 239, 246, 255}, {116, 174, 211, 255}, 3},
    {"Seaside", "A bright beach and a tiny harbor", {540, 500}, {255, 226, 158, 255}, {69, 174, 201, 255}, 4},
    {"Highland Park", "Open green spaces and a lookout", {830, 515}, {194, 225, 163, 255}, {122, 170, 101, 255}, 5}
}};

int selectedPlace = -1;
Vector2 player{640, 390};
bool showMap = true;
float worldTime = 0.0f;
float uiScale = 1.0f;

Color darkInk{55, 76, 79, 255};
Color paper{255, 250, 237, 245};
Color coral{244, 132, 112, 255};
Color mint{125, 198, 161, 255};

Rectangle FitRect(float x, float y, float w, float h) {
    return {x * uiScale, y * uiScale, w * uiScale, h * uiScale};
}

Vector2 PointerPosition() {
    return GetMousePosition();
}

bool PointerPressed() {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool Hit(Rectangle r, Vector2 p) {
    return CheckCollisionPointRec(p, r);
}

void DrawCloud(float x, float y, float s) {
    DrawCircleV({x, y}, 20*s, Fade(WHITE, 0.88f));
    DrawCircleV({x + 22*s, y - 9*s}, 25*s, Fade(WHITE, 0.92f));
    DrawCircleV({x + 48*s, y}, 19*s, Fade(WHITE, 0.88f));
    DrawRectangleRec({x - 2*s, y, 52*s, 17*s}, Fade(WHITE, 0.9f));
}

void DrawMap() {
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                           {155, 218, 231, 255}, {224, 241, 205, 255});
    DrawCloud(100, 105, 1.2f);
    DrawCloud(930, 110, 0.85f);
    DrawCloud(1080, 390, 0.7f);

    // Soft rolling landscape, drawn procedurally so the project has no ripped game art.
    DrawEllipse(650, 390, 520, 230, {178, 220, 157, 255});
    DrawEllipse(260, 420, 230, 160, {196, 228, 164, 255});
    DrawEllipse(1040, 410, 250, 180, {170, 215, 151, 255});
    DrawEllipse(690, 555, 360, 110, {198, 226, 160, 255});

    // Winding road.
    DrawLineEx({160, 380}, {390, 330}, 54, {248, 239, 207, 255});
    DrawLineEx({390, 330}, {640, 380}, 54, {248, 239, 207, 255});
    DrawLineEx({640, 380}, {890, 315}, 54, {248, 239, 207, 255});
    DrawLineEx({890, 315}, {1090, 360}, 54, {248, 239, 207, 255});
    DrawLineEx({640, 380}, {620, 550}, 48, {248, 239, 207, 255});
    DrawLineEx({620, 550}, {850, 555}, 48, {248, 239, 207, 255});
    DrawLineEx({160, 380}, {390, 330}, 3, {224, 205, 162, 255});
    DrawLineEx({390, 330}, {640, 380}, 3, {224, 205, 162, 255});
    DrawLineEx({640, 380}, {890, 315}, 3, {224, 205, 162, 255});
    DrawLineEx({890, 315}, {1090, 360}, 3, {224, 205, 162, 255});

    for (int i = 0; i < 22; ++i) {
        const float x = 55.0f + (i * 53.0f);
        const float y = 610.0f + std::sin(i * 0.8f) * 25.0f;
        DrawCircle((int)x, (int)y, 8, {116, 184, 113, 255});
        DrawCircle((int)(x - 5), (int)(y - 7), 7, {133, 197, 126, 255});
    }

    DrawRectangleRounded({34, 24, 410, 86}, 0.28f, 18, paper);
    DrawText("POCKET PLACES", 60, 40, 30, darkInk);
    DrawText("A little world to explore", 62, 77, 17, {108, 132, 126, 255});

    for (size_t i = 0; i < places.size(); ++i) {
        const Place& p = places[i];
        const Vector2 c = p.mapPos;
        const bool active = selectedPlace == (int)i;
        DrawCircleV(c, active ? 47 : 40, Fade(WHITE, 0.94f));
        DrawCircleV(c, active ? 39 : 33, p.accent);
        DrawCircleV({c.x - 9, c.y - 8}, 11, Fade(WHITE, 0.7f));
        DrawCircleV({c.x + 10, c.y + 7}, 8, Fade(darkInk, 0.13f));
        DrawRectangleRounded({c.x - 77, c.y + 45, 154, 35}, 0.5f, 12, paper);
        DrawText(p.name, (int)c.x - MeasureText(p.name, 16)/2, (int)c.y + 54, 16, darkInk);
        if (active) DrawCircleLines((int)c.x, (int)c.y, 49, coral);
    }

    DrawRectangleRounded({GetScreenWidth() - 300.0f, 24, 266, 52}, 0.45f, 16, paper);
    DrawText("6 places to discover", GetScreenWidth() - 270, 41, 19, darkInk);
    DrawText("Choose a place on the map", 38, GetScreenHeight() - 42, 18, darkInk);
}

void DrawPlaceScene() {
    const Place& p = places[(size_t)selectedPlace];
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                           {166, 220, 232, 255}, p.ground);
    DrawCircle(1070, 100, 48, {255, 224, 133, 255});
    DrawEllipse(640, 570, 700, 170, Fade(p.accent, 0.42f));

    // Three cheerful, original buildings.
    for (int i = 0; i < 3; ++i) {
        const float x = 260.0f + i * 260.0f;
        const float h = 150.0f + (i % 2) * 55.0f;
        DrawRectangleRounded({x, 390.0f - h, 190, h}, 0.08f, 8, i == 1 ? p.accent : paper);
        DrawTriangle({x - 12, 390.0f - h, x + 95, 320.0f - h, x + 202, 390.0f - h}, darkInk, darkInk);
        for (int w = 0; w < 3; ++w) {
            DrawRectangleRounded({x + 22 + w*55.0f, 415.0f - h, 34, 42}, 0.12f, 5, {145, 205, 213, 255});
        }
        DrawRectangle({x + 75, 350, 42, 80}, {145, 102, 79, 255});
    }

    // Character.
    DrawEllipse(player.x, player.y + 42, 25, 9, Fade(darkInk, 0.2f));
    DrawRectangleRounded({player.x - 18, player.y - 2, 36, 43}, 0.35f, 10, coral);
    DrawCircleV({player.x, player.y - 17}, 19, {246, 195, 150, 255});
    DrawCircleV({player.x - 7, player.y - 19}, 3, darkInk);
    DrawCircleV({player.x + 7, player.y - 19}, 3, darkInk);
    DrawArc((int)player.x, (int)player.y - 10, 7, 2, 10, 170, 12, darkInk);

    DrawRectangleRounded({28, 24, 500, 88}, 0.25f, 16, paper);
    DrawText(p.name, 54, 40, 30, darkInk);
    DrawText(p.subtitle, 56, 80, 17, {108, 132, 126, 255});

    DrawRectangleRounded({GetScreenWidth() - 190.0f, 24, 160, 54}, 0.4f, 16, paper);
    DrawText("WORLD MAP", GetScreenWidth() - 168, 42, 18, darkInk);

    // Mobile-friendly directional pad; desktop can also use WASD/arrow keys.
    const float padX = GetScreenWidth() - 120.0f;
    const float padY = GetScreenHeight() - 116.0f;
    DrawCircleV({padX, padY}, 72, Fade(paper, 0.75f));
    DrawCircleV({padX, padY}, 27, Fade(mint, 0.9f));
    DrawTriangle({padX, padY - 58}, {padX - 14, padY - 34}, {padX + 14, padY - 34}, darkInk);
    DrawTriangle({padX, padY + 58}, {padX - 14, padY + 34}, {padX + 14, padY + 34}, darkInk);
    DrawTriangle({padX - 58, padY}, {padX - 34, padY - 14}, {padX - 34, padY + 14}, darkInk);
    DrawTriangle({padX + 58, padY}, {padX + 34, padY - 14}, {padX + 34, padY + 14}, darkInk);
}

void UpdateGame(float dt) {
    if (showMap) {
        if (PointerPressed()) {
            const Vector2 m = PointerPosition();
            for (size_t i = 0; i < places.size(); ++i) {
                if (CheckCollisionPointCircle(m, places[i].mapPos, 48.0f)) {
                    selectedPlace = (int)i;
                    showMap = false;
                    player = {640, 490};
                    break;
                }
            }
        }
        return;
    }

    const float speed = 245.0f * dt;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) player.x -= speed;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player.x += speed;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) player.y -= speed;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) player.y += speed;

    const Vector2 m = PointerPosition();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && m.x > GetScreenWidth() - 220 && m.y > GetScreenHeight() - 220) {
        const float dx = m.x - (GetScreenWidth() - 120.0f);
        const float dy = m.y - (GetScreenHeight() - 116.0f);
        if (std::fabs(dx) > std::fabs(dy)) player.x += (dx > 0 ? speed : -speed);
        else player.y += (dy > 0 ? speed : -speed);
    }

    player.x = std::clamp(player.x, 70.0f, (float)GetScreenWidth() - 70.0f);
    player.y = std::clamp(player.y, 190.0f, (float)GetScreenHeight() - 85.0f);

    if (PointerPressed() && Hit({GetScreenWidth() - 190.0f, 24, 160, 54}, m)) showMap = true;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) showMap = true;
}
}

int main(int, char**) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(DESIGN_W, DESIGN_H, "Pocket Places");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.04f);
        worldTime += dt;
        UpdateGame(dt);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (showMap) DrawMap();
        else DrawPlaceScene();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
