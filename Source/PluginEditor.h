#pragma once
#include "PluginProcessor.h"
#include "Look.h"
#include "Panels.h"

//==============================================================================
// LCD-окна с кривыми: 0 = Time, 1 = Volume, 2 = Slicer
class CurveView : public juce::Component
{
public:
    CurveView (BeatMorphProcessor& p, int viewKind) : proc (p), kind (viewKind) {}
    void paint (juce::Graphics& g) override;

private:
    BeatMorphProcessor& proc;
    int kind;
};

//==============================================================================
// Осциллограф / анализатор спектра (клик по окну переключает режим)
class AnalyzerView : public juce::Component
{
public:
    explicit AnalyzerView (BeatMorphProcessor& p);
    void tick();
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override { mode = 1 - mode; repaint(); }

private:
    BeatMorphProcessor& proc;
    int mode = 1;
    juce::dsp::FFT fft { 10 };
    std::array<float, 2048> fftData {};
    std::array<float, 1024> window {}, block {};
    std::array<float, 64>   bars {}, peaks {};
    std::array<float, 512>  wave {};
};

//==============================================================================
class VuView : public juce::Component
{
public:
    explicit VuView (const juce::String& t) : tag (t) {}
    void paint (juce::Graphics& g) override { Hw::vuMeter (g, getLocalBounds().toFloat().reduced (2.0f), needle, tag); }
    float needle = 0.0f;

private:
    juce::String tag;
};

//==============================================================================
class ReelView : public juce::Component
{
public:
    void tick (float speed)
    {
        angle += speed;
        progress += speed * 0.002f;
        if (progress > 1.0f) progress -= 1.0f;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        const float d = juce::jmin (r.getWidth() * 0.5f, r.getHeight());
        const juce::Rectangle<float> left (r.getX(), r.getCentreY() - d * 0.5f, d, d);
        const juce::Rectangle<float> right (r.getRight() - d, r.getCentreY() - d * 0.5f, d, d);

        g.setColour (juce::Colour (0xff3a2819));
        g.drawLine (left.getCentreX(), left.getBottom() - 2.0f, right.getCentreX(), right.getBottom() - 2.0f, 1.5f);

        Hw::reel (g, left,  angle, 0.95f - 0.55f * progress);
        Hw::reel (g, right, angle * 1.1f, 0.40f + 0.55f * progress);
    }

private:
    float angle = 0.0f, progress = 0.3f;
};

//==============================================================================
class StatusView : public juce::Component
{
public:
    struct Item { juce::String label; std::function<bool()> on; juce::Colour colour; };

    void add (Item i) { items.push_back (std::move (i)); }
    void paint (juce::Graphics& g) override;

private:
    std::vector<Item> items;
};

//==============================================================================
class TransportView : public juce::Component
{
public:
    void set (double b, double p, bool pl) { bpm = b; ppq = p; playing = pl; repaint(); }
    void paint (juce::Graphics& g) override;

private:
    double bpm = 120.0, ppq = 0.0;
    bool playing = false;
};

//==============================================================================
// LCD с названием пресета (клик открывает браузер)
class PresetLcd : public juce::Component
{
public:
    std::function<void()> onClick;

    PresetLcd() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void setInfo (const juce::String& l1, const juce::String& n) { if (l1 != line1 || n != name) { line1 = l1; name = n; repaint(); } }
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent&) override { if (onClick) onClick(); }

private:
    juce::String line1, name;
};

//==============================================================================
class BeatMorphEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit BeatMorphEditor (BeatMorphProcessor&);
    ~BeatMorphEditor() override { stopTimer(); setLookAndFeel (nullptr); }

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void showPage (int index);
    void renderPanel();
    void updatePresetLcd();
    void openBrowser (bool focusSave);
    void closeBrowser();
    void openAbout();
    void showModPopup (ModSlider&);
    bool anyModAssigned() const;
    float val (const char* id) const { return proc.apvts.getRawParameterValue (id)->load(); }

    using Combo  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using Knob   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using Toggle = juce::AudioProcessorValueTreeState::ButtonAttachment;

    BeatMorphLookAndFeel laf;           // объявлен первым — удаляется последним
    BeatMorphProcessor& proc;

    // ---- дисплеи ----
    CurveView timeView, volView, slicerView;
    TransportView transport;
    PresetLcd presetLcd;

    // ---- шапка ----
    juce::TextButton prevButton { "<" }, nextButton { ">" }, browseButton { "BROWSE" }, saveButton { "SAVE" },
                     randomButton { "RANDOM" }, aboutButton { "ABOUT" };
    ModSlider  mixSlider, outSlider;
    LockButton mixLock, outLock;
    KeyButton  bypassKey { "BYPASS", juce::Colour (0xffff4a3d) }, panicKey { "PANIC", juce::Colour (0xffffb02e) };

    // ---- мониторинг ----
    VuView vuIn { "IN" }, vuOut { "OUT" };
    AnalyzerView analyzer;
    ReelView reels;
    StatusView status;
    float needleIn = 0.0f, needleOut = 0.0f;
    int clipHold = 0;

    // ---- страницы модулей (0..8), страница LFO (9) ----
    juce::OwnedArray<ModulePage> pages;
    juce::OwnedArray<ModuleTab>  tabs;
    LfoPage lfoPage;
    juce::OwnedArray<juce::Component> diagrams;      // по одной на страницу модуля
    int currentPage = 0;

    // ---- контролы (создаются по таблице) ----
    struct KnobInfo { ModSlider* slider; int modIdx; bool modded; };
    std::vector<KnobInfo> knobInfos;

    juce::OwnedArray<ModSlider>          knobs;
    juce::OwnedArray<juce::ComboBox>     combos;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::Label>        labels;

    std::vector<std::unique_ptr<Combo>>  comboAtt;
    std::vector<std::unique_ptr<Knob>>   knobAtt;
    std::vector<std::unique_ptr<Toggle>> toggleAtt;

    // ---- оверлеи ----
    PresetBrowser browser;
    AboutPage about;

    // ---- геометрия ----
    juce::Rectangle<int> rectName, rectProgram, rectMaster, rectMonitor, rectBay, rectOverlay;
    juce::Image panelImage;
    int tickCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatMorphEditor)
};
