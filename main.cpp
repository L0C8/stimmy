#include <raylib.h>
#include <string>
#include <vector>
#include <cmath>

// Small utility lerp
static float Lerp(float a, float b, float t) { return a + (b - a) * t; }

int main() {
    const int screenWidth = 256;
    const int screenHeight = 256;

    InitWindow(screenWidth, screenHeight, "Stimmy - Juicy Menu Demo");
    SetWindowState(FLAG_WINDOW_TOPMOST);
    SetTargetFPS(60);

    // Load shader from files in `shaders/` (vertex, fragment)
    Shader shader = LoadShader("shaders/gradient.vs", "shaders/gradient.fs");
    int uTimeLoc = GetShaderLocation(shader, "uTime");
    int uResLoc = GetShaderLocation(shader, "uResolution");

    // Menu data
    Font font = GetFontDefault();
    const float baseFontSize = 48.0f;
    const std::vector<std::string> items = {"New Game", "Exit"};

    // Pre-create render textures for menu items (we'll recreate if size changes)
    std::vector<RenderTexture2D> itemTex(items.size());
    for (size_t i = 0; i < itemTex.size(); ++i) itemTex[i].id = 0;

    // animation state per item
    struct ItemState { float alpha = 0.6f; float scale = 1.0f; float rotation = 0.0f; };
    std::vector<ItemState> states(items.size());

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float time = GetTime();

        // update shader uniforms
        SetShaderValue(shader, uTimeLoc, &time, SHADER_UNIFORM_FLOAT);
        float res[2] = { (float)screenWidth, (float)screenHeight };
        SetShaderValue(shader, uResLoc, res, SHADER_UNIFORM_VEC2);

        // layout menu: centered
        float totalHeight = items.size() * baseFontSize * 1.4f;
        float startY = screenHeight*0.5f - totalHeight*0.5f;

        Vector2 mouse = GetMousePosition();

        // prepare draw (we'll use the shader for background)
        BeginDrawing();
            ClearBackground(BLANK);

            // shader background
            BeginShaderMode(shader);
                DrawRectangle(0, 0, screenWidth, screenHeight, WHITE);
            EndShaderMode();

            // Draw menu items
            for (size_t i = 0; i < items.size(); ++i) {
                std::string txt = items[i];
                Vector2 txtSize = MeasureTextEx(font, txt.c_str(), baseFontSize, 2.0f);
                int padding = 16;
                int texW = (int)(txtSize.x) + padding*2;
                int texH = (int)(txtSize.y) + padding*2;

                // (re)create render texture if needed
                if (itemTex[i].id == 0 || itemTex[i].texture.width != texW || itemTex[i].texture.height != texH) {
                    if (itemTex[i].id != 0) UnloadRenderTexture(itemTex[i]);
                    itemTex[i] = LoadRenderTexture(texW, texH);
                }

                // render text to texture
                BeginTextureMode(itemTex[i]);
                    ClearBackground(BLANK);
                    DrawTextEx(font, txt.c_str(), {(float)padding, (float)padding}, baseFontSize, 2.0f, WHITE);
                EndTextureMode();

                // compute position for this item (centered horizontally)
                float x = screenWidth*0.5f;
                float y = startY + i * (baseFontSize * 1.4f) + texH*0.5f;

                // RenderTexture's texture coordinates are flipped vertically in raylib,
                // so set src.height negative to draw the texture upright.
                Rectangle src = { 0.0f, (float)texH, (float)texW, -(float)texH };
                Rectangle dst = { x - texW*0.5f, y - texH*0.5f, (float)texW, (float)texH };
                Vector2 origin = { (float)texW*0.5f, (float)texH*0.5f };

                // mouse hover detection in destination bounds (use untransformed bounds for hover)
                Rectangle hoverRect = dst;
                bool hovered = CheckCollisionPointRec(mouse, hoverRect);

                // target states
                float targetAlpha = hovered ? 1.0f : 0.6f;
                float targetScale = hovered ? 1.06f : 1.0f;
                float targetRot = hovered ? (sinf(time*4.0f) * 5.0f) : 0.0f; // degrees

                // smooth transitions
                float speed = 8.0f;
                states[i].alpha = Lerp(states[i].alpha, targetAlpha, dt*speed);
                states[i].scale = Lerp(states[i].scale, targetScale, dt*speed);
                states[i].rotation = Lerp(states[i].rotation, targetRot, dt*6.0f);

                // apply pulsating growth when hovered
                float pulse = hovered ? (1.0f + 0.02f * sin(time*8.0f)) : 1.0f;
                float finalScale = states[i].scale * pulse;

                // adjust dst to scale around center
                Rectangle dstScaled = dst;
                dstScaled.width *= finalScale;
                dstScaled.height *= finalScale;
                dstScaled.x = x - dstScaled.width*0.5f;
                dstScaled.y = y - dstScaled.height*0.5f;

                // draw the textured text with rotation + alpha
                DrawTexturePro(itemTex[i].texture, src, dstScaled, origin, states[i].rotation, Fade(WHITE, states[i].alpha));
            }

        EndDrawing();
    }

    // cleanup
    for (size_t i = 0; i < itemTex.size(); ++i) if (itemTex[i].id != 0) UnloadRenderTexture(itemTex[i]);
    UnloadShader(shader);
    CloseWindow();

    return 0;
}