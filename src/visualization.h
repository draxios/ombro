#pragma once
// Ombro — Wireframe math-soundscape visualization engine

#include "renderer.h"
#include <vector>
#include <cstdint>

namespace ombro {

// Grid dimensions for the wireframe mesh
static const int GRID_SIZE = 80;
static const int GRID_VERTICES = GRID_SIZE * GRID_SIZE;
static const int GRID_LINES = 2 * GRID_SIZE * (GRID_SIZE - 1);
static const int GRID_INDICES = GRID_LINES * 2;

struct AudioData {
    float bass;
    float mid;
    float treble;
    float totalEnergy;
    float beatIntensity;
    float spectralFlux;

    float spectrum[256];
    float waveform[576];
};

struct Preset {
    const char* name;

    // Surface: crossing sine waves
    float waveFreqX, waveFreqY;
    float waveSpeedX, waveSpeedY;
    float waveAmp;

    // Surface: triangle wave ridges (sharper than sine)
    float triWaveAmp;

    // Surface: square wave plateaus (flat-top mesas)
    float sqWaveAmp;

    // Surface: concentric ripple
    float rippleFreq, rippleSpeed, rippleAmp;

    // Surface: spiral arms
    float spiralArms, spiralTightness, spiralSpeed, spiralAmp;

    // Spectrum-driven displacement (WhiteCap signature)
    float spectrumScale;
    float spectrumExponent;

    // Waveform overlay
    float waveformScale;

    // Wyvill organic blob peaks — smooth metaball-like bumps driven by bass
    float wyvillScale;

    // Grid XY warping — organic flowing distortion
    float warpAmountX, warpAmountY;
    float warpFreqX, warpFreqY;
    float warpSpeed;

    // Flow field — directional streaming displacement
    float flowAngle;            // flow direction in radians
    float flowSpeed;            // flow animation speed
    float flowScale;            // displacement amplitude

    // Surface topology — 0 = flat grid, 1 = radial/circular
    float radialMix;

    // Audio reactivity
    float bassScale;
    float trebleDetail;
    float amplitude;

    // Edge falloff
    float edgeFalloff;

    // Color: two-tone (bass hue → treble hue)
    float hueLow;
    float hueHigh;
    float hueSpeed;
    float saturation;
    float brightness;
    float glowIntensity;

    // Camera
    float camDist;
    float camHeight;
    float camSpeed;
    float camFOV;
    float camShake;
    float camTilt;

    // Afterglow
    float fadeAmount;
};

class Visualization {
public:
    void Init();
    void Update(float deltaTime,
                const unsigned char spectrum[2][576],
                const unsigned char waveform[2][576]);

    const Vertex* GetVertices() const { return vertices_.data(); }
    int GetVertexCount() const { return (int)vertices_.size(); }
    const uint32_t* GetIndices() const { return indices_.data(); }
    int GetIndexCount() const { return (int)indices_.size(); }

    void GetViewProjectionMatrix(float* outMatrix, float aspectRatio) const;
    float GetFadeAmount() const;

    void NextPreset();
    void PrevPreset();
    void RandomPreset();
    const char* GetCurrentPresetName() const;

private:
    void AnalyzeAudio(const unsigned char spectrum[2][576],
                      const unsigned char waveform[2][576]);
    void GenerateSurface(float time);
    void ComputeCamera(float time, float aspectRatio, float* outMatrix) const;
    Preset LerpPreset(const Preset& a, const Preset& b, float t) const;
    Preset GetActivePreset() const;

    static void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b);
    static float Wyvill(float r);

    std::vector<Vertex>   vertices_;
    std::vector<uint32_t> indices_;

    AudioData audio_ = {};
    float prevBassEnergy_ = 0;
    float prevSpectrum_[256] = {};

    float time_ = 0;
    int currentPreset_ = 0;
    int targetPreset_ = 0;
    float morphT_ = 1.0f;
    float presetTimer_ = 0;
    float presetInterval_ = 15.0f;
};

} // namespace ombro
