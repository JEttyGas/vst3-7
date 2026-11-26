#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

// Стерео-дилей: Stereo / Ping-Pong / Tape, фильтры в обратной связи, модуляция, ducking.
class StereoDelay
{
public:
    struct Params
    {
        float timeSamples = 1000.0f;
        float feedback    = 0.4f;
        float lpHz        = 8000.0f;   // верхний срез в фидбеке
        float hpHz        = 20.0f;     // нижний срез в фидбеке
        float modDepth    = 0.0f;      // в сэмплах
        float modRate     = 0.5f;      // Гц
        float duck        = 0.0f;      // 0..1
        int   mode        = 0;         // 0 Stereo, 1 Ping-Pong, 2 Tape
    };

    void prepare (double sr)
    {
        sampleRate = sr;
        size = (int) (sr * 4.5) + 16;
        for (auto& b : buf) b.assign ((size_t) size, 0.0f);
        reset();
    }

    void reset()
    {
        for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
        pos = 0; lfo = 0.0;
        for (int c = 0; c < 2; ++c) { lp[c] = 0.0f; hp[c] = 0.0f; }
    }

    int maxDelaySamples() const { return size - 16; }

    // inL/inR: вход. outL/outR: только «эхо» (без сухого сигнала). env: огибающая входа 0..1
    void process (float inL, float inR, bool stereo, const Params& p, float env, float& outL, float& outR)
    {
        const float twoPi = 6.28318530718f;
        lfo += (double) p.modRate / sampleRate;
        if (lfo >= 1.0) lfo -= 1.0;

        const float tBase = std::min (std::max (p.timeSamples, 2.0f + p.modDepth), (float) (size - 8) - p.modDepth);
        const float mL = std::sin (twoPi * (float) lfo) * p.modDepth;
        const float mR = std::sin (twoPi * ((float) lfo + 0.25f)) * p.modDepth;

        const float rl = readInterp (0, tBase + mL);
        const float rr = stereo ? readInterp (1, tBase + mR) : 0.0f;

        const float lpC = 1.0f - std::exp (-twoPi * std::min (p.lpHz, 20000.0f) / (float) sampleRate);
        const float hpC = 1.0f - std::exp (-twoPi * std::max (p.hpHz, 10.0f)    / (float) sampleRate);

        float fl = rl, fr = rr;
        lp[0] += lpC * (fl - lp[0]);  fl = lp[0];  hp[0] += hpC * (fl - hp[0]);  fl -= hp[0];
        lp[1] += lpC * (fr - lp[1]);  fr = lp[1];  hp[1] += hpC * (fr - hp[1]);  fr -= hp[1];

        if (p.mode == 2)   // Tape: мягкая сатурация в обратной связи
        {
            fl = std::tanh (fl * 1.6f) / 1.6f;
            fr = std::tanh (fr * 1.6f) / 1.6f;
        }

        float wl, wr;
        if (stereo && p.mode == 1)   // Ping-Pong
        {
            const float m = 0.5f * (inL + inR);
            wl = m + p.feedback * fr;
            wr = p.feedback * fl;
        }
        else
        {
            wl = inL + p.feedback * fl;
            wr = inR + p.feedback * fr;
        }

        buf[0][(size_t) pos] = wl;
        if (stereo) buf[1][(size_t) pos] = wr;
        if (++pos >= size) pos = 0;

        const float g = 1.0f - p.duck * env;
        outL = rl * g;
        outR = rr * g;
    }

private:
    float readInterp (int ch, float delay) const
    {
        float rp = (float) pos - delay;
        while (rp < 0.0f) rp += (float) size;
        const int   i0 = (int) rp;
        const float fr = rp - (float) i0;
        const int   i1 = (i0 + 1 >= size) ? 0 : i0 + 1;
        const auto& b = buf[(size_t) ch];
        return b[(size_t) i0] + fr * (b[(size_t) i1] - b[(size_t) i0]);
    }

    std::vector<float> buf[2];
    double sampleRate = 44100.0;
    int    size = 1, pos = 0;
    double lfo = 0.0;
    float  lp[2] { 0.0f, 0.0f }, hp[2] { 0.0f, 0.0f };
};
