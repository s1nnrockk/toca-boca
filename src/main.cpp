#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {
constexpr int WORLD_W = 1280;
constexpr int WORLD_H = 720;
constexpr float PI_F = 3.14159265358979323846f;

struct Place {
    const char* name;
    const char* description;
    Vector2 pos;
    Color land;
    Color accent;
    int icon;
};

const std::array<Place, 6> places{{
    {"Sunny City", "Apartments, shops and a busy plaza", {310, 300}, {255, 221, 161, 255}, {237, 132, 101, 255}, 0},
    {"Green Valley", "A quiet village among rolling hills", {535, 205}, {188, 225, 158, 255}, {105, 177, 116, 255}, 1},
    {"Autumn Woods", "A forest path and cozy cabins", {790, 310}, {242, 194, 145, 255}, {190, 119, 78, 255}, 2},
    {"Snowy Town", "A little town beneath the mountains", {1010, 190}, {224, 239, 247, 255}, {118, 174, 211, 255}, 3},
    {"Seaside", "A bright beach and a tiny harbor", {520, 500}, {255, 229, 171, 255}, {75, 174, 199, 255}, 4},
    {"Highland Park", "Open green spaces and a lookout", {850, 520}, {197, 226, 169, 255}, {123, 169, 105, 255}, 5}
}};

int selectedPlace = -1;
Vector2 player{640, 500};
bool showMap = true;
bool insideBuilding = false;
int characterStyle = 0;
float animationTime = 0.0f;
float walkTime = 0.0f;
float messageUntil = 0.0f;
std::string sceneMessage;
std::string currentInteriorLabel = "HOME";
bool discoveredProps[6][3]{};
float facing = 1.0f;
Camera2D camera{};

const Color ink{55, 76, 79, 255};
const Color muted{105, 130, 126, 255};
const Color paper{255, 251, 240, 248};
const Color coral{244, 132, 112, 255};
const Color mint{125, 198, 161, 255};

void UpdateCamera() {
    const float scale = std::min((float)GetScreenWidth() / WORLD_W,
                                 (float)GetScreenHeight() / WORLD_H);
    camera.target = {WORLD_W * 0.5f, WORLD_H * 0.5f};
    camera.offset = {GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    camera.rotation = 0.0f;
    camera.zoom = scale;
}

Vector2 WorldPointer() {
    return GetScreenToWorld2D(GetMousePosition(), camera);
}

bool InRect(Rectangle r, Vector2 p) {
    return CheckCollisionPointRec(p, r);
}

void Cloud(float x, float y, float s) {
    DrawCircleV({x, y}, 18*s, Fade(WHITE, 0.9f));
    DrawCircleV({x + 22*s, y - 8*s}, 24*s, Fade(WHITE, 0.94f));
    DrawCircleV({x + 47*s, y}, 18*s, Fade(WHITE, 0.9f));
    DrawRectangleRec({x - 2*s, y, 51*s, 16*s}, Fade(WHITE, 0.92f));
}

void Tree(float x, float y, float s, Color leaves) {
    DrawRectangle((int)(x - 4*s), (int)(y + 8*s), (int)(8*s), (int)(22*s), {139, 105, 76, 255});
    DrawCircle((int)x, (int)y, 17*s, leaves);
    DrawCircle((int)(x - 9*s), (int)(y + 5*s), 11*s, Fade(leaves, 0.95f));
    DrawCircle((int)(x + 9*s), (int)(y + 6*s), 11*s, Fade(leaves, 0.95f));
}

void DrawMapBackground() {
    DrawRectangleGradientV(0, 0, WORLD_W, WORLD_H,
                           {155, 218, 231, 255}, {225, 241, 207, 255});
    Cloud(120, 132, 1.05f);
    Cloud(910, 110, 0.78f);
    Cloud(1110, 410, 0.72f);

    DrawEllipse(635, 370, 540, 245, {180, 220, 157, 255});
    DrawEllipse(235, 425, 250, 175, {197, 228, 165, 255});
    DrawEllipse(1050, 390, 260, 190, {173, 216, 153, 255});
    DrawEllipse(680, 565, 380, 115, {200, 228, 166, 255});
    DrawEllipse(470, 145, 230, 95, {199, 229, 169, 255});

    // Coastline and water.
    DrawEllipse(1050, 610, 310, 115, {111, 196, 211, 255});
    DrawEllipse(1080, 610, 245, 72, {137, 210, 218, 255});
    DrawEllipse(1050, 610, 190, 42, {245, 226, 177, 255});

    // Connected roads make the world map readable as one place.
    const Color road{248, 239, 207, 255};
    const Color roadEdge{225, 207, 166, 255};
    const std::array<Vector2, 7> route{{
        {145, 365}, {310, 300}, {535, 205}, {790, 310},
        {1010, 190}, {850, 520}, {520, 500}
    }};
    for (size_t i = 0; i + 1 < route.size(); ++i) {
        DrawLineEx(route[i], route[i + 1], 43, road);
        DrawLineEx(route[i], route[i + 1], 2.5f, roadEdge);
    }
    DrawLineEx({520, 500}, {310, 300}, 37, road);
    DrawLineEx({520, 500}, {310, 300}, 2.5f, roadEdge);

    for (int i = 0; i < 27; ++i) {
        const float x = 42.0f + i * 45.0f;
        const float y = 640.0f + std::sin(i * 0.72f) * 17.0f;
        Tree(x, y, 0.52f + (i % 3) * 0.08f, {116, 184, 113, 255});
    }
    Tree(165, 220, 0.9f, {105, 177, 111, 255});
    Tree(710, 120, 0.8f, {105, 177, 111, 255});
    Tree(1160, 300, 0.9f, {105, 177, 111, 255});
    Tree(930, 600, 0.75f, {105, 177, 111, 255});
}

void DrawPlaceIcon(const Place& p, bool active) {
    const Vector2 c = p.pos;
    const float pulse = active ? 3.0f + std::sin(animationTime * 4.0f) * 2.0f : 0.0f;
    DrawCircleV(c, 42 + pulse, Fade(WHITE, 0.5f));
    DrawCircleV(c, 35 + pulse, WHITE);
    DrawCircleV(c, 29, p.accent);

    // A distinct simple pictogram for each original region marker.
    switch (p.icon) {
        case 0: // city
            DrawRectangle((int)c.x - 14, (int)c.y - 9, 10, 23, paper);
            DrawRectangle((int)c.x - 2, (int)c.y - 17, 13, 31, paper);
            DrawRectangle((int)c.x + 13, (int)c.y - 4, 8, 18, paper);
            break;
        case 1: // valley
            DrawTriangle({c.x - 21, c.y + 12}, {c.x - 2, c.y - 15}, {c.x + 16, c.y + 12}, paper);
            DrawTriangle({c.x - 5, c.y + 12}, {c.x + 13, c.y - 11}, {c.x + 27, c.y + 12}, Fade(WHITE, 0.8f));
            break;
        case 2: // woods
            DrawTriangle({c.x, c.y - 20}, {c.x - 20, c.y + 10}, {c.x + 20, c.y + 10}, paper);
            DrawRectangle((int)c.x - 4, (int)c.y + 7, 8, 13, {139, 105, 76, 255});
            break;
        case 3: // snow
            DrawLineEx({c.x - 17, c.y}, {c.x + 17, c.y}, 4, paper);
            DrawLineEx({c.x, c.y - 17}, {c.x, c.y + 17}, 4, paper);
            DrawLineEx({c.x - 12, c.y - 12}, {c.x + 12, c.y + 12}, 3, paper);
            DrawLineEx({c.x - 12, c.y + 12}, {c.x + 12, c.y - 12}, 3, paper);
            break;
        case 4: // seaside
            DrawCircleV({c.x - 7, c.y - 7}, 10, paper);
            DrawLineEx({c.x - 19, c.y + 8}, {c.x + 19, c.y + 8}, 4, paper);
            DrawLineEx({c.x - 13, c.y + 16}, {c.x + 13, c.y + 16}, 3, paper);
            break;
        default: // park
            DrawCircleV({c.x, c.y - 5}, 13, paper);
            DrawRectangle((int)c.x - 4, (int)c.y + 5, 8, 16, {139, 105, 76, 255});
            break;
    }

    DrawRectangleRounded({c.x - 82, c.y + 48, 164, 34}, 0.5f, 12, paper);
    DrawText(p.name, (int)c.x - MeasureText(p.name, 16)/2, (int)c.y + 57, 16, ink);
    if (active) DrawCircleLines((int)c.x, (int)c.y, 46 + pulse, coral);
}

void DrawMap() {
    DrawMapBackground();

    DrawRectangleRounded({28, 22, 430, 88}, 0.22f, 18, paper);
    DrawText("POCKET PLACES", 54, 38, 30, ink);
    DrawText("WORLD MAP  /  EXPLORE", 56, 77, 16, muted);

    for (size_t i = 0; i < places.size(); ++i)
        DrawPlaceIcon(places[i], selectedPlace == (int)i);

    DrawRectangleRounded({930, 24, 320, 54}, 0.4f, 16, paper);
    DrawText("6 REGIONS", 955, 42, 19, ink);
    DrawText("Tap a marker to visit", 40, 675, 18, ink);

    if (selectedPlace >= 0) {
        const Place& p = places[(size_t)selectedPlace];
        DrawRectangleRounded({28, 530, 400, 132}, 0.12f, 18, paper);
        DrawRectangleRounded({42, 544, 9, 104}, 0.5f, 6, p.accent);
        DrawText("SELECTED LOCATION", 66, 548, 14, muted);
        DrawText(p.name, 66, 571, 25, ink);
        DrawText(p.description, 66, 608, 16, muted);
        DrawRectangleRounded({290, 615, 116, 34}, 0.5f, 12, p.accent);
        DrawText("VISIT", 325, 624, 16, WHITE);
    }
}

struct SceneProp { Vector2 position; const char* label; const char* message; Color color; };
const std::array<std::array<SceneProp, 3>, 6> sceneProps{{
    {{{{255,485},"SHOP","The little shop is open!",{246,169,105,255}},{{640,510},"CAFE","Fresh treats at the cafe!",{226,135,111,255}},{{1040,510},"BUS","The bus is ready for a ride!",{108,166,198,255}}}},
    {{{{210,485},"GARDEN","The village garden is growing!",{117,183,112,255}},{{640,500},"HOUSE","A cozy home in Green Valley.",{222,164,111,255}},{{1030,500},"POND","A peaceful spot by the water.",{105,185,201,255}}}},
    {{{{250,500},"CAMP","A warm forest camp!",{218,135,83,255}},{{640,520},"CABIN","Someone lives in this cabin.",{172,117,83,255}},{{1030,500},"TREE","Autumn leaves are falling.",{209,143,75,255}}}},
    {{{{220,500},"BAKERY","Warm bread on a snowy day!",{167,202,218,255}},{{640,500},"HOME","A snug little winter home.",{126,171,199,255}},{{1030,500},"PINE","The snow-covered pine is quiet.",{89,148,133,255}}}},
    {{{{230,500},"UMBRELLA","A perfect beach day!",{239,139,119,255}},{{640,520},"BOAT","The boat is ready at the shore.",{102,163,190,255}},{{1030,500},"PIER","Look out over the sea!",{145,112,86,255}}}},
    {{{{220,500},"PICNIC","Time for a picnic!",{225,157,105,255}},{{640,500},"LOOKOUT","What a lovely view!",{130,169,111,255}},{{1030,500},"POND","There are fish in the pond.",{103,184,196,255}}}}
}};
int DiscoveredCount() {
    int total = 0;
    for (int region = 0; region < 6; ++region)
        for (int prop = 0; prop < 3; ++prop)
            if (discoveredProps[region][prop]) ++total;
    return total;
}

void DrawSceneProps() {
    for (size_t i = 0; i < sceneProps[(size_t)selectedPlace].size(); ++i) {
        const SceneProp& p = sceneProps[(size_t)selectedPlace][i];
        const bool found = discoveredProps[selectedPlace][i];
        DrawEllipse(p.position.x,p.position.y+18,29,8,Fade(ink,0.18f));
        DrawRectangleRounded({p.position.x-25,p.position.y-15,50,32},0.28f,8,found ? Fade(p.color,0.55f) : p.color);
        DrawCircleV({p.position.x,p.position.y-17},13,found ? mint : Fade(WHITE,0.9f));
        DrawText(found ? "+" : p.label, (int)p.position.x - (found ? 5 : MeasureText(p.label,12)/2),
                 (int)p.position.y + (found ? -23 : 24), found ? 18 : 12, found ? WHITE : ink);
        if (found) DrawText(p.label,(int)p.position.x-MeasureText(p.label,10)/2,(int)p.position.y+24,10,muted);
    }
}
const SceneProp* FindNearbyProp() {
    for (const SceneProp& p : sceneProps[(size_t)selectedPlace]) {
        const float dx=player.x-p.position.x, dy=player.y-p.position.y;
        if (dx*dx+dy*dy < 105.0f*105.0f) return &p;
    }
    return nullptr;
}

bool IsBuildingProp(const SceneProp* prop) {
    if (!prop) return false;
    const std::string label = prop->label;
    return label == "SHOP" || label == "CAFE" || label == "HOUSE" ||
           label == "CABIN" || label == "BAKERY" || label == "HOME";
}

void DrawInterior() {
    const Place& place = places[(size_t)selectedPlace];
    const Color wall = selectedPlace == 3 ? Color{205, 225, 235, 255} :
                       selectedPlace == 2 ? Color{235, 204, 169, 255} :
                       Color{250, 224, 190, 255};
    DrawRectangleGradientV(0, 0, WORLD_W, WORLD_H, wall, {239, 204, 161, 255});
    DrawRectangle(0, 500, WORLD_W, 220, {170, 125, 91, 255});
    DrawRectangle(0, 490, WORLD_W, 18, {235, 192, 143, 255});
    DrawRectangle(105, 105, 260, 210, {139, 195, 208, 255});
    DrawRectangleLinesEx({105, 105, 260, 210}, 12, {255, 247, 225, 255});
    DrawLineEx({235, 105}, {235, 315}, 9, {255, 247, 225, 255});
    DrawLineEx({105, 210}, {365, 210}, 9, {255, 247, 225, 255});
    DrawRectangleRounded({440, 350, 390, 135}, 0.08f, 12, {185, 126, 87, 255});
    DrawRectangleRounded({465, 320, 340, 85}, 0.12f, 12, {247, 225, 194, 255});
    DrawRectangleRounded({490, 295, 95, 45}, 0.2f, 10, {255, 247, 225, 255});
    DrawRectangleRounded({680, 295, 95, 45}, 0.2f, 10, {255, 247, 225, 255});
    DrawRectangleRounded({920, 350, 210, 135}, 0.08f, 12, {125, 165, 129, 255});
    DrawRectangleRounded({945, 320, 160, 60}, 0.18f, 12, {202, 224, 186, 255});
    DrawCircle(1025, 275, 42, {103, 165, 113, 255});
    DrawRectangle(1017, 310, 16, 38, {139, 105, 76, 255});

    // Each enterable place gets a recognizable interior feature.
    if (currentInteriorLabel == "SHOP") {
        for (int row = 0; row < 3; ++row) {
            DrawRectangle(75, 355 + row * 42, 250, 9, {139, 105, 76, 255});
            for (int col = 0; col < 4; ++col)
                DrawRectangleRounded({88.0f + col * 57.0f, 322.0f + row * 42.0f, 34, 30},
                                     0.18f, 6, (row + col) % 2 ? coral : mint);
        }
        DrawText("FRESH  •  LOCAL  •  FUN", 95, 280, 18, ink);
    } else if (currentInteriorLabel == "CAFE") {
        for (int i = 0; i < 2; ++i) {
            const int x = 250 + i * 690;
            DrawRectangleRounded({(float)x, 430, 145, 15}, 0.4f, 8, {139, 105, 76, 255});
            DrawRectangle(x + 16, 445, 10, 55, {139, 105, 76, 255});
            DrawRectangle(x + 119, 445, 10, 55, {139, 105, 76, 255});
            DrawCircle(x + 70, 411, 16, {225, 139, 110, 255});
            DrawCircle(x + 70, 411, 7, {255, 235, 196, 255});
        }
        DrawText("CAFE  /  OPEN", 485, 155, 28, ink);
    } else if (currentInteriorLabel == "BAKERY") {
        DrawRectangleRounded({820, 245, 320, 95}, 0.12f, 12, {188, 129, 88, 255});
        for (int i = 0; i < 5; ++i) {
            DrawEllipse(850 + i * 58, 280, 20, 13, {226, 174, 103, 255});
            DrawLineEx({840.0f + i * 58, 280}, {860.0f + i * 58, 280}, 2, {255, 231, 186, 255});
        }
        DrawText("BAKED TODAY", 870, 215, 20, ink);
    } else {
        DrawRectangleRounded({470, 430, 330, 34}, 0.4f, 12, {218, 179, 133, 255});
        DrawRectangleRounded({520, 448, 18, 55}, 0.3f, 8, {139, 105, 76, 255});
        DrawRectangleRounded({730, 448, 18, 55}, 0.3f, 8, {139, 105, 76, 255});
    }

    DrawRectangleRounded({28, 24, 540, 88}, 0.22f, 16, paper);
    DrawText(currentInteriorLabel.c_str(), 54, 40, 30, ink);
    DrawText(place.name, 56, 80, 16, muted);
    DrawRectangleRounded({1080, 24, 170, 54}, 0.4f, 16, paper);
    DrawText("GO OUTSIDE", 1092, 42, 16, ink);
    DrawRectangleRounded({28, 650, 430, 42}, 0.35f, 12, Fade(paper, 0.92f));
    DrawText("ESC / BACKSPACE: GO OUTSIDE", 45, 663, 16, ink);
    DrawCircle(640, 550, 34, {240, 190, 137, 255});
    DrawRectangleRounded({622, 570, 36, 48}, 0.35f, 10, characterStyle == 0 ? coral :
                         characterStyle == 1 ? Color{104, 163, 204, 255} :
                         characterStyle == 2 ? Color{133, 185, 124, 255} :
                                               Color{185, 132, 190, 255});
    DrawCircle(628, 546, 3, ink);
    DrawCircle(652, 546, 3, ink);
}

void DrawCharacter() {
    const float moving = walkTime > 0.0f ? 1.0f : 0.0f;
    const float bob = moving * std::fabs(std::sin(animationTime * 12.0f)) * 3.0f;
    const float legSwing = moving * std::sin(animationTime * 12.0f) * 7.0f;
    const float y = player.y - bob;
    DrawEllipse(player.x, player.y + 43, 25, 9, Fade(ink, 0.2f));
    DrawLineEx({player.x - 7, y + 30}, {player.x - 8 - legSwing, y + 45}, 7, {89, 116, 145, 255});
    DrawLineEx({player.x + 7, y + 30}, {player.x + 8 + legSwing, y + 45}, 7, {89, 116, 145, 255});
    const Color shirt = characterStyle == 0 ? coral : characterStyle == 1 ? Color{104,163,204,255} : characterStyle == 2 ? Color{133,185,124,255} : Color{185,132,190,255};
    DrawRectangleRounded({player.x - 18, y - 2, 36, 43}, 0.35f, 10, shirt);
    DrawCircleV({player.x, y - 18}, 19, {246, 195, 150, 255});
    const float eyeOffset = facing >= 0 ? 1.0f : -1.0f;
    DrawCircleV({player.x - 7 + eyeOffset*2, y - 19}, 3, ink);
    DrawCircleV({player.x + 7 + eyeOffset*2, y - 19}, 3, ink);
    DrawLineEx({player.x - 6, y - 8}, {player.x + 6, y - 8}, 2, ink);
}

void DrawRegionScene() {
    const Place& p = places[(size_t)selectedPlace];
    DrawRectangleGradientV(0, 0, WORLD_W, WORLD_H, {166, 220, 232, 255}, p.land);
    DrawCircle(1080, 105, 48, {255, 224, 133, 255});

    // Every region now has its own recognizable outdoor layout and landmarks.
    switch (p.icon) {
        case 0: { // Sunny City
            DrawRectangle(0, 430, WORLD_W, 290, {231, 202, 159, 255});
            DrawRectangle(0, 485, WORLD_W, 235, {190, 190, 177, 255});
            DrawRectangle(0, 570, WORLD_W, 150, {117, 157, 151, 255});
            for (int i = 0; i < 5; ++i) {
                const int x = 55 + i * 245;
                const int h = 170 + (i % 3) * 55;
                DrawRectangle(x, 430 - h, 190, h, i % 2 ? Color{248, 184, 135, 255} : Color{244, 218, 166, 255});
                DrawRectangle(x - 8, 430 - h - 12, 206, 16, {210, 133, 105, 255});
                for (int row = 0; row < 3; ++row)
                    for (int col = 0; col < 3; ++col)
                        DrawRectangle(x + 22 + col * 53, 430 - h + 25 + row * 47, 27, 31, {130, 190, 204, 255});
            }
            DrawRectangle(470, 370, 300, 155, {246, 145, 113, 255});
            DrawRectangle(455, 355, 330, 22, {255, 222, 169, 255});
            DrawText("CAFE", 575, 415, 30, WHITE);
            break;
        }
        case 1: { // Green Valley
            DrawEllipse(300, 445, 390, 210, {137, 194, 125, 255});
            DrawEllipse(770, 440, 470, 235, {117, 181, 112, 255});
            DrawEllipse(1050, 515, 220, 110, {119, 193, 207, 255});
            DrawTriangle({115, 410}, {330, 180}, {550, 410}, {142, 188, 135, 255});
            DrawTriangle({520, 420}, {770, 215}, {1000, 420}, {158, 202, 145, 255});
            for (int i = 0; i < 4; ++i) {
                const int x = 145 + i * 265;
                DrawRectangle(x, 360, 145, 115, {244, 218, 169, 255});
                DrawTriangle({(float)(x - 12), 360.0f}, {(float)(x + 72), 285.0f}, {(float)(x + 157), 360.0f}, {190, 119, 94, 255});
                DrawRectangle(x + 55, 400, 34, 75, {139, 98, 75, 255});
                Tree((float)x + 190, 430, 0.9f, {91, 158, 97, 255});
            }
            break;
        }
        case 2: { // Autumn Woods
            DrawRectangle(0, 420, WORLD_W, 300, {196, 143, 94, 255});
            DrawTriangle({0, 450}, {240, 170}, {470, 450}, {112, 151, 108, 255});
            DrawTriangle({430, 450}, {710, 130}, {1010, 450}, {121, 160, 111, 255});
            DrawTriangle({850, 450}, {1080, 210}, {1280, 450}, {102, 146, 105, 255});
            for (int i = 0; i < 12; ++i) {
                const float x = 45.0f + i * 108.0f;
                const float y = 390.0f + (i % 3) * 25.0f;
                Tree(x, y, 1.2f + (i % 2) * 0.25f,
                     i % 3 == 0 ? Color{211, 115, 69, 255} : Color{226, 160, 78, 255});
            }
            DrawRectangle(515, 355, 245, 150, {211, 151, 105, 255});
            DrawTriangle({495, 360}, {637, 245}, {780, 360}, {110, 83, 68, 255});
            DrawRectangle(615, 410, 48, 95, {112, 79, 63, 255});
            DrawRectangle(548, 390, 42, 42, {135, 195, 207, 255});
            DrawRectangle(690, 390, 42, 42, {135, 195, 207, 255});
            break;
        }
        case 3: { // Snowy Town
            DrawRectangle(0, 420, WORLD_W, 300, {239, 246, 249, 255});
            DrawTriangle({-80, 430}, {245, 105}, {580, 430}, {172, 203, 218, 255});
            DrawTriangle({420, 430}, {790, 75}, {1170, 430}, {188, 214, 225, 255});
            DrawTriangle({145, 220}, {245, 105}, {340, 220}, WHITE);
            DrawTriangle({690, 170}, {790, 75}, {890, 170}, WHITE);
            for (int i = 0; i < 5; ++i) {
                const int x = 75 + i * 240;
                DrawRectangle(x, 340, 170, 125, {173, 207, 222, 255});
                DrawTriangle({(float)(x - 14), 342.0f}, {(float)(x + 85), 255.0f}, {(float)(x + 184), 342.0f}, {105, 146, 174, 255});
                DrawRectangle(x + 68, 390, 36, 75, {133, 98, 81, 255});
                DrawRectangle(x + 20, 365, 35, 34, {123, 183, 201, 255});
                Tree((float)x + 205, 430, 0.95f, {75, 133, 122, 255});
            }
            break;
        }
        case 4: { // Seaside
            DrawRectangle(0, 390, WORLD_W, 330, {103, 194, 211, 255});
            DrawRectangle(0, 390, WORLD_W, 105, {250, 222, 163, 255});
            DrawEllipse(270, 525, 330, 125, {255, 231, 180, 255});
            for (int i = 0; i < 5; ++i) {
                const int x = 110 + i * 250;
                DrawLineEx({(float)x, 430.0f}, {(float)x + 40, 560.0f}, 5, {135, 99, 74, 255});
                DrawTriangle({(float)x - 50, 430}, {(float)x + 40, 360}, {(float)x + 105, 430},
                             i % 2 ? Color{245, 139, 119, 255} : Color{250, 240, 211, 255});
            }
            DrawTriangle({760, 495}, {900, 495}, {835, 390}, {255, 250, 232, 255});
            DrawLineEx({835, 385}, {835, 530}, 6, {125, 91, 70, 255});
            DrawRectangle(795, 530, 85, 12, {132, 94, 72, 255});
            for (int i = 0; i < 7; ++i)
                DrawLineEx({(float)(i * 190), 585.0f + (i % 2) * 35.0f},
                           {(float)(i * 190 + 90), 585.0f + (i % 2) * 35.0f}, 3, Fade(WHITE, 0.65f));
            break;
        }
        default: { // Highland Park
            DrawRectangle(0, 425, WORLD_W, 295, {160, 205, 139, 255});
            DrawEllipse(680, 555, 250, 95, {106, 188, 202, 255});
            for (int i = 0; i < 11; ++i) {
                const float x = 45.0f + i * 118.0f;
                Tree(x, 420.0f + (i % 3) * 30.0f, 1.0f + (i % 2) * 0.25f, {83, 157, 99, 255});
            }
            DrawRectangle(500, 310, 280, 18, {133, 95, 72, 255});
            DrawRectangle(520, 328, 12, 120, {133, 95, 72, 255});
            DrawRectangle(748, 328, 12, 120, {133, 95, 72, 255});
            DrawTriangle({480, 310}, {640, 215}, {800, 310}, {226, 160, 103, 255});
            DrawRectangle(560, 375, 160, 15, {133, 95, 72, 255});
            break;
        }
    }

    DrawSceneProps();
    DrawCharacter();
    if (FindNearbyProp()) {
        DrawRectangleRounded({555, 610, 170, 42}, 0.35f, 12, coral);
        DrawText("E  INTERACT", 580, 623, 17, WHITE);
    }

    DrawRectangleRounded({28, 24, 510, 88}, 0.22f, 16, paper);
    DrawText(p.name, 54, 40, 30, ink);
    DrawText(p.description, 56, 80, 16, muted);
    DrawRectangleRounded({1080, 24, 170, 54}, 0.4f, 16, paper);
    DrawText("WORLD MAP", 1101, 42, 18, ink);
    DrawRectangleRounded({28, 650, 440, 42}, 0.35f, 12, Fade(paper, 0.92f));
    DrawText("WASD: MOVE   E: INTERACT   C: CHANGE OUTFIT", 45, 663, 16, ink);
    DrawRectangleRounded({920, 90, 330, 48}, 0.35f, 12, paper);
    DrawText(TextFormat("DISCOVERIES  %02i / 18", DiscoveredCount()), 945, 105, 18, ink);
    if (animationTime < messageUntil) {
        DrawRectangleRounded({465, 610, 350, 52}, 0.35f, 14, paper);
        DrawText(sceneMessage.c_str(), 490, 627, 19, ink);
    }

    const float padX = 1130.0f, padY = 565.0f;
    DrawCircleV({padX, padY}, 72, Fade(paper, 0.75f));
    DrawCircleV({padX, padY}, 27, Fade(mint, 0.9f));
    DrawTriangle({padX, padY - 58}, {padX - 14, padY - 34}, {padX + 14, padY - 34}, ink);
    DrawTriangle({padX, padY + 58}, {padX - 14, padY + 34}, {padX + 14, padY + 34}, ink);
    DrawTriangle({padX - 58, padY}, {padX - 34, padY - 14}, {padX - 34, padY + 14}, ink);
    DrawTriangle({padX + 58, padY}, {padX + 34, padY - 14}, {padX + 34, padY + 14}, ink);
}
void UpdateGame(float dt) {
    animationTime += dt;
    UpdateCamera();
    const Vector2 pointer = WorldPointer();

    if (showMap) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < places.size(); ++i) {
                const Vector2 p = places[i].pos;
                if (CheckCollisionPointCircle(pointer, p, 48)) {
                    selectedPlace = (int)i;
                    break;
                }
            }
            if (selectedPlace >= 0 && InRect({290, 615, 116, 34}, pointer)) {
                showMap = false;
                player = {640, 500};
            }
        }
        if (IsKeyPressed(KEY_ENTER) && selectedPlace >= 0) {
            showMap = false;
            player = {640, 500};
        }
        if (IsKeyPressed(KEY_RIGHT)) selectedPlace = (selectedPlace + 1) % (int)places.size();
        if (IsKeyPressed(KEY_LEFT)) selectedPlace = (selectedPlace <= 0) ? (int)places.size() - 1 : selectedPlace - 1;
        return;
    }

    if (insideBuilding) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) ||
            (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && InRect({1080, 24, 170, 54}, pointer))) {
            insideBuilding = false;
            return;
        }
        if (IsKeyPressed(KEY_C)) characterStyle = (characterStyle + 1) % 4;
        return;
    }

    Vector2 direction{0, 0};
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;

    // Touch-friendly directional pad: drag from its center toward a direction.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && pointer.x > 1035 && pointer.y > 470) {
        const float dx = pointer.x - 1130.0f;
        const float dy = pointer.y - 565.0f;
        const float length = std::sqrt(dx*dx + dy*dy);
        if (length > 10.0f) {
            direction.x += dx / length;
            direction.y += dy / length;
        }
    }

    const float magnitude = std::sqrt(direction.x*direction.x + direction.y*direction.y);
    if (magnitude > 0.0f) {
        direction.x /= magnitude;
        direction.y /= magnitude;
        player.x += direction.x * 245.0f * dt;
        player.y += direction.y * 245.0f * dt;
        walkTime = 1.0f;
        if (std::fabs(direction.x) > 0.1f) facing = direction.x > 0 ? 1.0f : -1.0f;
    } else {
        walkTime = 0.0f;
    }

    player.x = std::clamp(player.x, 55.0f, 1225.0f);
    player.y = std::clamp(player.y, 175.0f, 630.0f);

    if (IsKeyPressed(KEY_E) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && InRect({555, 610, 170, 42}, pointer))) {
        if (const SceneProp* prop = FindNearbyProp()) {
            const int propIndex = (int)(prop - sceneProps[(size_t)selectedPlace].data());
            const bool firstVisit = !discoveredProps[selectedPlace][propIndex];
            discoveredProps[selectedPlace][propIndex] = true;
            sceneMessage = firstVisit ? std::string("Added to your discovery book: ") + prop->label : prop->message;
            if (IsBuildingProp(prop)) {
                insideBuilding = true;
                currentInteriorLabel = prop->label;
            }
        } else sceneMessage = "Walk closer to an object to interact!";
        messageUntil = animationTime + 2.8f;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && InRect({1080, 24, 170, 54}, pointer)) showMap = true;
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) showMap = true;
}
}

int main(int, char**) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(WORLD_W, WORLD_H, "Pocket Places");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const float dt = std::min(GetFrameTime(), 0.04f);
        UpdateGame(dt);

        BeginDrawing();
        ClearBackground({225, 241, 232, 255});
        BeginMode2D(camera);
        if (showMap) DrawMap();
        else if (insideBuilding) DrawInterior();
        else DrawRegionScene();
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
