// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "effect/effect.h"

#include <map>
#include <memory>

namespace KWin
{

class GLTexture;
class GLFramebuffer;
class GLShader;

class CrtEffect : public Effect
{
    Q_OBJECT

public:
    CrtEffect();
    ~CrtEffect() override;

    void reconfigure(ReconfigureFlags flags) override;
    void prePaintScreen(ScreenPrePaintData &data) override;
    void paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport,
                     int mask, const Region &deviceRegion, LogicalOutput *screen) override;

    bool isActive() const override;
    int requestedEffectChainPosition() const override { return 0; }

    static bool supported();

private:
    struct OutputState
    {
        std::unique_ptr<GLTexture> texture;
        std::unique_ptr<GLFramebuffer> framebuffer;
    };

    void slotScreenRemoved(LogicalOutput *screen);

    std::map<LogicalOutput *, OutputState> m_states;
    std::unique_ptr<GLShader> m_shader;
    bool m_valid = false;

    int m_locMvp = -1;
    int m_locResolution = -1;
    int m_locTexture = -1;
    int m_locBrightness = -1;
    int m_locContrast = -1;
    int m_locGamma = -1;
    int m_locMaskType = -1;
    int m_locMaskStrength = -1;
    int m_locMaskSize = -1;
    int m_locColorScheme = -1;
    int m_locScanlineStrength = -1;
    int m_locScanlineSize = -1;
    int m_locScanlineBeam = -1;
    int m_locGlow = -1;
    int m_locGlowRadius = -1;
    int m_locSaturation = -1;
    int m_locCurvature = -1;
    int m_locVignette = -1;
};

}
