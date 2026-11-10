#include "animated_logo.hpp"

#include <algorithm>
#include <cmath>

namespace {
    constexpr const char SHEET_PATH[] = "romfs:/branding/logo_anim.png";
    constexpr int SHEET_FRAMES = 28;  // 27 build-up frames + 1 finished-logo frame
    constexpr int INTRO_FRAMES = SHEET_FRAMES - 1;
    constexpr int INTRO_FPS = 12;

    constexpr int INTRO_MS = INTRO_FRAMES * 1000 / INTRO_FPS;  // 2250
    constexpr int HOLD_MS = 4500;
    constexpr int FADE_MS = 500;
    constexpr int GAP_MS = 500;
    constexpr int CYCLE_MS = INTRO_MS + HOLD_MS + FADE_MS + GAP_MS;

    constexpr float LOGO_HEIGHT = 58.0f;  // header is 88 px high

    // One texture for the whole process: MainFrame is re-created after every
    // download, so loading it per instance would leak GPU memory.
    int g_sheet = -1;
    bool g_loadTried = false;
}  // namespace

AnimatedLogo::AnimatedLogo(const std::string& title) : title(title), start(std::chrono::steady_clock::now())
{
}

void AnimatedLogo::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    if (!g_loadTried) {
        g_loadTried = true;
        g_sheet = nvgCreateImage(vg, SHEET_PATH, 0);
    }

    // (x, y) is the 52x52 icon slot: left padding 64, top padding 20 -> header top = y - 20.
    const float headerTop = static_cast<float>(y) - style->AppletFrame.imageTopPadding;
    float logoRight = static_cast<float>(x);

    if (g_sheet > 0) {
        int sheetW = 0, sheetH = 0;
        nvgImageSize(vg, g_sheet, &sheetW, &sheetH);
        if (sheetW > 0 && sheetH >= SHEET_FRAMES) {
            const float frameH = static_cast<float>(sheetH) / SHEET_FRAMES;
            const float drawH = LOGO_HEIGHT;
            const float drawW = drawH * static_cast<float>(sheetW) / frameH;
            const float drawX = static_cast<float>(x);
            const float drawY = headerTop + (style->AppletFrame.headerHeightRegular - drawH) / 2.0f;
            logoRight = drawX + drawW;

            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->start).count();
            const int t = static_cast<int>(elapsed % CYCLE_MS);

            int frame = INTRO_FRAMES;  // finished logo
            float alpha = 1.0f;
            if (t < INTRO_MS)
                frame = std::min(INTRO_FRAMES - 1, t * INTRO_FPS / 1000);
            else if (t >= INTRO_MS + HOLD_MS + FADE_MS)
                alpha = 0.0f;
            else if (t >= INTRO_MS + HOLD_MS)
                alpha = 1.0f - static_cast<float>(t - INTRO_MS - HOLD_MS) / FADE_MS;

            if (alpha > 0.0f) {
                // The sheet is stretched to drawW x (drawH * SHEET_FRAMES) and shifted so
                // that exactly one frame lies inside the rectangle we fill.
                NVGpaint paint = nvgImagePattern(vg, drawX, drawY - frame * drawH, drawW, drawH * SHEET_FRAMES, 0.0f, g_sheet, alpha * this->getAlpha());
                nvgBeginPath(vg);
                nvgRect(vg, drawX, drawY, drawW, drawH);
                nvgFillPaint(vg, paint);
                nvgFill(vg);
            }
        }
    }

    // App title right next to the logo, same vertical position/size as the stock title.
    if (!this->title.empty()) {
        nvgBeginPath(vg);
        nvgFillColor(vg, this->a(ctx->theme->textColor));
        nvgFontSize(vg, style->AppletFrame.titleSize);
        nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgText(vg, logoRight + 28.0f, headerTop + style->AppletFrame.headerHeightRegular / 2 + style->AppletFrame.titleOffset, this->title.c_str(), nullptr);
    }
}
