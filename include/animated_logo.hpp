#pragma once

#include <borealis.hpp>
#include <chrono>
#include <string>

// Animated "CustomLegends" neon wordmark for the header of the main screen.
// Plays: build-up (about 2 s) -> finished logo -> fade out -> short pause -> again.
// The frames are one tall sprite sheet (romfs:/branding/logo_anim.png), the LAST
// frame of the sheet is the finished logo. It is used as the "icon" of the frame
// but draws wider than the 52x52 icon slot, and it also draws the app title next
// to the logo (so the title is not hidden behind it).
class AnimatedLogo : public brls::View
{
public:
    explicit AnimatedLogo(const std::string& title);

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
    void layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash) override {}

private:
    std::string title;
    std::chrono::steady_clock::time_point start;
};
