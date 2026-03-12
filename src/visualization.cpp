// Ombro — Wireframe math-soundscape visualization engine
// AGPL-3.0 License

#include "visualization.h"
#include <cmath>
#include <cstring>
#include <cstdlib>

namespace ombro {

static const float PI = 3.14159265358979323846f;

// --- Preset library ---
static const Preset g_presets[] = {
    // Ocean Dream — gentle waves, blue-cyan, slow orbit
    {"Ocean Dream",
     2.0f, 1.5f,  0.4f, 0.3f,  0.3f,    // wave
     3.0f, 1.2f,  0.25f,                  // ripple
     0.0f, 0.0f,  0.0f,                   // spiral (off)
     1.5f, 0.3f,  0.4f,                   // audio
     0.55f, 0.15f, 0.02f, 0.8f, 0.9f,    // color (blue-cyan)
     3.5f, 1.8f, 0.15f,                   // camera
     0.06f},                               // fade

    // Neon Storm — aggressive, pink-purple, fast
    {"Neon Storm",
     4.0f, 3.5f,  1.2f, 0.9f,  0.2f,
     6.0f, 2.5f,  0.35f,
     2.0f, 3.0f,  0.15f,
     2.5f, 0.6f,  0.5f,
     0.85f, 0.2f, 0.05f, 0.9f, 1.0f,
     3.0f, 2.0f, 0.35f,
     0.04f},

    // Crystal Cave — geometric, green-white
    {"Crystal Cave",
     3.0f, 3.0f,  0.5f, 0.5f,  0.35f,
     4.5f, 0.8f,  0.2f,
     3.0f, 2.0f,  0.2f,
     1.8f, 0.5f,  0.45f,
     0.35f, 0.25f, 0.01f, 0.6f, 0.95f,
     3.2f, 1.5f, 0.2f,
     0.05f},

    // Solar Wind — flowing spirals, orange-yellow
    {"Solar Wind",
     1.5f, 2.0f,  0.6f, 0.4f,  0.25f,
     2.0f, 1.0f,  0.15f,
     4.0f, 5.0f,  0.35f,
     2.0f, 0.3f,  0.4f,
     0.1f, 0.12f, 0.03f, 0.9f, 1.0f,
     3.8f, 1.6f, 0.25f,
     0.07f},

    // Deep Space — slow undulation, deep purple, long trails
    {"Deep Space",
     1.0f, 0.8f,  0.2f, 0.15f,  0.35f,
     1.5f, 0.5f,  0.3f,
     1.0f, 1.0f,  0.1f,
     2.0f, 0.2f,  0.35f,
     0.75f, 0.1f, 0.01f, 0.7f, 0.85f,
     4.0f, 2.2f, 0.1f,
     0.03f},

    // Electric Rain — fine detail, cyan-white
    {"Electric Rain",
     5.0f, 4.5f,  0.8f, 1.0f,  0.15f,
     8.0f, 3.0f,  0.2f,
     0.0f, 0.0f,  0.0f,
     1.2f, 0.8f,  0.35f,
     0.5f, 0.1f, 0.04f, 0.5f, 1.0f,
     3.0f, 1.4f, 0.3f,
     0.05f},

    // Magma Flow — low-frequency, red-orange
    {"Magma Flow",
     1.2f, 1.0f,  0.3f, 0.25f,  0.4f,
     2.0f, 0.6f,  0.35f,
     2.0f, 2.5f,  0.25f,
     3.0f, 0.2f,  0.5f,
     0.05f, 0.1f, 0.015f, 0.95f, 0.9f,
     3.5f, 2.0f, 0.18f,
     0.05f},

    // Aurora — rainbow, smooth flowing
    {"Aurora",
     2.5f, 2.0f,  0.5f, 0.35f,  0.3f,
     3.0f, 1.0f,  0.25f,
     1.0f, 3.0f,  0.15f,
     1.5f, 0.4f,  0.4f,
     0.0f, 1.0f, 0.06f, 0.85f, 0.95f,
     3.5f, 1.8f, 0.2f,
     0.06f},
};

static const int NUM_PRESETS = sizeof(g_presets) / sizeof(g_presets[0]);

// --- HSV to RGB conversion ---
void Visualization::HSVtoRGB(float h, float s, float v, float& r, float& g, float& b) {
    h = fmodf(h, 1.0f);
    if (h < 0) h += 1.0f;
    int i = (int)(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = g = b = 0; break;
    }
}

// --- Smooth interpolation ---
static float smoothstep(float t) {
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    return t * t * (3.0f - 2.0f * t);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// --- Initialization ---
void Visualization::Init() {
    vertices_.resize(GRID_VERTICES);
    indices_.reserve(GRID_INDICES);

    // Build index buffer for line list (static — grid topology doesn't change)
    indices_.clear();
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE - 1; col++) {
            indices_.push_back(row * GRID_SIZE + col);
            indices_.push_back(row * GRID_SIZE + col + 1);
        }
    }
    for (int row = 0; row < GRID_SIZE - 1; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            indices_.push_back(row * GRID_SIZE + col);
            indices_.push_back((row + 1) * GRID_SIZE + col);
        }
    }

    currentPreset_ = 0;
    targetPreset_ = 0;
    morphT_ = 1.0f;
    time_ = 0;
    presetTimer_ = 0;

    memset(&audio_, 0, sizeof(audio_));
}

// --- Audio analysis ---
void Visualization::AnalyzeAudio(const unsigned char spectrum[2][576],
                                  const unsigned char waveform[2][576]) {
    // Mix stereo spectrum to mono and normalize
    float specNorm[256];
    for (int i = 0; i < 256; i++) {
        float val = ((float)spectrum[0][i] + (float)spectrum[1][i]) * 0.5f / 255.0f;
        specNorm[i] = val;
        audio_.spectrum[i] = lerp(audio_.spectrum[i], val, 0.3f); // smooth
    }

    // Band energies
    float bass = 0, mid = 0, treble = 0;
    for (int i = 0; i < 16; i++)   bass   += specNorm[i];
    for (int i = 16; i < 80; i++)  mid    += specNorm[i];
    for (int i = 80; i < 256; i++) treble += specNorm[i];
    bass   /= 16.0f;
    mid    /= 64.0f;
    treble /= 176.0f;

    // Smooth with exponential decay
    audio_.bass    = lerp(audio_.bass,    bass,   0.25f);
    audio_.mid     = lerp(audio_.mid,     mid,    0.2f);
    audio_.treble  = lerp(audio_.treble,  treble, 0.15f);
    audio_.totalEnergy = audio_.bass * 0.5f + audio_.mid * 0.3f + audio_.treble * 0.2f;

    // Simple beat detection: sharp rise in bass
    float bassDerivative = bass - prevBassEnergy_;
    prevBassEnergy_ = bass;
    if (bassDerivative > 0.15f)
        audio_.beatIntensity = 1.0f;
    else
        audio_.beatIntensity *= 0.9f;
}

// --- Surface generation ---
void Visualization::GenerateSurface(float time) {
    float t = smoothstep(morphT_);
    Preset p = (morphT_ >= 1.0f)
        ? g_presets[currentPreset_]
        : LerpPreset(g_presets[currentPreset_], g_presets[targetPreset_], t);

    float gridScale = 2.0f / (GRID_SIZE - 1);

    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            float x = -1.0f + col * gridScale;
            float y = -1.0f + row * gridScale;
            float z = 0;

            // Crossing sine waves
            z += sinf(x * p.freqX * PI + time * p.speedX)
               * cosf(y * p.freqY * PI + time * p.speedY)
               * p.waveAmp;

            // Concentric ripple
            float r = sqrtf(x * x + y * y);
            z += sinf(r * p.rippleFreq * PI - time * p.rippleSpeed) * p.rippleAmp;

            // Spiral
            if (p.spiralAmp > 0.001f) {
                float angle = atan2f(y, x);
                z += sinf(angle * p.spiralArms + r * p.spiralTightness * PI - time * 0.8f)
                   * p.spiralAmp;
            }

            // Audio modulation
            z *= (1.0f + audio_.bass * p.bassScale);
            // Treble adds fine detail
            z += audio_.treble * sinf(x * 15.0f * PI + time * 2.0f)
               * sinf(y * 15.0f * PI + time * 1.5f) * p.trebleDetail * 0.15f;

            // Beat pulse
            z *= (1.0f + audio_.beatIntensity * 0.3f);

            z *= p.amplitude;

            // Color: hue from height + time, saturation and brightness from preset
            float hue = p.hueBase + z * p.hueRange + time * p.hueSpeed;
            float bright = p.brightness * (0.6f + audio_.totalEnergy * 0.4f + fabsf(z) * 0.8f);
            bright = bright > 1.0f ? 1.0f : bright;

            float cr, cg, cb;
            HSVtoRGB(hue, p.saturation, bright);
            HSVtoRGB(hue, p.saturation, bright, cr, cg, cb);

            int idx = row * GRID_SIZE + col;
            vertices_[idx] = {x, z, y, cr, cg, cb, 1.0f};
        }
    }
}

// --- Camera ---
void Visualization::ComputeCamera(float time, float aspectRatio, float* outMatrix) const {
    Preset p = (morphT_ >= 1.0f)
        ? g_presets[currentPreset_]
        : LerpPreset(g_presets[currentPreset_], g_presets[targetPreset_], smoothstep(morphT_));

    float angle = time * p.camSpeed;
    float dist = p.camDist + audio_.bass * 0.5f;
    float height = p.camHeight + sinf(time * 0.2f) * 0.3f + audio_.mid * 0.3f;

    float eyeX = cosf(angle) * dist;
    float eyeZ = sinf(angle) * dist;
    float eyeY = height;

    // Look-at: target is origin
    float fwdX = -eyeX, fwdY = -eyeY, fwdZ = -eyeZ;
    float fwdLen = sqrtf(fwdX * fwdX + fwdY * fwdY + fwdZ * fwdZ);
    fwdX /= fwdLen; fwdY /= fwdLen; fwdZ /= fwdLen;

    // Up = (0, 1, 0)
    float rightX = fwdZ, rightY = 0, rightZ = -fwdX;
    float rightLen = sqrtf(rightX * rightX + rightZ * rightZ);
    if (rightLen > 0.0001f) {
        rightX /= rightLen; rightZ /= rightLen;
    }

    float upX = rightY * fwdZ - rightZ * fwdY;
    float upY = rightZ * fwdX - rightX * fwdZ;
    float upZ = rightX * fwdY - rightY * fwdX;

    // View matrix (row-major)
    float view[16] = {
        rightX,  upX,  fwdX,  0,
        rightY,  upY,  fwdY,  0,
        rightZ,  upZ,  fwdZ,  0,
        -(rightX*eyeX + rightY*eyeY + rightZ*eyeZ),
        -(upX*eyeX + upY*eyeY + upZ*eyeZ),
        -(fwdX*eyeX + fwdY*eyeY + fwdZ*eyeZ),
        1
    };

    // Perspective projection (row-major, LH)
    float fov = 0.8f; // ~45 degrees
    float nearZ = 0.1f, farZ = 50.0f;
    float yScale = 1.0f / tanf(fov * 0.5f);
    float xScale = yScale / aspectRatio;
    float zRange = farZ / (farZ - nearZ);

    float proj[16] = {
        xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, zRange, 1,
        0, 0, -nearZ * zRange, 0
    };

    // Multiply view * proj (row-major)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += view[i * 4 + k] * proj[k * 4 + j];
            }
            outMatrix[i * 4 + j] = sum;
        }
    }
}

// --- Preset interpolation ---
Preset Visualization::LerpPreset(const Preset& a, const Preset& b, float t) const {
    Preset r = {};
    r.name = b.name;
    r.freqX = lerp(a.freqX, b.freqX, t);
    r.freqY = lerp(a.freqY, b.freqY, t);
    r.speedX = lerp(a.speedX, b.speedX, t);
    r.speedY = lerp(a.speedY, b.speedY, t);
    r.waveAmp = lerp(a.waveAmp, b.waveAmp, t);
    r.rippleFreq = lerp(a.rippleFreq, b.rippleFreq, t);
    r.rippleSpeed = lerp(a.rippleSpeed, b.rippleSpeed, t);
    r.rippleAmp = lerp(a.rippleAmp, b.rippleAmp, t);
    r.spiralArms = lerp(a.spiralArms, b.spiralArms, t);
    r.spiralTightness = lerp(a.spiralTightness, b.spiralTightness, t);
    r.spiralAmp = lerp(a.spiralAmp, b.spiralAmp, t);
    r.bassScale = lerp(a.bassScale, b.bassScale, t);
    r.trebleDetail = lerp(a.trebleDetail, b.trebleDetail, t);
    r.amplitude = lerp(a.amplitude, b.amplitude, t);
    r.hueBase = lerp(a.hueBase, b.hueBase, t);
    r.hueRange = lerp(a.hueRange, b.hueRange, t);
    r.hueSpeed = lerp(a.hueSpeed, b.hueSpeed, t);
    r.saturation = lerp(a.saturation, b.saturation, t);
    r.brightness = lerp(a.brightness, b.brightness, t);
    r.camDist = lerp(a.camDist, b.camDist, t);
    r.camHeight = lerp(a.camHeight, b.camHeight, t);
    r.camSpeed = lerp(a.camSpeed, b.camSpeed, t);
    r.fadeAmount = lerp(a.fadeAmount, b.fadeAmount, t);
    return r;
}

// --- Public interface ---
void Visualization::Update(float deltaTime,
                           const unsigned char spectrum[2][576],
                           const unsigned char waveform[2][576]) {
    time_ += deltaTime;

    AnalyzeAudio(spectrum, waveform);

    // Auto-preset switching
    presetTimer_ += deltaTime;
    if (presetTimer_ >= presetInterval_ && morphT_ >= 1.0f) {
        presetTimer_ = 0;
        int next = (currentPreset_ + 1 + (rand() % (NUM_PRESETS - 1))) % NUM_PRESETS;
        targetPreset_ = next;
        morphT_ = 0;
    }

    // Advance morph
    if (morphT_ < 1.0f) {
        morphT_ += deltaTime * 0.4f; // ~2.5 second transition
        if (morphT_ >= 1.0f) {
            morphT_ = 1.0f;
            currentPreset_ = targetPreset_;
        }
    }

    GenerateSurface(time_);
}

void Visualization::GetViewProjectionMatrix(float* outMatrix, float aspectRatio) const {
    const_cast<Visualization*>(this)->ComputeCamera(time_, aspectRatio, outMatrix);
}

float Visualization::GetFadeAmount() const {
    if (morphT_ >= 1.0f)
        return g_presets[currentPreset_].fadeAmount;
    float t = smoothstep(morphT_);
    return lerp(g_presets[currentPreset_].fadeAmount, g_presets[targetPreset_].fadeAmount, t);
}

void Visualization::NextPreset() {
    if (morphT_ < 1.0f) {
        currentPreset_ = targetPreset_;
        morphT_ = 1.0f;
    }
    targetPreset_ = (currentPreset_ + 1) % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

void Visualization::PrevPreset() {
    if (morphT_ < 1.0f) {
        currentPreset_ = targetPreset_;
        morphT_ = 1.0f;
    }
    targetPreset_ = (currentPreset_ - 1 + NUM_PRESETS) % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

void Visualization::RandomPreset() {
    if (morphT_ < 1.0f) {
        currentPreset_ = targetPreset_;
        morphT_ = 1.0f;
    }
    targetPreset_ = rand() % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

const char* Visualization::GetCurrentPresetName() const {
    if (morphT_ < 1.0f)
        return g_presets[targetPreset_].name;
    return g_presets[currentPreset_].name;
}

} // namespace ombro
