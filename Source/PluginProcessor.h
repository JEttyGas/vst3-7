#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include "Patterns.h"
#include "Spectral.h"
#include "Delay.h"
#include "Dist.h"

class BeatMorphProcessor : public juce::AudioProcessor
{
public:
    BeatMorphProcessor();
    ~BeatMorphProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BeatMorph"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    // ---- фабричные пресеты ----
    int getNumPrograms() override;
    int getCurrentProgram() override { return juce::jmax (0, currentProgram); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}
    int getFactoryIndex() const { return currentProgram; }     // -1, если загружен пользовательский пресет

    /** Случайный «музыкальный» пресет. Вызывать из GUI-потока. */
    void randomize();

    // ---- пользовательские пресеты (папка User) ----
    static juce::File getUserPresetDir();
    juce::Array<juce::File> listUserPresets() const;
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::File& file);
    bool deleteUserPreset (const juce::File& file);
    juce::String userPresetName;                               // только из GUI-потока

    // ---- матрица модуляции: любой из 4 LFO -> любое число параметров ----
    float getModAmount (int lfo, int idx) const { return modAmount[(size_t) idx][(size_t) lfo].load(); }
    void  setModAmount (int lfo, int idx, float a) { modAmount[(size_t) idx][(size_t) lfo].store (a); }
    void  clearModMatrix (int lfo = -1);

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    /** Сбросить все буферы и хвосты (кнопка PANIC). */
    void panic() { panicFlag = true; }

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> displayPhase   { 0.0f };
    std::atomic<float> displaySlPhase { 0.0f };
    std::atomic<bool>  randomActive   { false };

    // ---- данные для приборов GUI ----
    std::atomic<double> hostBpm { 120.0 }, hostPpq { 0.0 };
    std::atomic<bool>   hostPlaying { false };
    std::array<std::atomic<float>, 2> peakIn {}, peakOut {};
    static constexpr int scopeSize = 4096;
    std::array<std::atomic<float>, scopeSize> scope {};
    std::atomic<int> scopePos { 0 };
    std::array<std::atomic<float>, Patterns::numLfo> lfoValue {}, lfoPhase {};      // текущее значение и фаза каждого LFO
    std::array<std::atomic<float>, Patterns::numModParams> modOffset {};            // суммарная модуляция каждого параметра

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void processChunk (juce::AudioBuffer<float>& buffer, int start, int len, double bpm, bool playing, double ppq);
    void doPanic();
    void resetParametersToDefault();
    void setParam (const juce::String& id, float realValue);

    int currentProgram = 0;
    std::atomic<bool> panicFlag { false };
    juce::String lfoIds[Patterns::numLfo][4];                  // Shape, Sync, Rate, Level
    double lfoAcc[Patterns::numLfo] {};
    std::array<std::array<std::atomic<float>, Patterns::numLfo>, Patterns::numModParams> modAmount {};

    // ---- Time/Volume ----
    juce::AudioBuffer<float> history;
    int    writePos = 0;
    double sampleRate = 44100.0;
    double freeBeats  = 0.0;
    float  smoothVol = 1.0f;
    double prevDelay = 0.0;
    int    fadeCounter = 0, fadeLen = 96;

    // ---- Slicer ----
    juce::AudioBuffer<float> slicerBuf;
    int    slWrite = 0;
    double slBeatPos = 0.0;
    std::array<int, 32> perm {};
    int      permType = -1, permN = -1;
    uint32_t permSeed = 0xffffffffu;

    // ---- Granular ----
    struct Grain
    {
        bool   active = false;
        double pos = 0.0, inc = 1.0;
        int    age = 0, len = 1;
        float  gL = 1.0f, gR = 1.0f;
    };
    static constexpr int maxGrains = 64;
    std::array<Grain, maxGrains> grains;
    juce::AudioBuffer<float> grainBuf;
    int    grainWrite = 0;
    double grainTimer = 0.0;
    double scanPhase = 0.0;
    juce::Random rng;

    // ---- Spectral ----
    std::array<SpectralChannel, 2> spec;
    double specLfoPhase = 0.0;
    static constexpr int dryMask = 2047;
    std::array<std::array<float, 2048>, 2> dryBuf {};
    int dryPos = 0;

    // ---- Ring / Dist / Filter / Delay / Reverb ----
    double ringPhase = 0.0, ringLfoPhase = 0.0, filtLfoPhase = 0.0;
    std::array<DistState, 2> dist;
    juce::dsp::StateVariableTPTFilter<float> filterA, filterB;
    StereoDelay dly;
    juce::Reverb reverb;
    std::array<std::vector<float>, 2> preBuf;
    int prePos = 0;

    float inEnv = 0.0f, envAtk = 0.01f, envRel = 0.001f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatMorphProcessor)
};
