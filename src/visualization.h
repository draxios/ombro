#pragma once
// Ombro — Wireframe math-soundscape visualization engine

#include "renderer.h"
#include <vector>
#include <cstdint>

namespace ombro {

// Grid dimensions for the wireframe mesh
static const int GRID_SIZE = 80;
static const int GRID_VERTICES = GRID_SIZE * GRID_SIZE;
// Lines: horizontal (GRID_SIZE rows * (GRID_SIZE-1) segments) + vertical ((GRID_SIZE-1) * GRID_SIZE)
static const int GRID_LINES = 2 * GRID_SIZE * (GRID_SIZE - 1);
static const int GRID_INDICES = GRID_LINES * 2;

struct AudioData {
    float bass;             // Low-frequency energy (0..1)
    float mid;              // Mid-frequency energy (0..1)
    float treble;           // High-frequency energy (0..1)
    float totalEnergy;      // Overall energy (0..1)
    float beatIntensity;    // Beat detection pulse (0..1)
    float spectrum[256];    // Normalized spectrum bins (0..1)
};

struct Preset {
    const char* name;

    // Wave surface
    float freqX, freqY;
    float speedX, speedY;
    float waveAmp;

    // Ripple
    float rippleFreq;
    float rippleSpeed;
    float rippleAmp;

    // Spiral
    float spiralArms;
    float spiralTightness;
    float spiralAmp;

    // Audio reactivity
    float bassScale;
    float trebleDetail;
    float amplitude;

    // Color
    float hueBase;
    float hueRange;
    float hueSpeed;
    float saturation;
    float brightness;

    // Camera
    float camDist;
    float camHeight;
    float camSpeed;

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

    // Returns row-major 4x4 view-projection matrix
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

    static void HSVtoRGB(float h, float s, float v, float& r, float& g, float& b);

    std::vector<Vertex>   vertices_;
    std::vector<uint32_t> indices_;

    AudioData audio_ = {};
    float prevBassEnergy_ = 0;

    float time_ = 0;
    int currentPreset_ = 0;
    int targetPreset_ = 0;
    float morphT_ = 1.0f;       // 1 = fully at current preset
    float presetTimer_ = 0;
    float presetInterval_ = 15.0f;  // Auto-switch every 15 seconds
};

} // namespace ombro
