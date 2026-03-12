// Ombro — Wireframe math-soundscape visualization engine
// AGPL-3.0 License

#include "visualization.h"
#include <cmath>
#include <cstring>
#include <cstdlib>

namespace ombro {

static const float PI = 3.14159265358979323846f;

// ============================================================
// Preset library — 8 visually distinctive presets
// ============================================================

static const Preset g_presets[] = {

    // 0: Spectrum Ridge — classic WhiteCap: spectrum IS the landscape
    {"Spectrum Ridge",
     1.5f, 1.0f,  0.3f, 0.2f,  0.15f,      // gentle wave underlayer
     2.0f, 0.8f,  0.1f,                      // subtle ripple
     0.0f, 0.0f,  0.0f, 0.0f,               // no spiral
     1.2f, 1.8f,                              // spectrum: high scale, sharp peaks
     0.15f,                                   // light waveform overlay
     0.06f, 0.04f,  2.5f, 3.0f,  0.3f,      // gentle grid warp
     1.0f, 0.15f, 0.35f,                     // audio reactivity
     10.0f,                                   // edge falloff
     0.55f, 0.48f, 0.02f,                    // color: blue → cyan
     0.85f, 0.8f, 1.4f,                      // color: high sat, bright, glow
     3.5f, 2.0f, 0.12f,                      // camera: orbit
     0.85f, 0.08f, 0.15f,                    // cam: FOV, shake, tilt
     0.05f},                                  // fade

    // 1: Pulse Engine — heavy bass, aggressive, red-orange
    {"Pulse Engine",
     3.0f, 2.5f,  1.0f, 0.8f,  0.2f,
     4.0f, 2.0f,  0.25f,
     2.0f, 3.0f,  1.5f, 0.15f,
     0.9f, 1.5f,
     0.25f,
     0.12f, 0.10f,  2.0f, 2.5f,  0.6f,      // heavy grid warp
     2.5f, 0.4f, 0.5f,
     8.0f,
     0.02f, 0.12f, 0.04f,                    // red → orange
     0.95f, 0.9f, 1.8f,
     3.0f, 1.8f, 0.28f,
     0.9f, 0.15f, 0.1f,
     0.04f},

    // 2: Crystal Lattice — sharp peaks, green-white, minimal warp
    {"Crystal Lattice",
     3.5f, 3.5f,  0.4f, 0.4f,  0.3f,
     5.0f, 1.0f,  0.2f,
     3.0f, 2.0f,  0.8f, 0.2f,
     1.0f, 2.2f,                              // sharp spectrum peaks
     0.1f,
     0.02f, 0.02f,  4.0f, 4.0f,  0.2f,      // very subtle warp
     1.5f, 0.5f, 0.4f,
     12.0f,
     0.30f, 0.42f, 0.01f,                    // green → teal
     0.6f, 0.95f, 1.2f,
     3.2f, 2.5f, 0.18f,                      // higher camera
     0.7f, 0.06f, 0.08f,
     0.05f},

    // 3: Nebula — smooth, flowing, purple-pink, long trails
    {"Nebula",
     1.0f, 0.8f,  0.2f, 0.15f,  0.35f,
     1.5f, 0.5f,  0.3f,
     1.0f, 1.5f,  0.4f, 0.15f,
     0.8f, 1.0f,                              // softer spectrum (low exponent)
     0.2f,
     0.15f, 0.12f,  1.5f, 2.0f,  0.2f,      // flowing warp
     1.5f, 0.2f, 0.35f,
     6.0f,                                    // wide edge falloff
     0.78f, 0.92f, 0.015f,                   // purple → pink
     0.75f, 0.85f, 1.0f,
     4.0f, 2.2f, 0.08f,                      // far, slow orbit
     0.75f, 0.04f, 0.2f,                     // more tilt
     0.025f},                                 // very slow fade = long trails

    // 4: Waveform Canyon — strong waveform presence, teal-yellow
    {"Waveform Canyon",
     2.0f, 1.5f,  0.5f, 0.3f,  0.2f,
     3.0f, 1.2f,  0.15f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.7f, 1.3f,
     0.5f,                                    // STRONG waveform overlay
     0.08f, 0.06f,  2.0f, 2.5f,  0.35f,
     1.8f, 0.3f, 0.4f,
     9.0f,
     0.48f, 0.16f, 0.03f,                    // teal → yellow
     0.8f, 0.9f, 1.3f,
     3.0f, 1.5f, 0.2f,
     0.85f, 0.1f, 0.12f,
     0.06f},

    // 5: Storm Front — aggressive all-spectrum, electric blue-white
    {"Storm Front",
     4.5f, 4.0f,  1.2f, 1.0f,  0.15f,
     7.0f, 3.0f,  0.2f,
     2.0f, 4.0f,  2.0f, 0.1f,
     1.5f, 2.0f,
     0.2f,
     0.10f, 0.08f,  3.0f, 3.5f,  0.5f,
     2.0f, 0.7f, 0.45f,
     8.0f,
     0.58f, 0.52f, 0.05f,                    // blue → slightly different blue
     0.5f, 1.0f, 2.0f,                       // desaturated, very bright, strong glow
     2.8f, 1.6f, 0.35f,
     0.95f, 0.18f, 0.05f,                    // wide FOV, strong shake
     0.04f},

    // 6: Ember Flow — spiral-heavy, red-gold, warm
    {"Ember Flow",
     1.2f, 1.0f,  0.3f, 0.25f,  0.2f,
     2.0f, 0.6f,  0.2f,
     4.0f, 5.0f,  0.7f, 0.3f,               // prominent spirals
     0.6f, 1.2f,
     0.15f,
     0.10f, 0.08f,  2.0f, 2.5f,  0.3f,
     2.5f, 0.2f, 0.45f,
     8.0f,
     0.03f, 0.1f, 0.02f,                     // red → gold
     0.9f, 0.9f, 1.5f,
     3.5f, 2.0f, 0.15f,
     0.8f, 0.1f, 0.18f,
     0.05f},

    // 7: Aurora Borealis — full rainbow, smooth, beautiful color cycling
    {"Aurora Borealis",
     2.0f, 1.5f,  0.4f, 0.3f,  0.25f,
     3.0f, 1.0f,  0.2f,
     1.0f, 3.0f,  0.5f, 0.15f,
     0.9f, 1.2f,
     0.2f,
     0.08f, 0.06f,  2.0f, 2.5f,  0.25f,
     1.5f, 0.35f, 0.4f,
     9.0f,
     0.0f, 1.0f, 0.06f,                      // full spectrum: 0 → 1.0 hue range
     0.85f, 0.95f, 1.2f,
     3.5f, 1.8f, 0.15f,
     0.8f, 0.08f, 0.15f,
     0.055f},
};

static const int NUM_PRESETS = sizeof(g_presets) / sizeof(g_presets[0]);

// ============================================================
// Math utilities
// ============================================================

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

// Wyvill falloff: smooth organic decay from 1 at r=0 to 0 at r>=1
float Visualization::Wyvill(float r) {
    if (r >= 1.0f) return 0.0f;
    float r2 = r * r;
    float t = 1.0f - r2;
    return t * t * t;
}

static float smoothstep(float t) {
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    return t * t * (3.0f - 2.0f * t);
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static float clamp01(float x) {
    return x < 0 ? 0 : (x > 1 ? 1 : x);
}

// ============================================================
// Initialization
// ============================================================

void Visualization::Init() {
    vertices_.resize(GRID_VERTICES);
    indices_.reserve(GRID_INDICES);

    // Build line-list index buffer (static topology)
    indices_.clear();
    // Horizontal lines
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE - 1; col++) {
            indices_.push_back(row * GRID_SIZE + col);
            indices_.push_back(row * GRID_SIZE + col + 1);
        }
    }
    // Vertical lines
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
    memset(prevSpectrum_, 0, sizeof(prevSpectrum_));
}

// ============================================================
// Audio analysis — fast attack, slow decay per bin
// ============================================================

void Visualization::AnalyzeAudio(const unsigned char spectrum[2][576],
                                  const unsigned char waveform[2][576]) {
    // Per-bin spectrum with asymmetric smoothing (fast attack, slow decay)
    float raw[256];
    for (int i = 0; i < 256; i++) {
        raw[i] = ((float)spectrum[0][i] + (float)spectrum[1][i]) * 0.5f / 255.0f;

        if (raw[i] > audio_.spectrum[i])
            audio_.spectrum[i] = lerp(audio_.spectrum[i], raw[i], 0.55f);   // fast attack
        else
            audio_.spectrum[i] = lerp(audio_.spectrum[i], raw[i], 0.06f);   // slow decay
    }

    // Normalize waveform to -1..1 (unsigned char centered at 128)
    for (int i = 0; i < 576; i++) {
        float sample = ((float)waveform[0][i] + (float)waveform[1][i]) * 0.5f;
        float normalized = (sample - 128.0f) / 128.0f;
        audio_.waveform[i] = lerp(audio_.waveform[i], normalized, 0.4f);
    }

    // Band energies from smoothed spectrum
    float bass = 0, mid = 0, treble = 0;
    for (int i = 0; i < 16; i++)   bass   += audio_.spectrum[i];
    for (int i = 16; i < 80; i++)  mid    += audio_.spectrum[i];
    for (int i = 80; i < 256; i++) treble += audio_.spectrum[i];
    bass   /= 16.0f;
    mid    /= 64.0f;
    treble /= 176.0f;

    audio_.bass    = lerp(audio_.bass,    bass,   0.25f);
    audio_.mid     = lerp(audio_.mid,     mid,    0.2f);
    audio_.treble  = lerp(audio_.treble,  treble, 0.15f);
    audio_.totalEnergy = audio_.bass * 0.5f + audio_.mid * 0.3f + audio_.treble * 0.2f;

    // Beat detection: sharp bass onset
    float bassDerivative = bass - prevBassEnergy_;
    prevBassEnergy_ = bass;
    if (bassDerivative > 0.12f)
        audio_.beatIntensity = 1.0f;
    else
        audio_.beatIntensity *= 0.92f;

    // Spectral flux: sum of positive changes across all bins
    float flux = 0;
    for (int i = 0; i < 256; i++) {
        float diff = raw[i] - prevSpectrum_[i];
        if (diff > 0) flux += diff;
        prevSpectrum_[i] = raw[i];
    }
    audio_.spectralFlux = lerp(audio_.spectralFlux, flux / 64.0f, 0.3f);
}

// ============================================================
// Surface generation — the core visual engine
// ============================================================

void Visualization::GenerateSurface(float time) {
    Preset p = GetActivePreset();
    float gridScale = 2.0f / (GRID_SIZE - 1);

    for (int row = 0; row < GRID_SIZE; row++) {
        // Frequency mapping: row 0 = bass, row N = treble
        float freqT = (float)row / (float)(GRID_SIZE - 1);
        int freqBin = (int)(freqT * 255.0f);
        float specEnergy = audio_.spectrum[freqBin];

        for (int col = 0; col < GRID_SIZE; col++) {
            float x = -1.0f + col * gridScale;
            float y = -1.0f + row * gridScale;

            // --- Grid XY warping (organic flowing distortion) ---
            float wx = x + sinf(y * p.warpFreqY * PI + time * p.warpSpeed)
                          * p.warpAmountX * (1.0f + audio_.bass * 0.5f);
            float wy = y + cosf(x * p.warpFreqX * PI + time * p.warpSpeed * 0.7f)
                          * p.warpAmountY * (1.0f + audio_.mid * 0.5f);

            float z = 0;

            // --- Spectrum-driven displacement (WhiteCap signature) ---
            // Each row reacts to its corresponding frequency bin
            float specZ = powf(specEnergy, p.spectrumExponent) * p.spectrumScale;
            z += specZ;

            // --- Waveform overlay (audio shape embedded into surface) ---
            if (p.waveformScale > 0.001f) {
                int waveSample = (int)((float)col / (float)(GRID_SIZE - 1) * 575.0f);
                z += audio_.waveform[waveSample] * p.waveformScale * (0.5f + specEnergy);
            }

            // --- Crossing sine waves ---
            z += sinf(wx * p.waveFreqX * PI + time * p.waveSpeedX)
               * cosf(wy * p.waveFreqY * PI + time * p.waveSpeedY)
               * p.waveAmp;

            // --- Concentric ripple ---
            float r = sqrtf(wx * wx + wy * wy);
            z += sinf(r * p.rippleFreq * PI - time * p.rippleSpeed) * p.rippleAmp;

            // --- Spiral arms ---
            if (p.spiralAmp > 0.001f) {
                float angle = atan2f(wy, wx);
                z += sinf(angle * p.spiralArms + r * p.spiralTightness * PI - time * p.spiralSpeed)
                   * p.spiralAmp;
            }

            // --- Audio modulation ---
            z *= (1.0f + audio_.bass * p.bassScale);
            // Treble adds fine-grained detail
            z += audio_.treble * sinf(wx * 15.0f * PI + time * 2.0f)
               * sinf(wy * 15.0f * PI + time * 1.5f) * p.trebleDetail * 0.15f;
            // Beat pulse
            z *= (1.0f + audio_.beatIntensity * 0.25f);

            z *= p.amplitude;

            // --- Two-tone color: frequency-mapped hue ---
            float hue = lerp(p.hueLow, p.hueHigh, freqT) + time * p.hueSpeed;
            // Brightness from per-vertex spectrum energy and overall audio
            float bright = p.brightness
                         * (0.3f + specEnergy * p.glowIntensity * 0.7f
                            + audio_.totalEnergy * 0.3f
                            + fabsf(z) * 0.4f);
            bright = clamp01(bright);

            float cr, cg, cb;
            HSVtoRGB(hue, p.saturation, bright, cr, cg, cb);

            // --- Edge falloff: soft transparency at grid boundaries ---
            float edgeDistX = fminf((float)col, (float)(GRID_SIZE - 1 - col));
            float edgeDistY = fminf((float)row, (float)(GRID_SIZE - 1 - row));
            float edgeDist = fminf(edgeDistX, edgeDistY);
            float alpha = smoothstep(clamp01(edgeDist / p.edgeFalloff));

            int idx = row * GRID_SIZE + col;
            vertices_[idx] = {wx, z, wy, cr, cg, cb, alpha};
        }
    }
}

// ============================================================
// Camera — orbit with beat shake, drift, and tilt
// ============================================================

void Visualization::ComputeCamera(float time, float aspectRatio, float* outMatrix) const {
    Preset p = GetActivePreset();

    float angle = time * p.camSpeed;
    float dist = p.camDist + audio_.bass * 0.4f;
    // Zoom pulse on beat
    dist *= (1.0f - audio_.beatIntensity * 0.04f);
    float height = p.camHeight + sinf(time * 0.2f) * 0.3f + audio_.mid * 0.3f;

    float eyeX = cosf(angle) * dist;
    float eyeZ = sinf(angle) * dist;
    float eyeY = height;

    // Beat-reactive camera shake
    float shakeX = sinf(time * 47.0f) * audio_.beatIntensity * p.camShake * 0.1f;
    float shakeY = cosf(time * 53.0f) * audio_.beatIntensity * p.camShake * 0.1f;
    eyeX += shakeX;
    eyeY += shakeY;

    // Look-at target: slowly drifts around origin
    float lookX = sinf(time * 0.1f) * 0.15f;
    float lookZ = cosf(time * 0.13f) * 0.15f;

    // Forward vector
    float fwdX = lookX - eyeX;
    float fwdY = 0.0f - eyeY;
    float fwdZ = lookZ - eyeZ;
    float fwdLen = sqrtf(fwdX * fwdX + fwdY * fwdY + fwdZ * fwdZ);
    fwdX /= fwdLen; fwdY /= fwdLen; fwdZ /= fwdLen;

    // World up, then apply camera tilt (roll)
    float roll = sinf(time * 0.15f) * p.camTilt;
    float worldUpX = sinf(roll);
    float worldUpY = cosf(roll);
    float worldUpZ = 0.0f;

    // Right = forward x up
    float rightX = fwdY * worldUpZ - fwdZ * worldUpY;
    float rightY = fwdZ * worldUpX - fwdX * worldUpZ;
    float rightZ = fwdX * worldUpY - fwdY * worldUpX;
    float rightLen = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLen > 0.0001f) {
        rightX /= rightLen; rightY /= rightLen; rightZ /= rightLen;
    }

    // True up = right x forward
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
    float fov = p.camFOV;
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

    // view * proj (row-major)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++)
                sum += view[i * 4 + k] * proj[k * 4 + j];
            outMatrix[i * 4 + j] = sum;
        }
    }
}

// ============================================================
// Preset interpolation & helpers
// ============================================================

Preset Visualization::GetActivePreset() const {
    if (morphT_ >= 1.0f)
        return g_presets[currentPreset_];
    return LerpPreset(g_presets[currentPreset_], g_presets[targetPreset_], smoothstep(morphT_));
}

Preset Visualization::LerpPreset(const Preset& a, const Preset& b, float t) const {
    Preset r = {};
    r.name = b.name;

    #define LP(field) r.field = lerp(a.field, b.field, t)
    LP(waveFreqX); LP(waveFreqY); LP(waveSpeedX); LP(waveSpeedY); LP(waveAmp);
    LP(rippleFreq); LP(rippleSpeed); LP(rippleAmp);
    LP(spiralArms); LP(spiralTightness); LP(spiralSpeed); LP(spiralAmp);
    LP(spectrumScale); LP(spectrumExponent);
    LP(waveformScale);
    LP(warpAmountX); LP(warpAmountY); LP(warpFreqX); LP(warpFreqY); LP(warpSpeed);
    LP(bassScale); LP(trebleDetail); LP(amplitude);
    LP(edgeFalloff);
    LP(hueLow); LP(hueHigh); LP(hueSpeed); LP(saturation); LP(brightness); LP(glowIntensity);
    LP(camDist); LP(camHeight); LP(camSpeed); LP(camFOV); LP(camShake); LP(camTilt);
    LP(fadeAmount);
    #undef LP

    return r;
}

// ============================================================
// Public interface
// ============================================================

void Visualization::Update(float deltaTime,
                           const unsigned char spectrum[2][576],
                           const unsigned char waveform[2][576]) {
    time_ += deltaTime;

    AnalyzeAudio(spectrum, waveform);

    // Auto-preset cycling
    presetTimer_ += deltaTime;
    if (presetTimer_ >= presetInterval_ && morphT_ >= 1.0f) {
        presetTimer_ = 0;
        int next = (currentPreset_ + 1 + (rand() % (NUM_PRESETS - 1))) % NUM_PRESETS;
        targetPreset_ = next;
        morphT_ = 0;
    }

    // Advance morph — WhiteCap-style easing: y = 1-(1-x)^1.5
    if (morphT_ < 1.0f) {
        morphT_ += deltaTime * 0.4f;
        if (morphT_ >= 1.0f) {
            morphT_ = 1.0f;
            currentPreset_ = targetPreset_;
        }
    }

    GenerateSurface(time_);
}

void Visualization::GetViewProjectionMatrix(float* outMatrix, float aspectRatio) const {
    ComputeCamera(time_, aspectRatio, outMatrix);
}

float Visualization::GetFadeAmount() const {
    return GetActivePreset().fadeAmount;
}

void Visualization::NextPreset() {
    if (morphT_ < 1.0f) { currentPreset_ = targetPreset_; morphT_ = 1.0f; }
    targetPreset_ = (currentPreset_ + 1) % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

void Visualization::PrevPreset() {
    if (morphT_ < 1.0f) { currentPreset_ = targetPreset_; morphT_ = 1.0f; }
    targetPreset_ = (currentPreset_ - 1 + NUM_PRESETS) % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

void Visualization::RandomPreset() {
    if (morphT_ < 1.0f) { currentPreset_ = targetPreset_; morphT_ = 1.0f; }
    targetPreset_ = rand() % NUM_PRESETS;
    morphT_ = 0;
    presetTimer_ = 0;
}

const char* Visualization::GetCurrentPresetName() const {
    if (morphT_ < 1.0f) return g_presets[targetPreset_].name;
    return g_presets[currentPreset_].name;
}

} // namespace ombro
