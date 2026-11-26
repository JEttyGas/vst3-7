#pragma once
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <array>
#include <cmath>

// Параметры спектрального модуля
struct SpecParams
{
    int   type   = 0;      // см. Patterns::specNames
    float amount = 0.7f;   // сила эффекта
    float tone   = 0.5f;   // характер (интервал, гласная, центр зеркала …)
    float pitch  = 7.0f;   // -24..24: сдвиг/растяжение/наклон
    float motion = 0.0f;   // глубина модуляции tone/pitch от LFO
    float mix    = 1.0f;   // смесь «исходный спектр / обработанный» внутри кадра
    float lfo    = 0.0f;   // фаза LFO 0..1 (обновляется процессором каждый сэмпл)
    float binHz  = 43.0f;  // ширина бина = sampleRate / N
    bool  hold   = false;  // для Freeze
};

// STFT одного канала: Hann 1024, перекрытие 75%. Латентность ровно N сэмплов в любом режиме.
class SpectralChannel
{
public:
    static constexpr int order = 10;
    static constexpr int N     = 1 << order;   // 1024
    static constexpr int half  = N / 2;
    static constexpr int hop   = N / 4;

    SpectralChannel()
    {
        for (int i = 0; i < N; ++i)
            window[(size_t) i] = 0.5f - 0.5f * std::cos (twoPi * (float) i / (float) N);
        reset();
    }

    void reset()
    {
        inRing.fill (0.0f);  outRing.fill (0.0f);
        frozenMag.fill (0.0f);  phaseAcc.fill (0.0f);  blurMag.fill (0.0f);
        susMag.fill (0.0f);  susPh.fill (0.0f);
        inPos = outPos = counter = 0;
    }

    float process (float x, const SpecParams& sp, juce::Random& rng)
    {
        inRing[(size_t) inPos] = x;
        inPos = (inPos + 1) & (N - 1);

        const float y = outRing[(size_t) outPos];
        outRing[(size_t) outPos] = 0.0f;
        outPos = (outPos + 1) & (N - 1);

        if (++counter >= hop)
        {
            counter = 0;
            processFrame (sp, rng);
        }
        return y;
    }

private:
    static constexpr float twoPi = 6.28318530718f;
    static constexpr float pi    = 3.14159265359f;

    void processFrame (const SpecParams& sp, juce::Random& rng)
    {
        float* d = data.data();

        for (int i = 0; i < N; ++i)
            d[i] = inRing[(size_t) ((inPos + i) & (N - 1))] * window[(size_t) i];
        for (int i = N; i < 2 * N; ++i)
            d[i] = 0.0f;

        fft.performRealOnlyForwardTransform (d, false);

        if (sp.type != 0)
            modify (sp, rng);

        d[1] = 0.0f;
        d[N + 1] = 0.0f;
        for (int k = 1; k < half; ++k)
        {
            d[2 * (N - k)]     =  d[2 * k];
            d[2 * (N - k) + 1] = -d[2 * k + 1];
        }

        fft.performRealOnlyInverseTransform (d);

        const float scale = 1.0f / 1.5f;
        for (int i = 0; i < N; ++i)
            outRing[(size_t) ((outPos + i) & (N - 1))] += d[i] * window[(size_t) i] * scale;
    }

    float srcRe (int k) const { return (k >= 0 && k <= half) ? oRe[(size_t) k] : 0.0f; }
    float srcIm (int k) const { return (k >= 0 && k <= half) ? oIm[(size_t) k] : 0.0f; }

    void modify (const SpecParams& sp, juce::Random& rng)
    {
        float* d = data.data();
        const float amt  = juce::jlimit (0.0f, 1.0f, sp.amount);
        const float mixA = juce::jlimit (0.0f, 1.0f, sp.mix);
        const float lfoS = std::sin (twoPi * sp.lfo);
        const float tone  = juce::jlimit (0.0f, 1.0f, sp.tone + sp.motion * 0.45f * lfoS);
        const float pitch = sp.pitch + sp.motion * 12.0f * lfoS;

        for (int k = 0; k <= half; ++k)
        {
            oRe[(size_t) k] = d[2 * k];
            oIm[(size_t) k] = d[2 * k + 1];
        }

        auto mag = [this] (int k) { return std::hypot (oRe[(size_t) k], oIm[(size_t) k]); };

        switch (sp.type)
        {
            case 1: // Freeze
            {
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    if (! sp.hold)
                    {
                        frozenMag[ku] = mag (k);
                        phaseAcc[ku]  = std::atan2 (oIm[ku], oRe[ku]);
                        nRe[ku] = oRe[ku];  nIm[ku] = oIm[ku];
                    }
                    else
                    {
                        float ph = phaseAcc[ku] + (float) k * (pi * 0.5f)
                                   + tone * (rng.nextFloat() * 2.0f - 1.0f) * pi;
                        ph -= twoPi * std::round (ph / twoPi);
                        phaseAcc[ku] = ph;
                        const float m = frozenMag[ku] * 1.5f;
                        nRe[ku] = m * std::cos (ph);
                        nIm[ku] = m * std::sin (ph);
                    }
                }
                break;
            }
            case 2: // Blur
            {
                const float a = 0.5f + 0.49f * amt;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float m = mag (k);
                    blurMag[ku] = a * blurMag[ku] + (1.0f - a) * m;
                    if (m > 1.0e-9f) { const float s = blurMag[ku] / m; nRe[ku] = oRe[ku] * s; nIm[ku] = oIm[ku] * s; }
                    else             { nRe[ku] = blurMag[ku]; nIm[ku] = 0.0f; }
                }
                break;
            }
            case 3: // Robot
            {
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    nRe[ku] = oRe[ku] + amt * (mag (k) - oRe[ku]);
                    nIm[ku] = oIm[ku] * (1.0f - amt);
                }
                break;
            }
            case 4: // Whisper
            {
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float m  = mag (k);
                    const float ph = (rng.nextFloat() * 2.0f - 1.0f) * pi;
                    nRe[ku] = oRe[ku] + amt * (m * std::cos (ph) - oRe[ku]);
                    nIm[ku] = oIm[ku] + amt * (m * std::sin (ph) - oIm[ku]);
                }
                break;
            }
            case 5: // Gate
            {
                float maxMag = 0.0f;
                for (int k = 0; k <= half; ++k) maxMag = std::max (maxMag, mag (k));
                const float thr = amt * 0.35f * maxMag;
                const bool  inv = tone > 0.5f;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const bool loud = mag (k) >= thr;
                    const float g = inv ? ((loud && amt > 0.0f) ? 0.0f : 1.0f) : (loud ? 1.0f : 0.0f);
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 6: // Shift (частотный сдвиг в бинах)
            {
                const int s = (int) std::round (pitch * 2.0f);
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    nRe[ku] = oRe[ku] + amt * (srcRe (k - s) - oRe[ku]);
                    nIm[ku] = oIm[ku] + amt * (srcIm (k - s) - oIm[ku]);
                }
                break;
            }
            case 7: // Comb
            {
                const float spacing = 2.0f + tone * 30.0f;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float g = (1.0f - amt) + amt * (0.5f + 0.5f * std::cos (twoPi * (float) k / spacing));
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 8: // Warp (растяжение спектра, pitch в полутонах)
            {
                const float ratio = std::pow (2.0f, pitch / 12.0f);
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const int src = (int) std::round ((float) k / ratio);
                    nRe[ku] = oRe[ku] + amt * (srcRe (src) - oRe[ku]);
                    nIm[ku] = oIm[ku] + amt * (srcIm (src) - oIm[ku]);
                }
                break;
            }
            case 9: // Sustain: каждый бин «держится» и затухает (спектральный хвост)
            {
                const float decay = 0.80f + 0.195f * amt;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float dk  = std::pow (decay, 1.0f + tone * 3.0f * (float) k / (float) half);
                    const float hld = susMag[ku] * dk;
                    const float m   = mag (k);
                    if (m >= hld)
                    {
                        susMag[ku] = m;
                        susPh[ku]  = std::atan2 (oIm[ku], oRe[ku]);
                        nRe[ku] = oRe[ku];  nIm[ku] = oIm[ku];
                    }
                    else
                    {
                        susMag[ku] = hld;
                        float ph = susPh[ku] + (float) k * (pi * 0.5f);
                        ph -= twoPi * std::round (ph / twoPi);
                        susPh[ku] = ph;
                        const float tail = (hld - m) * 1.5f;
                        nRe[ku] = oRe[ku] + tail * std::cos (ph);
                        nIm[ku] = oIm[ku] + tail * std::sin (ph);
                    }
                }
                break;
            }
            case 10: // Crush: квантование амплитуд и фаз
            {
                float maxMag = 0.0f;
                for (int k = 0; k <= half; ++k) maxMag = std::max (maxMag, mag (k));
                const float step  = std::max (1.0e-9f, maxMag * (0.01f + 0.4f * amt));
                const float steps = 3.0f + (1.0f - tone) * 29.0f;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float m  = std::round (mag (k) / step) * step;
                    float ph = std::atan2 (oIm[ku], oRe[ku]);
                    ph = std::round (ph * steps / twoPi) * twoPi / steps;
                    nRe[ku] = m * std::cos (ph);
                    nIm[ku] = m * std::sin (ph);
                }
                break;
            }
            case 11: // Octaver: слои на октаву вниз (tone<0.5) и вверх (tone>0.5)
            {
                const float gDn = 1.0f - tone, gUp = tone;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float dnR = srcRe (2 * k), dnI = srcIm (2 * k);
                    const int kk = k >> 1;
                    float upR, upI;
                    if ((k & 1) == 0) { upR = srcRe (kk); upI = srcIm (kk); }
                    else              { upR = 0.5f * (srcRe (kk) + srcRe (kk + 1)); upI = 0.5f * (srcIm (kk) + srcIm (kk + 1)); }
                    nRe[ku] = oRe[ku] + amt * 1.4f * (gDn * dnR + gUp * upR);
                    nIm[ku] = oIm[ku] + amt * 1.4f * (gDn * dnI + gUp * upI);
                }
                break;
            }
            case 12: // Tilt: сильный наклон спектра (pitch = ярче/темнее)
            {
                const float tiltOct = (pitch / 24.0f) * amt * 4.0f;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float g = std::pow (2.0f, tiltOct * (2.0f * (float) k / (float) half - 1.0f));
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 13: // Sweep: движущаяся гребёнка (скорость задаёт Rate)
            {
                const float spacing = 2.0f + tone * 30.0f;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float g = (1.0f - amt) + amt * (0.5f + 0.5f * std::cos (twoPi * ((float) k / spacing + sp.lfo)));
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 14: // Vowel: формантный фильтр A-E-I-O-U
            {
                static const float F[5][3] = { { 700, 1220, 2600 }, { 530, 1840, 2480 }, { 270, 2290, 3010 },
                                               { 570,  840, 2410 }, { 300,  870, 2240 } };
                const float v  = tone * 4.0f;
                const int   i0 = std::min (3, (int) v);
                const float fr = v - (float) i0;
                float fm[3];
                for (int j = 0; j < 3; ++j) fm[j] = F[i0][j] + fr * (F[i0 + 1][j] - F[i0][j]);

                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const float f = (float) k * sp.binHz;
                    float acc = 0.0f;
                    for (int j = 0; j < 3; ++j)
                    {
                        const float bw = 60.0f + 0.12f * fm[j];
                        const float z  = (f - fm[j]) / bw;
                        acc += std::exp (-0.5f * z * z) * (j == 0 ? 1.0f : (j == 1 ? 0.8f : 0.5f));
                    }
                    const float g = (1.0f - amt) + amt * (0.06f + 3.0f * acc);
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 15: // Shatter: случайная маска бинов
            {
                const int grp = 1 + (int) (tone * 15.0f);
                bool keep = true;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    if (k % grp == 0) keep = rng.nextFloat() >= amt * 0.95f;
                    const float g = keep ? 1.0f : 0.0f;
                    nRe[ku] = oRe[ku] * g;  nIm[ku] = oIm[ku] * g;
                }
                break;
            }
            case 16: // Mirror: отражение спектра (tone = центр)
            {
                const float c = tone * (float) half;
                for (int k = 0; k <= half; ++k)
                {
                    const auto ku = (size_t) k;
                    const int src = (int) std::round (2.0f * c - (float) k);
                    nRe[ku] = oRe[ku] + amt * (srcRe (src) - oRe[ku]);
                    nIm[ku] = oIm[ku] + amt * (srcIm (src) - oIm[ku]);
                }
                break;
            }
            default: return;
        }

        for (int k = 0; k <= half; ++k)
        {
            const auto ku = (size_t) k;
            d[2 * k]     = oRe[ku] + mixA * (nRe[ku] - oRe[ku]);
            d[2 * k + 1] = oIm[ku] + mixA * (nIm[ku] - oIm[ku]);
        }
    }

    juce::dsp::FFT fft { order };

    std::array<float, N>        window {};
    std::array<float, N>        inRing {}, outRing {};
    std::array<float, 2 * N>    data {};
    std::array<float, half + 1> oRe {}, oIm {}, nRe {}, nIm {};
    std::array<float, half + 1> frozenMag {}, phaseAcc {}, blurMag {}, susMag {}, susPh {};

    int inPos = 0, outPos = 0, counter = 0;
};
