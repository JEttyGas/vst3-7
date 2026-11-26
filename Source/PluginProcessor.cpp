#include "PluginProcessor.h"
#include <algorithm>
#include "PluginEditor.h"
#include "Patterns.h"
#include "Presets.h"
#include <cstring>

using namespace juce;

namespace
{
    enum class Unit { None, Int, Pct, Ms, Hz, St, Db };

    String formatValue (Unit u, float v)
    {
        switch (u)
        {
            case Unit::Int: return String (roundToInt (v));
            case Unit::Pct: return String (roundToInt (v * 100.0f)) + " %";
            case Unit::Ms:  return v >= 100.0f ? String (roundToInt (v)) + " ms" : String (v, 1) + " ms";
            case Unit::Hz:  return v >= 1000.0f ? String (v / 1000.0f, 2) + " kHz"
                                 : v >= 100.0f  ? String (roundToInt (v)) + " Hz"
                                                : String (v, 1) + " Hz";
            case Unit::St:  return String (v > 0.0f ? "+" : "") + String (v, 1) + " st";
            case Unit::Db:  return String (v, 1) + " dB";
            default:        return String (v, 2);
        }
    }

    float parseValue (Unit u, const String& t)
    {
        float v = t.getFloatValue();
        if (u == Unit::Pct) return v / 100.0f;
        if (u == Unit::Hz && t.containsIgnoreCase ("k")) return v * 1000.0f;
        return v;
    }

}

//==============================================================================
BeatMorphProcessor::BeatMorphProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    static const char* sfx[4] = { "Shape", "Sync", "Rate", "Level" };
    for (int l = 0; l < Patterns::numLfo; ++l)
        for (int k = 0; k < 4; ++k)
            lfoIds[l][k] = "lfo" + String (l + 1) + sfx[k];

    setLatencySamples (SpectralChannel::N);
}

AudioProcessorValueTreeState::ParameterLayout BeatMorphProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto choice = [&] (const char* id, const char* name, const StringArray& items, int def)
    { p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def)); };

    auto toggle = [&] (const char* id, const char* name, bool def)
    { p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def)); };

    auto flt = [&] (const char* id, const char* name, NormalisableRange<float> r, float def, Unit u = Unit::None)
    {
        auto attrs = AudioParameterFloatAttributes()
                         .withStringFromValueFunction ([u] (float v, int) { return formatValue (u, v); })
                         .withValueFromStringFunction ([u] (const String& t) { return parseValue (u, t); });
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def, attrs));
    };

    // ---- Rhythm ----
    choice ("length",     "Length",       Patterns::lengthNames, 1);
    flt    ("swing",      "Swing",        { 0.0f, 0.5f }, 0.0f, Unit::Pct);
    flt    ("shift",      "Phase",        { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    choice ("timeType",   "Time Type",    Patterns::timeNames, 0);
    choice ("timeDiv",    "Time Slices",  Patterns::sliceNames, 3);
    flt    ("timeAmount", "Time Amount",  { 0.0f, 1.0f }, 1.0f, Unit::Pct);
    choice ("volType",    "Vol Type",     Patterns::volNames, 0);
    choice ("volDiv",     "Vol Slices",   Patterns::sliceNames, 5);
    flt    ("volDepth",   "Vol Depth",    { 0.0f, 1.0f }, 1.0f, Unit::Pct);
    flt    ("volSmooth",  "Vol Smooth",   { 0.2f, 50.0f, 0.0f, 0.4f }, 1.5f, Unit::Ms);

    // ---- Slicer ----
    choice ("slType",    "Slicer Order",  Patterns::slicerNames, 0);
    choice ("slSteps",   "Slicer Steps",  Patterns::stepNames, 4);
    choice ("slLength",  "Slicer Length", Patterns::slLengthNames, 1);
    flt    ("slSeed",    "Slicer Seed",   { 0.0f, 63.0f, 1.0f }, 0.0f, Unit::Int);
    toggle ("slEvolve",  "Slicer Evolve", false);
    flt    ("slChance",  "Slicer Chance", { 0.0f, 1.0f }, 1.0f, Unit::Pct);
    flt    ("slReverse", "Slicer Reverse",{ 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("slMute",    "Slicer Mute",   { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("slMix",     "Slicer Mix",    { 0.0f, 1.0f }, 1.0f, Unit::Pct);

    // ---- Granular ----
    flt    ("grainMix",       "Grain Mix",        { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("grainSize",      "Grain Size",       { 5.0f, 500.0f, 0.0f, 0.5f }, 100.0f, Unit::Ms);
    flt    ("grainSizeRand",  "Grain Size Rand",  { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("grainDensity",   "Grain Density",    { 1.0f, 80.0f, 0.0f, 0.5f }, 20.0f);
    flt    ("grainPitch",     "Grain Pitch",      { -24.0f, 24.0f, 0.01f }, 0.0f, Unit::St);
    flt    ("grainPitchRand", "Grain Pitch Rand", { 0.0f, 12.0f }, 0.0f, Unit::St);
    choice ("grainQuant",     "Grain Quantize",   Patterns::quantNames, 0);
    flt    ("grainSpread",    "Grain Spread",     { 0.0f, 1000.0f }, 200.0f, Unit::Ms);
    flt    ("grainScan",      "Grain Scan",       { -2.0f, 2.0f }, 0.0f, Unit::Hz);
    flt    ("grainWidth",     "Grain Width",      { 0.0f, 1.0f }, 0.7f, Unit::Pct);
    flt    ("grainRev",       "Grain Reverse",    { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    choice ("grainWindow",    "Grain Window",     Patterns::windowNames, 0);
    toggle ("grainFreeze",    "Grain Freeze",     false);

    // ---- Spectral ----
    choice ("specType",   "Spectral Type",   Patterns::specNames, 0);
    toggle ("specHold",   "Spectral Hold",   false);
    flt    ("specAmount", "Spectral Amount", { 0.0f, 1.0f }, 0.7f, Unit::Pct);
    flt    ("specTone",   "Spectral Tone",   { 0.0f, 1.0f }, 0.5f, Unit::Pct);
    flt    ("specPitch",  "Spectral Pitch",  { -24.0f, 24.0f, 0.01f }, 7.0f, Unit::St);
    flt    ("specMotion", "Spectral Motion", { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("specRate",   "Spectral Rate",   { 0.05f, 8.0f, 0.0f, 0.5f }, 0.5f, Unit::Hz);
    flt    ("specMix",    "Spectral Mix",    { 0.0f, 1.0f }, 1.0f, Unit::Pct);

    // ---- Distortion ----
    choice ("distType", "Dist Type",   Patterns::distNames, 0);
    flt    ("drive",    "Drive",       { 0.0f, 1.0f }, 0.3f, Unit::Pct);
    flt    ("distBias", "Dist Bias",   { -1.0f, 1.0f }, 0.0f);
    flt    ("distBits", "Crush Bits",  { 2.0f, 16.0f, 1.0f }, 8.0f, Unit::Int);
    flt    ("distRate", "Crush Downsample", { 1.0f, 32.0f, 1.0f }, 4.0f, Unit::Int);
    flt    ("distTone", "Dist Tone",   { 500.0f, 20000.0f, 0.0f, 0.4f }, 20000.0f, Unit::Hz);
    flt    ("distMix",  "Dist Mix",    { 0.0f, 1.0f }, 1.0f, Unit::Pct);

    // ---- Filter ----
    choice ("filterType",  "Filter",       Patterns::filterNames, 0);
    choice ("filterSlope", "Filter Slope", Patterns::slopeNames, 0);
    flt    ("cutoff",      "Cutoff",       { 20.0f, 20000.0f, 0.0f, 0.3f }, 1000.0f, Unit::Hz);
    flt    ("res",         "Resonance",    { 0.5f, 10.0f, 0.0f, 0.5f }, 0.7f);
    flt    ("filterDrive", "Filter Drive", { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    choice ("filterSrc",   "Filter Mod Source", Patterns::filterSrcNames, 0);
    flt    ("filterMod",   "Filter Mod",   { -1.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("filterRate",  "Filter LFO Rate", { 0.05f, 12.0f, 0.0f, 0.5f }, 0.5f, Unit::Hz);

    // ---- Ring mod ----
    choice ("ringShape",  "Ring Shape",   Patterns::ringShapeNames, 0);
    choice ("ringMode",   "Ring Mode",    Patterns::ringModeNames, 0);
    flt    ("ringFreq",   "Ring Freq",    { 20.0f, 5000.0f, 0.0f, 0.3f }, 440.0f, Unit::Hz);
    flt    ("ringFM",     "Ring FM",      { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("ringFMRate", "Ring FM Rate", { 0.05f, 20.0f, 0.0f, 0.4f }, 2.0f, Unit::Hz);
    flt    ("ringMix",    "Ring Mix",     { 0.0f, 1.0f }, 0.0f, Unit::Pct);

    // ---- Delay ----
    choice ("dlySync",     "Delay Sync",     Patterns::dlySyncNames, 2);
    flt    ("dlyFree",     "Delay Time",     { 1.0f, 2000.0f, 0.0f, 0.5f }, 300.0f, Unit::Ms);
    choice ("dlyMode",     "Delay Mode",     Patterns::dlyModeNames, 0);
    flt    ("dlyFeedback", "Delay Feedback", { 0.0f, 0.95f }, 0.4f, Unit::Pct);
    flt    ("dlyTone",     "Delay High Cut", { 500.0f, 20000.0f, 0.0f, 0.4f }, 8000.0f, Unit::Hz);
    flt    ("dlyLow",      "Delay Low Cut",  { 20.0f, 1000.0f, 0.0f, 0.4f }, 20.0f, Unit::Hz);
    flt    ("dlyMod",      "Delay Mod",      { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("dlyModRate",  "Delay Mod Rate", { 0.05f, 8.0f, 0.0f, 0.5f }, 0.5f, Unit::Hz);
    flt    ("dlyDuck",     "Delay Duck",     { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("dlyMix",      "Delay Mix",      { 0.0f, 1.0f }, 0.0f, Unit::Pct);

    // ---- Reverb ----
    flt    ("revRoom",   "Reverb Room",   { 0.0f, 1.0f }, 0.5f, Unit::Pct);
    flt    ("revDamp",   "Reverb Damp",   { 0.0f, 1.0f }, 0.5f, Unit::Pct);
    flt    ("revWidth",  "Reverb Width",  { 0.0f, 1.0f }, 1.0f, Unit::Pct);
    flt    ("revPre",    "Reverb Pre-delay", { 0.0f, 250.0f }, 0.0f, Unit::Ms);
    toggle ("revFreeze", "Reverb Freeze", false);
    flt    ("revDuck",   "Reverb Duck",   { 0.0f, 1.0f }, 0.0f, Unit::Pct);
    flt    ("revMix",    "Reverb Mix",    { 0.0f, 1.0f }, 0.0f, Unit::Pct);

    // ---- LFO-банк: 4 генератора; назначения лежат в матрице модуляции ----
    for (int l = 0; l < Patterns::numLfo; ++l)
    {
        const String pre = "lfo" + String (l + 1);
        const String nm  = "LFO " + String (l + 1) + " ";

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { pre + "Shape", 1 }, nm + "Shape", Patterns::lfoShapeNames, 0));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { pre + "Sync", 1 },  nm + "Sync",  Patterns::lfoSyncNames, 0));

        auto rateAttrs = AudioParameterFloatAttributes()
                             .withStringFromValueFunction ([] (float v, int) { return formatValue (Unit::Hz, v); })
                             .withValueFromStringFunction ([] (const String& t) { return parseValue (Unit::Hz, t); });
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { pre + "Rate", 1 }, nm + "Rate",
                         NormalisableRange<float> (0.02f, 20.0f, 0.0f, 0.4f), 0.5f, rateAttrs));

        auto levelAttrs = AudioParameterFloatAttributes()
                              .withStringFromValueFunction ([] (float v, int) { return formatValue (Unit::Pct, v); })
                              .withValueFromStringFunction ([] (const String& t) { return parseValue (Unit::Pct, t); });
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { pre + "Level", 1 }, nm + "Level",
                         NormalisableRange<float> (0.0f, 1.0f), 1.0f, levelAttrs));
    }

    // ---- Master ----
    flt ("mix", "Mix",    { 0.0f, 1.0f }, 1.0f, Unit::Pct);
    flt ("out", "Output", { -24.0f, 12.0f }, 0.0f, Unit::Db);
    toggle ("mixLock", "Mix Lock", false);     // замочек: Mix не меняется при смене пресета
    toggle ("outLock", "Output Lock", false);
    toggle ("bypass",  "Bypass", false);

    return { p.begin(), p.end() };
}

//==============================================================================
bool BeatMorphProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != AudioChannelSet::mono() && out != AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void BeatMorphProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    history.setSize (2, (int) (sr * 12.0) + 4);   history.clear();   writePos = 0;
    slicerBuf.setSize (2, (int) (sr * 20.0) + 4); slicerBuf.clear(); slWrite = 0;
    grainBuf.setSize (2, (int) (sr * 4.0) + 4);   grainBuf.clear();  grainWrite = 0;

    grainTimer = 0.0;
    for (auto& g : grains) g.active = false;
    permType = -1;

    for (auto& s : spec) s.reset();
    for (auto& b : dryBuf) b.fill (0.0f);
    dryPos = 0;

    dsp::ProcessSpec ps { sr, (uint32) samplesPerBlock, 2 };
    filterA.prepare (ps);  filterA.reset();
    filterB.prepare (ps);  filterB.reset();

    dly.prepare (sr);
    reverb.setSampleRate (sr);
    reverb.reset();
    for (auto& b : preBuf) b.assign ((size_t) (sr * 0.3) + 8, 0.0f);
    prePos = 0;

    smoothVol   = 1.0f;
    prevDelay   = 0.0;
    fadeLen     = jmax (16, (int) (sr * 0.002));
    fadeCounter = 0;
    for (auto& d : dist) d = DistState();
    ringPhase = ringLfoPhase = filtLfoPhase = specLfoPhase = scanPhase = 0.0;
    for (auto& a : lfoAcc) a = 0.0;

    inEnv  = 0.0f;
    envAtk = 1.0f - std::exp (-1.0f / (0.003f * (float) sr));
    envRel = 1.0f - std::exp (-1.0f / (0.150f * (float) sr));

    setLatencySamples (SpectralChannel::N);
}

//==============================================================================
void BeatMorphProcessor::doPanic()
{
    history.clear();  slicerBuf.clear();  grainBuf.clear();
    for (auto& g : grains) g.active = false;
    for (auto& s : spec) s.reset();
    dly.reset();
    reverb.reset();
    for (auto& b : preBuf) std::fill (b.begin(), b.end(), 0.0f);
    filterA.reset();  filterB.reset();
    for (auto& d : dist) d = DistState();
    smoothVol = 1.0f;  prevDelay = 0.0;  fadeCounter = 0;
}

void BeatMorphProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const int nCh = jmin (2, buffer.getNumChannels());
    const int n   = buffer.getNumSamples();
    for (int ch = nCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, n);

    // ---- хост ----
    double bpm = 120.0, ppq = 0.0;
    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            if (pos->getIsPlaying())
                if (auto q = pos->getPpqPosition()) { ppq = *q; playing = true; }
        }
    bpm = jmax (20.0, bpm);
    hostBpm.store (bpm);  hostPpq.store (ppq);  hostPlaying.store (playing);

    if (panicFlag.exchange (false)) doPanic();

    // обработка короткими кусками (32 сэмпла), чтобы LFO двигались плавно
    const int chunk = 32;
    const double beatsPerSample = bpm / (60.0 * sampleRate);
    double ppqPos = ppq;
    for (int start = 0; start < n; start += chunk)
    {
        const int len = jmin (chunk, n - start);
        processChunk (buffer, start, len, bpm, playing, ppqPos);
        if (playing) ppqPos += (double) len * beatsPerSample;
        else         freeBeats += (double) len * beatsPerSample;
    }
}

void BeatMorphProcessor::processChunk (AudioBuffer<float>& buffer, int start, int len, double bpm, bool playing, double ppq)
{
    const int nCh = jmin (2, buffer.getNumChannels());
    const int n   = start + len;
    const double samplesPerBeat = 60.0 * sampleRate / bpm;

    // ======== LFO-банк и матрица модуляции: значения на начало куска ========
    struct ModEntry { const char* id; float offset; };
    std::array<ModEntry, Patterns::numModParams> mods {};
    int nMods = 0;
    {
        const double beatPos = playing ? ppq : freeBeats;
        float lv[Patterns::numLfo];
        for (int l = 0; l < Patterns::numLfo; ++l)
        {
            const int   shape = (int) apvts.getRawParameterValue (lfoIds[l][0])->load();
            const int   sync  = (int) apvts.getRawParameterValue (lfoIds[l][1])->load();
            const float rate  = apvts.getRawParameterValue (lfoIds[l][2])->load();
            const float level = apvts.getRawParameterValue (lfoIds[l][3])->load();

            double acc;
            if (sync == 0) { acc = lfoAcc[l]; lfoAcc[l] += (double) rate * (double) len / sampleRate; }
            else           { acc = beatPos / Patterns::lfoSyncBeats[sync - 1]; }

            const double fl = std::floor (acc);
            lv[l] = Patterns::lfoShape (shape, acc - fl, (int64_t) fl, l) * level;
            lfoValue[(size_t) l].store (lv[l]);
            lfoPhase[(size_t) l].store ((float) (acc - fl));
        }

        for (int k = 0; k < Patterns::numModParams; ++k)
        {
            float off = 0.0f;
            for (int l = 0; l < Patterns::numLfo; ++l)
                off += modAmount[(size_t) k][(size_t) l].load (std::memory_order_relaxed) * lv[l];

            modOffset[(size_t) k].store (off, std::memory_order_relaxed);
            if (std::abs (off) > 1.0e-6f)
                mods[(size_t) nMods++] = { Patterns::modParamTable[k].id, off };
        }
    }

    // чтение параметра с учётом модуляции любого числа LFO
    auto v = [this, &mods, &nMods] (const char* id) -> float
    {
        float base = apvts.getRawParameterValue (id)->load();
        for (int k = 0; k < nMods; ++k)
            if (std::strcmp (id, mods[(size_t) k].id) == 0)
                if (auto* prm = apvts.getParameter (id))
                {
                    const auto& r = prm->getNormalisableRange();
                    base = r.convertFrom0to1 (jlimit (0.0f, 1.0f, r.convertTo0to1 (base) + mods[(size_t) k].offset));
                }
        return base;
    };

    // ======== параметры: Rhythm ========
    const double patBeats = Patterns::lengthBeats[(int) v ("length")];
    const int    tType = (int) v ("timeType");
    const int    tDiv  = Patterns::sliceValues[(int) v ("timeDiv")];
    const float  tAmount = v ("timeAmount");
    const int    vType = (int) v ("volType");
    const int    vDiv  = Patterns::sliceValues[(int) v ("volDiv")];
    const float  depth = v ("volDepth");
    const float  swing = v ("swing");
    const float  pShift = v ("shift");
    const float  volCoef = 1.0f - std::exp (-1.0f / (jmax (0.2f, v ("volSmooth")) * 0.001f * (float) sampleRate));

    // ======== Slicer ========
    const int    slType  = (int) v ("slType");
    const int    slN     = Patterns::stepValues[(int) v ("slSteps")];
    const double slBeats = Patterns::slLengthBeats[(int) v ("slLength")];
    const int    slSeed  = (int) v ("slSeed");
    const bool   slEvolve = v ("slEvolve") > 0.5f;
    const float  slChance = v ("slChance"), slRevP = v ("slReverse"), slMuteP = v ("slMute"), slMix = v ("slMix");
    const bool   slActive = slType != 0 && slMix > 0.0005f;
    const double slL     = slBeats * samplesPerBeat;
    const int    slSize  = slicerBuf.getNumSamples();
    const double sliceLen = slL / (double) slN;
    const double wEdge   = jmin (0.5, jmax (8.0, 0.003 * sampleRate) / sliceLen);

    // ======== Granular ========
    const float  gMix       = v ("grainMix");
    const double gSizeMs    = v ("grainSize");
    const double gLenBase   = jmax (8.0, gSizeMs * 0.001 * sampleRate);
    const float  gSizeRand  = v ("grainSizeRand");
    const float  gDensity   = v ("grainDensity");
    const float  gPitch     = v ("grainPitch");
    const float  gPitchRand = v ("grainPitchRand");
    const int    gQuant     = (int) v ("grainQuant");
    const double gSpread    = (double) v ("grainSpread") * 0.001 * sampleRate;
    const double gScanInc   = (double) v ("grainScan") / sampleRate;
    const float  gWidth     = v ("grainWidth");
    const float  gRev       = v ("grainRev");
    const int    gWin       = (int) v ("grainWindow");
    const bool   gFreeze    = v ("grainFreeze") > 0.5f;
    const float  gOverlap   = gDensity * (float) gSizeMs * 0.001f;
    const float  gNorm      = 1.0f / std::sqrt (jmax (1.0f, gOverlap * 0.5f));
    const int    gSize      = grainBuf.getNumSamples();

    // ======== Spectral ========
    SpecParams sp;
    sp.type   = (int) v ("specType");
    sp.hold   = v ("specHold") > 0.5f;
    sp.amount = v ("specAmount");
    sp.tone   = v ("specTone");
    sp.pitch  = v ("specPitch");
    sp.motion = v ("specMotion");
    sp.mix    = v ("specMix");
    sp.binHz  = (float) (sampleRate / (double) SpectralChannel::N);
    const double specLfoInc = (double) v ("specRate") / sampleRate;

    // ======== Ring mod ========
    const float  ringMix   = v ("ringMix");
    const int    ringShape = (int) v ("ringShape");
    const bool   ringAM    = (int) v ("ringMode") == 1;
    const double ringInc   = (double) v ("ringFreq") / sampleRate;
    const float  ringFM    = v ("ringFM");
    const double ringLfoInc = (double) v ("ringFMRate") / sampleRate;

    // ======== Distortion ========
    DistParams dp;
    dp.type = (int) v ("distType");
    const float dDrive = v ("drive");
    dp.gain   = std::pow (10.0f, dDrive * 2.0f);
    dp.comp   = std::pow (dp.gain, -0.25f);
    dp.bias   = v ("distBias");
    dp.levels = std::pow (2.0f, v ("distBits") - 1.0f);
    dp.holdN  = jmax (1, (int) v ("distRate"));
    const float dToneHz = v ("distTone");
    dp.lpOn   = dToneHz < 19000.0f;
    dp.lpCoef = 1.0f - std::exp (-Patterns::twoPi * dToneHz / (float) sampleRate);
    const float dMix = v ("distMix");

    // ======== Filter ========
    const int   fType  = (int) v ("filterType");
    const bool  f24    = (int) v ("filterSlope") == 1;
    const float cutoff = v ("cutoff");
    const float fDrive = v ("filterDrive");
    const int   fSrc   = (int) v ("filterSrc");
    const float fMod   = v ("filterMod");
    const double fLfoInc = (double) v ("filterRate") / sampleRate;
    {
        const auto t = fType == 2 ? dsp::StateVariableTPTFilterType::highpass
                     : fType == 3 ? dsp::StateVariableTPTFilterType::bandpass
                                  : dsp::StateVariableTPTFilterType::lowpass;
        filterA.setType (t);  filterB.setType (t);
        filterA.setResonance (v ("res"));  filterB.setResonance (v ("res"));
    }

    // ======== Delay ========
    const float dlyMix = v ("dlyMix");
    StereoDelay::Params dyp;
    {
        const int sync = (int) v ("dlySync");
        dyp.timeSamples = sync >= 10 ? v ("dlyFree") * 0.001f * (float) sampleRate
                                     : (float) (Patterns::dlyBeats[sync] * samplesPerBeat);
        dyp.feedback = jmin (0.97f, v ("dlyFeedback"));
        dyp.mode     = (int) v ("dlyMode");
        dyp.lpHz     = v ("dlyTone");
        dyp.hpHz     = v ("dlyLow");
        dyp.modDepth = v ("dlyMod") * 0.008f * (float) sampleRate;
        dyp.modRate  = v ("dlyModRate");
        dyp.duck     = v ("dlyDuck");
    }

    // ======== Reverb ========
    const float revMix  = v ("revMix");
    const float revDuck = v ("revDuck");
    const int   revPreSamp = (int) (v ("revPre") * 0.001f * (float) sampleRate);
    {
        Reverb::Parameters rp;
        rp.roomSize   = v ("revRoom");
        rp.damping    = v ("revDamp");
        rp.wetLevel   = 0.33f;
        rp.dryLevel   = 0.0f;
        rp.width      = v ("revWidth");
        rp.freezeMode = v ("revFreeze") > 0.5f ? 1.0f : 0.0f;
        reverb.setParameters (rp);
    }

    const float mix     = v ("mix");
    const float outGain = Decibels::decibelsToGain (v ("out"));

    // ======== фазы ========
    const double L        = patBeats * samplesPerBeat;
    const double phaseInc = 1.0 / L;
    const int    histSize = history.getNumSamples();
    const int    preSize  = (int) preBuf[0].size();

    double phase = (playing ? ppq : freeBeats) / patBeats;
    phase -= std::floor (phase);
    slBeatPos = (playing ? ppq : freeBeats) / slBeats;

    // ---- запуск одного зерна ----
    auto spawnGrain = [&]
    {
        for (auto& g : grains)
        {
            if (g.active) continue;

            const bool   rev   = rng.nextFloat() < gRev;
            const float  jit   = (rng.nextFloat() * 2.0f - 1.0f) * gPitchRand;
            const double semis = (double) gPitch + (double) Patterns::snapPitch (gQuant, jit);
            const double ratio = std::pow (2.0, semis / 12.0);
            const double inc   = rev ? -ratio : ratio;

            const double len = jmax (8.0, gLenBase * (1.0 + (double) gSizeRand * (double) (rng.nextFloat() * 2.0f - 1.0f) * 0.8));

            // как меняется задержка чтения за время зерна (при Freeze запись стоит)
            const double dr   = (gFreeze ? 0.0 : 1.0) - inc;
            const double span = dr * len;
            const double minD = 4.0 + jmax (0.0, -span);
            const double maxD = (double) gSize - 8.0 - jmax (0.0, span);

            double range = jmax (0.0, maxD - minD - gSpread);
            range = jmin (range, 2.5 * sampleRate);

            double d0 = minD + scanPhase * range + (double) rng.nextFloat() * gSpread;
            d0 = jmin (d0, maxD);
            if (d0 < minD) return;

            double rp = (double) grainWrite - d0;
            if (rp < 0.0) rp += (double) gSize;

            g.active = true;
            g.pos = rp;
            g.inc = inc;
            g.age = 0;
            g.len = (int) len;

            const float pan = (rng.nextFloat() * 2.0f - 1.0f) * gWidth;
            const float th  = (pan + 1.0f) * (MathConstants<float>::pi * 0.25f);
            g.gL = std::cos (th) * 1.41421356f;
            g.gR = std::sin (th) * 1.41421356f;
            return;
        }
    };

    auto readBuf = [] (const AudioBuffer<float>& b, int ch, double rp, int size)
    {
        if (rp < 0.0) rp += (double) size;
        const int   i0 = (int) rp;
        const float fr = (float) (rp - (double) i0);
        const int   i1 = (i0 + 1 >= size) ? 0 : i0 + 1;
        const float a = b.getSample (ch, i0), c = b.getSample (ch, i1);
        return a + fr * (c - a);
    };

    float pkIn[2] = { 0.0f, 0.0f }, pkOut[2] = { 0.0f, 0.0f };
    const bool bypass = apvts.getRawParameterValue ("bypass")->load() > 0.5f;

    for (int i = start; i < n; ++i)
    {
        // ---- фаза с учётом Phase-сдвига и свинга ----
        float pp = (float) phase + pShift;
        pp -= std::floor (pp);
        const float pv = Patterns::swingWarp (pp, vDiv, swing);
        const float pt = Patterns::swingWarp (pp, tDiv, swing);

        // ---- Volume ----
        smoothVol += volCoef * (Patterns::volumeCurve (vType, pv, vDiv) - smoothVol);
        const float volGain = 1.0f - depth * (1.0f - smoothVol);

        // ---- Time ----
        const float f = Patterns::timeCurve (tType, pt, tDiv);
        const double delay = jlimit (0.0, (double) (histSize - 4), (double) (pt - f) * L * (double) tAmount);

        if (std::abs (delay - prevDelay) > 8.0)
            fadeCounter = fadeLen;
        prevDelay = delay;
        const float fade = 1.0f - (float) fadeCounter / (float) fadeLen;
        if (fadeCounter > 0) --fadeCounter;

        double rp = (double) writePos - delay;
        if (rp < 0.0) rp += (double) histSize;
        const int   i0 = (int) rp;
        const float fr = (float) (rp - (double) i0);
        const int   i1 = (i0 + 1 >= histSize) ? 0 : i0 + 1;

        float dryIn[2] = { 0.0f, 0.0f }, wet[2] = { 0.0f, 0.0f };
        for (int ch = 0; ch < nCh; ++ch)
        {
            dryIn[ch] = buffer.getSample (ch, i);
            pkIn[ch] = jmax (pkIn[ch], std::abs (dryIn[ch]));
            history.setSample (ch, writePos, dryIn[ch]);

            const float a = history.getSample (ch, i0);
            const float b = history.getSample (ch, i1);
            wet[ch] = (a + fr * (b - a)) * fade * volGain;
        }

        // ---- огибающая входа ----
        const float lvl = jmax (std::abs (dryIn[0]), std::abs (dryIn[1]));
        inEnv += (lvl > inEnv ? envAtk : envRel) * (lvl - inEnv);
        const float envNorm = jmin (1.0f, inEnv * 3.0f);

        // ---- Slicer: перестановка кусочков предыдущего цикла ----
        for (int ch = 0; ch < nCh; ++ch)
            slicerBuf.setSample (ch, slWrite, wet[ch]);

        if (slActive)
        {
            const double cyc = slBeatPos;
            const double cph = cyc - std::floor (cyc);
            const double t   = cph * (double) slN;
            int k = (int) t;
            if (k >= slN) k = slN - 1;
            const double x = t - (double) k;

            const uint32_t seedEff = (uint32_t) slSeed + (slEvolve ? (uint32_t) ((int64_t) std::floor (cyc) * 7919) : 0u);
            if (slType != permType || slN != permN || seedEff != permSeed)
            {
                Patterns::buildPermutation (slType, slN, seedEff, perm.data());
                permType = slType;  permN = slN;  permSeed = seedEff;
            }

            const int src = perm[(size_t) k];
            const int hk  = (int) (seedEff * 977u + (uint32_t) k * 131u);
            const bool act = Patterns::hash01 (hk + 1) < slChance;

            if (act)
            {
                const bool rev  = Patterns::hash01 (hk + 2) < slRevP;
                const bool mute = Patterns::hash01 (hk + 3) < slMuteP;
                const double xs = rev ? 1.0 - x : x;
                const double dFrac = ((double) k + x - (double) src - xs) / (double) slN + 1.0;
                const double d = jlimit (0.0, (double) (slSize - 4), dFrac * slL);

                double w = 1.0;
                if (x < wEdge)            w = x / wEdge;
                else if (x > 1.0 - wEdge) w = (1.0 - x) / wEdge;
                w = w * w * (3.0 - 2.0 * w);

                for (int ch = 0; ch < nCh; ++ch)
                {
                    const float tap = mute ? 0.0f : readBuf (slicerBuf, ch, (double) slWrite - d, slSize);
                    const float sl  = wet[ch] * (1.0f - (float) w) + tap * (float) w;
                    wet[ch] += slMix * (sl - wet[ch]);
                }
            }
        }
        slBeatPos += 1.0 / slL;
        if (++slWrite >= slSize) slWrite = 0;

        // ---- Granular ----
        if (! gFreeze)
            for (int ch = 0; ch < nCh; ++ch)
                grainBuf.setSample (ch, grainWrite, wet[ch]);

        scanPhase += gScanInc;
        scanPhase -= std::floor (scanPhase);

        if (gMix > 0.0005f)
        {
            grainTimer -= 1.0;
            if (grainTimer <= 0.0)
            {
                grainTimer += sampleRate / (double) gDensity;
                spawnGrain();
            }

            float gOut[2] = { 0.0f, 0.0f };
            for (auto& g : grains)
            {
                if (! g.active) continue;

                const float w  = Patterns::grainWindow (gWin, (float) g.age / (float) g.len);
                const int   j0 = (int) g.pos;
                const float jf = (float) (g.pos - (double) j0);
                const int   j1 = (j0 + 1 >= gSize) ? 0 : j0 + 1;

                for (int ch = 0; ch < nCh; ++ch)
                {
                    const float a  = grainBuf.getSample (ch, j0);
                    const float b  = grainBuf.getSample (ch, j1);
                    const float pg = (nCh == 2) ? (ch == 0 ? g.gL : g.gR) : 1.0f;
                    gOut[ch] += (a + jf * (b - a)) * w * pg;
                }

                g.pos += g.inc;
                if (g.pos >= (double) gSize) g.pos -= (double) gSize;
                else if (g.pos < 0.0)        g.pos += (double) gSize;
                if (++g.age >= g.len) g.active = false;
            }

            for (int ch = 0; ch < nCh; ++ch)
                wet[ch] = wet[ch] * (1.0f - gMix) + gOut[ch] * gNorm * gMix;
        }

        if (! gFreeze && ++grainWrite >= gSize) grainWrite = 0;

        // ---- LFO / несущие ----
        specLfoPhase += specLfoInc;  specLfoPhase -= std::floor (specLfoPhase);
        sp.lfo = (float) specLfoPhase;

        float carrier = 0.0f;
        if (ringMix > 0.0005f)
        {
            ringLfoPhase += ringLfoInc;  ringLfoPhase -= std::floor (ringLfoPhase);
            const double inc = ringInc * std::pow (2.0, (double) ringFM * std::sin (6.283185307 * ringLfoPhase));
            ringPhase += inc;
            ringPhase -= std::floor (ringPhase);
            const float ph = (float) ringPhase;
            switch (ringShape)
            {
                case 1:  carrier = 1.0f - 4.0f * std::abs (ph - 0.5f);        break;   // Triangle
                case 2:  carrier = ph < 0.5f ? 1.0f : -1.0f;                  break;   // Square
                case 3:  carrier = 2.0f * ph - 1.0f;                          break;   // Saw
                default: carrier = std::sin (Patterns::twoPi * ph);           break;   // Sine
            }
        }

        // ---- Filter: модуляция частоты ----
        filtLfoPhase += fLfoInc;  filtLfoPhase -= std::floor (filtLfoPhase);
        if (fType != 0 && (i & 7) == 0)
        {
            const float modSig = fSrc == 1 ? 0.5f * std::sin (Patterns::twoPi * (float) filtLfoPhase)
                               : fSrc == 2 ? envNorm - 0.5f
                                           : smoothVol - 0.5f;
            const float fc = jlimit (20.0f, (float) (0.45 * sampleRate), cutoff * std::pow (2.0f, fMod * modSig * 6.0f));
            filterA.setCutoffFrequency (fc);
            filterB.setCutoffFrequency (fc);
        }

        // ---- Spectral -> Ring -> Distortion -> Filter ----
        for (int ch = 0; ch < nCh; ++ch)
        {
            float x = spec[(size_t) ch].process (wet[ch], sp, rng);   // всегда: постоянная латентность

            if (ringMix > 0.0005f)
            {
                const float y = ringAM ? x * (0.5f + 0.5f * carrier) : x * carrier;
                x = x * (1.0f - ringMix) + y * ringMix;
            }

            if (dp.type != 0 && dMix > 0.0005f)
            {
                const float y = distProcess (dp, x, dist[(size_t) ch]);
                x = x * (1.0f - dMix) + y * dMix;
            }

            if (fType != 0)
            {
                if (fDrive > 0.001f)
                {
                    const float g = 1.0f + fDrive * 6.0f;
                    x = std::tanh (x * g) / std::sqrt (g);
                }
                x = filterA.processSample (ch, x);
                if (f24) x = filterB.processSample (ch, x);
            }

            wet[ch] = x;
        }

        // ---- Delay ----
        if (dlyMix > 0.0005f)
        {
            float oL = 0.0f, oR = 0.0f;
            dly.process (wet[0], nCh > 1 ? wet[1] : 0.0f, nCh > 1, dyp, envNorm, oL, oR);
            wet[0] += oL * dlyMix;
            if (nCh > 1) wet[1] += oR * dlyMix;
        }

        // ---- Reverb ----
        if (revMix > 0.0005f)
        {
            float rin[2] = { wet[0], nCh > 1 ? wet[1] : wet[0] };

            if (revPreSamp > 0)
            {
                for (int c = 0; c < 2; ++c)
                {
                    auto& pb = preBuf[(size_t) c];
                    pb[(size_t) prePos] = rin[c];
                    int rpos = prePos - jmin (revPreSamp, preSize - 2);
                    if (rpos < 0) rpos += preSize;
                    rin[c] = pb[(size_t) rpos];
                }
                if (++prePos >= preSize) prePos = 0;
            }

            if (nCh == 2) reverb.processStereo (&rin[0], &rin[1], 1);
            else          reverb.processMono (&rin[0], 1);

            const float rg = revMix * (1.0f - revDuck * envNorm);
            for (int ch = 0; ch < nCh; ++ch)
                wet[ch] += rin[ch] * rg;
        }

        // ---- Master: сухой сигнал с той же латентностью ----
        for (int ch = 0; ch < nCh; ++ch)
        {
            auto& db = dryBuf[(size_t) ch];
            db[(size_t) dryPos] = dryIn[ch];
            const float dryDelayed = db[(size_t) ((dryPos - SpectralChannel::N) & dryMask)];

            const float o = bypass ? dryDelayed : (dryDelayed * (1.0f - mix) + wet[ch] * mix) * outGain;
            buffer.setSample (ch, i, o);
            pkOut[ch] = jmax (pkOut[ch], std::abs (o));
        }
        dryPos = (dryPos + 1) & dryMask;

        {   // осциллограф/анализатор в GUI
            const float mono = nCh > 1 ? 0.5f * (buffer.getSample (0, i) + buffer.getSample (1, i)) : buffer.getSample (0, i);
            const int sp = scopePos.load (std::memory_order_relaxed);
            scope[(size_t) (sp & (scopeSize - 1))].store (mono, std::memory_order_relaxed);
            scopePos.store (sp + 1, std::memory_order_relaxed);
        }

        if (++writePos >= histSize) writePos = 0;
        phase += phaseInc;
        if (phase >= 1.0) phase -= 1.0;
    }

    for (int ch = 0; ch < nCh; ++ch)
    {
        auto upd = [] (std::atomic<float>& a, float val) { float cur = a.load(); while (cur < val && ! a.compare_exchange_weak (cur, val)) {} };
        upd (peakIn[(size_t) ch], pkIn[ch]);
        upd (peakOut[(size_t) ch], pkOut[ch]);
    }

    displayPhase.store ((float) phase);
    displaySlPhase.store ((float) (slBeatPos - std::floor (slBeatPos)));
}

//==============================================================================
//  Пресеты и рандомизация
//==============================================================================
int BeatMorphProcessor::getNumPrograms() { return (int) factoryPresets().size(); }

const String BeatMorphProcessor::getProgramName (int index)
{
    const auto& ps = factoryPresets();
    if (index < 0 || index >= (int) ps.size()) return {};
    const auto& pr = ps[(size_t) index];
    return String (pr.category) + " / " + pr.name;
}

void BeatMorphProcessor::setParam (const String& id, float realValue)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (realValue));
}

void BeatMorphProcessor::resetParametersToDefault()
{
    const bool keepMix = apvts.getRawParameterValue ("mixLock")->load() > 0.5f;
    const bool keepOut = apvts.getRawParameterValue ("outLock")->load() > 0.5f;

    for (auto* p : getParameters())
    {
        if (auto* withId = dynamic_cast<AudioProcessorParameterWithID*> (p))
        {
            const String& id = withId->paramID;
            if (id == "mixLock" || id == "outLock" || id == "bypass") continue;
            if (keepMix && id == "mix") continue;
            if (keepOut && id == "out") continue;
        }
        p->setValueNotifyingHost (p->getDefaultValue());
    }
}

void BeatMorphProcessor::setCurrentProgram (int index)
{
    const auto& ps = factoryPresets();
    if (index < 0 || index >= (int) ps.size()) return;

    currentProgram = index;
    randomActive = false;
    resetParametersToDefault();
    const bool keepMix = apvts.getRawParameterValue ("mixLock")->load() > 0.5f;
    const bool keepOut = apvts.getRawParameterValue ("outLock")->load() > 0.5f;
    clearModMatrix();
    userPresetName = {};
    for (const auto& kv : ps[(size_t) index].values)
    {
        const char* k = kv.first;
        if (k[0] == 'm' && k[1] >= '1' && k[1] <= '4' && k[2] == '.')       // назначение модуляции: m<LFO>.<параметр>
        {
            const int idx = Patterns::modIndex (k + 3);
            if (idx >= 0) setModAmount (k[1] - '1', idx, kv.second);
            continue;
        }
        if (keepMix && std::strcmp (k, "mix") == 0) continue;
        if (keepOut && std::strcmp (k, "out") == 0) continue;
        setParam (k, kv.second);
    }
}

void BeatMorphProcessor::randomize()
{
    resetParametersToDefault();
    randomActive = true;

    auto chance = [this] (float prob) { return rng.nextFloat() < prob; };
    auto uni    = [this] (float lo, float hi) { return lo + (hi - lo) * rng.nextFloat(); };
    auto pick   = [this] (int count) { return (float) rng.nextInt (count); };

    // Rhythm
    setParam ("length",   pick (3));
    setParam ("timeType", chance (0.60f) ? (float) (1 + rng.nextInt (11)) : 0.0f);
    setParam ("timeDiv",  pick (8));
    setParam ("volType",  chance (0.75f) ? (float) (1 + rng.nextInt (12)) : 0.0f);
    setParam ("volDiv",   (float) (1 + rng.nextInt (7)));
    setParam ("volDepth", uni (0.6f, 1.0f));
    if (chance (0.25f)) setParam ("swing", uni (0.1f, 0.35f));

    // Slicer
    if (chance (0.30f))
    {
        setParam ("slType",   (float) (1 + rng.nextInt (10)));
        setParam ("slSteps",  (float) (2 + rng.nextInt (5)));
        setParam ("slLength", (float) (1 + rng.nextInt (2)));
        setParam ("slSeed",   pick (64));
        setParam ("slEvolve", chance (0.5f) ? 1.0f : 0.0f);
        setParam ("slChance", uni (0.6f, 1.0f));
        setParam ("slReverse", chance (0.4f) ? uni (0.1f, 0.5f) : 0.0f);
        setParam ("slMute",   chance (0.3f) ? uni (0.1f, 0.3f) : 0.0f);
    }

    if (apvts.getRawParameterValue ("timeType")->load() < 0.5f
        && apvts.getRawParameterValue ("volType")->load() < 0.5f
        && apvts.getRawParameterValue ("slType")->load() < 0.5f)
        setParam ("volType", 1.0f);

    // Filter
    if (chance (0.40f))
    {
        setParam ("filterType",  (float) (1 + rng.nextInt (3)));
        setParam ("filterSlope", chance (0.4f) ? 1.0f : 0.0f);
        setParam ("cutoff",      200.0f * std::pow (2.0f, uni (0.0f, 5.0f)));
        setParam ("res",         uni (0.7f, 4.0f));
        setParam ("filterSrc",   pick (3));
        setParam ("filterMod",   uni (-0.8f, 0.8f));
        setParam ("filterRate",  uni (0.2f, 4.0f));
        setParam ("filterDrive", chance (0.3f) ? uni (0.1f, 0.5f) : 0.0f);
    }

    // Distortion
    if (chance (0.40f))
    {
        setParam ("distType", (float) (1 + rng.nextInt (7)));
        setParam ("drive",    uni (0.2f, 0.7f));
        setParam ("distMix",  uni (0.5f, 1.0f));
        setParam ("distBias", chance (0.3f) ? uni (-0.4f, 0.4f) : 0.0f);
        setParam ("distBits", (float) (5 + rng.nextInt (6)));
        setParam ("distRate", (float) (1 + rng.nextInt (8)));
    }

    // Granular
    if (chance (0.35f))
    {
        static const float pitches[] = { -12.0f, -7.0f, -5.0f, 0.0f, 0.0f, 5.0f, 7.0f, 12.0f };
        setParam ("grainMix",       uni (0.4f, 0.9f));
        setParam ("grainSize",      uni (30.0f, 280.0f));
        setParam ("grainSizeRand",  chance (0.4f) ? uni (0.2f, 0.8f) : 0.0f);
        setParam ("grainDensity",   uni (6.0f, 46.0f));
        setParam ("grainPitch",     pitches[rng.nextInt (8)]);
        setParam ("grainPitchRand", chance (0.4f) ? uni (2.0f, 12.0f) : 0.0f);
        setParam ("grainQuant",     (float) (1 + rng.nextInt (5)));
        setParam ("grainSpread",    uni (0.0f, 500.0f));
        setParam ("grainScan",      chance (0.3f) ? uni (-0.6f, 0.6f) : 0.0f);
        setParam ("grainWidth",     uni (0.2f, 1.0f));
        setParam ("grainRev",       uni (0.0f, 0.5f));
        setParam ("grainWindow",    pick (4));
    }

    // Spectral
    if (chance (0.35f))
    {
        const int t = 1 + rng.nextInt (16);
        setParam ("specType",   (float) t);
        setParam ("specHold",   t == 1 ? 1.0f : 0.0f);
        setParam ("specAmount", uni (0.5f, 1.0f));
        setParam ("specTone",   uni (0.15f, 0.85f));
        setParam ("specPitch",  uni (-12.0f, 12.0f));
        setParam ("specMotion", chance (0.4f) ? uni (0.3f, 1.0f) : 0.0f);
        setParam ("specRate",   uni (0.15f, 2.0f));
        setParam ("specMix",    uni (0.5f, 1.0f));
    }

    // Ring mod
    if (chance (0.15f))
    {
        setParam ("ringShape", pick (4));
        setParam ("ringMode",  chance (0.3f) ? 1.0f : 0.0f);
        setParam ("ringFreq",  30.0f * std::pow (2.0f, uni (0.0f, 6.0f)));
        setParam ("ringFM",    chance (0.4f) ? uni (0.2f, 0.8f) : 0.0f);
        setParam ("ringMix",   uni (0.3f, 0.9f));
    }

    // Delay
    if (chance (0.30f))
    {
        setParam ("dlySync",     (float) (1 + rng.nextInt (9)));
        setParam ("dlyMode",     pick (3));
        setParam ("dlyFeedback", uni (0.25f, 0.7f));
        setParam ("dlyTone",     uni (2500.0f, 12000.0f));
        setParam ("dlyMix",      uni (0.2f, 0.5f));
    }

    // Reverb
    if (chance (0.35f))
    {
        setParam ("revRoom", uni (0.3f, 0.9f));
        setParam ("revDamp", uni (0.3f, 0.7f));
        setParam ("revMix",  uni (0.15f, 0.5f));
    }

    // LFO: 1-2 генератора, каждый на 1-3 случайные цели (кроме Master)
    clearModMatrix();
    userPresetName = {};
    if (chance (0.6f))
    {
        const int count = 1 + rng.nextInt (2);
        for (int l = 0; l < count; ++l)
        {
            setParam (lfoIds[l][0], pick (7));
            setParam (lfoIds[l][1], chance (0.6f) ? (float) (1 + rng.nextInt (9)) : 0.0f);
            setParam (lfoIds[l][2], uni (0.1f, 6.0f));
            setParam (lfoIds[l][3], 1.0f);

            const int targets = 1 + rng.nextInt (3);
            for (int t = 0; t < targets; ++t)
                setModAmount (l, rng.nextInt (Patterns::numModParams - 2),
                              (chance (0.5f) ? 1.0f : -1.0f) * uni (0.12f, 0.45f));
        }
    }

    if (apvts.getRawParameterValue ("mixLock")->load() < 0.5f)
        setParam ("mix", uni (0.75f, 1.0f));
}

//==============================================================================
AudioProcessorEditor* BeatMorphProcessor::createEditor() { return new BeatMorphEditor (*this); }

void BeatMorphProcessor::clearModMatrix (int lfo)
{
    for (int k = 0; k < Patterns::numModParams; ++k)
        for (int l = 0; l < Patterns::numLfo; ++l)
            if (lfo < 0 || lfo == l)
                modAmount[(size_t) k][(size_t) l].store (0.0f);
}

void BeatMorphProcessor::getStateInformation (MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
    {
        auto* mm = xml->createNewChildElement ("MODMATRIX");
        for (int k = 0; k < Patterns::numModParams; ++k)
            for (int l = 0; l < Patterns::numLfo; ++l)
            {
                const float a = getModAmount (l, k);
                if (std::abs (a) > 1.0e-6f)
                {
                    auto* e = mm->createNewChildElement ("M");
                    e->setAttribute ("lfo", l);
                    e->setAttribute ("id", Patterns::modParamTable[k].id);
                    e->setAttribute ("amt", (double) a);
                }
            }
        copyXmlToBinary (*xml, dest);
    }
}

void BeatMorphProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            clearModMatrix();
            if (auto* mm = xml->getChildByName ("MODMATRIX"))
            {
                for (auto* e : mm->getChildWithTagNameIterator ("M"))
                {
                    const int l = e->getIntAttribute ("lfo", -1);
                    const int k = Patterns::modIndex (e->getStringAttribute ("id").toRawUTF8());
                    if (l >= 0 && l < Patterns::numLfo && k >= 0)
                        setModAmount (l, k, (float) e->getDoubleAttribute ("amt"));
                }
                xml->removeChildElement (mm, true);
            }
            apvts.replaceState (ValueTree::fromXml (*xml));
        }
}

//==============================================================================
//  Пользовательские пресеты: папка User
//==============================================================================
File BeatMorphProcessor::getUserPresetDir()
{
    return File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile ("BeatMorph").getChildFile ("User");
}

Array<File> BeatMorphProcessor::listUserPresets() const
{
    auto files = getUserPresetDir().findChildFiles (File::findFiles, false, "*.bmpreset");
    std::sort (files.begin(), files.end(), [] (const File& a, const File& b)
               { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
    return files;
}

bool BeatMorphProcessor::saveUserPreset (const String& rawName)
{
    const String name = rawName.trim();
    const String legal = File::createLegalFileName (name);
    if (legal.isEmpty()) return false;

    const auto dir = getUserPresetDir();
    if (! dir.createDirectory().wasOk()) return false;

    XmlElement root ("BMPRESET");
    root.setAttribute ("name", name);
    root.setAttribute ("version", 2);

    for (auto* prm : getParameters())
        if (auto* withId = dynamic_cast<AudioProcessorParameterWithID*> (prm))
        {
            auto* e = root.createNewChildElement ("P");
            e->setAttribute ("id", withId->paramID);
            e->setAttribute ("v", (double) prm->getValue());
        }

    auto* mm = root.createNewChildElement ("MODMATRIX");
    for (int k = 0; k < Patterns::numModParams; ++k)
        for (int l = 0; l < Patterns::numLfo; ++l)
        {
            const float a = getModAmount (l, k);
            if (std::abs (a) > 1.0e-6f)
            {
                auto* e = mm->createNewChildElement ("M");
                e->setAttribute ("lfo", l);
                e->setAttribute ("id", Patterns::modParamTable[k].id);
                e->setAttribute ("amt", (double) a);
            }
        }

    if (! root.writeTo (dir.getChildFile (legal + ".bmpreset"))) return false;

    userPresetName = name;
    currentProgram = -1;
    randomActive = false;
    return true;
}

bool BeatMorphProcessor::loadUserPreset (const File& file)
{
    auto xml = XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("BMPRESET")) return false;

    const bool keepMix = apvts.getRawParameterValue ("mixLock")->load() > 0.5f;
    const bool keepOut = apvts.getRawParameterValue ("outLock")->load() > 0.5f;

    resetParametersToDefault();
    for (auto* e : xml->getChildWithTagNameIterator ("P"))
    {
        const String id = e->getStringAttribute ("id");
        if (id == "mixLock" || id == "outLock" || id == "bypass") continue;
        if (keepMix && id == "mix") continue;
        if (keepOut && id == "out") continue;
        if (auto* prm = apvts.getParameter (id))
            prm->setValueNotifyingHost (jlimit (0.0f, 1.0f, (float) e->getDoubleAttribute ("v")));
    }

    clearModMatrix();
    if (auto* mm = xml->getChildByName ("MODMATRIX"))
        for (auto* e : mm->getChildWithTagNameIterator ("M"))
        {
            const int l = e->getIntAttribute ("lfo", -1);
            const int k = Patterns::modIndex (e->getStringAttribute ("id").toRawUTF8());
            if (l >= 0 && l < Patterns::numLfo && k >= 0)
                setModAmount (l, k, (float) e->getDoubleAttribute ("amt"));
        }

    userPresetName = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
    currentProgram = -1;
    randomActive = false;
    return true;
}

bool BeatMorphProcessor::deleteUserPreset (const File& file)
{
    return file.existsAsFile() && file.deleteFile();
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BeatMorphProcessor(); }
