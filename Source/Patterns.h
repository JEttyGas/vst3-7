#pragma once
#include <juce_core/juce_core.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

// Кривые «громкость»/«время», перестановки слайсера, окна гранулятора и таблицы названий.
//   p — фаза паттерна 0..1,  N — число слайсов (повторов кривой) внутри паттерна
namespace Patterns
{
    // ---------------- названия (choice-параметры) ----------------
    inline const juce::StringArray timeNames   { "Normal", "Half Speed", "Tape Stop", "Tape Start", "Stutter", "Reverse",
                                                 "Scratch", "Quarter Speed", "Fast Stutter", "Backspin", "Wobble", "Double Repeat" };
    inline const juce::StringArray volNames    { "Off", "Gate", "Pump", "Ramp Down", "Ramp Up", "Tremolo", "Random",
                                                 "Short Gate", "Long Gate", "Stairs Down", "Stairs Up", "Offbeat", "Swing Gate" };
    inline const juce::StringArray sliceNames  { "1", "2", "3", "4", "6", "8", "12", "16" };
    inline const juce::StringArray lengthNames { "1 beat", "2 beats", "4 beats" };

    inline const juce::StringArray slicerNames { "Off", "Shuffle", "Random Pick", "Reverse", "Swap Pairs", "Skip x2",
                                                 "Skip x3", "Mirror", "Hold Groups", "Half Time", "Lag" };
    inline const juce::StringArray stepNames   { "2", "3", "4", "6", "8", "12", "16", "32" };
    inline const juce::StringArray slLengthNames { "1 beat", "2 beats", "4 beats", "8 beats" };

    inline const juce::StringArray windowNames { "Hann", "Flat", "Pluck", "Swell" };
    inline const juce::StringArray quantNames  { "Off", "Octaves", "Fifths", "Major Pent", "Minor Pent", "Whole Tone", "Chromatic" };
    inline const juce::StringArray specNames   { "Off", "Freeze", "Blur", "Robot", "Whisper", "Gate", "Shift", "Comb", "Warp",
                                                 "Sustain", "Crush", "Octaver", "Tilt", "Sweep", "Vowel", "Shatter", "Mirror" };
    inline const juce::StringArray distNames   { "Off", "Soft", "Hard", "Fold", "Crush", "Tube", "Rectify", "Sine" };
    inline const juce::StringArray filterNames { "Off", "Low-pass", "High-pass", "Band-pass" };
    inline const juce::StringArray slopeNames  { "12 dB", "24 dB" };
    inline const juce::StringArray filterSrcNames { "Pattern", "LFO", "Envelope" };
    inline const juce::StringArray ringShapeNames { "Sine", "Triangle", "Square", "Saw" };
    inline const juce::StringArray ringModeNames  { "Ring", "AM" };
    inline const juce::StringArray dlySyncNames { "1/16", "1/16T", "1/8", "1/8T", "1/8.", "1/4", "1/4T", "1/4.", "1/2", "1/1", "Free" };
    inline const juce::StringArray dlyModeNames { "Stereo", "Ping-Pong", "Tape" };

    // ---------------- числовые таблицы ----------------
    inline constexpr int    sliceValues[]  = { 1, 2, 3, 4, 6, 8, 12, 16 };
    inline constexpr double lengthBeats[]  = { 1.0, 2.0, 4.0 };
    inline constexpr int    stepValues[]   = { 2, 3, 4, 6, 8, 12, 16, 32 };
    inline constexpr double slLengthBeats[] = { 1.0, 2.0, 4.0, 8.0 };
    inline constexpr double dlyBeats[]     = { 0.25, 1.0 / 6.0, 0.5, 1.0 / 3.0, 0.75, 1.0, 2.0 / 3.0, 1.5, 2.0, 4.0 };

    inline constexpr float twoPi = 6.28318530718f;

    inline float frac (float x) { return x - std::floor (x); }

    inline float hash01 (int n)
    {
        unsigned h = (unsigned) n * 2654435761u;
        h ^= h >> 15; h *= 2246822519u; h ^= h >> 13;
        return (float) (h & 0xffffu) / 65535.0f;
    }

    // ---------------- свинг: сдвиг каждого второго слайса ----------------
    inline float swingWarp (float p, int N, float s)
    {
        if (s <= 0.0001f || N < 2) return p;
        const float t = p * (float) N;
        const float m = std::floor (t * 0.5f);
        const float u = t - 2.0f * m;                       // 0..2 внутри пары слайсов
        const float tt = (u < 1.0f + s) ? 2.0f * m + u / (1.0f + s)
                                        : 2.0f * m + 1.0f + (u - (1.0f + s)) / (1.0f - s);
        return tt / (float) N;
    }

    // ---------------- огибающая громкости: 1 = открыто ----------------
    inline float volumeCurve (int type, float p, int N)
    {
        const float t = p * (float) N;
        const int   k = (int) std::floor (t);
        const float x = t - (float) k;

        switch (type)
        {
            case 1:  return x < 0.5f ? 1.0f : 0.0f;                              // Gate
            case 2:  return 1.0f - std::pow (1.0f - x, 3.0f);                     // Pump
            case 3:  return 1.0f - x;                                             // Ramp Down
            case 4:  return x;                                                    // Ramp Up
            case 5:  return 0.5f - 0.5f * std::cos (twoPi * x);                   // Tremolo
            case 6:  return hash01 (k) > 0.5f ? 1.0f : 0.0f;                      // Random gate
            case 7:  return x < 0.25f ? 1.0f : 0.0f;                              // Short Gate
            case 8:  return x < 0.75f ? 1.0f : 0.0f;                              // Long Gate
            case 9:  return 1.0f - std::floor (x * 4.0f) * 0.25f;                 // Stairs Down
            case 10: return 0.25f + std::floor (x * 4.0f) * 0.25f;                // Stairs Up
            case 11: return (k & 1) ? 1.0f : 0.0f;                                // Offbeat
            case 12: return (k & 1) ? (x < 0.34f ? 1.0f : 0.0f)                   // Swing Gate
                                    : (x < 0.66f ? 1.0f : 0.0f);
            default: return 1.0f;                                                 // Off
        }
    }

    // ---------------- «позиция ленты» f(p) <= p ----------------
    inline float timeCurve (int type, float p, int N)
    {
        const float t    = p * (float) N;
        const int   k    = (int) std::floor (t);
        const float x    = t - (float) k;
        const float invN = 1.0f / (float) N;
        const float fk   = (float) k;

        switch (type)
        {
            case 1:  return (fk + 0.5f * x) * invN;                                   // Half Speed
            case 2:  return (fk + x - 0.5f * x * x) * invN;                           // Tape Stop
            case 3:  return (fk + 0.5f * x * x) * invN;                               // Tape Start
            case 4:  return (fk + frac (x * 4.0f) * 0.25f) * invN;                    // Stutter
            case 5:  return (fk - x) * invN;                                          // Reverse
            case 6:  return (fk + std::sin (twoPi * x) / twoPi) * invN;               // Scratch
            case 7:  return (fk + 0.25f * x) * invN;                                  // Quarter Speed
            case 8:  return (fk + frac (x * 8.0f) * 0.125f) * invN;                   // Fast Stutter
            case 9:  return (fk + x - 1.5f * x * x) * invN;                           // Backspin
            case 10: return (fk + x - 0.5f * (1.0f - std::cos (2.0f * twoPi * x)) / (2.0f * twoPi)) * invN; // Wobble
            case 11: return (fk + frac (x * 2.0f) * 0.5f) * invN;                     // Double Repeat
            default: return p;                                                        // Normal
        }
    }

    // ---------------- окно зерна ----------------
    inline float grainWindow (int type, float u)
    {
        switch (type)
        {
            case 1:  return std::min (1.0f, 4.0f * std::min (u, 1.0f - u));
            case 2:  return u < 0.04f ? u / 0.04f : std::exp (-(u - 0.04f) * 6.0f);
            case 3:  { const float v = 1.0f - u; return v < 0.04f ? v / 0.04f : std::exp (-(v - 0.04f) * 6.0f); }
            default: return 0.5f - 0.5f * std::cos (twoPi * u);
        }
    }

    // ---------------- квантизация случайного питча зерна к ступеням ----------------
    inline float snapPitch (int mode, float j)
    {
        static const float oct[]   = { 0.0f };
        static const float fifth[] = { 0.0f, 7.0f };
        static const float majP[]  = { 0.0f, 2.0f, 4.0f, 7.0f, 9.0f };
        static const float minP[]  = { 0.0f, 3.0f, 5.0f, 7.0f, 10.0f };
        static const float whole[] = { 0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f };

        const float* set = nullptr; int n = 0;
        switch (mode)
        {
            case 1: set = oct;   n = 1; break;
            case 2: set = fifth; n = 2; break;
            case 3: set = majP;  n = 5; break;
            case 4: set = minP;  n = 5; break;
            case 5: set = whole; n = 6; break;
            case 6: return std::round (j);
            default: return j;
        }

        float best = 0.0f, bestD = 1.0e9f;
        for (int o = -3; o <= 3; ++o)
            for (int i = 0; i < n; ++i)
            {
                const float c = (float) o * 12.0f + set[i];
                const float d = std::abs (c - j);
                if (d < bestD) { bestD = d; best = c; }
            }
        return best;
    }

    // ---------------- слайсер: перестановка кусочков ----------------
    // perm[k] = индекс исходного слайса, который играет на месте k
    inline uint32_t xorshift (uint32_t& s) { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }

    inline void buildPermutation (int type, int N, uint32_t seed, int* perm)
    {
        uint32_t s = seed * 2654435761u + 0x9E3779B9u;
        if (s == 0) s = 1;
        for (int i = 0; i < 4; ++i) xorshift (s);

        for (int k = 0; k < N; ++k) perm[k] = k;

        switch (type)
        {
            case 1: for (int i = N - 1; i > 0; --i) std::swap (perm[i], perm[(int) (xorshift (s) % (uint32_t) (i + 1))]); break; // Shuffle
            case 2: for (int k = 0; k < N; ++k) perm[k] = (int) (xorshift (s) % (uint32_t) N);                                 break; // Random Pick
            case 3: for (int k = 0; k < N; ++k) perm[k] = N - 1 - k;                                                          break; // Reverse
            case 4: for (int k = 0; k < N; ++k) perm[k] = ((k ^ 1) < N) ? (k ^ 1) : k;                                        break; // Swap Pairs
            case 5: for (int k = 0; k < N; ++k) perm[k] = (2 * k) % N;                                                        break; // Skip x2
            case 6: for (int k = 0; k < N; ++k) perm[k] = (3 * k) % N;                                                        break; // Skip x3
            case 7: for (int k = 0; k < N; ++k) perm[k] = (k < N / 2) ? k : N - 1 - k;                                        break; // Mirror
            case 8: for (int k = 0; k < N; ++k) perm[k] = (k / 4) * 4;                                                        break; // Hold Groups
            case 9: for (int k = 0; k < N; ++k) perm[k] = k / 2;                                                              break; // Half Time
            case 10: for (int k = 0; k < N; ++k) perm[k] = (k + N - 1) % N;                                                   break; // Lag
            default: break;
        }
    }

    // ---------------- LFO: банк из 4 генераторов + матрица модуляции ----------------
    inline const juce::StringArray lfoShapeNames { "Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Random", "Smooth Rnd" };
    inline const juce::StringArray lfoSyncNames  { "Free", "4 bars", "2 bars", "1 bar", "1/2", "1/4", "1/8", "1/16", "1/4T", "1/8T" };
    inline constexpr double lfoSyncBeats[] = { 16.0, 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 2.0 / 3.0, 1.0 / 3.0 };   // для индексов 1..9
    inline constexpr int numLfo = 4;

    // параметры, которые можно модулировать любым из LFO (в любом количестве)
    struct ModParam { const char* id; const char* name; };
    inline constexpr ModParam modParamTable[] =
    {
        { "swing", "Swing" }, { "shift", "Phase" }, { "timeAmount", "Time Amount" }, { "volDepth", "Vol Depth" }, { "volSmooth", "Vol Smooth" },
        { "slChance", "Slicer Chance" }, { "slReverse", "Slicer Reverse" }, { "slMute", "Slicer Mute" }, { "slMix", "Slicer Mix" },
        { "grainMix", "Grain Mix" }, { "grainSize", "Grain Size" }, { "grainSizeRand", "Grain Size Rand" }, { "grainDensity", "Grain Density" },
        { "grainPitch", "Grain Pitch" }, { "grainPitchRand", "Grain Pitch Rand" }, { "grainSpread", "Grain Spread" }, { "grainScan", "Grain Scan" },
        { "grainWidth", "Grain Width" }, { "grainRev", "Grain Reverse" },
        { "specAmount", "Spectral Amount" }, { "specTone", "Spectral Tone" }, { "specPitch", "Spectral Pitch" }, { "specMotion", "Spectral Motion" },
        { "specRate", "Spectral Rate" }, { "specMix", "Spectral Mix" },
        { "drive", "Drive" }, { "distBias", "Dist Bias" }, { "distTone", "Dist Tone" }, { "distMix", "Dist Mix" },
        { "cutoff", "Filter Cutoff" }, { "res", "Filter Resonance" }, { "filterDrive", "Filter Drive" }, { "filterMod", "Filter Mod" }, { "filterRate", "Filter LFO Rate" },
        { "ringFreq", "Ring Frequency" }, { "ringFM", "Ring FM Depth" }, { "ringFMRate", "Ring FM Rate" }, { "ringMix", "Ring Mix" },
        { "dlyFree", "Delay Time" }, { "dlyFeedback", "Delay Feedback" }, { "dlyTone", "Delay High Cut" }, { "dlyLow", "Delay Low Cut" },
        { "dlyMod", "Delay Mod" }, { "dlyModRate", "Delay Mod Rate" }, { "dlyDuck", "Delay Duck" }, { "dlyMix", "Delay Mix" },
        { "revRoom", "Reverb Room" }, { "revDamp", "Reverb Damping" }, { "revWidth", "Reverb Width" }, { "revPre", "Reverb Pre-delay" },
        { "revDuck", "Reverb Duck" }, { "revMix", "Reverb Mix" },
        { "mix", "Master Mix" }, { "out", "Master Output" },      // mix/out всегда в конце
    };
    inline constexpr int numModParams = (int) (sizeof (modParamTable) / sizeof (modParamTable[0]));

    inline int modIndex (const char* id)
    {
        for (int i = 0; i < numModParams; ++i)
            if (std::strcmp (modParamTable[i].id, id) == 0) return i;
        return -1;
    }

    // форма LFO: -1..1. ph = фаза 0..1, cyc = номер цикла (для случайных форм)
    inline float lfoShape (int shape, double ph, int64_t cyc, int salt)
    {
        const float p = (float) ph;
        auto rnd = [&] (int64_t c) { return hash01 ((int) (c * 31 + salt * 7)) * 2.0f - 1.0f; };
        switch (shape)
        {
            case 1:  return p < 0.5f ? 4.0f * p - 1.0f : 3.0f - 4.0f * p;           // Triangle
            case 2:  return 2.0f * p - 1.0f;                                         // Saw Up
            case 3:  return 1.0f - 2.0f * p;                                         // Saw Down
            case 4:  return p < 0.5f ? 1.0f : -1.0f;                                 // Square
            case 5:  return rnd (cyc);                                               // Random (S&H)
            case 6:  { const float a = rnd (cyc), b = rnd (cyc + 1); const float s = p * p * (3.0f - 2.0f * p); return a + (b - a) * s; }
            default: return std::sin (twoPi * p);                                    // Sine
        }
    }
}
