#pragma once
#include <algorithm>
#include <cmath>

// Дисторшн: Soft, Hard, Fold, Crush, Tube, Rectify, Sine + bias, DC-блокер и пост-тон.
struct DistState
{
    float crushHold = 0.0f;
    int   crushCount = 0;
    float dcX = 0.0f, dcY = 0.0f;
    float lp = 0.0f;
};

struct DistParams
{
    int   type   = 0;
    float gain   = 1.0f;     // предусиление
    float comp   = 1.0f;     // компенсация громкости
    float bias   = 0.0f;     // асимметрия
    float levels = 128.0f;   // уровни квантования (Crush)
    int   holdN  = 1;        // даунсэмплинг (Crush)
    bool  lpOn   = false;    // пост-фильтр tone
    float lpCoef = 1.0f;
};

inline float foldbackFn (float x)
{
    x = x * 0.25f + 0.25f;
    return 4.0f * std::abs (x - std::round (x)) - 1.0f;
}

inline float distProcess (const DistParams& d, float x, DistState& s)
{
    const float in = x * d.gain + d.bias;
    float y = x;

    switch (d.type)
    {
        case 1:  y = std::tanh (in) * d.comp; break;                                          // Soft
        case 2:  y = std::max (-1.0f, std::min (1.0f, in)) * d.comp; break;                   // Hard
        case 3:  y = foldbackFn (in) * d.comp; break;                                         // Fold
        case 4:                                                                               // Crush
            if (s.crushCount <= 0)
            {
                s.crushHold  = std::round ((x + d.bias * 0.1f) * d.levels) / d.levels;
                s.crushCount = d.holdN;
            }
            --s.crushCount;
            y = s.crushHold;
            break;
        case 5:  y = (in >= 0.0f ? std::tanh (in) : 0.7f * std::tanh (in * 1.3f)) * d.comp; break; // Tube
        case 6:  y = std::tanh (std::abs (in)) * d.comp; break;                               // Rectify
        case 7:  y = std::sin (in) * d.comp; break;                                           // Sine
        default: break;
    }

    if (d.type != 4 && (std::abs (d.bias) > 0.001f || d.type == 5 || d.type == 6))
    {
        const float dc = y - s.dcX + 0.9995f * s.dcY;   // блокировка постоянной составляющей
        s.dcX = y;  s.dcY = dc;  y = dc;
    }

    if (d.lpOn)
    {
        s.lp += d.lpCoef * (y - s.lp);
        y = s.lp;
    }
    return y;
}
