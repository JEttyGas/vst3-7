#pragma once
#include "PluginProcessor.h"
#include "Look.h"
#include "Presets.h"
#include "AboutText.h"
#include "Version.h"

//==============================================================================
//  Всплывающее окно назначения LFO (правый клик по ручке):
//  каждый из 4 LFO можно привязать к параметру с любой глубиной (-100..+100 %)
//==============================================================================
class ModPopup : public juce::Component
{
public:
    ModPopup (BeatMorphProcessor& p, int paramIdx) : proc (p), idx (paramIdx)
    {
        for (int l = 0; l < 4; ++l)
        {
            auto& s = sliders[(size_t) l];
            s.setSliderStyle (juce::Slider::LinearHorizontal);
            s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
            s.setRange (-1.0, 1.0, 0.01);
            s.setDoubleClickReturnValue (true, 0.0);
            s.setColour (juce::Slider::trackColourId, Theme::lfoColour (l));
            s.textFromValueFunction = [] (double v) { return juce::String (juce::roundToInt (v * 100.0)) + " %"; };
            s.valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue() / 100.0; };
            s.setValue (proc.getModAmount (l, idx), juce::dontSendNotification);
            s.updateText();
            s.onValueChange = [this, l] { proc.setModAmount (l, idx, (float) sliders[(size_t) l].getValue()); };
            addAndMakeVisible (s);
        }

        clearBtn.setButtonText ("CLEAR ALL");
        clearBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b2e26));
        clearBtn.onClick = [this] { for (auto& s : sliders) s.setValue (0.0, juce::sendNotificationSync); };
        addAndMakeVisible (clearBtn);

        setSize (340, 46 + 4 * 34 + 46);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat();
        g.setGradientFill (ColourGradient (Colour (0xff3c3e44), 0.0f, 0.0f, Colour (0xff202125), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        Hw::engraved (g, "LFO ASSIGN  -  " + String (Patterns::modParamTable[idx].name).toUpperCase(),
                      { 12.0f, 8.0f, r.getWidth() - 24.0f, 20.0f }, Theme::font (12.5f, true), Justification::centredLeft, Theme::gold);
        for (int l = 0; l < 4; ++l)
            Hw::engraved (g, "LFO " + String (l + 1), { 12.0f, 40.0f + (float) l * 34.0f, 54.0f, 28.0f },
                          Theme::font (12.0f, true), Justification::centredLeft, Theme::lfoColour (l));
    }

    void resized() override
    {
        for (int l = 0; l < 4; ++l)
            sliders[(size_t) l].setBounds (68, 40 + l * 34, getWidth() - 78, 30);
        clearBtn.setBounds (getWidth() - 134, getHeight() - 40, 122, 28);
    }

private:
    BeatMorphProcessor& proc;
    int idx;
    std::array<juce::Slider, 4> sliders;
    juce::TextButton clearBtn;
};

//==============================================================================
//  Страница LFO: 4 генератора. Назначения делаются правым кликом по любой ручке.
//==============================================================================
class LfoPage : public juce::Component
{
public:
    explicit LfoPage (BeatMorphProcessor& p) : proc (p)
    {
        using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;

        for (int l = 0; l < 4; ++l)
        {
            auto& u = units[(size_t) l];
            const juce::String pre = "lfo" + juce::String (l + 1);
            const auto col = Theme::lfoColour (l);

            u.shape.addItemList (Patterns::lfoShapeNames, 1);
            u.sync.addItemList (Patterns::lfoSyncNames, 1);
            u.shape.setColour (juce::ComboBox::focusedOutlineColourId, col);
            u.sync.setColour (juce::ComboBox::focusedOutlineColourId, col);

            for (auto* s : { &u.rate, &u.level })
            {
                s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 17);
                s->setColour (juce::Slider::rotarySliderFillColourId, col);
            }

            u.clear.setButtonText ("CLEAR ASSIGNMENTS");
            u.clear.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff5a3a2e));
            u.clear.onClick = [this, l] { proc.clearModMatrix (l); };

            addAndMakeVisible (u.shape);
            addAndMakeVisible (u.sync);
            addAndMakeVisible (u.rate);
            addAndMakeVisible (u.level);
            addAndMakeVisible (u.clear);

            cAtt.push_back (std::make_unique<ComboAtt> (proc.apvts, pre + "Shape", u.shape));
            cAtt.push_back (std::make_unique<ComboAtt> (proc.apvts, pre + "Sync", u.sync));
            sAtt.push_back (std::make_unique<SliderAtt> (proc.apvts, pre + "Rate", u.rate));
            sAtt.push_back (std::make_unique<SliderAtt> (proc.apvts, pre + "Level", u.level));
        }
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto val = [this] (const String& id) { return proc.apvts.getRawParameterValue (id)->load(); };

        for (int l = 0; l < 4; ++l)
        {
            auto& u = units[(size_t) l];
            const auto col = Theme::lfoColour (l);

            g.setColour (Colours::black.withAlpha (0.16f));
            g.fillRoundedRectangle (u.card.toFloat(), 8.0f);
            Hw::silkBox (g, u.card.toFloat(), "LFO " + String (l + 1), col);

            // осциллограф формы
            const auto sr = u.scope.toFloat();
            Hw::lcdWindow (g, sr, 5.0f);
            const int shape = (int) val ("lfo" + String (l + 1) + "Shape");
            const float level = val ("lfo" + String (l + 1) + "Level");
            const auto plot = sr.reduced (8.0f, 6.0f);
            const float cycles = shape >= 5 ? 3.0f : 2.0f;

            Path path;
            for (int i = 0; i <= 200; ++i)
            {
                const float t = (float) i / 200.0f * cycles * 0.9999f;
                const float cyc = std::floor (t);
                const float v = Patterns::lfoShape (shape, (double) (t - cyc), (int64_t) cyc, l) * level;
                const float x = plot.getX() + plot.getWidth() * (float) i / 200.0f;
                const float y = plot.getCentreY() - v * plot.getHeight() * 0.5f;
                if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
            }
            g.setColour (col.withAlpha (0.22f));
            g.strokePath (path, PathStrokeType (4.5f, PathStrokeType::curved, PathStrokeType::rounded));
            g.setColour (col);
            g.strokePath (path, PathStrokeType (1.6f, PathStrokeType::curved, PathStrokeType::rounded));

            const float ph = proc.lfoPhase[(size_t) l].load(), lv = proc.lfoValue[(size_t) l].load();
            g.setColour (Colours::white);
            g.fillEllipse (plot.getX() + plot.getWidth() * (ph / cycles) - 3.5f, plot.getCentreY() - lv * plot.getHeight() * 0.5f - 3.5f, 7.0f, 7.0f);

            // подписи органов управления
            const Font lf = Theme::font (10.5f, true);
            Hw::engraved (g, "SHAPE", u.shapeR.toFloat().translated (0.0f, -14.0f).withHeight (12.0f), lf, Justification::centredLeft);
            Hw::engraved (g, "SYNC",  u.syncR.toFloat().translated (0.0f, -14.0f).withHeight (12.0f), lf, Justification::centredLeft);
            Hw::engraved (g, "RATE",  u.rateR.toFloat().translated (0.0f, -14.0f).withHeight (12.0f), lf, Justification::centred);
            Hw::engraved (g, "LEVEL", u.levelR.toFloat().translated (0.0f, -14.0f).withHeight (12.0f), lf, Justification::centred);

            // список назначений
            const auto ar = u.assign.toFloat();
            Hw::lcdWindow (g, ar, 5.0f);
            StringArray lines;
            int more = 0;
            for (int k = 0; k < Patterns::numModParams; ++k)
            {
                const float a = proc.getModAmount (l, k);
                if (std::abs (a) < 1.0e-4f) continue;
                if (lines.size() < 5)
                    lines.add (String (Patterns::modParamTable[k].name) + (a > 0 ? "  +" : "  ") + String (roundToInt (a * 100.0f)) + "%");
                else ++more;
            }
            g.setFont (Theme::mono (11.0f, true));
            if (lines.isEmpty())
            {
                g.setColour (Theme::lcdDim);
                g.drawFittedText ("NO TARGETS\nRIGHT-CLICK ANY KNOB", u.assign.reduced (10, 6), Justification::centredLeft, 3, 1.0f);
            }
            else
            {
                g.setColour (col.brighter (0.3f));
                for (int i = 0; i < lines.size(); ++i)
                    g.drawText (lines[i], Rectangle<float> (ar.getX() + 10.0f, ar.getY() + 5.0f + (float) i * 12.5f, ar.getWidth() - 14.0f, 12.0f), Justification::centredLeft, true);
                if (more > 0)
                {
                    g.setColour (Theme::lcdDim);
                    g.drawText ("+ " + String (more) + " MORE", Rectangle<float> (ar.getX() + 10.0f, ar.getY() + 5.0f + 5.0f * 12.5f, ar.getWidth() - 14.0f, 12.0f), Justification::centredLeft);
                }
            }
        }
    }

    void resized() override
    {
        const int gap = 10;
        const int uw = (getWidth() - 3 * gap) / 4;
        for (int l = 0; l < 4; ++l)
        {
            auto& u = units[(size_t) l];
            u.card = { l * (uw + gap), 4, uw, getHeight() - 8 };
            auto in = u.card.reduced (12, 0);
            int y = u.card.getY() + 22;

            u.scope = { in.getX(), y, in.getWidth(), 52 };              y += 52 + 16;
            const int half = in.getWidth() / 2;
            u.shapeR = { in.getX(), y, half - 3, 26 };
            u.syncR  = { in.getX() + half + 3, y, half - 3, 26 };       y += 26 + 16;
            u.rateR  = { in.getX(), y, half, 92 };
            u.levelR = { in.getX() + half, y, half, 92 };               y += 92 + 6;
            u.assign = { in.getX(), y, in.getWidth(), u.card.getBottom() - y - 34 };

            u.shape.setBounds (u.shapeR);
            u.sync.setBounds (u.syncR);
            u.rate.setBounds (u.rateR.reduced (4, 0));
            u.level.setBounds (u.levelR.reduced (4, 0));
            u.clear.setBounds (in.getX(), u.card.getBottom() - 30, in.getWidth(), 24);
        }
    }

    void tick() { repaint(); }

private:
    struct Unit
    {
        juce::ComboBox shape, sync;
        juce::Slider rate, level;
        juce::TextButton clear;
        juce::Rectangle<int> card, scope, assign, shapeR, syncR, rateR, levelR;
    };

    BeatMorphProcessor& proc;
    std::array<Unit, 4> units;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cAtt;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtt;
};

//==============================================================================
//  Браузер пресетов: папки слева, пресеты по центру, сохранение своих справа
//==============================================================================
class PresetBrowser : public juce::Component, private juce::Timer
{
public:
    std::function<void()> onClose, onChanged;

    explicit PresetBrowser (BeatMorphProcessor& p) : proc (p)
    {
        rebuildFolders();

        folderList.setRowHeight (26);
        presetList.setRowHeight (24);
        folderList.setOutlineThickness (0);
        presetList.setOutlineThickness (0);
        addAndMakeVisible (folderList);
        addAndMakeVisible (presetList);

        closeBtn.setButtonText ("CLOSE");
        closeBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b2e26));
        closeBtn.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible (closeBtn);

        nameEditor.setFont (Theme::mono (14.0f, true));
        nameEditor.setTextToShowWhenEmpty ("preset name...", Theme::lcdDim);
        nameEditor.setSelectAllWhenFocused (true);
        nameEditor.setIndents (8, 6);
        nameEditor.onReturnKey = [this] { doSave(); };
        addAndMakeVisible (nameEditor);

        saveBtn.setButtonText ("SAVE TO USER");
        saveBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2f6a45));
        saveBtn.onClick = [this] { doSave(); };
        addAndMakeVisible (saveBtn);

        deleteBtn.setButtonText ("DELETE");
        deleteBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b2e26));
        deleteBtn.onClick = [this] { doDelete(); };
        addAndMakeVisible (deleteBtn);

        folderBtn.setButtonText ("OPEN USER FOLDER");
        folderBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4a4c54));
        folderBtn.onClick = [this]
        {
            const auto dir = BeatMorphProcessor::getUserPresetDir();
            dir.createDirectory();
            dir.revealToUser();
        };
        addAndMakeVisible (folderBtn);

        setVisible (false);
    }

    // открыть (и обновить списки). focusSave = сразу на папку User с полем имени
    void open (bool focusSave)
    {
        folderList.updateContent();
        showFolder (focusSave ? 1 : currentFolder);
        nameEditor.setText (currentDisplayName(), false);
        status = {};
        setVisible (true);
        toFront (false);
        if (focusSave) { nameEditor.grabKeyboardFocus(); nameEditor.selectAll(); }
    }

    // стрелки < > в шапке: листать пресеты текущей папки
    void step (int dir)
    {
        if (entries.empty()) showFolder (currentFolder);
        if (entries.empty()) return;

        const int n = (int) entries.size();
        int pos = indexOfCurrent();
        if (pos < 0) pos = dir > 0 ? -1 : 0;
        pos = (pos + dir + n) % n;
        loadEntry (pos);
        presetList.selectRow (pos);
        presetList.scrollToEnsureRowIsOnscreen (pos);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat();
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (0.0f, 3.0f), 10.0f);
        g.setGradientFill (ColourGradient (Colour (0xff383a40), 0.0f, 0.0f, Colour (0xff1d1e22), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (Theme::silkDim.withAlpha (0.7f));
        g.drawRoundedRectangle (r.reduced (1.0f), 10.0f, 1.2f);

        Hw::engraved (g, "PRESET BROWSER", { 18.0f, 8.0f, 400.0f, 26.0f }, Theme::font (19.0f, true), Justification::centredLeft, Theme::gold);
        Hw::engraved (g, "FOLDERS", folderLcd.toFloat().translated (0.0f, -15.0f).withHeight (13.0f), Theme::font (10.5f, true), Justification::centredLeft);
        Hw::engraved (g, "PRESETS", presetLcd.toFloat().translated (0.0f, -15.0f).withHeight (13.0f), Theme::font (10.5f, true), Justification::centredLeft);
        Hw::engraved (g, "SAVE YOUR OWN", nameLabelR.toFloat(), Theme::font (10.5f, true), Justification::centredLeft);

        Hw::lcdWindow (g, folderLcd.toFloat(), 5.0f);
        Hw::lcdWindow (g, presetLcd.toFloat(), 5.0f);
        Hw::lcdWindow (g, statusR.toFloat(), 5.0f);

        g.setFont (Theme::mono (11.0f, true));
        g.setColour (status.startsWith ("SAVE FAILED") || status.startsWith ("LOAD FAILED") ? Theme::red : Theme::amber);
        g.drawFittedText (status.isEmpty() ? "CLICK A PRESET TO LOAD IT" : status, statusR.reduced (10, 4), Justification::centredLeft, 3, 1.0f);

        g.setColour (Theme::silkDim);
        g.setFont (Theme::font (10.5f));
        g.drawFittedText ("Your presets are stored as files in the User folder and keep all knobs and LFO assignments.",
                          noteR, Justification::topLeft, 4, 1.0f);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (14, 10);
        auto top = a.removeFromTop (34);
        closeBtn.setBounds (top.removeFromRight (100).reduced (0, 2));
        a.removeFromTop (22);

        auto left = a.removeFromLeft (190);
        folderLcd = left;
        folderList.setBounds (left.reduced (4, 6));
        a.removeFromLeft (14);

        auto right = a.removeFromRight (290);
        presetLcd = a;
        presetList.setBounds (a.reduced (4, 6));

        right.removeFromLeft (14);
        nameLabelR = right.removeFromTop (16);
        right.removeFromTop (4);
        nameEditor.setBounds (right.removeFromTop (32));
        right.removeFromTop (8);
        saveBtn.setBounds (right.removeFromTop (32));
        right.removeFromTop (8);
        deleteBtn.setBounds (right.removeFromTop (32));
        right.removeFromTop (8);
        folderBtn.setBounds (right.removeFromTop (32));
        right.removeFromTop (12);
        statusR = right.removeFromTop (70);
        right.removeFromTop (8);
        noteR = right;
    }

private:
    struct Entry { juce::String label; int factoryIndex = -1; juce::File file; };

    struct FolderModel : juce::ListBoxModel
    {
        explicit FolderModel (PresetBrowser& o) : owner (o) {}
        int getNumRows() override { return owner.folders.size(); }
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override
        {
            if (row < 0 || row >= owner.folders.size()) return;
            owner.paintRow (g, w, h, selected, owner.folders[row].toUpperCase(), owner.folderColour (row));
        }
        void listBoxItemClicked (int row, const juce::MouseEvent&) override { owner.showFolder (row); }
        PresetBrowser& owner;
    };

    struct PresetModel : juce::ListBoxModel
    {
        explicit PresetModel (PresetBrowser& o) : owner (o) {}
        int getNumRows() override { return (int) owner.entries.size(); }
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override
        {
            if (row < 0 || row >= (int) owner.entries.size()) return;
            const auto& e = owner.entries[(size_t) row];
            owner.paintRow (g, w, h, selected, e.label.toUpperCase(), e.factoryIndex < 0 ? Theme::amber : Theme::lcdOn);
        }
        void listBoxItemClicked (int row, const juce::MouseEvent&) override { owner.loadEntry (row); }
        void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override { owner.loadEntry (row); if (owner.onClose) owner.onClose(); }
        void returnKeyPressed (int row) override { owner.loadEntry (row); }
        PresetBrowser& owner;
    };

    void paintRow (juce::Graphics& g, int w, int h, bool selected, const juce::String& text, juce::Colour c)
    {
        if (selected)
        {
            g.setColour (Theme::lcdDim.withAlpha (0.6f));
            g.fillRect (0, 0, w, h);
            g.setColour (Theme::lcdOn.withAlpha (0.35f));
            g.drawRect (0, 0, w, h, 1);
        }
        g.setColour (selected ? juce::Colours::white : c.withAlpha (0.92f));
        g.setFont (Theme::mono (13.0f, selected));
        g.drawText (text, 10, 0, w - 14, h, juce::Justification::centredLeft, true);
    }

    juce::Colour folderColour (int row) const
    {
        const auto& f = folders[row];
        if (f == "USER") return Theme::amber;
        if (f == "v3")   return juce::Colour (0xff6fe3ff);
        if (f == "v2")   return Theme::green;
        if (f == "ALL")  return Theme::silk;
        return Theme::lcdOn;
    }

    void rebuildFolders()
    {
        folders.clear();
        folders.add ("ALL");
        folders.add ("USER");
        folders.add ("v3");
        folders.add ("v2");
        for (const auto& pr : factoryPresets())
            if (juce::String (pr.category) != "v2" && juce::String (pr.category) != "v3")
                folders.addIfNotAlreadyThere (juce::String (pr.category));
    }

    void showFolder (int row)
    {
        using juce::String;
        currentFolder = juce::jlimit (0, folders.size() - 1, row);
        const String f = folders[currentFolder];

        entries.clear();
        const auto& ps = factoryPresets();
        if (f != "USER")
            for (int i = 0; i < (int) ps.size(); ++i)
                if (f == "ALL" || f == ps[(size_t) i].category)
                    entries.push_back ({ f == "ALL" ? String (ps[(size_t) i].category) + " / " + ps[(size_t) i].name
                                                    : String (ps[(size_t) i].name), i, {} });

        if (f == "USER" || f == "ALL")
            for (const auto& file : proc.listUserPresets())
                entries.push_back ({ String (f == "ALL" ? "USER / " : "") + file.getFileNameWithoutExtension(), -1, file });

        folderList.selectRow (currentFolder);
        presetList.updateContent();
        const int cur = indexOfCurrent();
        if (cur >= 0) { presetList.selectRow (cur); presetList.scrollToEnsureRowIsOnscreen (cur); }
        else          presetList.deselectAllRows();
        repaint();
    }

    int indexOfCurrent() const
    {
        if (proc.randomActive.load()) return -1;
        for (int i = 0; i < (int) entries.size(); ++i)
        {
            const auto& e = entries[(size_t) i];
            if (e.factoryIndex >= 0 && e.factoryIndex == proc.getFactoryIndex()) return i;
            if (e.factoryIndex < 0 && proc.getFactoryIndex() < 0 && proc.userPresetName.isNotEmpty()
                && e.file.getFileNameWithoutExtension() == juce::File::createLegalFileName (proc.userPresetName))
                return i;
        }
        return -1;
    }

    juce::String currentDisplayName() const
    {
        if (proc.userPresetName.isNotEmpty() && proc.getFactoryIndex() < 0) return proc.userPresetName;
        const auto& ps = factoryPresets();
        const int cur = proc.getFactoryIndex();
        if (! proc.randomActive.load() && cur >= 0 && cur < (int) ps.size()) return ps[(size_t) cur].name;
        return "My Preset";
    }

    void loadEntry (int row)
    {
        if (row < 0 || row >= (int) entries.size()) return;
        const auto& e = entries[(size_t) row];
        bool ok = true;
        if (e.factoryIndex >= 0) proc.setCurrentProgram (e.factoryIndex);
        else                     ok = proc.loadUserPreset (e.file);

        status = ok ? "LOADED: " + e.label.toUpperCase() : juce::String ("LOAD FAILED");
        if (ok && onChanged) onChanged();
        repaint();
    }

    void doSave()
    {
        const auto n = nameEditor.getText().trim();
        if (n.isEmpty()) { status = "ENTER A NAME FIRST"; repaint(); return; }

        if (proc.saveUserPreset (n))
        {
            status = "SAVED: " + n.toUpperCase();
            showFolder (1);
            if (onChanged) onChanged();
        }
        else status = "SAVE FAILED";
        repaint();
    }

    void doDelete()
    {
        const int row = presetList.getSelectedRow();
        if (row < 0 || row >= (int) entries.size() || entries[(size_t) row].factoryIndex >= 0)
        {
            status = "SELECT A USER PRESET TO DELETE";
            repaint();
            return;
        }

        if (! deleteArmed)
        {
            deleteArmed = true;
            deleteBtn.setButtonText ("CONFIRM?");
            startTimer (2500);
            return;
        }

        stopTimer();
        deleteArmed = false;
        deleteBtn.setButtonText ("DELETE");
        const auto name = entries[(size_t) row].label;
        status = proc.deleteUserPreset (entries[(size_t) row].file) ? "DELETED: " + name.toUpperCase() : juce::String ("DELETE FAILED");
        showFolder (currentFolder);
    }

    void timerCallback() override
    {
        stopTimer();
        deleteArmed = false;
        deleteBtn.setButtonText ("DELETE");
    }

    BeatMorphProcessor& proc;
    juce::StringArray folders;
    std::vector<Entry> entries;
    int currentFolder = 0;
    bool deleteArmed = false;
    juce::String status;

    FolderModel folderModel { *this };
    PresetModel presetModel { *this };
    juce::ListBox folderList { "folders", &folderModel };
    juce::ListBox presetList { "presets", &presetModel };

    juce::TextButton closeBtn, saveBtn, deleteBtn, folderBtn;
    juce::TextEditor nameEditor;
    juce::Rectangle<int> folderLcd, presetLcd, nameLabelR, statusR, noteR;
};

//==============================================================================
//  О плагине: автор, история создания, ссылка на Telegram
//==============================================================================
class AboutPage : public juce::Component
{
public:
    std::function<void()> onClose;

    AboutPage()
    {
        closeBtn.setButtonText ("CLOSE");
        closeBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff6b2e26));
        closeBtn.onClick = [this] { if (onClose) onClose(); };
        addAndMakeVisible (closeBtn);

        langBtn.setButtonText ("RU / EN");
        langBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff4a4c54));
        langBtn.onClick = [this] { russian = ! russian; repaint(); };
        addAndMakeVisible (langBtn);

        link.setFont (Theme::font (18.0f, true), false);
        link.setColour (juce::HyperlinkButton::textColourId, Theme::amber);
        addAndMakeVisible (link);

        setVisible (false);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat();
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (0.0f, 3.0f), 10.0f);
        g.setGradientFill (ColourGradient (Colour (0xff383a40), 0.0f, 0.0f, Colour (0xff1d1e22), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (Theme::silkDim.withAlpha (0.7f));
        g.drawRoundedRectangle (r.reduced (1.0f), 10.0f, 1.2f);

        // латунная табличка
        Hw::brassPlate (g, plateR.toFloat());
        const auto pr = plateR.toFloat();
        Hw::engravedDark (g, "BEATMORPH  BM-900", pr.reduced (26.0f, 6.0f).withHeight (38.0f), Theme::font (32.0f, true), Justification::centredLeft);
        Hw::engravedDark (g, String ("VERSION ") + BMVersion::string, pr.reduced (26.0f, 6.0f).withHeight (38.0f), Theme::font (15.0f, true), Justification::centredRight);
        const String head = russian ? AboutText::headRu() : AboutText::headEn();
        Hw::engravedDark (g, head.upToFirstOccurrenceOf ("\n", false, false), pr.reduced (22.0f, 0.0f).withTrimmedTop (46.0f).withHeight (16.0f), Theme::font (12.5f, true), Justification::centredLeft);
        Hw::engravedDark (g, head.fromFirstOccurrenceOf ("\n", false, false), pr.reduced (22.0f, 0.0f).withTrimmedTop (62.0f).withHeight (18.0f), Theme::font (14.0f, true), Justification::centredLeft);
        for (float sx : { pr.getX() + 9.0f, pr.getRight() - 9.0f })
            Hw::screw (g, { sx, pr.getCentreY() }, 4.0f, sx);

        // история
        Hw::lcdWindow (g, storyR.toFloat(), 6.0f);
        g.setFont (Theme::mono (14.0f, true));
        g.setColour (Theme::amber);
        g.drawText (russian ? AboutText::titleRu() : AboutText::titleEn(), storyR.reduced (18, 10).withHeight (20), Justification::centredLeft);
        g.setFont (Theme::mono (13.0f, true));
        g.setColour (Theme::lcdOn);
        g.drawFittedText (russian ? AboutText::storyRu() : AboutText::storyEn(), storyR.reduced (18, 10).withTrimmedTop (28), Justification::topLeft, 40, 1.0f);

        // «бесплатно»
        Hw::lcdWindow (g, footR.toFloat(), 6.0f);
        g.setFont (Theme::mono (13.0f, true));
        g.setColour (Theme::green);
        g.drawFittedText (russian ? AboutText::footRu() : AboutText::footEn(), footR.reduced (16, 6), Justification::centredLeft, 3, 1.0f);

        Hw::engraved (g, "TELEGRAM:", telegramLabelR.toFloat(), Theme::font (12.0f, true), Justification::centredLeft, Theme::silk);
    }

    void resized() override
    {
        auto a = getLocalBounds().reduced (14, 12);
        auto bottom = a.removeFromBottom (36);
        closeBtn.setBounds (bottom.removeFromRight (100));
        bottom.removeFromRight (8);
        langBtn.setBounds (bottom.removeFromRight (100));
        telegramLabelR = bottom.removeFromLeft (84);
        link.setBounds (bottom.removeFromLeft (260));

        a.removeFromBottom (8);
        footR = a.removeFromBottom (52);
        a.removeFromBottom (10);
        plateR = a.removeFromTop (88);
        a.removeFromTop (12);
        storyR = a;
    }

private:
    bool russian = true;
    juce::TextButton closeBtn, langBtn;
    juce::HyperlinkButton link { "t.me/jetty808gas", juce::URL ("https://t.me/jetty808gas") };
    juce::Rectangle<int> plateR, storyR, footR, telegramLabelR;
};

//==============================================================================
//  Диаграммы модулей: заполняют место под карточками и показывают, что делает модуль
//  kind: 0 Rhythm (groove), 2 Granular (облако), 4 Dist (передаточная кривая),
//        5 Filter (АЧХ), 6 Ring (несущая), 7 Delay (отводы), 8 Reverb (спад)
//==============================================================================
class ModuleDiagram : public juce::Component
{
public:
    ModuleDiagram (BeatMorphProcessor& p, int moduleKind, juce::Colour acc) : proc (p), kind (moduleKind), accent (acc) {}

    void tick() { t += 0.033f; repaint(); }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat().reduced (3.0f);
        Hw::lcdWindow (g, r, 6.0f);

        auto val = [this] (const char* id) { return proc.apvts.getRawParameterValue (id)->load(); };
        const auto plot = r.reduced (16.0f, 12.0f).withTrimmedTop (14.0f);
        const Colour c = accent;

        static const char* titles[9] = { "GROOVE MAP", "", "GRAIN CLOUD", "", "TRANSFER CURVE", "FREQUENCY RESPONSE",
                                         "CARRIER x INPUT", "ECHO TAPS", "DECAY ENVELOPE" };
        g.setColour (c.withAlpha (0.85f));
        g.setFont (Theme::mono (11.5f, true));
        g.drawText (titles[kind], r.reduced (14.0f, 6.0f).withHeight (14.0f), Justification::centredLeft);

        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (r.toNearestInt());

        auto glow = [&] (const Path& p)
        {
            g.setColour (c.withAlpha (0.22f));
            g.strokePath (p, PathStrokeType (5.0f, PathStrokeType::curved, PathStrokeType::rounded));
            g.setColour (c);
            g.strokePath (p, PathStrokeType (1.8f, PathStrokeType::curved, PathStrokeType::rounded));
        };

        switch (kind)
        {
            case 0: // Rhythm: положение и длина слайсов с учётом свинга и фазы
            {
                const int vType = (int) val ("volType");
                const int N = Patterns::sliceValues[(int) val ("volDiv")];
                const float swing = val ("swing"), shift = val ("shift"), depth = val ("volDepth");

                g.setColour (c.withAlpha (0.14f));
                for (int b = 0; b <= 4; ++b)
                    g.drawVerticalLine ((int) (plot.getX() + plot.getWidth() * (float) b / 4.0f), plot.getY(), plot.getBottom());

                for (int k = 0; k < N; ++k)
                {
                    const float s = (k % 2 == 0) ? (float) k : (float) (k - 1) + 1.0f + swing;
                    const float e = (k % 2 == 0) ? (float) (k + 1) + swing : (float) (k + 1);
                    float x0 = s / (float) N + shift, x1 = jmin (e / (float) N, 1.0f) + shift;
                    const float lvl = vType == 0 ? 1.0f : Patterns::volumeCurve (vType, ((float) k + 0.25f) / (float) N, N);
                    const float h = (0.12f + 0.88f * (1.0f - depth * (1.0f - lvl))) * plot.getHeight();

                    for (int wrap = 0; wrap < 2; ++wrap)
                    {
                        const float a = jlimit (0.0f, 1.0f, x0 - (float) wrap), bb = jlimit (0.0f, 1.0f, x1 - (float) wrap);
                        if (bb > a)
                        {
                            g.setColour (c.withAlpha (0.18f + 0.55f * lvl));
                            g.fillRect (plot.getX() + a * plot.getWidth() + 1.0f, plot.getBottom() - h, (bb - a) * plot.getWidth() - 2.0f, h);
                        }
                    }
                }
                g.setColour (Colours::white.withAlpha (0.85f));
                g.drawVerticalLine ((int) (plot.getX() + proc.displayPhase.load() * plot.getWidth()), plot.getY(), plot.getBottom());
                break;
            }
            case 2: // Granular: анимация облака зёрен
            {
                const float size = val ("grainSize"), dens = val ("grainDensity"), spread = val ("grainSpread");
                const float pitch = val ("grainPitch"), mixv = val ("grainMix");
                const bool freeze = val ("grainFreeze") > 0.5f;
                const int n = jlimit (8, 90, (int) (6.0f + dens * 1.4f));

                for (int i = 0; i < n; ++i)
                {
                    const float h0 = Patterns::hash01 (i * 3 + 1), h1 = Patterns::hash01 (i * 3 + 2), h2 = Patterns::hash01 (i * 3 + 3);
                    const float life = Patterns::frac (t * (0.25f + h2 * 0.5f) + h0);
                    const float env = std::sin (3.14159f * life);
                    const float x = plot.getX() + plot.getWidth() * Patterns::frac (h1 + (freeze ? 0.0f : life * 0.15f)) * (0.35f + 0.65f * jmin (1.0f, spread / 500.0f + 0.2f));
                    const float yBase = 0.5f - (pitch / 48.0f) + (h0 - 0.5f) * 0.5f;
                    const float y = plot.getY() + plot.getHeight() * jlimit (0.05f, 0.95f, yBase);
                    const float rad = (2.0f + size / 500.0f * 14.0f) * (0.3f + env);
                    g.setColour (c.withAlpha (0.10f + 0.65f * env * (0.3f + mixv)));
                    g.fillEllipse (x - rad, y - rad, rad * 2.0f, rad * 2.0f);
                }
                if (freeze)
                {
                    g.setColour (Theme::amber);
                    g.setFont (Theme::mono (12.0f, true));
                    g.drawText ("FROZEN", r.reduced (14.0f, 6.0f).withHeight (14.0f), Justification::centredRight);
                }
                break;
            }
            case 4: // Distortion: передаточная кривая
            {
                DistParams dp;
                dp.type = (int) val ("distType");
                dp.gain = std::pow (10.0f, val ("drive") * 2.0f);
                dp.comp = std::pow (dp.gain, -0.25f);
                dp.bias = val ("distBias");
                dp.levels = std::pow (2.0f, val ("distBits") - 1.0f);
                dp.holdN = 1;
                const float mixv = val ("distMix");

                g.setColour (c.withAlpha (0.15f));
                g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());
                g.drawVerticalLine ((int) plot.getCentreX(), plot.getY(), plot.getBottom());
                g.setColour (c.withAlpha (0.25f));
                g.drawLine (plot.getX(), plot.getBottom(), plot.getRight(), plot.getY(), 1.0f);

                Path p;
                for (int i = 0; i <= 200; ++i)
                {
                    const float x = -1.0f + 2.0f * (float) i / 200.0f;
                    DistState st;
                    float y = dp.type == 0 ? x : distProcess (dp, x, st);
                    y = x * (1.0f - mixv) + y * mixv;
                    y = jlimit (-1.2f, 1.2f, y);
                    const float px = plot.getX() + plot.getWidth() * (x * 0.5f + 0.5f);
                    const float py = plot.getCentreY() - y * plot.getHeight() * 0.45f;
                    if (i == 0) p.startNewSubPath (px, py); else p.lineTo (px, py);
                }
                glow (p);
                break;
            }
            case 5: // Filter: АЧХ
            {
                const int ft = (int) val ("filterType");
                const bool s24 = val ("filterSlope") > 0.5f;
                const float fc = val ("cutoff"), Q = val ("res");

                auto yOf = [&] (float db) { return plot.getBottom() - (jlimit (-48.0f, 24.0f, db) + 48.0f) / 72.0f * plot.getHeight(); };
                g.setColour (c.withAlpha (0.15f));
                g.drawHorizontalLine ((int) yOf (0.0f), plot.getX(), plot.getRight());
                for (float f : { 100.0f, 1000.0f, 10000.0f })
                    g.drawVerticalLine ((int) (plot.getX() + plot.getWidth() * std::log10 (f / 20.0f) / 3.0f), plot.getY(), plot.getBottom());

                Path p;
                for (int i = 0; i <= 240; ++i)
                {
                    const float f = 20.0f * std::pow (1000.0f, (float) i / 240.0f);
                    const float s = f / fc;
                    const float den = std::sqrt ((1.0f - s * s) * (1.0f - s * s) + (s / Q) * (s / Q));
                    float h = 1.0f;
                    if (ft == 1) h = 1.0f / den; else if (ft == 2) h = s * s / den; else if (ft == 3) h = (s / Q) / den;
                    if (ft != 0 && s24) h *= h;
                    const float db = 20.0f * std::log10 (jmax (h, 1.0e-4f));
                    const float x = plot.getX() + plot.getWidth() * (float) i / 240.0f;
                    if (i == 0) p.startNewSubPath (x, yOf (db)); else p.lineTo (x, yOf (db));
                }
                glow (p);

                if (ft != 0)
                {
                    const float fx = plot.getX() + plot.getWidth() * std::log10 (jlimit (20.0f, 20000.0f, fc) / 20.0f) / 3.0f;
                    g.setColour (Colours::white.withAlpha (0.6f));
                    g.drawVerticalLine ((int) fx, plot.getY(), plot.getBottom());
                }
                break;
            }
            case 6: // Ring: несущая и результат
            {
                const int shape = (int) val ("ringShape");
                const bool am = (int) val ("ringMode") == 1;
                Path car, prod;
                for (int i = 0; i <= 300; ++i)
                {
                    const float ph = Patterns::frac ((float) i / 300.0f * 2.0f);
                    float cv;
                    switch (shape)
                    {
                        case 1:  cv = 1.0f - 4.0f * std::abs (ph - 0.5f); break;
                        case 2:  cv = ph < 0.5f ? 1.0f : -1.0f; break;
                        case 3:  cv = 2.0f * ph - 1.0f; break;
                        default: cv = std::sin (Patterns::twoPi * ph); break;
                    }
                    const float in = std::sin (Patterns::twoPi * (float) i / 300.0f * 9.0f);
                    const float pv = am ? in * (0.5f + 0.5f * cv) : in * cv;
                    const float x = plot.getX() + plot.getWidth() * (float) i / 300.0f;
                    const float yc = plot.getCentreY() - cv * plot.getHeight() * 0.45f;
                    const float yp = plot.getCentreY() - pv * plot.getHeight() * 0.45f;
                    if (i == 0) { car.startNewSubPath (x, yc); prod.startNewSubPath (x, yp); }
                    else        { car.lineTo (x, yc); prod.lineTo (x, yp); }
                }
                g.setColour (Colours::white.withAlpha (0.28f));
                g.strokePath (car, PathStrokeType (1.2f));
                glow (prod);
                break;
            }
            case 7: // Delay: отводы
            {
                const int sync = (int) val ("dlySync");
                const double bpm = jmax (20.0, proc.hostBpm.load());
                const float beats = sync >= 10 ? (float) (val ("dlyFree") / (60000.0 / bpm)) : (float) Patterns::dlyBeats[sync];
                const float fb = jmin (0.95f, val ("dlyFeedback")), mixv = val ("dlyMix");
                const bool pingPong = (int) val ("dlyMode") == 1;

                g.setColour (c.withAlpha (0.12f));
                for (int b = 0; b <= 4; ++b)
                    g.drawVerticalLine ((int) (plot.getX() + plot.getWidth() * (float) b / 4.0f), plot.getY(), plot.getBottom());

                for (int k = 0; k <= 12; ++k)
                {
                    const float x = plot.getX() + plot.getWidth() * ((float) k * beats / 4.0f);
                    if (x > plot.getRight()) break;
                    const float amp = k == 0 ? 1.0f : std::pow (fb, (float) k);
                    const float h = (k == 0 ? 1.0f : amp * (0.25f + 0.75f * mixv)) * plot.getHeight() * 0.9f;
                    const bool up = ! pingPong || (k % 2 == 0);
                    g.setColour (k == 0 ? Colours::white.withAlpha (0.75f) : c.withAlpha (0.35f + 0.6f * amp));
                    if (up) g.fillRect (x - 2.5f, plot.getBottom() - h, 5.0f, h);
                    else    g.fillRect (x - 2.5f, plot.getY(), 5.0f, h);
                }
                break;
            }
            case 8: // Reverb: спад
            {
                const float room = val ("revRoom"), damp = val ("revDamp"), pre = val ("revPre");
                const bool freeze = val ("revFreeze") > 0.5f;
                const float T = 0.15f + room * 3.5f;
                const float preX = plot.getWidth() * jlimit (0.0f, 0.4f, pre / 600.0f);

                Path p, p2;
                for (int i = 0; i <= 240; ++i)
                {
                    const float tt = (float) i / 240.0f * 4.0f;
                    float env = freeze ? 0.85f : std::exp (-6.9f * tt / T);
                    float env2 = freeze ? 0.6f : std::exp (-6.9f * tt / (T * (1.0f - 0.6f * damp)));
                    const float x = plot.getX() + preX + (plot.getWidth() - preX) * (float) i / 240.0f;
                    if (i == 0) { p.startNewSubPath (x, plot.getBottom() - env * plot.getHeight()); p2.startNewSubPath (x, plot.getBottom() - env2 * plot.getHeight()); }
                    else        { p.lineTo (x, plot.getBottom() - env * plot.getHeight()); p2.lineTo (x, plot.getBottom() - env2 * plot.getHeight()); }
                }
                g.setColour (Colours::white.withAlpha (0.25f));
                g.strokePath (p2, PathStrokeType (1.2f));
                glow (p);
                g.setColour (Colours::white.withAlpha (0.8f));
                g.fillRect (plot.getX(), plot.getY(), 2.0f, plot.getHeight());
                break;
            }
            default: break;
        }
    }

private:
    BeatMorphProcessor& proc;
    int kind;
    juce::Colour accent;
    float t = 0.0f;
};
