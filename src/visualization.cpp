// Ombro — Wireframe math-soundscape visualization engine
// AGPL-3.0 License

#include "visualization.h"
#include <cmath>
#include <cstring>
#include <cstdlib>

namespace ombro {

static const float PI = 3.14159265358979323846f;

// ============================================================
// Wave function helpers (WhiteCap vocabulary: trwv, sqwv)
// ============================================================

// Triangle wave: sharper ridges than sine, continuous
static float triWave(float x) {
    return asinf(sinf(x)) * (2.0f / PI);
}

// Square wave: flat plateaus, hard transitions
static float sqWave(float x) {
    return sinf(x) >= 0.0f ? 1.0f : -1.0f;
}

// ============================================================
// 32 presets — ordered as a curated visual journey (~8 min full cycle)
//
// The slideshow cycles sequentially: calm → explore → build → organic
// → intense → exotic/radial → digital → wind down → loop
// Each preset morphs smoothly into the next via parameter interpolation.
//
// Fields per preset (in order):
//   name,
//   waveFreqX, waveFreqY, waveSpeedX, waveSpeedY, waveAmp,
//   triWaveAmp, sqWaveAmp,
//   rippleFreq, rippleSpeed, rippleAmp,
//   spiralArms, spiralTightness, spiralSpeed, spiralAmp,
//   spectrumScale, spectrumExponent,
//   waveformScale,
//   wyvillScale,
//   warpAmountX, warpAmountY, warpFreqX, warpFreqY, warpSpeed,
//   flowAngle, flowSpeed, flowScale,
//   radialMix,
//   bassScale, trebleDetail, amplitude,
//   edgeFalloff,
//   hueLow, hueHigh, hueSpeed, saturation, brightness, glowIntensity,
//   camDist, camHeight, camSpeed, camFOV, camShake, camTilt,
//   fadeAmount
// ============================================================

static const Preset g_presets[] = {

    // ── ACT I: CALM AWAKENING ──────────────────────────────────

    // 0: Spectrum Ridge — classic WhiteCap: spectrum IS the landscape, gentle intro
    {"Spectrum Ridge",
     1.5f, 1.0f,  0.3f, 0.2f,  0.15f,
     0.0f, 0.0f,
     2.0f, 0.8f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     1.2f, 1.8f,
     0.15f,
     0.0f,
     0.06f, 0.04f,  2.5f, 3.0f,  0.3f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.0f, 0.15f, 0.35f,
     10.0f,
     0.55f, 0.48f, 0.02f,  0.85f, 0.8f, 1.4f,
     3.5f, 2.0f, 0.12f,  0.85f, 0.08f, 0.15f,
     0.05f},

    // 1: Aurora Borealis — rainbow colors, smooth flowing, builds from the calm
    {"Aurora Borealis",
     2.0f, 1.5f,  0.4f, 0.3f,  0.25f,
     0.0f, 0.0f,
     3.0f, 1.0f,  0.2f,
     1.0f, 3.0f,  0.5f, 0.15f,
     0.9f, 1.2f,
     0.2f,
     0.0f,
     0.08f, 0.06f,  2.0f, 2.5f,  0.25f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.5f, 0.35f, 0.4f,
     9.0f,
     0.0f, 1.0f, 0.06f,  0.85f, 0.95f, 1.2f,
     3.5f, 1.8f, 0.15f,  0.8f, 0.08f, 0.15f,
     0.055f},

    // 2: Mountain Range — majestic triangle wave peaks, earthy tones, high overview
    {"Mountain Range",
     1.5f, 1.2f,  0.2f, 0.15f,  0.1f,
     0.4f, 0.0f,                              // strong triangle = sharp mountain ridges
     2.0f, 0.5f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.8f, 1.5f,
     0.1f,
     0.15f,                                   // subtle Wyvill = foothills
     0.04f, 0.03f,  2.0f, 2.0f,  0.15f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.5f, 0.2f, 0.5f,
     8.0f,
     0.1f, 0.18f, 0.01f,  0.6f, 0.85f, 1.2f, // earth brown-green
     4.5f, 3.0f, 0.06f,  0.7f, 0.03f, 0.05f, // high, slow orbit = vista
     0.05f},

    // 3: Forest Canopy — green, organic Wyvill tree shapes, walking-through feel
    {"Forest Canopy",
     1.0f, 0.8f,  0.15f, 0.1f,  0.15f,
     0.05f, 0.0f,
     1.5f, 0.4f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f,
     0.1f,
     0.5f,                                    // organic Wyvill = tree canopy shapes
     0.12f, 0.1f,  1.5f, 1.8f,  0.2f,
     0.3f, 0.2f, 0.03f,                      // gentle forward drift = walking
     0.0f,
     1.2f, 0.15f, 0.3f,
     7.0f,
     0.28f, 0.38f, 0.015f,  0.75f, 0.8f, 1.0f, // green range
     3.0f, 1.2f, 0.08f,  0.8f, 0.03f, 0.15f, // low camera = forest floor
     0.04f},

    // ── ACT II: EXPLORATION ────────────────────────────────────

    // 4: Nebula — deep, slow, atmospheric, purple-pink, long trails
    {"Nebula",
     1.0f, 0.8f,  0.2f, 0.15f,  0.35f,
     0.0f, 0.0f,
     1.5f, 0.5f,  0.3f,
     1.0f, 1.5f,  0.4f, 0.15f,
     0.8f, 1.0f,
     0.2f,
     0.3f,
     0.15f, 0.12f,  1.5f, 2.0f,  0.2f,
     1.0f, 0.3f, 0.04f,
     0.0f,
     1.5f, 0.2f, 0.35f,
     6.0f,
     0.78f, 0.92f, 0.015f,  0.75f, 0.85f, 1.0f,
     4.0f, 2.2f, 0.08f,  0.75f, 0.04f, 0.2f,
     0.025f},

    // 5: Silk Curtain — high warp, low amplitude, flowing elegance, pink-white
    {"Silk Curtain",
     1.2f, 0.8f,  0.3f, 0.2f,  0.15f,
     0.1f, 0.0f,
     1.5f, 0.6f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 1.0f,
     0.1f,
     0.0f,
     0.2f, 0.18f,  1.5f, 1.8f,  0.25f,
     0.3f, 0.4f, 0.06f,
     0.0f,
     1.0f, 0.15f, 0.25f,
     7.0f,
     0.9f, 0.95f, 0.03f,  0.4f, 0.9f, 0.9f,
     3.8f, 1.6f, 0.1f,  0.8f, 0.05f, 0.22f,
     0.04f},

    // 6: Coral Reef — organic Wyvill blobs, aqua-orange, warm and alive
    {"Coral Reef",
     2.0f, 1.8f,  0.35f, 0.3f,  0.15f,
     0.1f, 0.0f,
     2.5f, 0.8f,  0.15f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.8f, 1.2f,
     0.15f,
     0.6f,
     0.1f, 0.08f,  2.0f, 2.5f,  0.25f,
     0.8f, 0.3f, 0.04f,
     0.0f,
     2.0f, 0.3f, 0.4f,
     8.0f,
     0.48f, 0.08f, 0.025f,  0.85f, 0.9f, 1.3f,
     3.3f, 1.8f, 0.15f,  0.8f, 0.08f, 0.12f,
     0.055f},

    // 7: Jellyfish — Wyvill blobs pulsing downward, aqua bioluminescent glow
    {"Jellyfish",
     1.5f, 1.2f,  0.25f, 0.2f,  0.15f,
     0.0f, 0.0f,
     2.0f, 0.6f,  0.15f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.6f, 1.0f,
     0.1f,
     0.7f,                                    // strong Wyvill = jellyfish bell shapes
     0.15f, 0.12f,  1.5f, 2.0f,  0.25f,
     -PI * 0.5f, 0.5f, 0.06f,                // downward flow = sinking jellyfish
     0.0f,
     1.5f, 0.2f, 0.35f,
     8.0f,
     0.5f, 0.58f, 0.02f,  0.7f, 0.85f, 1.3f, // aqua bioluminescent
     3.5f, 1.5f, 0.1f,  0.8f, 0.05f, 0.18f,
     0.035f},

    // ── ACT III: BUILDING ENERGY ───────────────────────────────

    // 8: Crystal Lattice — sharper geometry, green-white, picks up energy
    {"Crystal Lattice",
     3.5f, 3.5f,  0.4f, 0.4f,  0.15f,
     0.2f, 0.0f,
     5.0f, 1.0f,  0.2f,
     3.0f, 2.0f,  0.8f, 0.2f,
     1.0f, 2.2f,
     0.1f,
     0.0f,
     0.02f, 0.02f,  4.0f, 4.0f,  0.2f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.5f, 0.5f, 0.4f,
     12.0f,
     0.30f, 0.42f, 0.01f,  0.6f, 0.95f, 1.2f,
     3.2f, 2.5f, 0.18f,  0.7f, 0.06f, 0.08f,
     0.05f},

    // 9: Waveform Canyon — strong waveform presence, teal-yellow
    {"Waveform Canyon",
     2.0f, 1.5f,  0.5f, 0.3f,  0.2f,
     0.0f, 0.0f,
     3.0f, 1.2f,  0.15f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.7f, 1.3f,
     0.5f,
     0.0f,
     0.08f, 0.06f,  2.0f, 2.5f,  0.35f,
     0.5f, 0.5f, 0.03f,
     0.0f,
     1.8f, 0.3f, 0.4f,
     9.0f,
     0.48f, 0.16f, 0.03f,  0.8f, 0.9f, 1.3f,
     3.0f, 1.5f, 0.2f,  0.85f, 0.1f, 0.12f,
     0.06f},

    // 10: Arctic Ice — crystalline triangle ridges, cold blue-white, frozen
    {"Arctic Ice",
     4.0f, 3.5f,  0.2f, 0.15f,  0.1f,
     0.3f, 0.05f,                             // sharp triangle = ice crystals
     3.0f, 0.5f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.8f, 2.0f,                              // sharp spectrum peaks
     0.05f,
     0.0f,
     0.03f, 0.02f,  3.5f, 4.0f,  0.15f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.0f, 0.5f, 0.35f,
     10.0f,
     0.55f, 0.6f, 0.01f,  0.3f, 0.95f, 1.5f, // low sat = icy white-blue
     3.5f, 2.5f, 0.12f,  0.75f, 0.04f, 0.08f,
     0.05f},

    // 11: Tidal Wave — triangle wave ridges, blue-green, building intensity
    {"Tidal Wave",
     2.5f, 2.0f,  0.6f, 0.4f,  0.1f,
     0.35f, 0.0f,
     3.5f, 1.5f,  0.25f,
     0.0f, 0.0f,  0.0f, 0.0f,
     1.0f, 1.5f,
     0.2f,
     0.0f,
     0.1f, 0.08f,  2.0f, 2.5f,  0.4f,
     -0.3f, 0.8f, 0.06f,
     0.0f,
     2.0f, 0.4f, 0.45f,
     9.0f,
     0.52f, 0.38f, 0.025f,  0.8f, 0.85f, 1.4f,
     3.2f, 1.6f, 0.2f,  0.85f, 0.1f, 0.1f,
     0.05f},

    // 12: Quicksilver — smooth, metallic, very low saturation, liquid mercury
    {"Quicksilver",
     2.0f, 1.5f,  0.4f, 0.3f,  0.25f,
     0.0f, 0.0f,
     2.5f, 1.0f,  0.2f,
     1.0f, 2.0f,  0.5f, 0.1f,
     0.7f, 1.2f,
     0.15f,
     0.2f,                                    // subtle organic bumps
     0.15f, 0.12f,  2.0f, 2.5f,  0.35f,
     0.5f, 0.5f, 0.04f,
     0.0f,
     1.8f, 0.3f, 0.4f,
     9.0f,
     0.6f, 0.65f, 0.02f,  0.15f, 0.95f, 1.3f, // very low sat = silver
     3.2f, 1.8f, 0.18f,  0.82f, 0.08f, 0.1f,
     0.05f},

    // ── ACT IV: HEAT ───────────────────────────────────────────

    // 13: Ember Flow — spiral-heavy, red-gold, warm currents
    {"Ember Flow",
     1.2f, 1.0f,  0.3f, 0.25f,  0.2f,
     0.0f, 0.0f,
     2.0f, 0.6f,  0.2f,
     4.0f, 5.0f,  0.7f, 0.3f,
     0.6f, 1.2f,
     0.15f,
     0.15f,
     0.10f, 0.08f,  2.0f, 2.5f,  0.3f,
     2.5f, 0.6f, 0.05f,
     0.0f,
     2.5f, 0.2f, 0.45f,
     8.0f,
     0.03f, 0.1f, 0.02f,  0.9f, 0.9f, 1.5f,
     3.5f, 2.0f, 0.15f,  0.8f, 0.1f, 0.18f,
     0.05f},

    // 14: Solar Flare — extreme spirals outward, orange-white, coronal ejection
    {"Solar Flare",
     3.0f, 2.5f,  0.8f, 0.6f,  0.15f,
     0.1f, 0.0f,
     5.0f, 2.5f,  0.2f,
     5.0f, 6.0f,  1.5f, 0.3f,               // intense spirals = solar jets
     1.2f, 1.5f,
     0.15f,
     0.2f,
     0.1f, 0.08f,  2.5f, 3.0f,  0.5f,
     0.0f, 0.0f, 0.0f,
     0.3f,                                    // slight radial = solar disc
     2.5f, 0.5f, 0.5f,
     8.0f,
     0.05f, 0.15f, 0.03f,  0.95f, 1.0f, 2.0f, // orange-white, max glow
     3.0f, 1.5f, 0.25f,  0.9f, 0.15f, 0.1f,
     0.04f},

    // 15: Earthquake — seismic square wave plateaus, extreme shake, waveform seismograph
    {"Earthquake",
     2.5f, 2.0f,  0.8f, 0.6f,  0.1f,
     0.15f, 0.35f,                            // strong square wave = tectonic plates
     3.0f, 1.5f,  0.2f,
     0.0f, 0.0f,  0.0f, 0.0f,
     1.0f, 1.8f,
     0.3f,                                    // strong waveform = seismograph line
     0.0f,
     0.08f, 0.06f,  2.0f, 2.5f,  0.4f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     3.0f, 0.3f, 0.5f,                       // extreme bass reactivity
     8.0f,
     0.05f, 0.1f, 0.01f,  0.7f, 0.85f, 1.6f, // brown-red earthy
     2.5f, 1.0f, 0.2f,  0.9f, 0.25f, 0.05f,  // close, low camera, EXTREME shake
     0.045f},

    // ── ACT V: PEAK INTENSITY ──────────────────────────────────

    // 16: Pulse Engine — heavy bass, aggressive, red-orange, peak energy
    {"Pulse Engine",
     3.0f, 2.5f,  1.0f, 0.8f,  0.2f,
     0.1f, 0.1f,
     4.0f, 2.0f,  0.25f,
     2.0f, 3.0f,  1.5f, 0.15f,
     0.9f, 1.5f,
     0.25f,
     0.2f,
     0.12f, 0.10f,  2.0f, 2.5f,  0.6f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     2.5f, 0.4f, 0.5f,
     8.0f,
     0.02f, 0.12f, 0.04f,  0.95f, 0.9f, 1.8f,
     3.0f, 1.8f, 0.28f,  0.9f, 0.15f, 0.1f,
     0.04f},

    // 17: Storm Front — electric blue-white, maximum intensity
    {"Storm Front",
     4.5f, 4.0f,  1.2f, 1.0f,  0.15f,
     0.1f, 0.05f,
     7.0f, 3.0f,  0.2f,
     2.0f, 4.0f,  2.0f, 0.1f,
     1.5f, 2.0f,
     0.2f,
     0.15f,
     0.10f, 0.08f,  3.0f, 3.5f,  0.5f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     2.0f, 0.7f, 0.45f,
     8.0f,
     0.58f, 0.52f, 0.05f,  0.5f, 1.0f, 2.0f,
     2.8f, 1.6f, 0.35f,  0.95f, 0.18f, 0.05f,
     0.04f},

    // 18: Cathedral — massive reverberant space, purple-gold, majestic
    {"Cathedral",
     1.5f, 1.2f,  0.25f, 0.2f,  0.3f,
     0.15f, 0.0f,
     2.0f, 0.8f,  0.25f,
     2.0f, 3.0f,  0.6f, 0.2f,
     0.9f, 1.2f,
     0.15f,
     0.3f,                                    // organic pillars
     0.1f, 0.08f,  2.0f, 2.5f,  0.2f,
     0.0f, 0.0f, 0.0f,
     0.0f,
     2.0f, 0.2f, 0.5f,
     7.0f,
     0.75f, 0.12f, 0.02f,  0.8f, 0.9f, 1.4f, // purple → gold
     4.0f, 2.5f, 0.1f,  0.7f, 0.06f, 0.15f,
     0.03f},                                  // long trails = reverb

    // ── ACT VI: EXOTIC RADIAL ──────────────────────────────────

    // 19: Thunderdome — radial topology, aggressive, cyan-white, dramatic
    {"Thunderdome",
     3.0f, 3.0f,  0.8f, 0.6f,  0.2f,
     0.15f, 0.0f,
     5.0f, 2.0f,  0.2f,
     3.0f, 4.0f,  1.5f, 0.15f,
     1.2f, 1.8f,
     0.15f,
     0.0f,
     0.08f, 0.06f,  2.5f, 3.0f,  0.4f,
     0.0f, 0.0f, 0.0f,
     0.7f,
     2.2f, 0.5f, 0.45f,
     10.0f,
     0.52f, 0.55f, 0.04f,  0.5f, 1.0f, 1.8f,
     3.5f, 1.5f, 0.25f,  0.9f, 0.14f, 0.08f,
     0.045f},

    // 20: Neutron Star — radial, fast spin, white-blue, intense stellar
    {"Neutron Star",
     4.0f, 3.5f,  1.0f, 0.8f,  0.15f,
     0.1f, 0.0f,
     6.0f, 3.0f,  0.15f,
     3.0f, 5.0f,  2.0f, 0.15f,               // fast spirals = magnetic field lines
     1.3f, 2.0f,
     0.1f,
     0.0f,
     0.06f, 0.05f,  3.0f, 3.5f,  0.6f,
     0.0f, 1.5f, 0.06f,                      // outward flow
     0.9f,                                    // strong radial = spinning star
     2.0f, 0.6f, 0.4f,
     12.0f,
     0.55f, 0.58f, 0.03f,  0.4f, 1.0f, 2.2f, // white-blue, extreme glow
     2.5f, 0.8f, 0.4f,  1.0f, 0.15f, 0.05f,
     0.04f},

    // 21: Wormhole — full radial tunnel, tight spirals pulling inward, deep purple
    {"Wormhole",
     2.0f, 1.5f,  0.5f, 0.4f,  0.15f,
     0.0f, 0.0f,
     4.0f, 2.0f,  0.15f,
     3.0f, 8.0f,  1.0f, 0.2f,               // very tight spirals = wormhole rotation
     0.8f, 1.3f,
     0.1f,
     0.0f,
     0.08f, 0.06f,  2.5f, 3.0f,  0.4f,
     PI, 1.2f, 0.08f,                        // inward pull = entering wormhole
     1.0f,                                    // full radial = tunnel
     2.0f, 0.4f, 0.4f,
     14.0f,
     0.7f, 0.8f, 0.03f,  0.8f, 0.85f, 1.6f, // deep purple
     2.0f, 0.3f, 0.2f,  1.1f, 0.1f, 0.1f,   // close, inside the tunnel, wide FOV
     0.035f},

    // 22: Hyperdrive — radial, outward flow, high speed, rainbow tunnel
    {"Hyperdrive",
     2.5f, 2.0f,  0.6f, 0.5f,  0.15f,
     0.0f, 0.0f,
     4.0f, 2.5f,  0.15f,
     2.0f, 6.0f,  1.2f, 0.1f,
     1.0f, 1.5f,
     0.1f,
     0.0f,
     0.06f, 0.05f,  2.0f, 2.5f,  0.3f,
     0.0f, 1.5f, 0.08f,
     0.85f,
     1.8f, 0.4f, 0.35f,
     12.0f,
     0.0f, 1.0f, 0.08f,  0.9f, 0.9f, 1.5f,
     2.8f, 0.5f, 0.3f,  1.0f, 0.12f, 0.05f,
     0.035f},

    // 23: Pulsing Orb — radial sphere that pulses to the beat, morphs shape
    {"Pulsing Orb",
     2.5f, 2.0f,  0.5f, 0.4f,  0.2f,
     0.1f, 0.0f,
     3.0f, 1.5f,  0.2f,
     1.0f, 2.0f,  0.8f, 0.1f,
     1.0f, 1.5f,
     0.15f,
     0.3f,                                    // Wyvill = organic sphere deformation
     0.1f, 0.08f,  2.0f, 2.5f,  0.3f,
     0.0f, 0.0f, 0.0f,
     0.85f,                                   // mostly spherical
     3.0f, 0.3f, 0.45f,                      // very strong bass = pulsing sphere
     10.0f,
     0.5f, 0.55f, 0.04f,  0.6f, 0.95f, 1.8f, // cyan-white, strong glow
     3.0f, 1.5f, 0.15f,  0.85f, 0.12f, 0.08f,
     0.045f},

    // 24: Volcanic — radial with outward flow, red-orange lava eruption from core
    {"Volcanic",
     2.0f, 1.5f,  0.6f, 0.5f,  0.2f,
     0.1f, 0.0f,
     3.0f, 1.5f,  0.2f,
     2.0f, 3.0f,  1.0f, 0.2f,
     1.0f, 1.5f,
     0.15f,
     0.4f,                                    // Wyvill = lava blobs
     0.1f, 0.08f,  2.0f, 2.5f,  0.35f,
     0.0f, 1.0f, 0.07f,                      // outward flow = eruption
     0.6f,                                    // partial radial = volcanic shape
     2.5f, 0.3f, 0.45f,
     10.0f,
     0.0f, 0.08f, 0.02f,  0.95f, 0.9f, 1.8f, // red → orange, strong glow
     3.0f, 1.8f, 0.18f,  0.85f, 0.12f, 0.1f,
     0.04f},

    // 25: Event Horizon — radial, slow spiral inward, dark and fading, gravitational
    {"Event Horizon",
     1.5f, 1.2f,  0.3f, 0.2f,  0.2f,
     0.0f, 0.0f,
     2.0f, 0.8f,  0.2f,
     2.0f, 4.0f,  0.5f, 0.15f,               // spirals = accretion disc
     0.7f, 1.0f,
     0.1f,
     0.1f,
     0.08f, 0.06f,  2.0f, 2.5f,  0.2f,
     PI, 0.5f, 0.05f,                        // slow inward pull
     0.8f,
     1.5f, 0.2f, 0.3f,
     10.0f,
     0.65f, 0.7f, 0.01f,  0.6f, 0.7f, 1.0f, // dim purple, fading
     3.5f, 1.5f, 0.1f,  0.8f, 0.05f, 0.15f,
     0.025f},                                 // long ghostly trails

    // ── ACT VII: DIGITAL ───────────────────────────────────────

    // 26: Neon Grid — retro square waves, high saturation, cyan-magenta, synthwave
    {"Neon Grid",
     3.0f, 3.0f,  0.3f, 0.3f,  0.05f,
     0.0f, 0.3f,                              // strong square = clean grid plateaus
     2.0f, 0.5f,  0.05f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.8f, 2.0f,
     0.1f,
     0.0f,
     0.02f, 0.02f,  3.0f, 3.0f,  0.2f,      // minimal warp = clean grid
     0.0f, 0.0f, 0.0f,
     0.0f,
     1.5f, 0.5f, 0.35f,
     12.0f,
     0.5f, 0.85f, 0.04f,  1.0f, 0.9f, 1.8f, // cyan → magenta, max sat
     3.0f, 2.0f, 0.15f,  0.8f, 0.06f, 0.05f,
     0.05f},

    // 27: Binary Rain — square wave plateaus, green-on-black, digital/Matrix
    {"Binary Rain",
     3.0f, 2.5f,  0.5f, 0.4f,  0.05f,
     0.0f, 0.3f,
     2.0f, 0.8f,  0.1f,
     0.0f, 0.0f,  0.0f, 0.0f,
     1.0f, 2.5f,
     0.1f,
     0.0f,
     0.03f, 0.02f,  3.0f, 3.5f,  0.3f,
     -PI * 0.5f, 1.0f, 0.05f,
     0.0f,
     1.5f, 0.6f, 0.35f,
     10.0f,
     0.33f, 0.38f, 0.01f,  0.9f, 0.85f, 1.6f,
     3.0f, 2.0f, 0.15f,  0.8f, 0.06f, 0.05f,
     0.04f},

    // ── ACT VIII: WIND DOWN ────────────────────────────────────

    // 28: Cosmic Web — faint filaments, high treble detail, vast and quiet
    {"Cosmic Web",
     2.0f, 1.5f,  0.15f, 0.1f,  0.1f,
     0.0f, 0.0f,
     1.5f, 0.5f,  0.1f,
     1.0f, 2.0f,  0.3f, 0.1f,
     0.5f, 0.8f,
     0.05f,
     0.1f,
     0.1f, 0.08f,  2.0f, 2.5f,  0.2f,
     0.2f, 0.15f, 0.02f,                     // barely drifting
     0.0f,
     0.8f, 0.8f, 0.2f,                       // high treble detail, low amp = filaments
     6.0f,
     0.7f, 0.8f, 0.015f,  0.5f, 0.6f, 0.8f, // faint purple
     5.0f, 3.0f, 0.06f,  0.65f, 0.02f, 0.2f, // far away, vast
     0.03f},

    // 29: Firefly Marsh — dark, sparse Wyvill glows, yellow-green bioluminescence
    {"Firefly Marsh",
     1.0f, 0.8f,  0.1f, 0.08f,  0.05f,
     0.0f, 0.0f,
     1.0f, 0.3f,  0.05f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.3f, 0.8f,
     0.05f,
     0.6f,                                    // strong Wyvill = firefly glows
     0.1f, 0.08f,  1.5f, 2.0f,  0.15f,
     0.5f, 0.15f, 0.02f,                     // barely drifting
     0.0f,
     1.0f, 0.1f, 0.2f,
     6.0f,
     0.22f, 0.3f, 0.01f,  0.8f, 0.5f, 1.5f, // dim but glowy when active
     3.5f, 1.2f, 0.06f,  0.8f, 0.02f, 0.18f,
     0.03f},

    // 30: Lava Lamp — slow Wyvill blobs, warm colors, organic and relaxing
    {"Lava Lamp",
     0.8f, 0.6f,  0.15f, 0.1f,  0.2f,
     0.0f, 0.0f,
     1.0f, 0.3f,  0.15f,
     0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, 0.8f,
     0.1f,
     0.8f,
     0.18f, 0.15f,  1.2f, 1.5f,  0.15f,
     0.5f, 0.2f, 0.05f,
     0.0f,
     2.0f, 0.1f, 0.3f,
     7.0f,
     0.05f, 0.15f, 0.02f,  0.9f, 0.85f, 1.0f,
     3.8f, 2.0f, 0.08f,  0.75f, 0.04f, 0.2f,
     0.03f},

    // 31: Deep Space — slow undulation, deep purple, long trails, fadeout → loops to 0
    {"Deep Space",
     1.0f, 0.8f,  0.2f, 0.15f,  0.3f,
     0.0f, 0.0f,
     1.5f, 0.5f,  0.25f,
     1.0f, 1.0f,  0.3f, 0.1f,
     0.6f, 1.0f,
     0.15f,
     0.2f,
     0.12f, 0.10f,  1.5f, 2.0f,  0.18f,
     0.8f, 0.15f, 0.03f,
     0.0f,
     1.5f, 0.15f, 0.3f,
     8.0f,
     0.72f, 0.78f, 0.01f,  0.7f, 0.8f, 0.9f,
     4.2f, 2.5f, 0.08f,  0.7f, 0.03f, 0.2f,
     0.025f},
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
    memset(prevSpectrum_, 0, sizeof(prevSpectrum_));
}

// ============================================================
// Audio analysis — fast attack, slow decay per bin
// ============================================================

void Visualization::AnalyzeAudio(const unsigned char spectrum[2][576],
                                  const unsigned char waveform[2][576]) {
    float raw[256];
    for (int i = 0; i < 256; i++) {
        raw[i] = ((float)spectrum[0][i] + (float)spectrum[1][i]) * 0.5f / 255.0f;
        if (raw[i] > audio_.spectrum[i])
            audio_.spectrum[i] = lerp(audio_.spectrum[i], raw[i], 0.55f);
        else
            audio_.spectrum[i] = lerp(audio_.spectrum[i], raw[i], 0.06f);
    }

    for (int i = 0; i < 576; i++) {
        float sample = ((float)waveform[0][i] + (float)waveform[1][i]) * 0.5f;
        float normalized = (sample - 128.0f) / 128.0f;
        audio_.waveform[i] = lerp(audio_.waveform[i], normalized, 0.4f);
    }

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

    float bassDerivative = bass - prevBassEnergy_;
    prevBassEnergy_ = bass;
    if (bassDerivative > 0.12f)
        audio_.beatIntensity = 1.0f;
    else
        audio_.beatIntensity *= 0.92f;

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
        float freqT = (float)row / (float)(GRID_SIZE - 1);
        int freqBin = (int)(freqT * 255.0f);
        float specEnergy = audio_.spectrum[freqBin];

        for (int col = 0; col < GRID_SIZE; col++) {
            float colT = (float)col / (float)(GRID_SIZE - 1);
            float gx = -1.0f + col * gridScale;
            float gy = -1.0f + row * gridScale;

            // --- Radial topology blending ---
            // 0 = flat grid, 1 = circular (col maps to radius, row to angle)
            float flatX = gx, flatY = gy;
            if (p.radialMix > 0.001f) {
                float r = lerp(0.3f, 1.2f, colT);
                float theta = freqT * 2.0f * PI;
                float radX = r * cosf(theta);
                float radY = r * sinf(theta);
                gx = lerp(flatX, radX, p.radialMix);
                gy = lerp(flatY, radY, p.radialMix);
            }

            // --- Grid XY warping (organic flowing distortion) ---
            float wx = gx + sinf(gy * p.warpFreqY * PI + time * p.warpSpeed)
                           * p.warpAmountX * (1.0f + audio_.bass * 0.5f);
            float wy = gy + cosf(gx * p.warpFreqX * PI + time * p.warpSpeed * 0.7f)
                           * p.warpAmountY * (1.0f + audio_.mid * 0.5f);

            // --- Flow field displacement (directional streaming) ---
            if (p.flowScale > 0.001f) {
                float flowDirX = cosf(p.flowAngle);
                float flowDirY = sinf(p.flowAngle);
                // Phase = projection along flow direction + time
                float flowPhase = (gx * flowDirX + gy * flowDirY) * 3.0f
                                + time * p.flowSpeed;
                // Displace perpendicular to flow direction for streaming fabric effect
                wx += sinf(flowPhase) * p.flowScale * (-flowDirY)
                    * (1.0f + audio_.totalEnergy * 0.5f);
                wy += sinf(flowPhase * 1.3f) * p.flowScale * flowDirX
                    * (1.0f + audio_.totalEnergy * 0.5f);
            }

            float z = 0;

            // --- Spectrum-driven displacement (WhiteCap signature) ---
            float specZ = powf(specEnergy, p.spectrumExponent) * p.spectrumScale;
            z += specZ;

            // --- Waveform overlay (audio shape embedded into surface) ---
            if (p.waveformScale > 0.001f) {
                int waveSample = (int)(colT * 575.0f);
                z += audio_.waveform[waveSample] * p.waveformScale * (0.5f + specEnergy);
            }

            // --- Crossing sine waves ---
            z += sinf(wx * p.waveFreqX * PI + time * p.waveSpeedX)
               * cosf(wy * p.waveFreqY * PI + time * p.waveSpeedY)
               * p.waveAmp;

            // --- Triangle wave ridges (sharper peaks than sine) ---
            if (p.triWaveAmp > 0.001f) {
                z += triWave(wx * p.waveFreqX * PI + time * p.waveSpeedX * 1.1f)
                   * triWave(wy * p.waveFreqY * PI + time * p.waveSpeedY * 0.9f)
                   * p.triWaveAmp;
            }

            // --- Square wave plateaus (flat-top mesas) ---
            if (p.sqWaveAmp > 0.001f) {
                z += sqWave(wx * p.waveFreqX * PI * 0.5f + time * p.waveSpeedX * 0.6f)
                   * sqWave(wy * p.waveFreqY * PI * 0.5f + time * p.waveSpeedY * 0.5f)
                   * p.sqWaveAmp * 0.3f;
            }

            // --- Concentric ripple ---
            float r = sqrtf(wx * wx + wy * wy);
            z += sinf(r * p.rippleFreq * PI - time * p.rippleSpeed) * p.rippleAmp;

            // --- Spiral arms ---
            if (p.spiralAmp > 0.001f) {
                float angle = atan2f(wy, wx);
                z += sinf(angle * p.spiralArms + r * p.spiralTightness * PI - time * p.spiralSpeed)
                   * p.spiralAmp;
            }

            // --- Wyvill organic blob peaks (metaball-like bumps) ---
            // 4 drifting blobs whose height is driven by different bass frequencies
            if (p.wyvillScale > 0.001f) {
                for (int bi = 0; bi < 4; bi++) {
                    float bPhase = (float)bi * PI * 0.5f;
                    float bx = cosf(bPhase + time * 0.25f) * 0.5f;
                    float by = sinf(bPhase + time * 0.2f) * 0.5f;
                    float dist = sqrtf((wx - bx) * (wx - bx) + (wy - by) * (wy - by));
                    z += Wyvill(dist * 2.5f) * audio_.spectrum[bi * 16] * p.wyvillScale;
                }
            }

            // --- Audio modulation ---
            z *= (1.0f + audio_.bass * p.bassScale);
            z += audio_.treble * sinf(wx * 15.0f * PI + time * 2.0f)
               * sinf(wy * 15.0f * PI + time * 1.5f) * p.trebleDetail * 0.15f;
            z *= (1.0f + audio_.beatIntensity * 0.25f);

            z *= p.amplitude;

            // --- Two-tone color: frequency-mapped hue ---
            float hue = lerp(p.hueLow, p.hueHigh, freqT) + time * p.hueSpeed;
            float bright = p.brightness
                         * (0.3f + specEnergy * p.glowIntensity * 0.7f
                            + audio_.totalEnergy * 0.3f
                            + fabsf(z) * 0.4f);
            bright = clamp01(bright);

            float cr, cg, cb;
            HSVtoRGB(hue, p.saturation, bright, cr, cg, cb);

            // --- Edge falloff ---
            // In radial mode, only X (inner/outer radius) fades; Y wraps around
            float edgeDistX = fminf((float)col, (float)(GRID_SIZE - 1 - col));
            float edgeDistYFlat = fminf((float)row, (float)(GRID_SIZE - 1 - row));
            float edgeDistYRadial = (float)GRID_SIZE;
            float edgeDistY = lerp(edgeDistYFlat, edgeDistYRadial, p.radialMix);
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
    dist *= (1.0f - audio_.beatIntensity * 0.04f);
    float height = p.camHeight + sinf(time * 0.2f) * 0.3f + audio_.mid * 0.3f;

    float eyeX = cosf(angle) * dist;
    float eyeZ = sinf(angle) * dist;
    float eyeY = height;

    float shakeX = sinf(time * 47.0f) * audio_.beatIntensity * p.camShake * 0.1f;
    float shakeY = cosf(time * 53.0f) * audio_.beatIntensity * p.camShake * 0.1f;
    eyeX += shakeX;
    eyeY += shakeY;

    float lookX = sinf(time * 0.1f) * 0.15f;
    float lookZ = cosf(time * 0.13f) * 0.15f;

    float fwdX = lookX - eyeX;
    float fwdY = 0.0f - eyeY;
    float fwdZ = lookZ - eyeZ;
    float fwdLen = sqrtf(fwdX * fwdX + fwdY * fwdY + fwdZ * fwdZ);
    fwdX /= fwdLen; fwdY /= fwdLen; fwdZ /= fwdLen;

    float roll = sinf(time * 0.15f) * p.camTilt;
    float worldUpX = sinf(roll);
    float worldUpY = cosf(roll);
    float worldUpZ = 0.0f;

    float rightX = fwdY * worldUpZ - fwdZ * worldUpY;
    float rightY = fwdZ * worldUpX - fwdX * worldUpZ;
    float rightZ = fwdX * worldUpY - fwdY * worldUpX;
    float rightLen = sqrtf(rightX * rightX + rightY * rightY + rightZ * rightZ);
    if (rightLen > 0.0001f) {
        rightX /= rightLen; rightY /= rightLen; rightZ /= rightLen;
    }

    float upX = rightY * fwdZ - rightZ * fwdY;
    float upY = rightZ * fwdX - rightX * fwdZ;
    float upZ = rightX * fwdY - rightY * fwdX;

    float view[16] = {
        rightX,  upX,  fwdX,  0,
        rightY,  upY,  fwdY,  0,
        rightZ,  upZ,  fwdZ,  0,
        -(rightX*eyeX + rightY*eyeY + rightZ*eyeZ),
        -(upX*eyeX + upY*eyeY + upZ*eyeZ),
        -(fwdX*eyeX + fwdY*eyeY + fwdZ*eyeZ),
        1
    };

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
    LP(triWaveAmp); LP(sqWaveAmp);
    LP(rippleFreq); LP(rippleSpeed); LP(rippleAmp);
    LP(spiralArms); LP(spiralTightness); LP(spiralSpeed); LP(spiralAmp);
    LP(spectrumScale); LP(spectrumExponent);
    LP(waveformScale);
    LP(wyvillScale);
    LP(warpAmountX); LP(warpAmountY); LP(warpFreqX); LP(warpFreqY); LP(warpSpeed);
    LP(flowAngle); LP(flowSpeed); LP(flowScale);
    LP(radialMix);
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

    // Auto-preset cycling — sequential like WhiteCap's slideshow
    presetTimer_ += deltaTime;
    if (presetTimer_ >= presetInterval_ && morphT_ >= 1.0f) {
        presetTimer_ = 0;
        targetPreset_ = (currentPreset_ + 1) % NUM_PRESETS;
        morphT_ = 0;
    }

    // Advance morph — WhiteCap-style easing
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
