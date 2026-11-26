#include "PluginEditor.h"
#include "Patterns.h"
#include "Presets.h"
#include "Version.h"

using namespace juce;

//==============================================================================
//  Таблицы интерфейса
//==============================================================================
namespace
{
    struct PageInfo { const char* tab; const char* title; const char* desc; uint32 colour; };

    const PageInfo pageInfo[] =
    {
        { "Rhythm",   "RHYTHM  /  PATTERN ENGINE",  "TIME re-reads audio like tape, VOLUME gates and pumps it. Slices = repeats per pattern. Swing and Phase shift the groove.", 0xffffb347 },
        { "Slicer",   "SLICER  /  BEAT SHUFFLER",   "Cuts the pattern into steps and plays them in a new order. Plays the previous pattern, so it lags one cycle.", 0xffff6b8f },
        { "Granular", "GRANULAR  /  GRAIN CLOUD",   "Scatters tiny grains of sound. Grain Mix = 0 is off. Freeze holds the last 4 seconds and plays grains from it.", 0xff6dff9c },
        { "Spectral", "SPECTRAL  /  FFT LAB",       "Freeze, sustain, vowels, shatter, octaver and more. Motion wobbles Tone and Pitch with the Rate LFO.", 0xffc29bff },
        { "Dist",     "DISTORTION  /  SATURATOR",   "Seven waveshapers plus a bit crusher. Bias adds even harmonics, Tone is a post low-pass.", 0xffff7a6b },
        { "Filter",   "FILTER  /  12-24 DB",        "Resonant filter. The cutoff can follow the Volume pattern, a free LFO or the input envelope (auto-wah).", 0xff59d0ff },
        { "Ring",     "RING MOD  /  CARRIER",       "Multiplies the signal by a carrier wave: metal, bells, robots. FM wobbles the carrier. Mix = 0 is off.", 0xffffdc5e },
        { "Delay",    "DELAY  /  TAPE ECHO",        "Synced stereo delay: ping-pong, tape, filtered feedback, modulation and ducking. Mix = 0 is off.", 0xff7aa9ff },
        { "Reverb",   "REVERB  /  SPACE",           "Algorithmic reverb with pre-delay, freeze and ducking. Mix = 0 is off.", 0xff5ef2d6 },
    };
    constexpr int numPages = (int) (sizeof (pageInfo) / sizeof (pageInfo[0]));
    constexpr int lfoPageIndex = numPages;

    // kind: K = ручка, C = выпадающий список, T = кнопка-переключатель
    struct CtrlSpec { int page; const char* section; char kind; const char* id; const char* label; };

    const CtrlSpec specs[] =
    {
        // 0 Rhythm
        { 0, "Pattern", 'C', "length",     "Length" },
        { 0, "Pattern", 'K', "swing",      "Swing" },
        { 0, "Pattern", 'K', "shift",      "Phase" },
        { 0, "Time",    'C', "timeType",   "Type" },
        { 0, "Time",    'C', "timeDiv",    "Slices" },
        { 0, "Time",    'K', "timeAmount", "Amount" },
        { 0, "Volume",  'C', "volType",    "Type" },
        { 0, "Volume",  'C', "volDiv",     "Slices" },
        { 0, "Volume",  'K', "volDepth",   "Depth" },
        { 0, "Volume",  'K', "volSmooth",  "Smooth" },
        // 1 Slicer
        { 1, "Sequence",  'C', "slType",    "Order" },
        { 1, "Sequence",  'C', "slSteps",   "Steps" },
        { 1, "Sequence",  'C', "slLength",  "Length" },
        { 1, "Sequence",  'K', "slMix",     "Mix" },
        { 1, "Variation", 'K', "slSeed",    "Seed" },
        { 1, "Variation", 'T', "slEvolve",  "Evolve" },
        { 1, "Variation", 'K', "slChance",  "Chance" },
        { 1, "Variation", 'K', "slReverse", "Reverse" },
        { 1, "Variation", 'K', "slMute",    "Mute" },
        // 2 Granular
        { 2, "Source",   'T', "grainFreeze",   "Freeze" },
        { 2, "Source",   'C', "grainWindow",   "Window" },
        { 2, "Grains",   'K', "grainMix",      "Mix" },
        { 2, "Grains",   'K', "grainSize",     "Size" },
        { 2, "Grains",   'K', "grainSizeRand", "Size rand" },
        { 2, "Grains",   'K', "grainDensity",  "Density /s" },
        { 2, "Pitch",    'K', "grainPitch",    "Pitch" },
        { 2, "Pitch",    'K', "grainPitchRand","Pitch rand" },
        { 2, "Pitch",    'C', "grainQuant",    "Quantize" },
        { 2, "Position", 'K', "grainSpread",   "Spread" },
        { 2, "Position", 'K', "grainScan",     "Scan" },
        { 2, "Position", 'K', "grainWidth",    "Width" },
        { 2, "Position", 'K', "grainRev",      "Reverse" },
        // 3 Spectral
        { 3, "Effect", 'C', "specType",   "Type" },
        { 3, "Effect", 'T', "specHold",   "Hold" },
        { 3, "Shape",  'K', "specAmount", "Amount" },
        { 3, "Shape",  'K', "specTone",   "Tone" },
        { 3, "Shape",  'K', "specPitch",  "Pitch" },
        { 3, "Motion", 'K', "specMotion", "Motion" },
        { 3, "Motion", 'K', "specRate",   "Rate" },
        { 3, "Output", 'K', "specMix",    "Mix" },
        // 4 Distortion
        { 4, "Shaper",  'C', "distType", "Type" },
        { 4, "Shaper",  'K', "drive",    "Drive" },
        { 4, "Shaper",  'K', "distBias", "Bias" },
        { 4, "Crusher", 'K', "distBits", "Bits" },
        { 4, "Crusher", 'K', "distRate", "Downsample" },
        { 4, "Output",  'K', "distTone", "Tone" },
        { 4, "Output",  'K', "distMix",  "Mix" },
        // 5 Filter
        { 5, "Filter",     'C', "filterType",  "Type" },
        { 5, "Filter",     'C', "filterSlope", "Slope" },
        { 5, "Filter",     'K', "cutoff",      "Cutoff" },
        { 5, "Filter",     'K', "res",         "Resonance" },
        { 5, "Filter",     'K', "filterDrive", "Drive" },
        { 5, "Modulation", 'C', "filterSrc",   "Source" },
        { 5, "Modulation", 'K', "filterMod",   "Amount" },
        { 5, "Modulation", 'K', "filterRate",  "LFO rate" },
        // 6 Ring mod
        { 6, "Carrier", 'C', "ringShape",  "Shape" },
        { 6, "Carrier", 'C', "ringMode",   "Mode" },
        { 6, "Carrier", 'K', "ringFreq",   "Frequency" },
        { 6, "FM",      'K', "ringFM",     "FM depth" },
        { 6, "FM",      'K', "ringFMRate", "FM rate" },
        { 6, "Output",  'K', "ringMix",    "Mix" },
        // 7 Delay
        { 7, "Time",       'C', "dlySync",     "Sync" },
        { 7, "Time",       'K', "dlyFree",     "Free time" },
        { 7, "Time",       'C', "dlyMode",     "Mode" },
        { 7, "Feedback",   'K', "dlyFeedback", "Feedback" },
        { 7, "Feedback",   'K', "dlyTone",     "High cut" },
        { 7, "Feedback",   'K', "dlyLow",      "Low cut" },
        { 7, "Modulation", 'K', "dlyMod",      "Depth" },
        { 7, "Modulation", 'K', "dlyModRate",  "Rate" },
        { 7, "Output",     'K', "dlyDuck",     "Duck" },
        { 7, "Output",     'K', "dlyMix",      "Mix" },
        // 8 Reverb
        { 8, "Space",  'K', "revRoom",   "Room" },
        { 8, "Space",  'K', "revDamp",   "Damping" },
        { 8, "Space",  'K', "revWidth",  "Width" },
        { 8, "Pre",    'K', "revPre",    "Pre-delay" },
        { 8, "Pre",    'T', "revFreeze", "Freeze" },
        { 8, "Output", 'K', "revDuck",   "Duck" },
        { 8, "Output", 'K', "revMix",    "Mix" },
    };

    const char* specHints[17] =
    {
        "Choose an effect.",
        "Freeze: Hold captures the spectrum. Tone = diffusion.",
        "Blur: Amount = smear length.",
        "Robot: zero-phase buzz.",
        "Whisper: randomised phase.",
        "Gate: Tone <50% keeps loud partials, >50% keeps the quiet residue.",
        "Shift: Pitch = frequency shift (inharmonic).",
        "Comb: Tone = spacing, Amount = depth.",
        "Warp: Pitch = spectral pitch ratio.",
        "Sustain: partials ring on. Amount = decay, Tone = darker tail.",
        "Crush: Amount = amplitude steps, Tone = phase detail.",
        "Octaver: Tone = down (left) / up (right).",
        "Tilt: Pitch = brighter (+) or darker (-).",
        "Sweep: moving comb. Rate = speed.",
        "Vowel: Tone morphs A-E-I-O-U. Try Motion.",
        "Shatter: random bin mask. Tone = cluster size.",
        "Mirror: flips the spectrum. Tone = centre.",
    };
}

//==============================================================================
//  LCD-кривые Time / Volume / Slicer
//==============================================================================
void CurveView::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (5.0f);
    Hw::lcdWindow (g, r, 6.0f);

    auto val = [this] (const char* id) { return proc.apvts.getRawParameterValue (id)->load(); };
    const Colour acc = kind == 0 ? Theme::lcdOn : kind == 1 ? Theme::amber : Colour (0xff72e6ff);
    const auto plot = r.reduced (12.0f, 10.0f).withTrimmedTop (14.0f);

    String title;
    if (kind == 0)      title = "TIME  " + Patterns::timeNames[(int) val ("timeType")] + " x" + String (Patterns::sliceValues[(int) val ("timeDiv")]);
    else if (kind == 1) title = "VOLUME  " + Patterns::volNames[(int) val ("volType")] + " x" + String (Patterns::sliceValues[(int) val ("volDiv")]);
    else                title = "SLICER  " + Patterns::slicerNames[(int) val ("slType")] + " x" + String (Patterns::stepValues[(int) val ("slSteps")]);
    g.setColour (acc.withAlpha (0.9f));
    g.setFont (Theme::mono (11.5f, true));
    g.drawText (title.toUpperCase(), r.reduced (12.0f, 6.0f).withHeight (14.0f), Justification::centredLeft);

    if (kind == 2)
    {
        const int type = (int) val ("slType");
        const int N    = Patterns::stepValues[(int) val ("slSteps")];
        int perm[32];
        Patterns::buildPermutation (type, N, (uint32_t) (int) val ("slSeed"), perm);

        const float bw = plot.getWidth() / (float) N;
        const float rowH = plot.getHeight() * 0.36f;
        const float yTop = plot.getY(), yBot = plot.getBottom() - rowH;

        for (int k = 0; k < N; ++k)
        {
            const float x = plot.getX() + (float) k * bw;
            const Rectangle<float> a (x + 1.0f, yTop, bw - 2.0f, rowH), b (x + 1.0f, yBot, bw - 2.0f, rowH);
            const int src = type == 0 ? k : perm[k];

            g.setColour (acc.withAlpha (0.18f));
            g.fillRect (a);
            g.setColour (acc.withAlpha (type == 0 ? 0.22f : 0.25f + 0.70f * (float) src / (float) jmax (1, N - 1)));
            g.fillRect (b);

            if (bw > 15.0f)
            {
                g.setFont (Theme::mono (10.5f, true));
                g.setColour (Theme::lcdBg);
                g.drawText (String (src + 1), b, Justification::centred);
                g.setColour (acc.withAlpha (0.6f));
                g.drawText (String (k + 1), a, Justification::centred);
            }
        }

        g.setColour (acc.withAlpha (0.5f));
        g.setFont (Theme::mono (9.5f));
        g.drawText ("SRC", Rectangle<float> (plot.getX(), yTop + rowH, 60.0f, 11.0f), Justification::centredLeft);
        g.drawText ("OUT", Rectangle<float> (plot.getX(), yBot - 11.0f, 60.0f, 11.0f), Justification::centredLeft);

        const float px = plot.getX() + proc.displaySlPhase.load() * plot.getWidth();
        g.setColour (Colours::white.withAlpha (0.85f));
        g.drawVerticalLine ((int) px, plot.getY() - 2.0f, plot.getBottom() + 2.0f);
        return;
    }

    const bool time = kind == 0;
    const int type = (int) val (time ? "timeType" : "volType");
    const int div  = Patterns::sliceValues[(int) val (time ? "timeDiv" : "volDiv")];

    const float lo = time ? -0.5f : 0.0f, hi = 1.0f;
    auto toY = [&] (float v) { return plot.getBottom() - (v - lo) / (hi - lo) * plot.getHeight(); };
    auto curve = [&] (float p) { return time ? Patterns::timeCurve (type, p, div) : Patterns::volumeCurve (type, p, div); };

    Graphics::ScopedSaveState s (g);
    g.reduceClipRegion (r.toNearestInt());

    g.setColour (acc.withAlpha (0.12f));
    for (int k = 1; k < div; ++k)
        g.drawVerticalLine ((int) (plot.getX() + plot.getWidth() * (float) k / (float) div), plot.getY(), plot.getBottom());
    g.drawHorizontalLine ((int) toY (0.0f), plot.getX(), plot.getRight());

    Path path, area;
    const int steps = 360;
    for (int i = 0; i <= steps; ++i)
    {
        const float p = (float) i / (float) steps * 0.9999f;
        const float x = plot.getX() + p * plot.getWidth();
        const float y = toY (curve (p));
        if (i == 0) { path.startNewSubPath (x, y); area.startNewSubPath (x, toY (lo)); area.lineTo (x, y); }
        else        { path.lineTo (x, y); area.lineTo (x, y); }
    }
    area.lineTo (plot.getRight(), toY (lo));
    area.closeSubPath();

    g.setGradientFill (ColourGradient (acc.withAlpha (0.30f), 0.0f, plot.getY(), acc.withAlpha (0.0f), 0.0f, plot.getBottom(), false));
    g.fillPath (area);
    g.setColour (acc.withAlpha (0.22f));
    g.strokePath (path, PathStrokeType (6.0f, PathStrokeType::curved, PathStrokeType::rounded));
    g.setColour (acc);
    g.strokePath (path, PathStrokeType (2.0f, PathStrokeType::curved, PathStrokeType::rounded));

    const float ph = proc.displayPhase.load();
    const float px = plot.getX() + ph * plot.getWidth();
    g.setColour (Colours::white.withAlpha (0.5f));
    g.drawVerticalLine ((int) px, plot.getY(), plot.getBottom());
    g.setColour (Colours::white);
    g.fillEllipse (px - 3.5f, toY (curve (ph)) - 3.5f, 7.0f, 7.0f);
}

//==============================================================================
//  Анализатор / осциллограф
//==============================================================================
AnalyzerView::AnalyzerView (BeatMorphProcessor& p) : proc (p)
{
    for (int i = 0; i < 1024; ++i)
        window[(size_t) i] = 0.5f - 0.5f * std::cos (6.28318530718f * (float) i / 1024.0f);
}

void AnalyzerView::tick()
{
    const int pos = proc.scopePos.load (std::memory_order_relaxed);
    for (int i = 0; i < 1024; ++i)
        block[(size_t) i] = proc.scope[(size_t) ((pos - 1024 + i) & (BeatMorphProcessor::scopeSize - 1))].load (std::memory_order_relaxed);

    int trig = 0;
    for (int i = 1; i < 500; ++i)
        if (block[(size_t) i - 1] < 0.0f && block[(size_t) i] >= 0.0f) { trig = i; break; }
    for (int i = 0; i < 512; ++i)
        wave[(size_t) i] = block[(size_t) (trig + i)];

    for (int i = 0; i < 1024; ++i)
        fftData[(size_t) i] = block[(size_t) i] * window[(size_t) i];
    for (int i = 1024; i < 2048; ++i)
        fftData[(size_t) i] = 0.0f;
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    const double sr = proc.getSampleRate() > 0.0 ? proc.getSampleRate() : 44100.0;
    const float binHz = (float) (sr / 1024.0);
    for (int b = 0; b < 64; ++b)
    {
        const float f0 = 30.0f * std::pow (600.0f, (float) b / 64.0f);
        const float f1 = 30.0f * std::pow (600.0f, (float) (b + 1) / 64.0f);
        const int k0 = jmax (1, (int) (f0 / binHz));
        const int k1 = jlimit (k0 + 1, 511, (int) (f1 / binHz));
        float m = 0.0f;
        for (int k = k0; k < k1; ++k) m = jmax (m, fftData[(size_t) k]);
        const float db = 20.0f * std::log10 (m / 256.0f + 1.0e-9f);
        const float v = jlimit (0.0f, 1.0f, (db + 80.0f) / 80.0f);
        bars[(size_t) b]  = v > bars[(size_t) b] ? v : bars[(size_t) b] * 0.86f;
        peaks[(size_t) b] = v > peaks[(size_t) b] ? v : jmax (0.0f, peaks[(size_t) b] - 0.012f);
    }
    repaint();
}

void AnalyzerView::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (4.0f);
    Hw::lcdWindow (g, r, 6.0f);
    const auto plot = r.reduced (10.0f, 8.0f).withTrimmedTop (12.0f);

    g.setColour (Theme::lcdOn.withAlpha (0.55f));
    g.setFont (Theme::mono (10.5f, true));
    g.drawText (mode == 0 ? "SCOPE" : "SPECTRUM", r.reduced (10.0f, 4.0f).withHeight (12.0f), Justification::centredLeft);
    g.setColour (Theme::lcdDim);
    g.drawText ("CLICK TO SWITCH", r.reduced (10.0f, 4.0f).withHeight (12.0f), Justification::centredRight);

    Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (plot.toNearestInt());

    if (mode == 1)
    {
        const float bw = plot.getWidth() / 64.0f;
        ColourGradient grad (Theme::red, 0.0f, plot.getY(), Theme::green, 0.0f, plot.getBottom(), false);
        grad.addColour (0.30, Theme::amber);
        g.setGradientFill (grad);
        for (int b = 0; b < 64; ++b)
        {
            const float h = bars[(size_t) b] * plot.getHeight();
            g.fillRect (plot.getX() + (float) b * bw + 0.8f, plot.getBottom() - h, bw - 1.6f, h);
        }
        g.setColour (Theme::silk.withAlpha (0.85f));
        for (int b = 0; b < 64; ++b)
            g.fillRect (plot.getX() + (float) b * bw + 0.8f, plot.getBottom() - peaks[(size_t) b] * plot.getHeight() - 2.0f, bw - 1.6f, 1.6f);

        g.setColour (Theme::lcdBg);
        for (float y = plot.getY(); y < plot.getBottom(); y += 3.5f)
            g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
    }
    else
    {
        g.setColour (Theme::lcdOn.withAlpha (0.15f));
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

        Path p;
        for (int i = 0; i < 512; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / 511.0f;
            const float y = plot.getCentreY() - jlimit (-1.0f, 1.0f, wave[(size_t) i] * 1.6f) * plot.getHeight() * 0.5f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.setColour (Theme::lcdOn.withAlpha (0.25f));
        g.strokePath (p, PathStrokeType (5.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (Theme::lcdOn);
        g.strokePath (p, PathStrokeType (1.6f, PathStrokeType::curved, PathStrokeType::rounded));
    }
}

//==============================================================================
void StatusView::paint (Graphics& g)
{
    const int n = (int) items.size();
    const int rows = (n + 1) / 2;
    const float cw = (float) getWidth() * 0.5f, rh = (float) getHeight() / (float) jmax (1, rows);

    for (int i = 0; i < n; ++i)
    {
        const auto& it = items[(size_t) i];
        const float x = (float) (i / rows) * cw, y = (float) (i % rows) * rh;
        const bool on = it.on && it.on();

        const Rectangle<float> led (x + 6.0f, y + rh * 0.5f - 4.0f, 14.0f, 8.0f);
        g.setColour (Colour (0xff8d9097));
        g.fillRoundedRectangle (led.expanded (2.0f), 3.5f);
        g.setColour (Colour (0xff0b0c0e));
        g.fillRoundedRectangle (led.expanded (0.8f), 3.0f);
        if (on)
        {
            g.setColour (it.colour.withAlpha (0.35f));
            g.fillRoundedRectangle (led.expanded (4.0f), 5.0f);
            g.setColour (it.colour.brighter (0.5f));
        }
        else g.setColour (it.colour.darker (0.85f));
        g.fillRoundedRectangle (led, 2.5f);

        g.setFont (Theme::font (10.5f, true));
        g.setColour (on ? Theme::silk : Theme::silkDim);
        g.drawText (it.label.toUpperCase(), Rectangle<float> (x + 26.0f, y, cw - 28.0f, rh), Justification::centredLeft);
    }
}

//==============================================================================
void TransportView::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (5.0f);
    Hw::lcdWindow (g, r, 6.0f);
    const auto in = r.reduced (12.0f, 8.0f);
    const Colour on = Theme::red;

    auto digits = [&] (Rectangle<float> row, const String& s)
    {
        float x = row.getX();
        const float cw = 17.0f, gap = 5.0f;
        for (auto ch : s)
        {
            if (ch == '.')
            {
                g.setColour (on);
                g.fillEllipse (x - gap * 0.5f - 2.0f, row.getBottom() - 4.0f, 4.0f, 4.0f);
                continue;
            }
            Hw::sevenSeg (g, { x, row.getY(), cw, row.getHeight() }, (char) ch, on);
            x += cw + gap;
        }
    };

    g.setFont (Theme::mono (10.0f, true));
    g.setColour (on.withAlpha (0.65f));
    g.drawText ("TEMPO BPM", in.withHeight (11.0f), Justification::centredLeft);
    digits (Rectangle<float> (in.getX(), in.getY() + 14.0f, in.getWidth(), 30.0f), String (bpm, 1).paddedLeft (' ', 5));

    g.drawText ("BAR.BEAT.16TH", Rectangle<float> (in.getX(), in.getY() + 49.0f, in.getWidth(), 11.0f), Justification::centredLeft);
    const double pp = jmax (0.0, ppq);
    const int bar  = (int) std::floor (pp / 4.0) + 1;
    const int beat = (int) std::floor (pp) % 4 + 1;
    const int six  = (int) std::floor ((pp - std::floor (pp)) * 4.0) + 1;
    digits (Rectangle<float> (in.getX(), in.getY() + 63.0f, in.getWidth(), 30.0f),
            String (bar).paddedLeft ('0', 3) + "." + String (beat) + "." + String (six));

    g.setColour (playing ? Theme::green : Theme::lcdDim);
    g.setFont (Theme::mono (11.0f, true));
    g.drawText (playing ? "> PLAY" : "# STOP", Rectangle<float> (in.getX(), in.getBottom() - 12.0f, in.getWidth(), 12.0f), Justification::centredLeft);
}

//==============================================================================
void PresetLcd::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (4.0f);
    Hw::lcdWindow (g, r, 6.0f);
    const auto in = r.reduced (12.0f, 5.0f);

    g.setFont (Theme::mono (10.5f, true));
    g.setColour (Theme::lcdOn.withAlpha (0.55f));
    g.drawText (line1, in.withHeight (12.0f), Justification::centredLeft, true);

    g.setFont (Theme::mono (19.0f, true));
    g.setColour (Theme::lcdOn.withAlpha (0.20f));
    g.drawText (name, in.withTrimmedTop (14.0f).expanded (1.0f, 0.0f), Justification::centredLeft, true);
    g.setColour (Theme::lcdOn);
    g.drawText (name, in.withTrimmedTop (14.0f), Justification::centredLeft, true);

    g.setColour (Theme::lcdDim);
    g.setFont (Theme::mono (9.5f, true));
    g.drawText ("CLICK TO BROWSE", in.withHeight (12.0f), Justification::centredRight);
}

//==============================================================================
//  Редактор
//==============================================================================
BeatMorphEditor::BeatMorphEditor (BeatMorphProcessor& p)
    : AudioProcessorEditor (&p), proc (p), timeView (p, 0), volView (p, 1), slicerView (p, 2),
      analyzer (p), lfoPage (p), browser (p)
{
    setLookAndFeel (&laf);

    for (auto* c : { (Component*) &timeView, (Component*) &volView, (Component*) &slicerView,
                     (Component*) &transport, (Component*) &presetLcd,
                     (Component*) &prevButton, (Component*) &nextButton, (Component*) &browseButton,
                     (Component*) &saveButton, (Component*) &randomButton, (Component*) &aboutButton,
                     (Component*) &vuIn, (Component*) &vuOut, (Component*) &analyzer, (Component*) &reels, (Component*) &status })
        addAndMakeVisible (c);

    // ---------------- кнопки шапки ----------------
    prevButton.setColour (TextButton::buttonColourId, Colour (0xff4f525b));
    nextButton.setColour (TextButton::buttonColourId, Colour (0xff4f525b));
    browseButton.setColour (TextButton::buttonColourId, Colour (0xff2f6a45));
    saveButton.setColour (TextButton::buttonColourId, Colour (0xff3d5a8a));
    randomButton.setColour (TextButton::buttonColourId, Colour (0xff9a3b2e));
    aboutButton.setColour (TextButton::buttonColourId, Colour (0xff3a3220));
    aboutButton.setColour (TextButton::textColourOffId, Theme::gold);

    presetLcd.onClick   = [this] { if (browser.isVisible()) closeBrowser(); else openBrowser (false); };
    browseButton.onClick = [this] { if (browser.isVisible()) closeBrowser(); else openBrowser (false); };
    saveButton.onClick   = [this] { openBrowser (true); };
    prevButton.onClick   = [this] { browser.step (-1); updatePresetLcd(); };
    nextButton.onClick   = [this] { browser.step (+1); updatePresetLcd(); };
    randomButton.onClick = [this] { proc.randomize(); updatePresetLcd(); };
    aboutButton.onClick  = [this] { openAbout(); };

    browser.onClose   = [this] { closeBrowser(); };
    browser.onChanged = [this] { updatePresetLcd(); };
    about.onClose     = [this] { about.setVisible (false); };

    // ---------------- Master: Mix / Out + замочки, Bypass, Panic ----------------
    for (auto* s : { &mixSlider, &outSlider })
    {
        s->setSliderStyle (Slider::LinearHorizontal);
        s->setTextBoxStyle (Slider::TextBoxRight, false, 54, 18);
        addAndMakeVisible (*s);
    }
    mixSlider.setColour (Slider::trackColourId, Theme::amber);
    outSlider.setColour (Slider::trackColourId, Theme::green);
    knobAtt.push_back (std::make_unique<Knob> (proc.apvts, "mix", mixSlider));
    knobAtt.push_back (std::make_unique<Knob> (proc.apvts, "out", outSlider));
    for (auto* s : { &mixSlider, &outSlider })
    {
        s->paramIndex = Patterns::modIndex (s == &mixSlider ? "mix" : "out");
        s->onRightClick = [this] (ModSlider& sl) { showModPopup (sl); };
        knobInfos.push_back ({ s, s->paramIndex, false });
    }
    {
        auto* l1 = labels.add (new Label ({}, "MIX"));
        auto* l2 = labels.add (new Label ({}, "OUT"));
        l1->setFont (Theme::font (12.0f, true));  l1->attachToComponent (&mixSlider, true);
        l2->setFont (Theme::font (12.0f, true));  l2->attachToComponent (&outSlider, true);
    }
    addAndMakeVisible (mixLock);
    addAndMakeVisible (outLock);
    toggleAtt.push_back (std::make_unique<Toggle> (proc.apvts, "mixLock", mixLock));
    toggleAtt.push_back (std::make_unique<Toggle> (proc.apvts, "outLock", outLock));

    addAndMakeVisible (bypassKey);
    addAndMakeVisible (panicKey);
    toggleAtt.push_back (std::make_unique<Toggle> (proc.apvts, "bypass", bypassKey));
    panicKey.setClickingTogglesState (false);
    panicKey.onClick = [this] { proc.panic(); panicKey.flash(); };

    // ---------------- страницы и вкладки ----------------
    for (int i = 0; i < numPages; ++i)
    {
        const Colour acc (pageInfo[i].colour);
        auto* pg = pages.add (new ModulePage (pageInfo[i].title, pageInfo[i].desc, acc));
        addChildComponent (pg);

        auto* tab = tabs.add (new ModuleTab (pageInfo[i].tab, acc));
        addAndMakeVisible (tab);
        tab->onClick = [this, i] { showPage (i); };
    }
    {
        auto* tab = tabs.add (new ModuleTab ("LFO", Theme::gold));
        addAndMakeVisible (tab);
        tab->onClick = [this] { showPage (lfoPageIndex); };
        addChildComponent (lfoPage);
    }

    tabs[0]->isActive = [this] { return val ("timeType") > 0.5f || val ("volType") > 0.5f; };
    tabs[1]->isActive = [this] { return val ("slType") > 0.5f && val ("slMix") > 0.001f; };
    tabs[2]->isActive = [this] { return val ("grainMix") > 0.001f; };
    tabs[3]->isActive = [this] { return val ("specType") > 0.5f && val ("specMix") > 0.001f; };
    tabs[4]->isActive = [this] { return val ("distType") > 0.5f && val ("distMix") > 0.001f; };
    tabs[5]->isActive = [this] { return val ("filterType") > 0.5f; };
    tabs[6]->isActive = [this] { return val ("ringMix") > 0.001f; };
    tabs[7]->isActive = [this] { return val ("dlyMix") > 0.001f; };
    tabs[8]->isActive = [this] { return val ("revMix") > 0.001f; };
    tabs[lfoPageIndex]->isActive = [this] { return anyModAssigned(); };

    // ---------------- создание контролов ----------------
    auto addLabel = [this] (Component& ctrl, const String& text)
    {
        auto* l = labels.add (new Label ({}, text.toUpperCase()));
        l->setFont (Theme::font (10.5f, true));
        l->setJustificationType (Justification::centred);
        l->attachToComponent (&ctrl, false);
    };

    auto makeKnob = [&] (int page, const String& section, const String& id, const String& label)
    {
        auto* s = knobs.add (new ModSlider());
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (Slider::TextBoxBelow, false, 78, 17);
        s->setColour (Slider::rotarySliderFillColourId, Colour (pageInfo[page].colour));
        s->paramIndex = Patterns::modIndex (id.toRawUTF8());
        s->onRightClick = [this] (ModSlider& sl) { showModPopup (sl); };
        pages[page]->addControl (section, *s);
        knobAtt.push_back (std::make_unique<Knob> (proc.apvts, id, *s));
        addLabel (*s, label);
        knobInfos.push_back ({ s, s->paramIndex, false });
    };

    auto makeCombo = [&] (int page, const String& section, const String& id, const String& label)
    {
        auto* cb = combos.add (new ComboBox());
        if (auto* choiceParam = dynamic_cast<AudioParameterChoice*> (proc.apvts.getParameter (id)))
            cb->addItemList (choiceParam->choices, 1);
        cb->setColour (ComboBox::focusedOutlineColourId, Colour (pageInfo[page].colour));
        pages[page]->addControl (section, *cb);
        comboAtt.push_back (std::make_unique<Combo> (proc.apvts, id, *cb));
        addLabel (*cb, label);
    };

    auto makeToggle = [&] (int page, const String& section, const String& id, const String& label)
    {
        auto* tb = toggles.add (new ToggleButton (label));
        tb->setColour (ToggleButton::tickColourId, Colour (pageInfo[page].colour));
        pages[page]->addControl (section, *tb);
        toggleAtt.push_back (std::make_unique<Toggle> (proc.apvts, id, *tb));
    };

    for (const auto& sp : specs)
    {
        if (sp.kind == 'K')      makeKnob   (sp.page, sp.section, sp.id, sp.label);
        else if (sp.kind == 'C') makeCombo  (sp.page, sp.section, sp.id, sp.label);
        else                     makeToggle (sp.page, sp.section, sp.id, sp.label);
    }

    // ---------------- диаграммы модулей (заполняют место под карточками) ----------------
    diagrams.add (new ModuleDiagram (proc, 0, Colour (pageInfo[0].colour)));
    diagrams.add (new CurveView (proc, 2));
    diagrams.add (new ModuleDiagram (proc, 2, Colour (pageInfo[2].colour)));
    diagrams.add (new AnalyzerView (proc));
    diagrams.add (new ModuleDiagram (proc, 4, Colour (pageInfo[4].colour)));
    diagrams.add (new ModuleDiagram (proc, 5, Colour (pageInfo[5].colour)));
    diagrams.add (new ModuleDiagram (proc, 6, Colour (pageInfo[6].colour)));
    diagrams.add (new ModuleDiagram (proc, 7, Colour (pageInfo[7].colour)));
    diagrams.add (new ModuleDiagram (proc, 8, Colour (pageInfo[8].colour)));
    for (int i = 0; i < numPages; ++i)
        pages[i]->setDiagram (diagrams[i]);

    // ---------------- светодиоды состояния ----------------
    for (int i = 0; i < numPages; ++i)
        status.add ({ pageInfo[i].tab, [this, i] { return tabs[i]->isActive && tabs[i]->isActive(); }, Colour (pageInfo[i].colour) });
    status.add ({ "LFO",  [this] { return anyModAssigned(); }, Theme::gold });
    status.add ({ "Sync", [this] { return proc.hostPlaying.load(); }, Theme::green });
    status.add ({ "Clip", [this] { return clipHold > 0; }, Theme::red });

    // ---------------- оверлеи (поверх всего) ----------------
    addChildComponent (browser);
    addChildComponent (about);

    setSize (1120, 900);
    showPage (0);
    updatePresetLcd();
    startTimerHz (30);
}

//==============================================================================
void BeatMorphEditor::showPage (int index)
{
    currentPage = index;
    for (int i = 0; i < pages.size(); ++i)
        pages[i]->setVisible (i == index);
    lfoPage.setVisible (index == lfoPageIndex);
    tabs[index]->setToggleState (true, dontSendNotification);
}

void BeatMorphEditor::openBrowser (bool focusSave)
{
    about.setVisible (false);
    browser.setBounds (rectOverlay);
    browser.open (focusSave);
}

void BeatMorphEditor::closeBrowser() { browser.setVisible (false); }

void BeatMorphEditor::openAbout()
{
    browser.setVisible (false);
    about.setBounds (rectOverlay);
    about.setVisible (true);
    about.toFront (false);
}

void BeatMorphEditor::mouseDown (const MouseEvent& e)
{
    if (e.eventComponent == this && rectName.contains (e.getPosition()))
        openAbout();
}

void BeatMorphEditor::showModPopup (ModSlider& s)
{
    if (s.paramIndex < 0) return;
    auto content = std::make_unique<ModPopup> (proc, s.paramIndex);
    CallOutBox::launchAsynchronously (std::move (content), getLocalArea (&s, s.getLocalBounds()), this);
}

bool BeatMorphEditor::anyModAssigned() const
{
    for (int k = 0; k < Patterns::numModParams; ++k)
        for (int l = 0; l < Patterns::numLfo; ++l)
            if (std::abs (proc.getModAmount (l, k)) > 1.0e-4f) return true;
    return false;
}

void BeatMorphEditor::updatePresetLcd()
{
    const auto& ps = factoryPresets();
    const int total = (int) ps.size();
    const int cur = proc.getFactoryIndex();

    if (proc.randomActive.load())
        presetLcd.setInfo ("RANDOM PATCH", "RANDOM");
    else if (cur >= 0 && cur < total)
        presetLcd.setInfo ("PRG " + String (cur + 1).paddedLeft ('0', 3) + "/" + String (total).paddedLeft ('0', 3)
                               + "  " + String (ps[(size_t) cur].category).toUpperCase(),
                           String (ps[(size_t) cur].name).toUpperCase());
    else
        presetLcd.setInfo ("USER PRESET", proc.userPresetName.isNotEmpty() ? proc.userPresetName.toUpperCase() : String ("INIT"));
}

//==============================================================================
void BeatMorphEditor::timerCallback()
{
    ++tickCount;
    timeView.repaint();
    volView.repaint();
    slicerView.repaint();
    for (auto* t : tabs) t->repaint();
    status.repaint();

    // приборы
    auto dbToNeedle = [] (float peak)
    {
        const float db = 20.0f * std::log10 (jmax (peak, 1.0e-5f));
        return jlimit (0.0f, 1.0f, (db + 20.0f) / 23.0f);
    };
    const float pIn  = jmax (proc.peakIn[0].exchange (0.0f),  proc.peakIn[1].exchange (0.0f));
    const float pOut = jmax (proc.peakOut[0].exchange (0.0f), proc.peakOut[1].exchange (0.0f));
    const float tIn = dbToNeedle (pIn), tOut = dbToNeedle (pOut);
    needleIn  += (tIn  > needleIn  ? 0.40f : 0.10f) * (tIn  - needleIn);
    needleOut += (tOut > needleOut ? 0.40f : 0.10f) * (tOut - needleOut);
    vuIn.needle = needleIn;   vuIn.repaint();
    vuOut.needle = needleOut; vuOut.repaint();
    if (pOut > 0.99f) clipHold = 45; else if (clipHold > 0) --clipHold;

    analyzer.tick();
    reels.tick (proc.hostPlaying.load() ? 0.16f : (pOut > 0.003f ? 0.05f : 0.0f));
    transport.set (proc.hostBpm.load(), proc.hostPpq.load(), proc.hostPlaying.load());

    updatePresetLcd();

    // видимая страница: диаграмма / страница LFO
    if (currentPage == lfoPageIndex) lfoPage.tick();
    else if (currentPage >= 0 && currentPage < diagrams.size())
    {
        if (auto* a = dynamic_cast<AnalyzerView*> (diagrams[currentPage])) a->tick();
        else if (auto* d = dynamic_cast<ModuleDiagram*> (diagrams[currentPage])) d->tick();
        else diagrams[currentPage]->repaint();
    }

    pages[3]->setHint (specHints[jlimit (0, 16, (int) val ("specType"))]);

    // индикаторы LFO на ручках (белая точка)
    for (auto& ki : knobInfos)
    {
        if (ki.modIdx < 0) continue;
        const float off = proc.modOffset[(size_t) ki.modIdx].load();
        if (std::abs (off) > 1.0e-5f)
        {
            ki.slider->getProperties().set ("mod", off);
            if (ki.slider->isShowing()) ki.slider->repaint();
            ki.modded = true;
        }
        else if (ki.modded)
        {
            ki.slider->getProperties().remove ("mod");
            ki.slider->repaint();
            ki.modded = false;
        }
    }
}

//==============================================================================
//  Рисование панели: графит, орех, латунь, углубления
//==============================================================================
void BeatMorphEditor::renderPanel()
{
    const int w = getWidth(), h = getHeight();
    if (w <= 0 || h <= 0) return;

    const int cheek = 36;
    panelImage = Image (Image::RGB, w, h, false);

    {
        Image::BitmapData bd (panelImage, Image::BitmapData::writeOnly);
        Random rnd (77);
        std::vector<float> rowN ((size_t) h);
        for (auto& r : rowN) r = rnd.nextFloat() - 0.5f;

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const float t = (float) y / (float) h;
                float b = 0.215f - 0.095f * t + rowN[(size_t) y] * 0.035f + (rnd.nextFloat() - 0.5f) * 0.018f;
                const float dx = ((float) x / (float) w - 0.5f) * 2.0f;
                b *= 1.0f - 0.14f * dx * dx;
                b = jlimit (0.0f, 1.0f, b);
                bd.setPixelColour (x, y, Colour::fromFloatRGBA (b * 0.98f, b * 0.99f, b * 1.04f, 1.0f));
            }
    }

    Graphics g (panelImage);

    // боковины из ореха
    Hw::woodPanel (g, { 0, 0, cheek, h }, true);
    Hw::woodPanel (g, { w - cheek, 0, cheek, h }, false);
    for (int side = 0; side < 2; ++side)
    {
        const int x = side == 0 ? cheek : w - cheek - 1;
        g.setColour (Colours::black.withAlpha (0.8f));
        g.drawVerticalLine (side == 0 ? x : x, 0.0f, (float) h);
        g.setColour (Colours::white.withAlpha (0.18f));
        g.drawVerticalLine (side == 0 ? x + 1 : x - 1, 0.0f, (float) h);
        for (float y : { 16.0f, (float) h - 16.0f })
            Hw::screw (g, { (float) (side == 0 ? cheek / 2 : w - cheek / 2), y }, 5.0f, y * 0.01f + (float) side);
    }

    // кромки лицевой панели
    g.setColour (Colours::white.withAlpha (0.16f));
    g.drawHorizontalLine (1, (float) cheek, (float) (w - cheek));
    g.setColour (Colours::black.withAlpha (0.7f));
    g.drawHorizontalLine (h - 2, (float) cheek, (float) (w - cheek));

    // углубление под страницы модулей
    Hw::bay (g, rectBay.toFloat(), 10.0f);

    // сеточная печать вокруг блоков
    Hw::silkBox (g, rectProgram.toFloat(), "Program", Theme::lcdOn);
    Hw::silkBox (g, rectMaster.toFloat(),  "Master",  Theme::amber);
    Hw::silkBox (g, rectMonitor.toFloat(), "Monitor", Theme::green);

    // латунная табличка с названием
    {
        const auto r = rectName.toFloat();
        Hw::brassPlate (g, r);
        const float availW = r.getWidth() - 56.0f;            // запас под винты по краям
        const String title = "BEATMORPH";
        Hw::engravedDark (g, title, r.reduced (26.0f, 6.0f).withHeight (42.0f), Hw::fitFont (title, true, 35.0f, availW), Justification::centredLeft);

        const String sub = "BM-900  MULTI-DIMENSION PROCESSOR";
        Hw::engravedDark (g, sub, r.reduced (26.0f, 0.0f).withTrimmedTop (50.0f).withHeight (13.0f), Hw::fitFont (sub, true, 10.5f, availW), Justification::centredLeft);
        Hw::engravedDark (g, "RHYTHM * GRAIN * SPECTRUM", r.reduced (26.0f, 0.0f).withTrimmedTop (65.0f).withHeight (13.0f), Theme::font (10.0f), Justification::centredLeft);
        Hw::engravedDark (g, "DESIGNED BY JETTY GAS", r.reduced (26.0f, 0.0f).withTrimmedTop (80.0f).withHeight (13.0f), Theme::font (10.0f, true), Justification::centredLeft);

        const Rectangle<float> badge (r.getX() + 26.0f, r.getBottom() - 22.0f, 58.0f, 14.0f);
        g.setColour (Colour (0xff2a1f08));
        g.drawRoundedRectangle (badge, 3.0f, 1.2f);
        g.setFont (Theme::font (9.5f, true));
        g.drawText ("DIGITAL", badge, Justification::centred, false);
        g.setFont (Theme::mono (9.5f, true));
        g.drawText ("S/N 000421", Rectangle<float> (r.getX() + 94.0f, r.getBottom() - 22.0f, 78.0f, 14.0f), Justification::centredLeft, false);
        g.drawText (String ("V") + BMVersion::string, Rectangle<float> (r.getRight() - 100.0f, r.getBottom() - 22.0f, 74.0f, 14.0f), Justification::centredRight, false);

        for (float sx : { r.getX() + 9.0f, r.getRight() - 9.0f })
            for (float sy : { r.getY() + 9.0f, r.getBottom() - 9.0f })
                Hw::screw (g, { sx, sy }, 3.6f, sx + sy);
    }

    g.setFont (Theme::font (9.5f, true));
    g.setColour (Theme::silkDim.withAlpha (0.85f));
    g.drawText ("BEATMORPH LABS  *  V" + String (BMVersion::string) + "  *  FREE FOREVER  *  MADE IN 1987 (PROBABLY)  *  KEEP AWAY FROM WATER",
                Rectangle<int> (cheek, h - 26, w - 2 * cheek, 14), Justification::centred);
}

void BeatMorphEditor::paint (Graphics& g)
{
    if (panelImage.isValid()) g.drawImageAt (panelImage, 0, 0);
    else g.fillAll (Colour (0xff25272b));

    // индикатор питания на табличке (мигает)
    const auto r = rectName.toFloat();
    g.setColour (Colours::black.withAlpha (0.6f));
    g.fillEllipse (r.getRight() - 36.0f, r.getY() + 14.0f, 14.0f, 14.0f);
    g.setColour (Theme::green.withAlpha ((tickCount / 30) % 2 == 0 ? 1.0f : 0.5f));
    g.fillEllipse (r.getRight() - 33.0f, r.getY() + 17.0f, 8.0f, 8.0f);
    g.setColour (Colours::white.withAlpha (0.6f));
    g.fillEllipse (r.getRight() - 31.0f, r.getY() + 18.0f, 3.0f, 2.0f);
}

void BeatMorphEditor::resized()
{
    auto inner = getLocalBounds().reduced (48, 14);

    // ---------------- шапка ----------------
    auto header = inner.removeFromTop (120);
    rectName    = header.removeFromLeft (264).reduced (0, 4);
    header.removeFromLeft (10);
    rectProgram = header.removeFromLeft (420).reduced (0, 6);
    header.removeFromLeft (10);
    rectMaster  = header.reduced (0, 6);

    aboutButton.setBounds (rectName.getRight() - 78, rectName.getY() + 66, 62, 20);

    {
        auto c = rectProgram.reduced (14, 0).withTrimmedTop (16).withTrimmedBottom (10);
        presetLcd.setBounds (c.removeFromTop (54));
        c.removeFromTop (4);
        auto row = c;
        prevButton.setBounds (row.removeFromLeft (36));     row.removeFromLeft (5);
        nextButton.setBounds (row.removeFromLeft (36));     row.removeFromLeft (5);
        browseButton.setBounds (row.removeFromLeft (96));   row.removeFromLeft (5);
        saveButton.setBounds (row.removeFromLeft (72));     row.removeFromLeft (5);
        randomButton.setBounds (row);
    }

    {
        auto c = rectMaster.reduced (14, 0).withTrimmedTop (16).withTrimmedBottom (10);
        auto keys = c.removeFromRight (76);
        bypassKey.setBounds (keys.removeFromTop (keys.getHeight() / 2).reduced (0, 1));
        panicKey.setBounds (keys.reduced (0, 1));
        c.removeFromRight (10);

        auto row1 = c.removeFromTop (c.getHeight() / 2);
        mixLock.setBounds (row1.removeFromRight (26).reduced (0, 5));
        row1.removeFromLeft (34);
        mixSlider.setBounds (row1);

        outLock.setBounds (c.removeFromRight (26).reduced (0, 5));
        c.removeFromLeft (34);
        outSlider.setBounds (c);
    }

    inner.removeFromTop (8);

    // ---------------- дисплеи ----------------
    auto displays = inner.removeFromTop (140);
    const int displaysTop = displays.getY();
    transport.setBounds (displays.removeFromRight (158));
    displays.removeFromRight (6);
    const int cw = displays.getWidth() / 3;
    timeView.setBounds (displays.removeFromLeft (cw));
    volView.setBounds (displays.removeFromLeft (cw));
    slicerView.setBounds (displays);

    inner.removeFromTop (8);

    // ---------------- вкладки модулей ----------------
    auto tabRow = inner.removeFromTop (42);
    const int tw = tabRow.getWidth() / jmax (1, tabs.size());
    for (auto* t : tabs)
        t->setBounds (tabRow.removeFromLeft (tw));

    inner.removeFromTop (6);

    // ---------------- страницы (в углублении панели) ----------------
    auto pageArea = inner.removeFromTop (350);
    rectBay = pageArea;
    const auto pageBounds = pageArea.reduced (8, 4);
    for (auto* pg : pages) pg->setBounds (pageBounds);
    lfoPage.setBounds (pageBounds);

    rectOverlay = { pageArea.getX(), displaysTop, pageArea.getWidth(), pageArea.getBottom() - displaysTop };
    if (browser.isVisible()) browser.setBounds (rectOverlay);
    if (about.isVisible())   about.setBounds (rectOverlay);

    inner.removeFromTop (10);

    // ---------------- мониторинг ----------------
    rectMonitor = inner.removeFromTop (164).reduced (0, 6);
    {
        auto c = rectMonitor.reduced (12, 16);
        vuIn.setBounds (c.removeFromLeft (150));     c.removeFromLeft (8);
        vuOut.setBounds (c.removeFromLeft (150));    c.removeFromLeft (8);
        analyzer.setBounds (c.removeFromLeft (360)); c.removeFromLeft (8);
        reels.setBounds (c.removeFromLeft (170));    c.removeFromLeft (8);
        status.setBounds (c);
    }

    renderPanel();
}
