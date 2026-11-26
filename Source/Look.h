#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cstring>
#include <functional>
#include <map>

//==============================================================================
//  Стиль «дорогой студийный процессор»: графитовая шлифованная панель, орех,
//  латунная табличка, алюминиевые ручки с накаткой, LCD, светодиоды.
//==============================================================================
namespace Theme
{
    inline const juce::Colour silk     { 0xffeee6d0 };   // сеточная печать (тёплый белый)
    inline const juce::Colour silkDim  { 0xffa39c86 };
    inline const juce::Colour lcdBg    { 0xff07120c };
    inline const juce::Colour lcdOn    { 0xff93ffb4 };
    inline const juce::Colour lcdDim   { 0xff2a5a3a };
    inline const juce::Colour amber    { 0xffffb52e };
    inline const juce::Colour red      { 0xffff4a3d };
    inline const juce::Colour green    { 0xff5dff80 };
    inline const juce::Colour gold     { 0xffd9bf74 };

    inline juce::Font font (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
    }
    inline juce::Font mono (float h, bool bold = false)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h,
                                              bold ? juce::Font::bold : juce::Font::plain));
    }
    inline juce::Colour lfoColour (int l)
    {
        static const juce::uint32 c[4] = { 0xffffb347, 0xff6dff9c, 0xff59d0ff, 0xffff6b8f };
        return juce::Colour (c[l & 3]);
    }
}

//==============================================================================
//  Рисовалки железа
//==============================================================================
namespace Hw
{
    inline float textWidth (const juce::Font& f, const juce::String& t)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (f, t, 0.0f, 0.0f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }

    // шрифт, подогнанный по ширине (чтобы текст никогда не обрезался многоточием)
    inline juce::Font fitFont (const juce::String& text, bool bold, float maxHeight, float availableWidth, float minHeight = 8.0f)
    {
        float h = maxHeight;
        while (h > minHeight && textWidth (Theme::font (h, bold), text) > availableWidth)
            h -= 0.5f;
        return Theme::font (h, bold);
    }

    inline void screw (juce::Graphics& g, juce::Point<float> c, float r, float angle)
    {
        using namespace juce;
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (c.x - r - 0.5f, c.y - r + 1.0f, r * 2.0f + 1.0f, r * 2.0f + 1.0f);
        g.setGradientFill (ColourGradient (Colour (0xfff0f1f4), c.x - r * 0.5f, c.y - r * 0.6f,
                                           Colour (0xff585b62), c.x + r * 0.6f, c.y + r * 0.8f, true));
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
        g.setColour (Colour (0xff1f2023));
        g.drawEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f, 0.9f);
        const float dx = std::cos (angle) * r * 0.78f, dy = std::sin (angle) * r * 0.78f;
        g.setColour (Colour (0xff25262a));
        g.drawLine (c.x - dx, c.y - dy, c.x + dx, c.y + dy, 1.7f);
        g.setColour (Colours::white.withAlpha (0.25f));
        g.drawLine (c.x - dx, c.y - dy + 1.0f, c.x + dx, c.y + dy + 1.0f, 0.7f);
    }

    // стеклянное окошко дисплея: хромированная рамка, подсвеченное стекло, пиксельная сетка, блик
    inline void lcdWindow (juce::Graphics& g, juce::Rectangle<float> r, float corner = 6.0f)
    {
        using namespace juce;
        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (r.expanded (3.5f).translated (0.0f, 1.5f), corner + 3.5f);
        g.setGradientFill (ColourGradient (Colour (0xff9a9da5), 0.0f, r.getY() - 3.0f, Colour (0xff2a2b30), 0.0f, r.getBottom() + 3.0f, false));
        g.fillRoundedRectangle (r.expanded (3.0f), corner + 3.0f);
        g.setColour (Colour (0xff101012));
        g.fillRoundedRectangle (r.expanded (1.0f), corner + 1.0f);

        g.setGradientFill (ColourGradient (Colour (0xff10301f), r.getCentreX(), r.getCentreY(), Colour (0xff040b07), r.getX(), r.getY(), true));
        g.fillRoundedRectangle (r, corner);

        g.setColour (Colours::black.withAlpha (0.10f));
        for (float x = r.getX() + 2.0f; x < r.getRight() - 1.0f; x += 3.0f)
            g.drawVerticalLine ((int) x, r.getY() + 1.0f, r.getBottom() - 1.0f);
        for (float y = r.getY() + 2.0f; y < r.getBottom() - 1.0f; y += 3.0f)
            g.drawHorizontalLine ((int) y, r.getX() + 1.0f, r.getRight() - 1.0f);

        g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.6f), 0.0f, r.getY(), Colours::transparentBlack, 0.0f, r.getY() + r.getHeight() * 0.30f, false));
        g.fillRoundedRectangle (r, corner);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.09f), r.getX(), r.getY(), Colours::transparentWhite, r.getX() + r.getWidth() * 0.5f, r.getBottom(), false));
        g.fillRoundedRectangle (r.withTrimmedBottom (r.getHeight() * 0.55f), corner);
    }

    // углубление в панели («карман»)
    inline void bay (juce::Graphics& g, juce::Rectangle<float> r, float corner = 10.0f)
    {
        using namespace juce;
        g.setColour (Colours::black.withAlpha (0.28f));
        g.fillRoundedRectangle (r, corner);
        g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.55f), 0.0f, r.getY(), Colours::transparentBlack, 0.0f, r.getY() + 22.0f, false));
        g.fillRoundedRectangle (r, corner);
        g.setColour (Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (r.expanded (1.0f), corner + 1.0f, 1.0f);
        g.setColour (Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (r, corner, 1.0f);
    }

    // гравировка: светлая тень снизу + основной цвет
    inline void engraved (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, const juce::Font& f,
                          juce::Justification j, juce::Colour c = Theme::silk)
    {
        g.setFont (f);
        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.drawText (t, r.translated (0.0f, 1.0f), j, false);
        g.setColour (c);
        g.drawText (t, r, j, false);
    }

    // гравировка по латуни: тёмный текст, светлый блик
    inline void engravedDark (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> r, const juce::Font& f, juce::Justification j)
    {
        g.setFont (f);
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.drawText (t, r.translated (0.0f, 1.0f), j, false);
        g.setColour (juce::Colour (0xff2a1f08));
        g.drawText (t, r, j, false);
    }

    // рамка сеточной печати с заголовком в разрыве верхней линии
    inline void silkBox (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title, juce::Colour accent)
    {
        using namespace juce;
        const Font f = Theme::font (11.5f, true);
        const float tw = textWidth (f, title.toUpperCase());
        const float rr = 8.0f;
        const float gx0 = r.getX() + 16.0f, gx1 = gx0 + tw + 14.0f;

        Path p;
        p.startNewSubPath (gx1, r.getY());
        p.lineTo (r.getRight() - rr, r.getY());
        p.quadraticTo (r.getRight(), r.getY(), r.getRight(), r.getY() + rr);
        p.lineTo (r.getRight(), r.getBottom() - rr);
        p.quadraticTo (r.getRight(), r.getBottom(), r.getRight() - rr, r.getBottom());
        p.lineTo (r.getX() + rr, r.getBottom());
        p.quadraticTo (r.getX(), r.getBottom(), r.getX(), r.getBottom() - rr);
        p.lineTo (r.getX(), r.getY() + rr);
        p.quadraticTo (r.getX(), r.getY(), r.getX() + rr, r.getY());
        p.lineTo (gx0, r.getY());

        g.setColour (Colours::black.withAlpha (0.5f));
        g.strokePath (p, PathStrokeType (1.4f), AffineTransform::translation (0.0f, 1.0f));
        g.setColour (Theme::silkDim.withAlpha (0.8f));
        g.strokePath (p, PathStrokeType (1.1f));

        engraved (g, title.toUpperCase(), { gx0 + 7.0f, r.getY() - 8.0f, tw + 4.0f, 16.0f }, f, Justification::centredLeft, Theme::silk);

        g.setColour (accent);
        g.fillRoundedRectangle (r.getX() + 5.0f, r.getY() - 2.5f, 5.0f, 5.0f, 2.0f);
    }

    // латунная табличка со щёткой
    inline void brassPlate (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.5f), 6.0f);
        g.setGradientFill (ColourGradient (Colour (0xfff2e6bb), 0.0f, r.getY(), Colour (0xff9d7f3c), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 6.0f);

        Random rn (5);
        for (float y = r.getY() + 3.0f; y < r.getBottom() - 3.0f; y += 1.0f)
        {
            g.setColour ((rn.nextInt (2) == 0 ? Colours::white : Colours::black).withAlpha (rn.nextFloat() * 0.09f));
            g.drawHorizontalLine ((int) y, r.getX() + 3.0f, r.getRight() - 3.0f);
        }
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.30f), r.getX(), r.getY(), Colours::transparentWhite, r.getCentreX(), r.getBottom(), false));
        g.fillRoundedRectangle (r.withTrimmedBottom (r.getHeight() * 0.5f), 6.0f);
        g.setColour (Colour (0xff4a3a15));
        g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.3f);
        g.setColour (Colours::white.withAlpha (0.45f));
        g.drawRoundedRectangle (r.reduced (3.0f), 4.0f, 0.8f);
    }

    // ореховая боковина
    inline void woodPanel (juce::Graphics& g, juce::Rectangle<int> r, bool left)
    {
        using namespace juce;
        const auto rf = r.toFloat();
        g.setGradientFill (ColourGradient (Colour (0xff53311b), rf.getX(), 0.0f, Colour (0xff2a170b), rf.getRight(), 0.0f, false));
        g.fillRect (rf);

        Random rn (left ? 11 : 29);
        for (int i = 0; i < 80; ++i)
        {
            float x = rf.getX() + rn.nextFloat() * rf.getWidth();
            Path p;
            p.startNewSubPath (x, rf.getY());
            for (float y = rf.getY(); y < rf.getBottom(); y += 14.0f)
            {
                x = jlimit (rf.getX(), rf.getRight(), x + (rn.nextFloat() - 0.5f) * 1.8f);
                p.lineTo (x, y + 14.0f);
            }
            g.setColour ((i & 1) ? Colour (0xff140803).withAlpha (0.10f + rn.nextFloat() * 0.22f)
                                 : Colour (0xff9a6a3c).withAlpha (0.06f + rn.nextFloat() * 0.13f));
            g.strokePath (p, PathStrokeType (0.6f + rn.nextFloat() * 1.5f));
        }

        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.12f), left ? rf.getX() : rf.getRight(), 0.0f,
                                           Colours::transparentWhite, left ? rf.getRight() : rf.getX(), 0.0f, false));
        g.fillRect (rf);
        g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.35f), 0.0f, 0.0f, Colours::transparentBlack, 0.0f, rf.getHeight() * 0.12f, false));
        g.fillRect (rf);
        g.setGradientFill (ColourGradient (Colours::transparentBlack, 0.0f, rf.getHeight() * 0.88f, Colours::black.withAlpha (0.4f), 0.0f, rf.getBottom(), false));
        g.fillRect (rf);
    }

    // кэшированный колпак ручки: рифлёная чёрная «юбка» + шлифованный алюминиевый диск
    inline juce::Image knobCapImage (int S)
    {
        using namespace juce;
        static std::map<int, Image> cache;
        auto it = cache.find (S);
        if (it != cache.end()) return it->second;

        Image img (Image::ARGB, S, S, true);
        Graphics g (img);
        const float c = (float) S * 0.5f, fS = (float) S;
        const float twoPi = MathConstants<float>::twoPi;

        g.setGradientFill (ColourGradient (Colour (0xff3c3d43), c * 0.7f, c * 0.45f, Colour (0xff08080a), c * 1.45f, c * 1.8f, true));
        g.fillEllipse (0.0f, 0.0f, fS, fS);

        for (int k = 0; k < 72; ++k)
        {
            const float a = (float) k * twoPi / 72.0f, a2 = a + twoPi / 144.0f;
            g.setColour (Colours::white.withAlpha (0.11f));
            g.drawLine (c + std::sin (a) * c * 0.90f, c - std::cos (a) * c * 0.90f, c + std::sin (a) * c * 0.985f, c - std::cos (a) * c * 0.985f, fS * 0.012f);
            g.setColour (Colours::black.withAlpha (0.5f));
            g.drawLine (c + std::sin (a2) * c * 0.90f, c - std::cos (a2) * c * 0.90f, c + std::sin (a2) * c * 0.985f, c - std::cos (a2) * c * 0.985f, fS * 0.012f);
        }
        g.setColour (Colours::white.withAlpha (0.18f));
        g.drawEllipse (fS * 0.012f, fS * 0.012f, fS * 0.976f, fS * 0.976f, fS * 0.010f);

        const float r2 = c * 0.70f;
        for (int i = 0; i < 120; ++i)
        {
            const float a0 = (float) i * twoPi / 120.0f, a1 = a0 + twoPi / 120.0f + 0.02f;
            float b = 0.40f + 0.22f * std::cos (2.0f * (a0 - 0.9f)) + 0.10f * std::cos (4.0f * a0 + 0.3f) + 0.05f * std::cos (7.0f * a0);
            b = jlimit (0.12f, 0.92f, b);
            Path w;
            w.addPieSegment (c - r2, c - r2, r2 * 2.0f, r2 * 2.0f, a0, a1, 0.0f);
            g.setColour (Colour::fromFloatRGBA (b, b, b * 1.04f, 1.0f));
            g.fillPath (w);
        }
        for (float r = 2.0f; r < r2; r += 2.2f)
        {
            g.setColour (Colours::white.withAlpha (0.04f));
            g.drawEllipse (c - r, c - r, 2.0f * r, 2.0f * r, 0.6f);
            g.setColour (Colours::black.withAlpha (0.05f));
            g.drawEllipse (c - r - 1.1f, c - r - 1.1f, 2.0f * r + 2.2f, 2.0f * r + 2.2f, 0.6f);
        }
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawEllipse (c - r2, c - r2, r2 * 2.0f, r2 * 2.0f, fS * 0.016f);
        Path hl;
        hl.addCentredArc (c, c, r2 - 1.0f, r2 - 1.0f, 0.0f, -2.5f, -0.7f, true);
        g.setColour (Colours::white.withAlpha (0.40f));
        g.strokePath (hl, PathStrokeType (fS * 0.012f));

        g.setGradientFill (ColourGradient (Colour (0xff0f0f11), c - r2 * 0.15f, c - r2 * 0.15f, Colour (0xff75777e), c + r2 * 0.15f, c + r2 * 0.15f, false));
        g.fillEllipse (c - r2 * 0.14f, c - r2 * 0.14f, r2 * 0.28f, r2 * 0.28f);

        cache[S] = img;
        return img;
    }

    // сегмент 7-сегментного индикатора
    inline void sevenSeg (juce::Graphics& g, juce::Rectangle<float> r, char ch, juce::Colour on)
    {
        using namespace juce;
        static const char* masks[] = { "abcdef", "bc", "abdeg", "abcdg", "bcfg", "acdfg", "acdefg", "abc", "abcdefg", "abcdfg" };
        const char* m = (ch >= '0' && ch <= '9') ? masks[ch - '0'] : (ch == '-' ? "g" : "");
        const float t = r.getWidth() * 0.17f, w = r.getWidth(), h = r.getHeight(), x = r.getX(), y = r.getY();

        auto seg = [&] (char s, Rectangle<float> rc)
        {
            const bool lit = *m != '\0' && std::strchr (m, s) != nullptr;
            if (lit)
            {
                g.setColour (on.withAlpha (0.28f));
                g.fillRoundedRectangle (rc.expanded (1.8f), t * 0.7f);
                g.setColour (on);
            }
            else g.setColour (on.withAlpha (0.07f));
            g.fillRoundedRectangle (rc, t * 0.45f);
        };
        const float vh = h * 0.5f - t * 1.25f;
        seg ('a', { x + t, y, w - 2 * t, t });
        seg ('g', { x + t, y + h * 0.5f - t * 0.5f, w - 2 * t, t });
        seg ('d', { x + t, y + h - t, w - 2 * t, t });
        seg ('f', { x, y + t * 1.1f, t, vh });
        seg ('b', { x + w - t, y + t * 1.1f, t, vh });
        seg ('e', { x, y + h * 0.5f + t * 0.15f, t, vh });
        seg ('c', { x + w - t, y + h * 0.5f + t * 0.15f, t, vh });
    }

    inline void padlock (juce::Graphics& g, juce::Rectangle<float> r, bool locked)
    {
        using namespace juce;
        const Colour body = locked ? Colour (0xffffc23a) : Colour (0xff8a8d94);
        const float cx = r.getCentreX();
        const float bw = r.getWidth() * 0.62f, bh = r.getHeight() * 0.46f;
        const float by = r.getBottom() - bh - 1.0f;
        const float sr = bw * 0.33f;
        const float lift = locked ? 0.0f : sr * 0.55f;

        Path sh;
        sh.startNewSubPath (cx - sr, by);
        sh.lineTo (cx - sr, by - sr * 0.8f - lift);
        sh.addArc (cx - sr, by - sr * 0.8f - sr - lift, sr * 2.0f, sr * 2.0f, -MathConstants<float>::halfPi, MathConstants<float>::halfPi, false);
        sh.lineTo (cx + sr, by - sr * 0.8f - lift + (locked ? sr * 0.8f : -sr * 0.1f));
        g.setColour (Colour (0xffd5d7dc));
        g.strokePath (sh, PathStrokeType (2.4f, PathStrokeType::curved, PathStrokeType::rounded));

        const Rectangle<float> b (cx - bw * 0.5f, by, bw, bh);
        g.setGradientFill (ColourGradient (body.brighter (0.45f), b.getX(), b.getY(), body.darker (0.5f), b.getX(), b.getBottom(), false));
        g.fillRoundedRectangle (b, 2.5f);
        g.setColour (Colours::black.withAlpha (0.6f));
        g.drawRoundedRectangle (b, 2.5f, 1.0f);
        g.setColour (Colours::black.withAlpha (0.75f));
        g.fillEllipse (cx - 1.8f, by + bh * 0.28f, 3.6f, 3.6f);
        g.fillRect (cx - 0.8f, by + bh * 0.28f + 2.5f, 1.6f, bh * 0.28f);
    }

    // аналоговый VU-метр в латунной раме. needle01: 0..1 по шкале -20..+3 дБ
    inline void vuMeter (juce::Graphics& g, juce::Rectangle<float> r, float needle01, const juce::String& tag)
    {
        using namespace juce;
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.0f), 9.0f);
        g.setGradientFill (ColourGradient (Colour (0xffcdb26a), 0.0f, r.getY(), Colour (0xff6e5826), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 9.0f);
        g.setColour (Colour (0xff3a2c0e));
        g.drawRoundedRectangle (r.reduced (0.5f), 9.0f, 1.2f);

        const auto face = r.reduced (7.0f);
        g.setColour (Colours::black.withAlpha (0.8f));
        g.fillRoundedRectangle (face.expanded (1.5f), 6.0f);
        g.setGradientFill (ColourGradient (Colour (0xfffff0c2), face.getCentreX(), face.getBottom(), Colour (0xffcfb874), face.getX(), face.getY(), true));
        g.fillRoundedRectangle (face, 5.0f);

        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (face.toNearestInt());

        const float cx = face.getCentreX(), cy = face.getBottom() + face.getHeight() * 0.30f;
        const float rad = face.getHeight() * 1.12f;
        const float a0 = -0.80f, a1 = 0.80f;
        auto pt = [&] (float a, float rr) { return Point<float> (cx + std::sin (a) * rr, cy - std::cos (a) * rr); };
        const float redStart = 20.0f / 23.0f;

        Path red;
        red.addCentredArc (cx, cy, rad - 3.0f, rad - 3.0f, 0.0f, a0 + redStart * (a1 - a0), a1, true);
        g.setColour (Colour (0xffc3342a));
        g.strokePath (red, PathStrokeType (4.0f));

        g.setColour (Colour (0xff2b2418));
        for (int i = 0; i <= 23; ++i)
        {
            const float t = (float) i / 23.0f;
            const float a = a0 + t * (a1 - a0);
            const float len = (i % 5 == 0 || i == 23) ? 8.0f : 4.0f;
            g.drawLine (Line<float> (pt (a, rad - 8.0f - len), pt (a, rad - 8.0f)), 1.1f);
        }
        g.setFont (Theme::font (10.0f, true));
        const char* labels[] = { "-20", "-10", "-5", "0", "+3" };
        const float lt[] = { 0.0f, 10.0f / 23.0f, 15.0f / 23.0f, 20.0f / 23.0f, 1.0f };
        for (int i = 0; i < 5; ++i)
        {
            const auto p = pt (a0 + lt[i] * (a1 - a0), rad - 24.0f);
            g.setColour (i >= 3 ? Colour (0xffb02a1f) : Colour (0xff2b2418));
            g.drawText (labels[i], Rectangle<float> (p.x - 14.0f, p.y - 6.0f, 28.0f, 12.0f), Justification::centred);
        }

        g.setColour (Colour (0xff2b2418));
        g.setFont (Theme::font (15.0f, true));
        g.drawText ("VU", Rectangle<float> (face.getX(), face.getY() + face.getHeight() * 0.50f, face.getWidth(), 18.0f), Justification::centred);
        g.setFont (Theme::font (10.5f, true));
        g.drawText (tag, Rectangle<float> (face.getX() + 6.0f, face.getY() + 4.0f, 40.0f, 12.0f), Justification::centredLeft);

        const float a = a0 + jlimit (0.0f, 1.0f, needle01) * (a1 - a0);
        g.setColour (Colours::black.withAlpha (0.25f));
        g.drawLine (Line<float> (pt (a, 6.0f).translated (2.0f, 2.0f), pt (a, rad - 6.0f).translated (2.0f, 2.0f)), 2.0f);
        g.setColour (Colour (0xff15110c));
        g.drawLine (Line<float> (pt (a, 6.0f), pt (a, rad - 6.0f)), 1.7f);

        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.22f), face.getX(), face.getY(), Colours::transparentWhite, face.getRight(), face.getBottom(), false));
        g.fillRoundedRectangle (face.withTrimmedBottom (face.getHeight() * 0.45f), 5.0f);

        // винты рамы
        for (float sx : { r.getX() + 4.0f, r.getRight() - 4.0f })
            for (float sy : { r.getY() + 4.0f, r.getBottom() - 4.0f })
                screw (g, { sx, sy }, 2.4f, sx + sy);
    }

    inline void reel (juce::Graphics& g, juce::Rectangle<float> r, float angle, float tape01)
    {
        using namespace juce;
        const auto c = r.getCentre();
        const float R = jmin (r.getWidth(), r.getHeight()) * 0.5f;

        g.setColour (Colour (0xff2a1c12));
        const float tr = R * (0.40f + 0.52f * tape01);
        g.fillEllipse (c.x - tr, c.y - tr, tr * 2.0f, tr * 2.0f);
        g.setColour (Colour (0xff4a3322));
        g.drawEllipse (c.x - tr, c.y - tr, tr * 2.0f, tr * 2.0f, 1.0f);

        g.setGradientFill (ColourGradient (Colour (0xffd0d3d9), c.x - R * 0.5f, c.y - R * 0.6f, Colour (0xff50535a), c.x + R * 0.6f, c.y + R * 0.7f, true));
        g.fillEllipse (c.x - R * 0.42f, c.y - R * 0.42f, R * 0.84f, R * 0.84f);
        g.setColour (Colour (0xff202226));
        for (int i = 0; i < 3; ++i)
        {
            const float a = angle + (float) i * MathConstants<float>::twoPi / 3.0f;
            Path wedge;
            wedge.addPieSegment (c.x - R * 0.34f, c.y - R * 0.34f, R * 0.68f, R * 0.68f, a - 0.55f, a + 0.55f, 0.30f);
            g.fillPath (wedge);
        }
        g.setGradientFill (ColourGradient (Colour (0xfff2f2f2), c.x - 3.0f, c.y - 3.0f, Colour (0xff707278), c.x + 4.0f, c.y + 4.0f, true));
        g.fillEllipse (c.x - R * 0.11f, c.y - R * 0.11f, R * 0.22f, R * 0.22f);
    }
}

//==============================================================================
class BeatMorphLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BeatMorphLookAndFeel()
    {
        using namespace juce;
        setColour (ComboBox::textColourId, Theme::lcdOn);
        setColour (ComboBox::arrowColourId, Theme::lcdDim);
        setColour (ComboBox::backgroundColourId, Theme::lcdBg);
        setColour (ComboBox::outlineColourId, Colours::transparentBlack);
        setColour (Label::textColourId, Theme::silk);
        setColour (Slider::textBoxTextColourId, Theme::lcdOn);
        setColour (Slider::textBoxOutlineColourId, Colour (0xff050806));
        setColour (Slider::textBoxBackgroundColourId, Theme::lcdBg);
        setColour (Slider::textBoxHighlightColourId, Theme::lcdDim);
        setColour (PopupMenu::backgroundColourId, Colour (0xff09140e));
        setColour (PopupMenu::textColourId, Theme::lcdOn);
        setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff1f5236));
        setColour (PopupMenu::highlightedTextColourId, Colours::white);
        setColour (TextButton::textColourOffId, Theme::silk);
        setColour (TextButton::textColourOnId, Theme::silk);
        setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        setColour (ListBox::outlineColourId, Colours::transparentBlack);
        setColour (ScrollBar::thumbColourId, Colour (0xff4b6a58));
        setColour (TextEditor::backgroundColourId, Theme::lcdBg);
        setColour (TextEditor::textColourId, Theme::lcdOn);
        setColour (TextEditor::outlineColourId, Colour (0xff3a3d44));
        setColour (TextEditor::focusedOutlineColourId, Theme::lcdOn);
        setColour (TextEditor::highlightColourId, Theme::lcdDim);
        setColour (CaretComponent::caretColourId, Theme::lcdOn);
        setColour (HyperlinkButton::textColourId, Theme::amber);
    }

    //----------------------------------------------------------------------
    //  Ручка: алюминиевый диск на рифлёной юбке, кольцо светодиодов
    //----------------------------------------------------------------------
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                           float startAng, float endAng, juce::Slider& s) override
    {
        using namespace juce;
        auto b = Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (1.0f);
        const float size = jmin (b.getWidth(), b.getHeight());
        b = b.withSizeKeepingCentre (size, size);
        const auto  c = b.getCentre();
        const float R = size * 0.5f;
        const Colour accent = s.findColour (Slider::rotarySliderFillColourId);
        const bool hot = s.isMouseOverOrDragging() && s.isEnabled();

        // кольцо светодиодов
        const int nDots = 17;
        const float dotR = R - 3.2f;
        const bool bipolar = s.getMinimum() < 0.0 && s.getMaximum() > 0.0;
        const float zeroP = bipolar ? (float) s.valueToProportionOfLength (0.0) : 0.0f;

        for (int i = 0; i < nDots; ++i)
        {
            const float t = (float) i / (float) (nDots - 1);
            const float a = startAng + t * (endAng - startAng);
            const Point<float> p (c.x + std::sin (a) * dotR, c.y - std::cos (a) * dotR);

            bool lit;
            if (bipolar) lit = (pos >= zeroP) ? (t >= zeroP - 0.001f && t <= pos + 0.02f)
                                              : (t <= zeroP + 0.001f && t >= pos - 0.02f);
            else         lit = t <= pos + 0.02f;
            if (i == 0 && pos < 0.004f && ! bipolar) lit = false;

            g.setColour (Colour (0xff050506));
            g.fillEllipse (p.x - 2.9f, p.y - 2.9f, 5.8f, 5.8f);
            if (lit)
            {
                g.setColour (accent.withAlpha (0.28f));
                g.fillEllipse (p.x - 4.4f, p.y - 4.4f, 8.8f, 8.8f);
                g.setColour (accent);
                g.fillEllipse (p.x - 2.1f, p.y - 2.1f, 4.2f, 4.2f);
                g.setColour (Colours::white.withAlpha (0.55f));
                g.fillEllipse (p.x - 1.0f, p.y - 1.4f, 1.6f, 1.6f);
            }
            else
            {
                g.setColour (accent.darker (0.9f));
                g.fillEllipse (p.x - 1.9f, p.y - 1.9f, 3.8f, 3.8f);
            }
        }

        // индикатор модуляции LFO: белая метка снаружи кольца
        if (s.getProperties().contains ("mod"))
        {
            const float off = (float) s.getProperties()["mod"];
            const float tm = jlimit (0.0f, 1.0f, pos + off);
            const float a = startAng + tm * (endAng - startAng);
            const Point<float> p (c.x + std::sin (a) * (dotR + 3.2f), c.y - std::cos (a) * (dotR + 3.2f));
            g.setColour (Colours::white.withAlpha (0.35f));
            g.fillEllipse (p.x - 4.5f, p.y - 4.5f, 9.0f, 9.0f);
            g.setColour (Colours::white);
            g.fillEllipse (p.x - 2.4f, p.y - 2.4f, 4.8f, 4.8f);
        }

        // тень и колпак
        const float capR = R - 10.5f;
        g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.65f), c.x + 1.5f, c.y + 3.0f, Colours::transparentBlack, c.x + 1.5f, c.y + capR + 7.0f, true));
        g.fillEllipse (c.x - capR - 6.0f, c.y - capR - 3.0f, capR * 2.0f + 12.0f, capR * 2.0f + 12.0f);

        const int S = jmax (48, (int) (capR * 4.0f));
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (Hw::knobCapImage (S), Rectangle<float> (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f));

        if (hot)
        {
            g.setColour (accent.withAlpha (0.35f));
            g.drawEllipse (c.x - capR, c.y - capR, capR * 2.0f, capR * 2.0f, 1.6f);
        }

        // белый указатель на юбке
        const float angle = startAng + pos * (endAng - startAng);
        const float r0 = capR * 0.73f, r1 = capR * 0.96f;
        const Point<float> p0 (c.x + std::sin (angle) * r0, c.y - std::cos (angle) * r0);
        const Point<float> p1 (c.x + std::sin (angle) * r1, c.y - std::cos (angle) * r1);
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawLine (p0.x + 0.8f, p0.y + 0.9f, p1.x + 0.8f, p1.y + 0.9f, 3.2f);
        g.setColour (Colour (0xfff7f2e2));
        g.drawLine (p0.x, p0.y, p1.x, p1.y, 2.6f);
    }

    //----------------------------------------------------------------------
    //  Горизонтальный фейдер с рифлёной металлической кареткой
    //----------------------------------------------------------------------
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float minPos, float maxPos, const juce::Slider::SliderStyle style, juce::Slider& s) override
    {
        using namespace juce;
        if (style != Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minPos, maxPos, style, s);
            return;
        }

        const Colour accent = s.findColour (Slider::trackColourId);
        const float cy = (float) y + (float) height * 0.5f;
        const float left = (float) x + 4.0f, right = (float) (x + width) - 4.0f;

        g.setColour (Theme::silkDim.withAlpha (0.85f));
        for (int i = 0; i <= 10; ++i)
        {
            const float tx = left + (right - left) * (float) i / 10.0f;
            g.drawVerticalLine ((int) tx, cy - 12.0f, cy - (i % 5 == 0 ? 6.0f : 8.5f));
        }

        const Rectangle<float> slot (left, cy - 3.0f, right - left, 6.0f);
        g.setColour (Colours::black.withAlpha (0.8f));
        g.fillRoundedRectangle (slot.expanded (1.5f), 3.5f);
        g.setColour (Colour (0xff040405));
        g.fillRoundedRectangle (slot, 3.0f);
        g.setColour (accent.withAlpha (0.6f));
        g.fillRoundedRectangle (slot.withRight (sliderPos).reduced (0.0f, 1.2f), 2.0f);
        g.setColour (Colours::white.withAlpha (0.08f));
        g.drawHorizontalLine ((int) (cy + 2.5f), left, right);

        const Rectangle<float> cap (sliderPos - 9.5f, cy - 13.0f, 19.0f, 26.0f);
        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (cap.translated (1.5f, 2.5f), 3.0f);
        g.setGradientFill (ColourGradient (Colour (0xfff4f5f8), cap.getX(), cap.getY(), Colour (0xff62656c), cap.getX(), cap.getBottom(), false));
        g.fillRoundedRectangle (cap, 3.0f);
        for (int i = -3; i <= 3; ++i)      // рифление
        {
            const float rx = sliderPos + (float) i * 2.4f;
            g.setColour (Colours::black.withAlpha (0.30f));
            g.drawVerticalLine ((int) rx, cap.getY() + 3.0f, cap.getBottom() - 3.0f);
            g.setColour (Colours::white.withAlpha (0.40f));
            g.drawVerticalLine ((int) rx + 1, cap.getY() + 3.0f, cap.getBottom() - 3.0f);
        }
        g.setColour (Colour (0xff25262b));
        g.drawRoundedRectangle (cap, 3.0f, 1.0f);
        g.setColour (s.isMouseOverOrDragging() ? accent : Colour (0xffe8e8ec));
        g.fillRect (sliderPos - 1.0f, cap.getY() + 2.0f, 2.0f, cap.getHeight() - 4.0f);
    }

    //----------------------------------------------------------------------
    //  Переключатель: клавиша с хромированной линзой
    //----------------------------------------------------------------------
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool down) override
    {
        using namespace juce;
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        const bool on = b.getToggleState();
        const Colour accent = b.findColour (ToggleButton::tickColourId);

        g.setColour (Colours::black.withAlpha (0.6f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.0f), 5.0f);

        const Colour base = on ? Colour (0xff2a2c32) : Colour (0xff4d4f58);
        g.setGradientFill (ColourGradient (down || on ? base.darker (0.15f) : base.brighter (highlighted ? 0.35f : 0.18f), 0.0f, r.getY(),
                                           base.darker (0.5f), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (Colours::black.withAlpha (0.8f));
        g.drawRoundedRectangle (r, 5.0f, 1.0f);
        if (! on)
        {
            g.setColour (Colours::white.withAlpha (0.22f));
            g.drawLine (r.getX() + 4.0f, r.getY() + 1.5f, r.getRight() - 4.0f, r.getY() + 1.5f, 1.0f);
        }

        const Point<float> lc (r.getX() + 15.0f, r.getCentreY());
        g.setGradientFill (ColourGradient (Colour (0xffe9eaee), lc.x - 6.0f, lc.y - 6.0f, Colour (0xff5b5e66), lc.x + 6.0f, lc.y + 6.0f, true));
        g.fillEllipse (lc.x - 8.5f, lc.y - 8.5f, 17.0f, 17.0f);
        g.setColour (Colour (0xff0b0c0e));
        g.fillEllipse (lc.x - 6.5f, lc.y - 6.5f, 13.0f, 13.0f);
        if (on)
        {
            g.setColour (accent.withAlpha (0.35f));
            g.fillEllipse (lc.x - 13.0f, lc.y - 13.0f, 26.0f, 26.0f);
            g.setGradientFill (ColourGradient (accent.brighter (0.8f), lc.x, lc.y, accent.darker (0.2f), lc.x + 6.0f, lc.y + 6.0f, true));
        }
        else g.setColour (accent.darker (0.88f));
        g.fillEllipse (lc.x - 5.5f, lc.y - 5.5f, 11.0f, 11.0f);
        g.setColour (Colours::white.withAlpha (on ? 0.75f : 0.35f));
        g.fillEllipse (lc.x - 3.4f, lc.y - 4.4f, 3.6f, 2.4f);

        g.setFont (Theme::font (11.5f, true));
        g.setColour (on ? Theme::silk : Theme::silkDim);
        g.drawText (b.getButtonText().toUpperCase(), r.withTrimmedLeft (30.0f), Justification::centredLeft);
    }

    //----------------------------------------------------------------------
    //  Селектор в виде окошка LCD
    //----------------------------------------------------------------------
    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        using namespace juce;
        auto r = Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (3.0f);
        Hw::lcdWindow (g, r, 4.0f);

        const bool hot = box.isMouseOver() || box.isPopupActive();
        const Colour a = box.findColour (ComboBox::focusedOutlineColourId);
        if (hot)
        {
            g.setColour (a.withAlpha (0.18f));
            g.fillRoundedRectangle (r, 4.0f);
        }

        const float ax = r.getRight() - 10.0f, ay = r.getCentreY();
        Path up, dn;
        up.addTriangle (ax - 3.5f, ay - 1.5f, ax + 3.5f, ay - 1.5f, ax, ay - 5.5f);
        dn.addTriangle (ax - 3.5f, ay + 1.5f, ax + 3.5f, ay + 1.5f, ax, ay + 5.5f);
        g.setColour (hot ? Theme::lcdOn : Theme::lcdDim);
        g.fillPath (up);
        g.fillPath (dn);
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (7, 3, box.getWidth() - 24, box.getHeight() - 6);
        label.setFont (getComboBoxFont (box));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return Theme::mono (13.0f, true); }

    juce::Font getLabelFont (juce::Label& l) override
    {
        if (dynamic_cast<juce::Slider*> (l.getParentComponent()) != nullptr)
            return Theme::mono (12.0f, true);
        return l.getFont();
    }

    //----------------------------------------------------------------------
    //  Кнопки (prev / next / random / browse ...)
    //----------------------------------------------------------------------
    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& bg,
                               bool highlighted, bool down) override
    {
        using namespace juce;
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        Colour base = bg;
        if (highlighted) base = base.brighter (0.2f);

        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (r.translated (0.0f, down ? 0.5f : 2.2f), 5.0f);

        const auto face = down ? r.translated (0.0f, 1.5f) : r;
        g.setGradientFill (ColourGradient (down ? base.darker (0.2f) : base.brighter (0.4f), 0.0f, face.getY(),
                                           base.darker (0.55f), 0.0f, face.getBottom(), false));
        g.fillRoundedRectangle (face, 5.0f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (down ? 0.03f : 0.20f), 0.0f, face.getY(), Colours::transparentWhite, 0.0f, face.getCentreY(), false));
        g.fillRoundedRectangle (face.reduced (1.5f, 1.5f), 4.0f);
        g.setColour (Colours::black.withAlpha (0.8f));
        g.drawRoundedRectangle (face, 5.0f, 1.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override { return Theme::font (12.5f, true); }
};

//==============================================================================
//  Слайдер с правым кликом (назначение LFO)
//==============================================================================
class ModSlider : public juce::Slider
{
public:
    int paramIndex = -1;                                  // индекс в Patterns::modParamTable (или -1)
    std::function<void (ModSlider&)> onRightClick;

    void mouseDown (const juce::MouseEvent& e) override
    {
        rightClicked = e.mods.isRightButtonDown() && paramIndex >= 0 && onRightClick;
        if (rightClicked) { onRightClick (*this); return; }
        juce::Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override { if (! rightClicked) juce::Slider::mouseDrag (e); }
    void mouseUp (const juce::MouseEvent& e) override   { if (! rightClicked) juce::Slider::mouseUp (e); rightClicked = false; }

private:
    bool rightClicked = false;
};

//==============================================================================
//  Клавиша стойки с подсветкой (Bypass / Panic)
//==============================================================================
class KeyButton : public juce::ToggleButton
{
public:
    KeyButton (const juce::String& legendText, juce::Colour ledColour) : legend (legendText), led (ledColour) {}

    void flash() { flashFrames = 8; repaint(); }

    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        const bool lit = getToggleState() || flashFrames > 0;
        if (flashFrames > 0) --flashFrames;

        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (r.translated (0.0f, down ? 0.5f : 2.2f), 6.0f);

        const auto face = down ? r.translated (0.0f, 1.5f) : r;
        const Colour base (0xff50525b);
        g.setGradientFill (ColourGradient (down ? base.darker (0.2f) : base.brighter (over ? 0.38f : 0.22f), 0.0f, face.getY(), base.darker (0.6f), 0.0f, face.getBottom(), false));
        g.fillRoundedRectangle (face, 6.0f);
        g.setColour (Colours::black.withAlpha (0.85f));
        g.drawRoundedRectangle (face, 6.0f, 1.0f);
        g.setColour (Colours::white.withAlpha (down ? 0.05f : 0.22f));
        g.drawLine (face.getX() + 5.0f, face.getY() + 1.6f, face.getRight() - 5.0f, face.getY() + 1.6f, 1.0f);

        const Rectangle<float> lens (face.getCentreX() - 15.0f, face.getY() + 6.0f, 30.0f, 10.0f);
        g.setColour (Colour (0xffa9acb4));
        g.fillRoundedRectangle (lens.expanded (2.0f), 4.0f);
        g.setColour (Colour (0xff0b0c0e));
        g.fillRoundedRectangle (lens.expanded (0.6f), 3.0f);
        if (lit)
        {
            g.setColour (led.withAlpha (0.35f));
            g.fillRoundedRectangle (lens.expanded (6.0f), 7.0f);
            g.setGradientFill (ColourGradient (led.brighter (0.7f), lens.getCentreX(), lens.getCentreY(), led.darker (0.1f), lens.getX(), lens.getY(), true));
        }
        else g.setColour (led.darker (0.88f));
        g.fillRoundedRectangle (lens, 2.5f);
        g.setColour (Colours::white.withAlpha (lit ? 0.7f : 0.3f));
        g.fillRoundedRectangle (lens.getX() + 3.0f, lens.getY() + 1.2f, lens.getWidth() * 0.45f, 2.2f, 1.0f);

        g.setFont (Theme::font (11.5f, true));
        g.setColour (lit ? Theme::silk : Theme::silkDim);
        g.drawText (legend, face.withTrimmedTop (19.0f), Justification::centred);
    }

private:
    juce::String legend;
    juce::Colour led;
    int flashFrames = 0;
};

//==============================================================================
class LockButton : public juce::ToggleButton
{
public:
    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        auto r = getLocalBounds().toFloat().reduced (3.0f);
        if (over)
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRoundedRectangle (r.expanded (2.0f), 5.0f);
        }
        Hw::padlock (g, r, getToggleState());
    }
};

//==============================================================================
//  Клавиша-вкладка модуля (со светодиодом активности)
//==============================================================================
class ModuleTab : public juce::Button
{
public:
    ModuleTab (const juce::String& name, juce::Colour acc) : juce::Button (name), accent (acc)
    {
        setClickingTogglesState (true);
        setRadioGroupId (4242);
    }

    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        using namespace juce;
        auto r = getLocalBounds().toFloat().reduced (2.0f, 3.0f);
        const bool sel = getToggleState();

        g.setColour (Colours::black.withAlpha (0.65f));
        g.fillRoundedRectangle (r.translated (0.0f, sel ? 0.5f : 2.2f), 5.0f);

        const auto face = sel ? r.translated (0.0f, 1.5f) : r;
        const Colour base = sel ? Colour (0xff25272d) : Colour (0xff4e5058);
        g.setGradientFill (ColourGradient (sel ? base.darker (0.25f) : base.brighter (over ? 0.38f : 0.2f), 0.0f, face.getY(),
                                           base.darker (0.55f), 0.0f, face.getBottom(), false));
        g.fillRoundedRectangle (face, 5.0f);
        g.setColour (sel ? accent.withAlpha (0.95f) : Colours::black.withAlpha (0.85f));
        g.drawRoundedRectangle (face, 5.0f, sel ? 1.6f : 1.0f);
        if (! sel)
        {
            g.setColour (Colours::white.withAlpha (0.20f));
            g.drawLine (face.getX() + 5.0f, face.getY() + 1.6f, face.getRight() - 5.0f, face.getY() + 1.6f, 1.0f);
        }

        const bool act = isActive && isActive();
        const Rectangle<float> led (face.getCentreX() - 10.0f, face.getY() + 5.0f, 20.0f, 5.5f);
        g.setColour (Colour (0xff9a9da5));
        g.fillRoundedRectangle (led.expanded (1.6f), 3.2f);
        g.setColour (Colour (0xff0b0c0e));
        g.fillRoundedRectangle (led.expanded (0.5f), 2.8f);
        if (act)
        {
            g.setColour (accent.withAlpha (0.38f));
            g.fillRoundedRectangle (led.expanded (5.0f), 5.0f);
            g.setColour (accent.brighter (0.55f));
        }
        else g.setColour (accent.darker (0.88f));
        g.fillRoundedRectangle (led, 2.2f);

        g.setFont (Theme::font (12.5f, true));
        g.setColour (sel ? Theme::silk : Theme::silkDim);
        g.drawText (getButtonText().toUpperCase(), face.withTrimmedTop (14.0f), Justification::centred);
    }

    std::function<bool()> isActive;
    juce::Colour accent;
};

//==============================================================================
//  Страница модуля: карточки-«сеточная печать», LCD-описание, диаграмма
//==============================================================================
class ModulePage : public juce::Component
{
public:
    static constexpr int cellW = 96, cellH = 112, labelH = 20, titleH = 20, headH = 30;

    ModulePage (const juce::String& t, const juce::String& d, juce::Colour acc)
        : title (t), description (d), accent (acc) {}

    void addControl (const juce::String& section, juce::Component& c)
    {
        addAndMakeVisible (c);
        for (auto& s : sections)
            if (s.title == section) { s.comps.add (&c); return; }

        sections.push_back ({ section, {}, {} });
        sections.back().comps.add (&c);
    }

    void setDiagram (juce::Component* c)
    {
        diagram = c;
        addAndMakeVisible (c);
    }

    void setHint (const juce::String& h)
    {
        if (h != hint) { hint = h; repaint(); }
    }

    void paint (juce::Graphics& g) override
    {
        using namespace juce;
        Hw::engraved (g, title, { 6.0f, 2.0f, (float) getWidth() - 12.0f, 22.0f }, Theme::font (17.0f, true), Justification::centredLeft, accent.brighter (0.25f));

        for (auto& s : sections)
        {
            const auto r = s.bounds.toFloat();
            g.setColour (Colours::black.withAlpha (0.16f));
            g.fillRoundedRectangle (r, 8.0f);
            Hw::silkBox (g, r, s.title, accent);
        }

        if (! infoRect.isEmpty())
        {
            Hw::lcdWindow (g, infoRect.toFloat(), 6.0f);
            g.setFont (Theme::mono (12.0f, true));
            g.setColour (Theme::lcdOn);
            g.drawFittedText (description, infoRect.reduced (12, 10).withTrimmedBottom (infoRect.getHeight() / 2), Justification::topLeft, 6, 1.0f);
            if (hint.isNotEmpty())
            {
                g.setColour (Theme::amber);
                g.drawFittedText ("> " + hint, infoRect.reduced (12, 10).withTrimmedTop (infoRect.getHeight() / 2 - 4), Justification::topLeft, 5, 1.0f);
            }
        }
    }

    void resized() override
    {
        const int avail = getWidth() - 4;
        int x = 4, y = headH + 6;
        const int rowH = titleH + cellH + 12;
        int lastRowY = y, lastX = x;

        for (auto& s : sections)
        {
            const int w = s.comps.size() * cellW + 24;
            if (x + w > avail && x > 4) { x = 4; y += rowH + 12; }

            s.bounds = { x, y, w, rowH };
            lastRowY = y;

            for (int i = 0; i < s.comps.size(); ++i)
            {
                auto* c = s.comps[i];
                juce::Rectangle<int> cell (x + 12 + i * cellW, y + titleH, cellW, cellH);
                cell = cell.reduced (3, 0);
                cell.removeFromTop (labelH);

                if (dynamic_cast<juce::Slider*> (c) != nullptr) c->setBounds (cell);
                else                                            c->setBounds (cell.removeFromTop (30));
            }
            x += w + 12;
            lastX = x;
        }

        infoRect = {};
        if (avail - lastX >= 220)
            infoRect = { lastX, lastRowY, avail - lastX, rowH };

        // диаграмма модуля в свободном месте под карточками
        if (diagram != nullptr)
        {
            const int top = lastRowY + rowH + 12;
            const int h = getHeight() - top - 2;
            diagram->setVisible (h >= 80);
            if (h >= 80) diagram->setBounds (4, top, getWidth() - 8, h);
        }
        repaint();
    }

private:
    struct Section { juce::String title; juce::Array<juce::Component*> comps; juce::Rectangle<int> bounds; };
    std::vector<Section> sections;
    juce::String title, description, hint;
    juce::Colour accent;
    juce::Rectangle<int> infoRect;
    juce::Component* diagram = nullptr;
};
