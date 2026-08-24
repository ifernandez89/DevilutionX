/**
 * @file remaster_render.cpp
 *
 * Subtle Graphics Remaster GPU Post-Processing Pipeline Implementation.
 */
#include "engine/render/remaster_render.hpp"
#include "utils/log.hpp"

#if defined(__EMSCRIPTEN__)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#define REMASTER_HAS_GLES 1
#elif defined(USE_SDL3)
#include <SDL3/SDL_opengl.h>
#define REMASTER_HAS_GLES 0
#else
#include <SDL_opengl.h>
#define REMASTER_HAS_GLES 0
#endif

namespace devilution {

namespace {

bool g_remasterInitialized = false;
bool g_remasterAvailable = false;
int g_texWidth = 0;
int g_texHeight = 0;

#if defined(__EMSCRIPTEN__)

GLuint g_program = 0;
GLuint g_quadVbo = 0;
GLint g_uTextureLoc = -1;
GLint g_uTexSizeLoc = -1;
GLint g_uTimeLoc = -1;

const char *kVertexShaderSource = R"(
attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

const char *kFragmentShaderSource = R"(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uTexSize;
uniform float uTime;

// RGB to Luminance
float getLuminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

// Subtle S-Curve Contrast Adjustment
vec3 applyContrast(vec3 color, float contrast) {
    return (color - 0.5) * contrast + 0.5;
}

// Subtle Saturation Enhancement
vec3 applySaturation(vec3 color, float sat) {
    float lum = getLuminance(color);
    return mix(vec3(lum), color, sat);
}

void main() {
    vec2 uv = vTexCoord;
    vec2 texel = 1.0 / uTexSize;

    // 1. Base 5-tap Unsharp Mask for Subtle Sharpening
    vec3 cCenter = texture2D(uTexture, uv).rgb;
    vec3 cUp     = texture2D(uTexture, uv + vec2(0.0, -texel.y)).rgb;
    vec3 cDown   = texture2D(uTexture, uv + vec2(0.0,  texel.y)).rgb;
    vec3 cLeft   = texture2D(uTexture, uv + vec2(-texel.x, 0.0)).rgb;
    vec3 cRight  = texture2D(uTexture, uv + vec2( texel.x, 0.0)).rgb;

    // Subtle edge sharpness without ringing
    const float kSharpenWeight = 0.08;
    vec3 sharpened = cCenter * (1.0 + 4.0 * kSharpenWeight) - (cUp + cDown + cLeft + cRight) * kSharpenWeight;
    sharpened = clamp(sharpened, 0.0, 1.0);

    // 2. Subtle Bloom (Extract bright highlights: fire, torches, spells, gold)
    vec3 bloomAccum = vec3(0.0);
    const float kBloomThreshold = 0.70;

    // Sample 4 diagonal neighbors for high-light glow
    vec3 cUL = texture2D(uTexture, uv + vec2(-texel.x * 1.5, -texel.y * 1.5)).rgb;
    vec3 cUR = texture2D(uTexture, uv + vec2( texel.x * 1.5, -texel.y * 1.5)).rgb;
    vec3 cDL = texture2D(uTexture, uv + vec2(-texel.x * 1.5,  texel.y * 1.5)).rgb;
    vec3 cDR = texture2D(uTexture, uv + vec2( texel.x * 1.5,  texel.y * 1.5)).rgb;

    vec3 brightSamples[5];
    brightSamples[0] = cCenter;
    brightSamples[1] = cUL;
    brightSamples[2] = cUR;
    brightSamples[3] = cDL;
    brightSamples[4] = cDR;

    for (int i = 0; i < 5; i++) {
        float lum = getLuminance(brightSamples[i]);
        if (lum > kBloomThreshold) {
            bloomAccum += (brightSamples[i] - kBloomThreshold) * (1.0 / (1.0 - kBloomThreshold));
        }
    }
    bloomAccum = (bloomAccum / 5.0) * 0.16; // Subdued, elegant bloom gain

    // 3. Subtle Color Grading & Dynamic Tone Curve
    vec3 graded = sharpened;
    // Gentle gamma tweak for rich dungeon shadows
    graded = pow(graded, vec3(1.04));
    // Subtle S-curve contrast boost
    graded = clamp(applyContrast(graded, 1.05), 0.0, 1.0);
    // Slight richness in warm tones
    graded = applySaturation(graded, 1.06);

    // 4. Combine Bloom
    graded += bloomAccum;

    // 5. Atmospheric Vignette (Soft distance-based edge darkening)
    vec2 distVec = (uv - 0.5) * vec2(uTexSize.x / uTexSize.y, 1.0);
    float dist = length(distVec);
    float vignette = smoothstep(0.85, 0.35, dist);
    vignette = mix(0.78, 1.0, vignette);
    graded *= vignette;

    gl_FragColor = vec4(clamp(graded, 0.0, 1.0), 1.0);
}
)";

GLuint CompileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    if (!shader) return 0;
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        Log("Remaster Shader compilation error: {}", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool InitGLPipeline()
{
    if (g_program != 0) return true;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVertexShaderSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glBindAttribLocation(g_program, 0, "aPosition");
    glBindAttribLocation(g_program, 1, "aTexCoord");
    glLinkProgram(g_program);

    GLint linked = 0;
    glGetProgramiv(g_program, GL_LINK_STATUS, &linked);
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(g_program, sizeof(infoLog), nullptr, infoLog);
        Log("Remaster Program link error: {}", infoLog);
        glDeleteProgram(g_program);
        g_program = 0;
        return false;
    }

    g_uTextureLoc = glGetUniformLocation(g_program, "uTexture");
    g_uTexSizeLoc = glGetUniformLocation(g_program, "uTexSize");
    g_uTimeLoc    = glGetUniformLocation(g_program, "uTime");

    // Fullscreen Quad [X, Y, U, V]
    const float kQuadVertices[] = {
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };

    glGenBuffers(1, &g_quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Log("{}", "DevilutionX Web Subtle Graphics Remaster GPU Pipeline initialized successfully");
    return true;
}

#endif

} // namespace

void RemasterInit(int width, int height)
{
    g_texWidth = width;
    g_texHeight = height;
    g_remasterInitialized = true;
#if defined(__EMSCRIPTEN__)
    g_remasterAvailable = InitGLPipeline();
#else
    g_remasterAvailable = false;
#endif
}

void RemasterCleanup()
{
#if defined(__EMSCRIPTEN__)
    if (g_quadVbo) {
        glDeleteBuffers(1, &g_quadVbo);
        g_quadVbo = 0;
    }
    if (g_program) {
        glDeleteProgram(g_program);
        g_program = 0;
    }
#endif
    g_remasterInitialized = false;
    g_remasterAvailable = false;
}

bool RemasterProcessAndPresent(SDL_Renderer *renderer, SDL_Texture *sourceTexture, int screenWidth, int screenHeight)
{
#if defined(__EMSCRIPTEN__)
    if (!g_remasterAvailable || g_program == 0) {
        return false;
    }

    // Lazy initialization if needed
    if (!g_remasterInitialized) {
        RemasterInit(screenWidth, screenHeight);
        if (!g_remasterAvailable) return false;
    }

    // Bind Shader Program
    glUseProgram(g_program);

    if (g_uTextureLoc >= 0) {
        glUniform1i(g_uTextureLoc, 0);
    }
    if (g_uTexSizeLoc >= 0) {
        glUniform2f(g_uTexSizeLoc, static_cast<float>(screenWidth), static_cast<float>(screenHeight));
    }
    if (g_uTimeLoc >= 0) {
        static float s_time = 0.0f;
        s_time += 0.016f;
        glUniform1f(g_uTimeLoc, s_time);
    }

    // Render fullscreen quad
    glBindBuffer(GL_ARRAY_BUFFER, g_quadVbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    return true;
#else
    (void)renderer;
    (void)sourceTexture;
    (void)screenWidth;
    (void)screenHeight;
    return false;
#endif
}

bool RemasterIsEnabled()
{
    return g_remasterAvailable;
}

} // namespace devilution
