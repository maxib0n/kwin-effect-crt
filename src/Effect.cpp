// SPDX-License-Identifier: GPL-3.0-or-later
#include "Effect.h"
#include "Config.h"

#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "opengl/glframebuffer.h"
#include "opengl/glshader.h"
#include "opengl/glshadermanager.h"
#include "opengl/gltexture.h"
#include "opengl/glvertexbuffer.h"

#include <QMatrix4x4>
#include <QVector2D>
#include <span>
#include <epoxy/gl.h>

namespace KWin
{

static const QByteArray s_vertSource = QByteArrayLiteral(R"(
#version 140
uniform mat4 modelViewProjectionMatrix;
in vec2 position;
in vec2 texcoord;
out vec2 texcoord0;
void main()
{
    gl_Position = modelViewProjectionMatrix * vec4(position, 0.0, 1.0);
    texcoord0 = texcoord;
}
)");

static const QByteArray s_fragSource = QByteArrayLiteral(R"(
#version 140
uniform sampler2D sampler;
uniform vec2  resolution;
uniform float brightness;
uniform float contrast;
uniform float gammaAdj;
uniform int   maskType;
uniform float maskStrength;
uniform float maskSize;
uniform int   colorScheme;
uniform float scanlineStrength;
uniform float scanlineSize;
uniform float scanlineBeam;
uniform float glow;
uniform float glowRadius;
uniform float saturation;
uniform float curvature;
uniform float vignette;

in  vec2 texcoord0;
out vec4 fragColor;

const float TAU  = 6.28318530718;
const vec3  LUMA = vec3(0.2126, 0.7152, 0.0722);

vec3 toLinear(vec3 c)
{
    c = max(c, vec3(0.0));
    bvec3 lo = lessThanEqual(c, vec3(0.04045));
    vec3 a = c / 12.92;
    vec3 b = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(b, a, vec3(lo));
}
vec3 toSRGB(vec3 c)
{
    c = max(c, vec3(0.0));
    bvec3 lo = lessThanEqual(c, vec3(0.0031308));
    vec3 a = c * 12.92;
    vec3 b = 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055;
    return mix(b, a, vec3(lo));
}
vec3 sampleLin(vec2 uv)
{
    return toLinear(texture(sampler, clamp(uv, vec2(0.0), vec2(1.0))).rgb);
}
vec3 stripeRGB(float phase)
{
    vec3 g = 0.5 + 0.5 * cos((phase - vec3(0.0, 1.0/3.0, 2.0/3.0)) * TAU);
    g *= 3.0 / max(g.r + g.g + g.b, 1e-3);
    return g;
}
vec3 maskApertureGrille(vec2 px, float pitch)
{
    return stripeRGB(fract(px.x / max(pitch, 1.0)));
}
vec3 maskShadow(vec2 px, float pitch)
{
    float p    = max(pitch, 1.0);
    float row  = floor(px.y / p);
    float stag = mod(row, 2.0) * 0.5;
    vec3  rgb  = stripeRGB(fract(px.x / p + stag));
    float vy   = 0.5 + 0.5 * cos(fract(px.y / p) * TAU);
    vy = pow(vy, 0.5);
    vy = mix(0.5, 1.0, vy);
    vy *= 1.0 / 0.75;
    return rgb * vy;
}
vec3 maskSlot(vec2 px, float pitch)
{
    float p     = max(pitch, 1.0);
    float slotH = p * 2.0;
    float row   = floor(px.y / slotH);
    float stag  = mod(row, 2.0) * 0.5;
    vec3  rgb   = stripeRGB(fract(px.x / p + stag));
    float vy    = 0.5 + 0.5 * cos(fract(px.y / slotH) * TAU);
    vy = mix(0.6, 1.0, vy);
    vy *= 1.0 / 0.8;
    return rgb * vy;
}
vec3 selectMask(vec2 px, int type, float pitch)
{
    if (type == 1) return maskShadow(px, pitch);
    if (type == 2) return maskSlot(px, pitch);
    return maskApertureGrille(px, pitch);
}
float scanlineWeight(vec2 px, float period, float beam, float strength)
{
    float per   = max(period, 1.0);
    float phase = fract(px.y / per) - 0.5;
    float d     = phase * per;
    float sigma = max(beam, 0.05) * per * 0.25;
    float w     = exp(-(d * d) / (2.0 * sigma * sigma));
    float avg   = clamp(sigma * 2.50662827 / per, 0.05, 1.0);
    float wn    = min(w / avg, 2.5);
    return mix(1.0, wn, strength);
}
vec3 applyScheme(vec3 lin, int scheme)
{
    if (scheme == 0) return lin;
    float y = dot(lin, LUMA);
    vec3 tint;
    if      (scheme == 1) tint = vec3(0.20, 1.00, 0.30);
    else if (scheme == 2) tint = vec3(1.00, 0.65, 0.10);
    else                  tint = vec3(1.00, 1.00, 1.00);
    return y * toLinear(tint);
}

void main(void)
{
    vec2 uv = texcoord0;
    if (curvature > 0.0) {
        vec2 cc  = uv * 2.0 - 1.0;
        vec2 off = abs(cc.yx) * curvature;
        cc += cc * off * off;
        uv = cc * 0.5 + 0.5;
        if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
    }

    vec2 px = uv * resolution;
    vec3 lin = toLinear(texture(sampler, uv).rgb);

    if (glow > 0.0) {
        vec2 r = glowRadius / max(resolution, vec2(1.0));
        vec3 acc = sampleLin(uv) * 0.25;
        acc += (sampleLin(uv + vec2( r.x, 0.0))
              + sampleLin(uv + vec2(-r.x, 0.0))
              + sampleLin(uv + vec2( 0.0, r.y))
              + sampleLin(uv + vec2( 0.0,-r.y))) * 0.125;
        acc += (sampleLin(uv + vec2( r.x, r.y))
              + sampleLin(uv + vec2(-r.x, r.y))
              + sampleLin(uv + vec2( r.x,-r.y))
              + sampleLin(uv + vec2(-r.x,-r.y))) * 0.0625;
        lin += max(acc - 0.5, vec3(0.0)) * glow;
    }

    if (colorScheme == 0) {
        float y = dot(lin, LUMA);
        lin = max(mix(vec3(y), lin, saturation), vec3(0.0));
    }
    lin = applyScheme(lin, colorScheme);

    if (maskStrength > 0.0) {
        vec3 m = selectMask(px, maskType, maskSize);
        lin *= mix(vec3(1.0), m, maskStrength);
    }
    if (scanlineStrength > 0.0) {
        lin *= scanlineWeight(px, scanlineSize, scanlineBeam, scanlineStrength);
    }

    lin = max((lin - 0.5) * contrast + 0.5, vec3(0.0));
    lin *= brightness;

    if (vignette > 0.0) {
        float vg = (uv.x * (1.0 - uv.x)) * (uv.y * (1.0 - uv.y));
        lin *= clamp(pow(vg * 16.0, vignette), 0.0, 1.0);
    }

    lin = pow(max(lin, vec3(0.0)), vec3(1.0 / max(gammaAdj, 0.05)));
    fragColor = vec4(clamp(toSRGB(lin), vec3(0.0), vec3(1.0)), 1.0);
}
)");

CrtEffect::CrtEffect()
{
    CrtTrinitron::Config::self()->config()->reparseConfiguration();
    CrtTrinitron::Config::self()->load();

    m_shader = ShaderManager::instance()->generateCustomShader(
        ShaderTrait::MapTexture, s_vertSource, s_fragSource);

    m_valid = m_shader != nullptr;
    if (m_valid) {
        m_locMvp = m_shader->uniformLocation("modelViewProjectionMatrix");
        if (m_locMvp < 0) {
            m_valid = false;
        }
    }
    if (m_valid) {
        m_locResolution       = m_shader->uniformLocation("resolution");
        m_locTexture          = m_shader->uniformLocation("sampler");
        m_locBrightness       = m_shader->uniformLocation("brightness");
        m_locContrast         = m_shader->uniformLocation("contrast");
        m_locGamma            = m_shader->uniformLocation("gammaAdj");
        m_locMaskType         = m_shader->uniformLocation("maskType");
        m_locMaskStrength     = m_shader->uniformLocation("maskStrength");
        m_locMaskSize         = m_shader->uniformLocation("maskSize");
        m_locColorScheme      = m_shader->uniformLocation("colorScheme");
        m_locScanlineStrength = m_shader->uniformLocation("scanlineStrength");
        m_locScanlineSize     = m_shader->uniformLocation("scanlineSize");
        m_locScanlineBeam     = m_shader->uniformLocation("scanlineBeam");
        m_locGlow             = m_shader->uniformLocation("glow");
        m_locGlowRadius       = m_shader->uniformLocation("glowRadius");
        m_locSaturation       = m_shader->uniformLocation("saturation");
        m_locCurvature        = m_shader->uniformLocation("curvature");
        m_locVignette         = m_shader->uniformLocation("vignette");
    }

    connect(effects, &EffectsHandler::screenRemoved, this, &CrtEffect::slotScreenRemoved);
}

CrtEffect::~CrtEffect()
{
    if (effects) {
        effects->makeOpenGLContextCurrent();
    }
    m_states.clear();
    m_shader.reset();
}

bool CrtEffect::supported()
{
    return effects->isOpenGLCompositing();
}

bool CrtEffect::isActive() const
{
    return m_valid;
}

void CrtEffect::reconfigure(ReconfigureFlags)
{
    CrtTrinitron::Config::self()->config()->reparseConfiguration();
    CrtTrinitron::Config::self()->load();
    effects->addRepaintFull();
}

void CrtEffect::slotScreenRemoved(LogicalOutput *screen)
{
    if (effects) {
        effects->makeOpenGLContextCurrent();
    }
    m_states.erase(screen);
}

void CrtEffect::prePaintScreen(ScreenPrePaintData &data)
{
    effects->prePaintScreen(data);
}

void CrtEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport,
                            int mask, const Region &deviceRegion, LogicalOutput *screen)
{
    if (!m_valid || !renderTarget.texture()) {
        effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);
        return;
    }

    OutputState &st = m_states[screen];
    const qreal scale = viewport.scale();
    const QSize nativeSize = (QSizeF(screen->geometry().size()) * scale).toSize();
    const GLenum fmt = renderTarget.texture()->internalFormat();

    if (nativeSize.isEmpty()) {
        effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);
        return;
    }

    if (!st.texture || st.texture->size() != nativeSize
        || st.texture->internalFormat() != fmt) {
        st.framebuffer.reset();
        st.texture = GLTexture::allocate(fmt, nativeSize);
        if (!st.texture) {
            m_states.erase(screen);
            effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);
            return;
        }
        st.texture->setFilter(GL_LINEAR);
        st.texture->setWrapMode(GL_CLAMP_TO_EDGE);
        st.framebuffer = std::make_unique<GLFramebuffer>(st.texture.get());
        if (!st.framebuffer || !st.framebuffer->valid()) {
            m_states.erase(screen);
            effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);
            return;
        }
    }

    RenderTarget fboTarget(st.framebuffer.get(), renderTarget.colorDescription());
    RenderViewport fboViewport(viewport.renderRect(), viewport.scale(), fboTarget, QPoint());
    GLFramebuffer::pushFramebuffer(st.framebuffer.get());
    effects->paintScreen(fboTarget, fboViewport, mask, deviceRegion, screen);
    GLFramebuffer::popFramebuffer();

    const RectF dev = screen->geometry().scaled(scale);
    const float x0 = dev.left();
    const float y0 = dev.top();
    const float x1 = dev.right();
    const float y1 = dev.bottom();
    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();
    vbo->reset();
    vbo->setAttribLayout(std::span(GLVertexBuffer::GLVertex2DLayout), sizeof(GLVertex2D));
    const auto opt = vbo->map<GLVertex2D>(6);
    if (!opt) {
        effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);
        return;
    }
    const auto vmap = *opt;
    vmap[0] = GLVertex2D{ .position = QVector2D(x0, y0), .texcoord = QVector2D(0.0f, 1.0f) };
    vmap[1] = GLVertex2D{ .position = QVector2D(x1, y1), .texcoord = QVector2D(1.0f, 0.0f) };
    vmap[2] = GLVertex2D{ .position = QVector2D(x0, y1), .texcoord = QVector2D(0.0f, 0.0f) };
    vmap[3] = GLVertex2D{ .position = QVector2D(x0, y0), .texcoord = QVector2D(0.0f, 1.0f) };
    vmap[4] = GLVertex2D{ .position = QVector2D(x1, y0), .texcoord = QVector2D(1.0f, 1.0f) };
    vmap[5] = GLVertex2D{ .position = QVector2D(x1, y1), .texcoord = QVector2D(1.0f, 0.0f) };
    vbo->unmap();

    ShaderManager *sm = ShaderManager::instance();
    sm->pushShader(m_shader.get());

    using C = CrtTrinitron::Config;
    m_shader->setUniform(m_locMvp, viewport.projectionMatrix());
    m_shader->setUniform(m_locResolution, QVector2D(nativeSize.width(), nativeSize.height()));
    m_shader->setUniform(m_locTexture, 0);
    m_shader->setUniform(m_locBrightness, static_cast<float>(C::brightness()));
    m_shader->setUniform(m_locContrast, static_cast<float>(C::contrast()));
    m_shader->setUniform(m_locGamma, static_cast<float>(C::gamma()));
    m_shader->setUniform(m_locMaskType, static_cast<int>(C::maskType()));
    m_shader->setUniform(m_locMaskStrength, static_cast<float>(C::maskStrength()));
    m_shader->setUniform(m_locMaskSize, static_cast<float>(C::maskSize()));
    m_shader->setUniform(m_locColorScheme, static_cast<int>(C::colorScheme()));
    m_shader->setUniform(m_locScanlineStrength, static_cast<float>(C::scanlineStrength()));
    m_shader->setUniform(m_locScanlineSize, static_cast<float>(C::scanlineSize()));
    m_shader->setUniform(m_locScanlineBeam, static_cast<float>(C::scanlineBeam()));
    m_shader->setUniform(m_locGlow, static_cast<float>(C::glow()));
    m_shader->setUniform(m_locGlowRadius, static_cast<float>(C::glowRadius()));
    m_shader->setUniform(m_locSaturation, static_cast<float>(C::saturation()));
    m_shader->setUniform(m_locCurvature, static_cast<float>(C::curvature()));
    m_shader->setUniform(m_locVignette, static_cast<float>(C::vignette()));

    const GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
    if (blendWasEnabled) {
        glDisable(GL_BLEND);
    }
    glActiveTexture(GL_TEXTURE0);
    st.texture->bind();
    vbo->bindArrays();
    vbo->draw(GL_TRIANGLES, 0, 6);
    vbo->unbindArrays();
    st.texture->unbind();
    if (blendWasEnabled) {
        glEnable(GL_BLEND);
    }
    sm->popShader();

    effects->addRepaintFull();
}

} // namespace KWin
