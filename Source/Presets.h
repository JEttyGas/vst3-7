#pragma once
#include <string>
#include <utility>
#include <vector>

// Индексы choice-параметров для читаемости пресетов
namespace Ln { enum { Beat1, Beat2, Beat4 }; }
namespace Dv { enum { s1, s2, s3, s4, s6, s8, s12, s16 }; }   // слайсов на паттерн: 1,2,3,4,6,8,12,16
namespace Tm { enum { Normal, Half, TapeStop, TapeStart, Stutter, Reverse, Scratch, Quarter, FastStutter, Backspin, Wobble, DoubleRepeat }; }
namespace Vo { enum { Off, Gate, Pump, RampDown, RampUp, Tremolo, Random, ShortGate, LongGate, StairsDown, StairsUp, Offbeat, Swing }; }
namespace Fi { enum { Off, LP, HP, BP }; }
namespace Di { enum { Off, Soft, Hard, Fold, Crush, Tube, Rectify, Sine }; }
namespace Sp { enum { Off, Freeze, Blur, Robot, Whisper, Gate, Shift, Comb, Warp, Sustain, Crush, Octaver, Tilt, Sweep, Vowel, Shatter, Mirror }; }
namespace Wn { enum { Hann, Flat, Pluck, Swell }; }
namespace Sl { enum { Off, Shuffle, RandomPick, Reverse, SwapPairs, Skip2, Skip3, Mirror, HoldGroups, HalfTime, Lag }; }   // slType
namespace St { enum { n2, n3, n4, n6, n8, n12, n16, n32 }; }                                                          // slSteps
namespace Sn { enum { b1, b2, b4, b8 }; }                                                                             // slLength
namespace Qn { enum { Off, Octaves, Fifths, MajPent, MinPent, Whole, Chromatic }; }                                   // grainQuant
namespace Fs { enum { Pattern, LFO, Envelope }; }                                                                     // filterSrc
namespace Rs { enum { Sine, Triangle, Square, Saw }; }                                                                // ringShape
namespace Dy { enum { s16, s16T, s8, s8T, s8D, s4, s4T, s4D, s2, s1, Free }; }                                        // dlySync
namespace Dm { enum { Stereo, PingPong, Tape }; }
namespace Ls { enum { Sine, Triangle, SawUp, SawDown, Square, Random, SmoothRnd }; }                                    // lfo*Shape
namespace Ly { enum { Free, Bars4, Bars2, Bar1, Half, Quarter, Eighth, Sixteenth, QuarterT, EighthT }; }               // lfo*Sync                                                                     // dlyMode

struct Preset
{
    const char* category;
    const char* name;
    std::vector<std::pair<const char*, float>> values;   // не указанные параметры = значения по умолчанию
};

inline const std::vector<Preset>& presetsV1()
{
    static const std::vector<Preset> presets =
    {
        { "Init", "Init (clean)", {} },

        // ============================ RHYTHMIC ============================
        { "Rhythmic", "Classic 8th Gate",   { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s2} } },
        { "Rhythmic", "16th Gate",          { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4} } },
        { "Rhythmic", "Trance Gate",        { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"filterType",Fi::LP}, {"cutoff",3500}, {"res",1.5f}, {"filterMod",0.4f} } },
        { "Rhythmic", "Triplet Gate",       { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s3} } },
        { "Rhythmic", "Offbeat Chop",       { {"length",Ln::Beat1}, {"volType",Vo::Offbeat}, {"volDiv",Dv::s2} } },
        { "Rhythmic", "Swing Gate",         { {"length",Ln::Beat2}, {"volType",Vo::Swing}, {"volDiv",Dv::s4} } },
        { "Rhythmic", "Staccato 16th",      { {"length",Ln::Beat1}, {"volType",Vo::ShortGate}, {"volDiv",Dv::s4} } },
        { "Rhythmic", "Legato Pulse",       { {"length",Ln::Beat1}, {"volType",Vo::LongGate}, {"volDiv",Dv::s2}, {"volDepth",0.8f} } },
        { "Rhythmic", "Random Gate 16th",   { {"length",Ln::Beat4}, {"volType",Vo::Random}, {"volDiv",Dv::s16} } },
        { "Rhythmic", "Random Gate 8th",    { {"length",Ln::Beat4}, {"volType",Vo::Random}, {"volDiv",Dv::s8} } },
        { "Rhythmic", "Syncopated Chop",    { {"length",Ln::Beat4}, {"volType",Vo::Random}, {"volDiv",Dv::s12} } },
        { "Rhythmic", "Stairs Down",        { {"length",Ln::Beat1}, {"volType",Vo::StairsDown}, {"volDiv",Dv::s1}, {"volDepth",0.9f} } },
        { "Rhythmic", "Stairs Up",          { {"length",Ln::Beat1}, {"volType",Vo::StairsUp}, {"volDiv",Dv::s1}, {"volDepth",0.9f} } },
        { "Rhythmic", "Fast Tremolo",       { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s4}, {"volDepth",0.7f} } },
        { "Rhythmic", "Slow Tremolo",       { {"length",Ln::Beat2}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s2}, {"volDepth",0.6f} } },
        { "Rhythmic", "Pump 1/4",           { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s1}, {"volDepth",0.9f} } },
        { "Rhythmic", "Pump 1/8",           { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s2}, {"volDepth",0.8f} } },
        { "Rhythmic", "Saw Down 8th",       { {"length",Ln::Beat1}, {"volType",Vo::RampDown}, {"volDiv",Dv::s2} } },
        { "Rhythmic", "Saw Up 8th",         { {"length",Ln::Beat1}, {"volType",Vo::RampUp}, {"volDiv",Dv::s2} } },

        // ============================== DRUMS =============================
        { "Drums", "Hat Roll 16th",         { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s1} } },
        { "Drums", "Roll 32nd",             { {"length",Ln::Beat1}, {"timeType",Tm::FastStutter}, {"timeDiv",Dv::s1} } },
        { "Drums", "Snare Roll Tremolo",    { {"length",Ln::Beat1}, {"timeType",Tm::FastStutter}, {"timeDiv",Dv::s1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s4}, {"volDepth",0.5f} } },
        { "Drums", "Kick Sidechain",        { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s1}, {"volDepth",1.0f} } },
        { "Drums", "Half-Time Feel",        { {"length",Ln::Beat2}, {"timeType",Tm::Half}, {"timeDiv",Dv::s1} } },
        { "Drums", "Quarter-Time Crawl",    { {"length",Ln::Beat4}, {"timeType",Tm::Quarter}, {"timeDiv",Dv::s1} } },
        { "Drums", "Tape Stop Bar",         { {"length",Ln::Beat4}, {"timeType",Tm::TapeStop}, {"timeDiv",Dv::s1} } },
        { "Drums", "Reverse Fill",          { {"length",Ln::Beat2}, {"timeType",Tm::Reverse}, {"timeDiv",Dv::s2} } },
        { "Drums", "Double Hit Groove",     { {"length",Ln::Beat1}, {"timeType",Tm::DoubleRepeat}, {"timeDiv",Dv::s2} } },
        { "Drums", "Crushed Break",         { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s1}, {"volDepth",0.6f}, {"distType",Di::Crush}, {"distBits",7}, {"distRate",3} } },
        { "Drums", "Dirty Drum Bus",        { {"distType",Di::Hard}, {"drive",0.35f}, {"distMix",0.5f} } },
        { "Drums", "Fat Drums",             { {"distType",Di::Soft}, {"drive",0.45f}, {"distMix",0.6f} } },
        { "Drums", "Room Smack",            { {"distType",Di::Soft}, {"drive",0.3f}, {"distMix",0.5f}, {"revRoom",0.3f}, {"revMix",0.25f} } },
        { "Drums", "Drum Filter Sweep",     { {"length",Ln::Beat4}, {"volType",Vo::RampUp}, {"volDiv",Dv::s1}, {"volDepth",0.0f}, {"filterType",Fi::LP}, {"cutoff",600}, {"res",2.5f}, {"filterMod",1.0f} } },
        { "Drums", "Tape Wobble Loop",      { {"length",Ln::Beat2}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s2} } },
        { "Drums", "Scratch Fill",          { {"length",Ln::Beat1}, {"timeType",Tm::Scratch}, {"timeDiv",Dv::s3} } },

        // ============================== BASS ==============================
        { "Bass", "Bass Gate 8th",          { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s2}, {"volDepth",0.9f}, {"filterType",Fi::LP}, {"cutoff",400}, {"res",1.0f} } },
        { "Bass", "Wobble 1/8",             { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s2}, {"volDepth",0.0f}, {"filterType",Fi::LP}, {"cutoff",250}, {"res",6.0f}, {"filterMod",1.0f} } },
        { "Bass", "Wobble 1/4",             { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s1}, {"volDepth",0.0f}, {"filterType",Fi::LP}, {"cutoff",300}, {"res",5.0f}, {"filterMod",1.0f} } },
        { "Bass", "Wobble Triplet",         { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s3}, {"volDepth",0.0f}, {"filterType",Fi::LP}, {"cutoff",250}, {"res",6.0f}, {"filterMod",0.9f} } },
        { "Bass", "Fold Bass",              { {"distType",Di::Fold}, {"drive",0.6f}, {"filterType",Fi::LP}, {"cutoff",1800}, {"res",1.0f} } },
        { "Bass", "Sub Saturation",         { {"distType",Di::Soft}, {"drive",0.3f}, {"distMix",0.5f}, {"filterType",Fi::LP}, {"cutoff",900} } },
        { "Bass", "Pumping Sub",            { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s1}, {"volDepth",0.85f}, {"filterType",Fi::LP}, {"cutoff",500} } },
        { "Bass", "Growl",                  { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s2}, {"volDepth",0.0f}, {"distType",Di::Hard}, {"drive",0.7f}, {"distMix",0.8f}, {"filterType",Fi::LP}, {"cutoff",1200}, {"res",3.0f}, {"filterMod",0.6f} } },
        { "Bass", "Octave Down",            { {"grainMix",0.8f}, {"grainSize",60}, {"grainDensity",40}, {"grainPitch",-12}, {"grainSpread",0}, {"grainWidth",0.2f} } },
        { "Bass", "Reese Comb",             { {"specType",Sp::Comb}, {"specAmount",0.6f}, {"specTone",0.2f}, {"filterType",Fi::LP}, {"cutoff",2000} } },

        // ============================ MELODIES ============================
        { "Melodies", "Trance Arp Gate",    { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"revRoom",0.6f}, {"revMix",0.25f} } },
        { "Melodies", "Pluck Gate",         { {"length",Ln::Beat1}, {"volType",Vo::ShortGate}, {"volDiv",Dv::s4}, {"revRoom",0.5f}, {"revMix",0.3f} } },
        { "Melodies", "Dream Shimmer",      { {"grainMix",0.5f}, {"grainSize",150}, {"grainDensity",30}, {"grainPitch",12}, {"grainSpread",200}, {"grainWidth",1.0f}, {"revRoom",0.8f}, {"revMix",0.45f} } },
        { "Melodies", "Fifth Shimmer",      { {"grainMix",0.5f}, {"grainSize",150}, {"grainDensity",30}, {"grainPitch",7}, {"grainSpread",200}, {"grainWidth",1.0f}, {"revRoom",0.8f}, {"revMix",0.4f} } },
        { "Melodies", "Octave Sparkle",     { {"grainMix",0.45f}, {"grainSize",50}, {"grainDensity",40}, {"grainPitch",12}, {"grainSpread",60}, {"grainWidth",0.9f} } },
        { "Melodies", "Detune Chorus",      { {"grainMix",0.5f}, {"grainSize",80}, {"grainDensity",40}, {"grainPitch",0.15f}, {"grainPitchRand",0.3f}, {"grainSpread",20}, {"grainWidth",1.0f} } },
        { "Melodies", "Tape Melody",        { {"length",Ln::Beat2}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s2}, {"filterType",Fi::LP}, {"cutoff",6000}, {"distType",Di::Soft}, {"drive",0.2f}, {"distMix",0.5f} } },
        { "Melodies", "Stuttered Lead",     { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s2}, {"revMix",0.2f} } },
        { "Melodies", "Bell Resonator",     { {"specType",Sp::Comb}, {"specAmount",0.8f}, {"specTone",0.3f}, {"specMix",0.6f}, {"revRoom",0.6f}, {"revMix",0.4f} } },
        { "Melodies", "Glass Pad",          { {"specType",Sp::Blur}, {"specAmount",0.85f}, {"revRoom",0.8f}, {"revMix",0.5f} } },
        { "Melodies", "Slow Swell",         { {"length",Ln::Beat4}, {"volType",Vo::RampUp}, {"volDiv",Dv::s1}, {"revRoom",0.8f}, {"revMix",0.4f} } },
        { "Melodies", "Reverse Echo",       { {"length",Ln::Beat1}, {"timeType",Tm::Reverse}, {"timeDiv",Dv::s2}, {"revMix",0.4f} } },
        { "Melodies", "Chiptune Crunch",    { {"distType",Di::Crush}, {"distBits",6}, {"distRate",8} } },
        { "Melodies", "Harmonic Warp",      { {"specType",Sp::Warp}, {"specAmount",1.0f}, {"specPitch",5}, {"specMix",0.5f} } },

        // ============================= VOCALS =============================
        { "Vocals", "Vocal Chops 16th",     { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"revRoom",0.4f}, {"revMix",0.2f} } },
        { "Vocals", "Vocal Stutter",        { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s2} } },
        { "Vocals", "Robot Voice",          { {"specType",Sp::Robot}, {"specAmount",1.0f} } },
        { "Vocals", "Whisper",              { {"specType",Sp::Whisper}, {"specAmount",1.0f} } },
        { "Vocals", "Ghost Whisper",        { {"specType",Sp::Whisper}, {"specAmount",0.9f}, {"specMix",0.8f}, {"revRoom",0.7f}, {"revMix",0.45f} } },
        { "Vocals", "Alien Voice",          { {"specType",Sp::Shift}, {"specAmount",1.0f}, {"specPitch",4} } },
        { "Vocals", "Octave Down Voice",    { {"grainMix",0.7f}, {"grainSize",70}, {"grainDensity",45}, {"grainPitch",-12}, {"grainSpread",0}, {"grainWidth",0.3f} } },
        { "Vocals", "Choir Cloud",          { {"grainMix",0.45f}, {"grainSize",200}, {"grainDensity",20}, {"grainPitch",12}, {"grainSpread",300}, {"grainWidth",1.0f}, {"revRoom",0.85f}, {"revMix",0.6f} } },
        { "Vocals", "Warp Voice",           { {"specType",Sp::Warp}, {"specAmount",1.0f}, {"specPitch",4}, {"specMix",0.8f} } },
        { "Vocals", "Vocal Freeze Pad",     { {"specType",Sp::Freeze}, {"specHold",1}, {"specTone",0.3f}, {"specMix",0.8f}, {"revMix",0.4f} } },
        { "Vocals", "Distant Voice",        { {"filterType",Fi::LP}, {"cutoff",3000}, {"revRoom",0.8f}, {"revMix",0.6f} } },

        // ============================== MIXING ============================
        { "Mixing", "Warm Tape Saturation", { {"distType",Di::Soft}, {"drive",0.25f}, {"distMix",0.5f} } },
        { "Mixing", "Gentle Pump",          { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s1}, {"volDepth",0.35f} } },
        { "Mixing", "Subtle Tremolo",       { {"length",Ln::Beat2}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s2}, {"volDepth",0.25f} } },
        { "Mixing", "Stereo Widener",       { {"grainMix",0.35f}, {"grainSize",40}, {"grainDensity",60}, {"grainPitch",0.05f}, {"grainPitchRand",0.1f}, {"grainSpread",15}, {"grainWidth",1.0f} } },
        { "Mixing", "Air Room",             { {"revRoom",0.25f}, {"revDamp",0.6f}, {"revMix",0.18f} } },
        { "Mixing", "Plate Space",          { {"revRoom",0.5f}, {"revDamp",0.5f}, {"revMix",0.25f} } },
        { "Mixing", "Big Hall",             { {"revRoom",0.85f}, {"revDamp",0.4f}, {"revMix",0.35f} } },
        { "Mixing", "Low Cut 120 Hz",       { {"filterType",Fi::HP}, {"cutoff",120} } },
        { "Mixing", "High Cut 8 kHz",       { {"filterType",Fi::LP}, {"cutoff",8000} } },
        { "Mixing", "Subtle Crunch",        { {"distType",Di::Crush}, {"distBits",12}, {"distRate",2}, {"distMix",0.4f} } },
        { "Mixing", "Fold Exciter",         { {"distType",Di::Fold}, {"drive",0.2f}, {"distMix",0.25f} } },

        // ============================= CREATIVE ===========================
        { "Creative", "Spectral Freeze",    { {"specType",Sp::Freeze}, {"specHold",1}, {"revMix",0.3f} } },
        { "Creative", "Freeze Granular",    { {"grainFreeze",1}, {"grainMix",1.0f}, {"grainSize",250}, {"grainDensity",15}, {"grainSpread",800}, {"grainWidth",1.0f} } },
        { "Creative", "Spectral Blur",      { {"specType",Sp::Blur}, {"specAmount",0.9f}, {"specMix",0.85f} } },
        { "Creative", "Smear Reverb",       { {"specType",Sp::Blur}, {"specAmount",0.95f}, {"revRoom",0.7f}, {"revMix",0.5f} } },
        { "Creative", "Robot Army",         { {"specType",Sp::Robot}, {"specAmount",1.0f}, {"ringFreq",110}, {"ringMix",0.3f} } },
        { "Creative", "Whisper Cloud",      { {"specType",Sp::Whisper}, {"specAmount",1.0f}, {"grainMix",0.4f}, {"grainSize",120}, {"grainWidth",1.0f} } },
        { "Creative", "Comb Resonator",     { {"specType",Sp::Comb}, {"specAmount",0.9f}, {"specTone",0.25f} } },
        { "Creative", "Bin Shifter Up",     { {"specType",Sp::Shift}, {"specAmount",1.0f}, {"specPitch",2} } },
        { "Creative", "Bin Shifter Down",   { {"specType",Sp::Shift}, {"specAmount",1.0f}, {"specPitch",-2} } },
        { "Creative", "Warp Up",            { {"specType",Sp::Warp}, {"specAmount",1.0f}, {"specPitch",7} } },
        { "Creative", "Warp Down",          { {"specType",Sp::Warp}, {"specAmount",1.0f}, {"specPitch",-7} } },
        { "Creative", "Spectral Skeleton",  { {"specType",Sp::Gate}, {"specAmount",0.6f}, {"specTone",0.3f} } },
        { "Creative", "Spectral Ghost",     { {"specType",Sp::Gate}, {"specAmount",0.5f}, {"specTone",0.8f} } },
        { "Creative", "Ring Metal",         { {"ringFreq",800}, {"ringMix",0.7f} } },
        { "Creative", "Ring Bell",          { {"ringFreq",220}, {"ringMix",1.0f}, {"revRoom",0.6f}, {"revMix",0.5f} } },
        { "Creative", "Dalek",              { {"ringFreq",30}, {"ringMix",1.0f} } },
        { "Creative", "Tremolo Ring",       { {"ringFreq",20}, {"ringMix",0.8f} } },
        { "Creative", "Granular Cloud",     { {"grainMix",0.8f}, {"grainSize",120}, {"grainDensity",25}, {"grainSpread",300}, {"grainWidth",0.8f}, {"grainRev",0.1f} } },
        { "Creative", "Grain Rewind",       { {"grainMix",0.9f}, {"grainSize",200}, {"grainDensity",12}, {"grainPitch",-5}, {"grainSpread",400}, {"grainWidth",1.0f}, {"grainRev",0.7f} } },
        { "Creative", "Alien Texture",      { {"specType",Sp::Warp}, {"specPitch",5}, {"specAmount",1.0f}, {"grainMix",0.5f}, {"grainPitch",-7}, {"grainPitchRand",3}, {"ringFreq",90}, {"ringMix",0.25f} } },
        { "Creative", "Underwater",         { {"length",Ln::Beat1}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s1}, {"filterType",Fi::LP}, {"cutoff",900}, {"res",1.0f}, {"revMix",0.4f} } },
        { "Creative", "Radio Scratch",      { {"length",Ln::Beat1}, {"timeType",Tm::Scratch}, {"timeDiv",Dv::s2}, {"distType",Di::Crush}, {"distBits",10}, {"distRate",3} } },
        { "Creative", "Metallic Rain",      { {"specType",Sp::Comb}, {"specAmount",0.8f}, {"specTone",0.1f}, {"grainMix",0.5f}, {"grainSize",20}, {"grainDensity",60}, {"grainSpread",300}, {"grainWidth",1.0f} } },
        { "Creative", "Time Smear",         { {"length",Ln::Beat2}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s1}, {"specType",Sp::Blur}, {"specAmount",0.7f} } },

        // ============================== GLITCH ============================
        { "Glitch", "Glitch Machine",       { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s6}, {"volType",Vo::Random}, {"volDiv",Dv::s16}, {"distType",Di::Soft}, {"drive",0.3f}, {"grainMix",0.3f}, {"grainSize",30}, {"grainDensity",40}, {"grainSpread",100} } },
        { "Glitch", "Buffer Chop",          { {"length",Ln::Beat1}, {"timeType",Tm::DoubleRepeat}, {"timeDiv",Dv::s4}, {"volType",Vo::Random}, {"volDiv",Dv::s8} } },
        { "Glitch", "Stutter Gate 16",      { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s4}, {"volType",Vo::Gate}, {"volDiv",Dv::s8}, {"volDepth",0.8f} } },
        { "Glitch", "Fast Stutter Roll",    { {"length",Ln::Beat1}, {"timeType",Tm::FastStutter}, {"timeDiv",Dv::s2} } },
        { "Glitch", "Scratch Gate",         { {"length",Ln::Beat1}, {"timeType",Tm::Scratch}, {"timeDiv",Dv::s3}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"volDepth",0.7f} } },
        { "Glitch", "Reverse Glitch",       { {"length",Ln::Beat1}, {"timeType",Tm::Reverse}, {"timeDiv",Dv::s4}, {"volType",Vo::Random}, {"volDiv",Dv::s8}, {"volDepth",0.7f} } },
        { "Glitch", "Crushed Stutter",      { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s2}, {"distType",Di::Crush}, {"distBits",8}, {"distRate",5} } },
        { "Glitch", "Digital Chaos",        { {"length",Ln::Beat2}, {"timeType",Tm::FastStutter}, {"timeDiv",Dv::s4}, {"volType",Vo::Random}, {"volDiv",Dv::s16}, {"distType",Di::Crush}, {"distBits",9}, {"distRate",4}, {"grainMix",0.3f}, {"grainSize",25}, {"grainDensity",50}, {"grainRev",0.4f} } },
        { "Glitch", "Granular Glitch",      { {"length",Ln::Beat2}, {"volType",Vo::Random}, {"volDiv",Dv::s8}, {"volDepth",0.7f}, {"grainMix",0.6f}, {"grainSize",20}, {"grainDensity",50}, {"grainSpread",400}, {"grainRev",0.4f} } },
        { "Glitch", "Skipping CD",          { {"length",Ln::Beat1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s1}, {"volType",Vo::Random}, {"volDiv",Dv::s16}, {"volDepth",0.6f} } },
        { "Glitch", "Tape Chew",            { {"length",Ln::Beat1}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s1}, {"distType",Di::Crush}, {"distBits",12}, {"distRate",2} } },

        // ============================== AMBIENT ===========================
        { "Ambient", "Hazy Cloud",          { {"grainMix",0.8f}, {"grainSize",220}, {"grainDensity",18}, {"grainSpread",500}, {"grainWidth",1.0f}, {"revRoom",0.7f}, {"revMix",0.4f} } },
        { "Ambient", "Infinite Freeze Verb",{ {"specType",Sp::Freeze}, {"specHold",1}, {"specTone",0.2f}, {"revRoom",0.95f}, {"revDamp",0.3f}, {"revMix",0.7f} } },
        { "Ambient", "Slow Motion",         { {"length",Ln::Beat4}, {"timeType",Tm::Quarter}, {"timeDiv",Dv::s1}, {"filterType",Fi::LP}, {"cutoff",5000}, {"revMix",0.4f} } },
        { "Ambient", "Dream Tape",          { {"length",Ln::Beat2}, {"timeType",Tm::Wobble}, {"timeDiv",Dv::s1}, {"filterType",Fi::LP}, {"cutoff",4500}, {"distType",Di::Soft}, {"drive",0.15f}, {"distMix",0.5f}, {"revRoom",0.7f}, {"revMix",0.5f} } },
        { "Ambient", "Floating Grains",     { {"grainMix",0.8f}, {"grainSize",300}, {"grainDensity",6}, {"grainPitch",12}, {"grainSpread",700}, {"grainWidth",1.0f}, {"revRoom",0.85f}, {"revMix",0.7f} } },
        { "Ambient", "Deep Space",          { {"grainMix",0.6f}, {"grainSize",400}, {"grainDensity",8}, {"grainPitch",-12}, {"grainSpread",600}, {"grainWidth",1.0f}, {"revRoom",0.95f}, {"revDamp",0.6f}, {"revMix",0.6f} } },
        { "Ambient", "Shimmer Verb",        { {"grainMix",0.5f}, {"grainSize",160}, {"grainDensity",28}, {"grainPitch",12}, {"grainSpread",150}, {"grainWidth",1.0f}, {"revRoom",0.9f}, {"revMix",0.7f} } },
        { "Ambient", "Spectral Mist",       { {"specType",Sp::Blur}, {"specAmount",0.97f}, {"revRoom",0.8f}, {"revMix",0.5f} } },
        { "Ambient", "Whisper Pad",         { {"specType",Sp::Whisper}, {"specAmount",0.7f}, {"specMix",0.7f}, {"revRoom",0.85f}, {"revMix",0.6f} } },
        { "Ambient", "Glass Texture",       { {"specType",Sp::Comb}, {"specAmount",0.6f}, {"grainMix",0.4f}, {"grainPitch",12}, {"grainSize",120}, {"grainWidth",1.0f}, {"revRoom",0.8f}, {"revMix",0.5f} } },

        // ============================ TRANSITIONS =========================
        { "Transitions", "Tape Stop 1 Bar",    { {"length",Ln::Beat4}, {"timeType",Tm::TapeStop}, {"timeDiv",Dv::s1} } },
        { "Transitions", "Tape Stop 2 Beats",  { {"length",Ln::Beat2}, {"timeType",Tm::TapeStop}, {"timeDiv",Dv::s1} } },
        { "Transitions", "Tape Start Rise",    { {"length",Ln::Beat4}, {"timeType",Tm::TapeStart}, {"timeDiv",Dv::s1} } },
        { "Transitions", "Backspin 1 Bar",     { {"length",Ln::Beat4}, {"timeType",Tm::Backspin}, {"timeDiv",Dv::s1} } },
        { "Transitions", "Backspin 2 Beats",   { {"length",Ln::Beat2}, {"timeType",Tm::Backspin}, {"timeDiv",Dv::s1} } },
        { "Transitions", "Reverse Swoosh",     { {"length",Ln::Beat4}, {"timeType",Tm::Reverse}, {"timeDiv",Dv::s1}, {"revMix",0.4f} } },
        { "Transitions", "Filter Riser",       { {"length",Ln::Beat4}, {"volType",Vo::RampUp}, {"volDiv",Dv::s1}, {"volDepth",0.5f}, {"filterType",Fi::LP}, {"cutoff",1500}, {"res",3.0f}, {"filterMod",1.0f} } },
        { "Transitions", "Filter Drop",        { {"length",Ln::Beat4}, {"volType",Vo::RampDown}, {"volDiv",Dv::s1}, {"volDepth",0.3f}, {"filterType",Fi::LP}, {"cutoff",2000}, {"res",3.0f}, {"filterMod",1.0f} } },
        { "Transitions", "Whisper Sweep",      { {"length",Ln::Beat4}, {"volType",Vo::RampUp}, {"volDiv",Dv::s1}, {"specType",Sp::Whisper}, {"specAmount",0.8f} } },
        { "Transitions", "Reverb Swell",       { {"length",Ln::Beat4}, {"volType",Vo::RampUp}, {"volDiv",Dv::s1}, {"revRoom",0.95f}, {"revMix",0.6f} } },
        { "Transitions", "Scratch Transition", { {"length",Ln::Beat2}, {"timeType",Tm::Scratch}, {"timeDiv",Dv::s4} } },
        { "Transitions", "Slow Down Fade",     { {"length",Ln::Beat4}, {"timeType",Tm::Half}, {"timeDiv",Dv::s1}, {"filterType",Fi::LP}, {"cutoff",4000} } },

        // ============================ REARRANGE (слайсер) =================
        { "Rearrange", "Shuffle 8 Steps",       { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b2}, {"slSeed",3} } },
        { "Rearrange", "Shuffle 16 / 1 Bar",    { {"slType",Sl::Shuffle}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"slSeed",7} } },
        { "Rearrange", "Evolving Shuffle",      { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b2}, {"slEvolve",1} } },
        { "Rearrange", "Random Pick 16",        { {"slType",Sl::RandomPick}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"slEvolve",1} } },
        { "Rearrange", "Reverse Bar",           { {"slType",Sl::Reverse}, {"slSteps",St::n8}, {"slLength",Sn::b4} } },
        { "Rearrange", "Reverse 16th Slices",   { {"slType",Sl::Reverse}, {"slSteps",St::n4}, {"slLength",Sn::b1} } },
        { "Rearrange", "Swap Pairs",            { {"slType",Sl::SwapPairs}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Skip x2",               { {"slType",Sl::Skip2}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Skip x3",               { {"slType",Sl::Skip3}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Mirror Loop",           { {"slType",Sl::Mirror}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Hold Groups",           { {"slType",Sl::HoldGroups}, {"slSteps",St::n16}, {"slLength",Sn::b4} } },
        { "Rearrange", "Half-Time Slices",      { {"slType",Sl::HalfTime}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Lag Groove",            { {"slType",Sl::Lag}, {"slSteps",St::n8}, {"slLength",Sn::b2} } },
        { "Rearrange", "Chaos Reverse",         { {"slType",Sl::Shuffle}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"slReverse",0.5f}, {"slEvolve",1} } },
        { "Rearrange", "Gated Shuffle",         { {"slType",Sl::Shuffle}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"slMute",0.3f}, {"slChance",0.8f}, {"slSeed",11} } },
        { "Rearrange", "Subtle Variations",     { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b4}, {"slChance",0.25f}, {"slEvolve",1} } },
        { "Rearrange", "Glitch Shuffle",        { {"slType",Sl::Shuffle}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"slEvolve",1}, {"timeType",Tm::Stutter}, {"timeDiv",Dv::s4} } },
        { "Rearrange", "Shuffle Verb",          { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b2}, {"slSeed",5}, {"revRoom",0.7f}, {"revMix",0.3f} } },
        { "Rearrange", "Triplet Shuffle",       { {"slType",Sl::Shuffle}, {"slSteps",St::n12}, {"slLength",Sn::b4}, {"slSeed",21} } },
        { "Rearrange", "Stutter Skip",          { {"slType",Sl::Skip2}, {"slSteps",St::n16}, {"slLength",Sn::b4}, {"timeType",Tm::DoubleRepeat}, {"timeDiv",Dv::s4} } },
        { "Rearrange", "Reverse Shuffle Echo",  { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b2}, {"slReverse",0.4f}, {"dlySync",Dy::s8}, {"dlyFeedback",0.45f}, {"dlyMix",0.3f} } },

        // ============================== ECHO (delay) ======================
        { "Echo", "Classic 1/8 Echo",     { {"dlySync",Dy::s8}, {"dlyFeedback",0.45f}, {"dlyTone",6000}, {"dlyMix",0.35f} } },
        { "Echo", "Dotted Eighth",        { {"dlySync",Dy::s8D}, {"dlyFeedback",0.5f}, {"dlyMix",0.4f} } },
        { "Echo", "Ping-Pong 1/4",        { {"dlySync",Dy::s4}, {"dlyMode",Dm::PingPong}, {"dlyFeedback",0.55f}, {"dlyMix",0.4f} } },
        { "Echo", "Triplet Bounce",       { {"dlySync",Dy::s8T}, {"dlyMode",Dm::PingPong}, {"dlyFeedback",0.5f}, {"dlyMix",0.4f} } },
        { "Echo", "Tape Echo",            { {"dlySync",Dy::s8}, {"dlyMode",Dm::Tape}, {"dlyFeedback",0.55f}, {"dlyTone",3500}, {"dlyMod",0.4f}, {"dlyMix",0.4f} } },
        { "Echo", "Slapback",             { {"dlySync",Dy::Free}, {"dlyFree",90}, {"dlyFeedback",0.1f}, {"dlyMix",0.3f} } },
        { "Echo", "Dub Echo",             { {"dlySync",Dy::s4D}, {"dlyMode",Dm::Tape}, {"dlyFeedback",0.7f}, {"dlyTone",2500}, {"dlyLow",200}, {"dlyMix",0.45f} } },
        { "Echo", "Ambient Echo",         { {"dlySync",Dy::s2}, {"dlyFeedback",0.65f}, {"dlyTone",5000}, {"dlyMix",0.4f}, {"revRoom",0.8f}, {"revMix",0.3f} } },
        { "Echo", "Ducked Delay",         { {"dlySync",Dy::s8}, {"dlyFeedback",0.5f}, {"dlyDuck",0.8f}, {"dlyMix",0.5f} } },
        { "Echo", "Wobble Delay",         { {"dlySync",Dy::s8D}, {"dlyMode",Dm::Tape}, {"dlyMod",0.8f}, {"dlyModRate",2.5f}, {"dlyFeedback",0.6f}, {"dlyMix",0.4f} } },
        { "Echo", "Infinite Repeats",     { {"dlySync",Dy::s4}, {"dlyFeedback",0.93f}, {"dlyTone",4000}, {"dlyLow",150}, {"dlyMix",0.5f} } },
        { "Echo", "16th Pulse Echo",      { {"dlySync",Dy::s16}, {"dlyFeedback",0.4f}, {"dlyMode",Dm::PingPong}, {"dlyMix",0.35f} } },
        { "Echo", "Filtered Echo Trail",  { {"dlySync",Dy::s8D}, {"dlyFeedback",0.6f}, {"dlyTone",1800}, {"dlyLow",300}, {"dlyMix",0.5f} } },
        { "Echo", "Gate + Echo",          { {"length",Ln::Beat1}, {"volType",Vo::ShortGate}, {"volDiv",Dv::s4}, {"dlySync",Dy::s8D}, {"dlyFeedback",0.5f}, {"dlyMix",0.5f} } },

        // ===== новые спектральные =====
        { "Creative", "Vowel Talk",             { {"specType",Sp::Vowel}, {"specTone",0.1f}, {"specAmount",1.0f}, {"specMotion",0.8f}, {"specRate",0.7f} } },
        { "Creative", "Formant Sweep",          { {"specType",Sp::Vowel}, {"specTone",0.5f}, {"specAmount",1.0f}, {"specMotion",1.0f}, {"specRate",0.4f}, {"specMix",0.9f} } },
        { "Creative", "Spectral Sustain Pad",   { {"specType",Sp::Sustain}, {"specAmount",0.9f}, {"specTone",0.3f}, {"revRoom",0.7f}, {"revMix",0.4f} } },
        { "Creative", "Infinite Sustain",       { {"specType",Sp::Sustain}, {"specAmount",1.0f}, {"specTone",0.1f}, {"revMix",0.5f} } },
        { "Creative", "Spectral Crush",         { {"specType",Sp::Crush}, {"specAmount",0.6f}, {"specTone",0.3f} } },
        { "Creative", "Octave Layers",          { {"specType",Sp::Octaver}, {"specAmount",1.0f}, {"specTone",0.5f} } },
        { "Creative", "Octave Down Layer",      { {"specType",Sp::Octaver}, {"specAmount",1.0f}, {"specTone",0.05f} } },
        { "Creative", "Bright Tilt",            { {"specType",Sp::Tilt}, {"specAmount",1.0f}, {"specPitch",14} } },
        { "Creative", "Dark Tilt",              { {"specType",Sp::Tilt}, {"specAmount",1.0f}, {"specPitch",-14} } },
        { "Creative", "Phaser Comb Sweep",      { {"specType",Sp::Sweep}, {"specTone",0.3f}, {"specAmount",0.9f}, {"specRate",0.35f} } },
        { "Creative", "Shatter Glass",          { {"specType",Sp::Shatter}, {"specAmount",0.7f}, {"specTone",0.2f} } },
        { "Creative", "Shatter Dust",           { {"specType",Sp::Shatter}, {"specAmount",0.9f}, {"specTone",0.8f} } },
        { "Creative", "Spectral Mirror",        { {"specType",Sp::Mirror}, {"specAmount",1.0f}, {"specTone",0.5f} } },
        { "Creative", "Wobble Shift",           { {"specType",Sp::Shift}, {"specAmount",1.0f}, {"specPitch",5}, {"specMotion",1.0f}, {"specRate",0.5f} } },
        { "Creative", "Drifting Warp",          { {"specType",Sp::Warp}, {"specAmount",1.0f}, {"specPitch",0}, {"specMotion",0.9f}, {"specRate",0.3f} } },
        { "Vocals",   "Robot Vowel",            { {"specType",Sp::Vowel}, {"specTone",0.3f}, {"specAmount",0.9f}, {"ringFreq",60}, {"ringShape",Rs::Square}, {"ringMix",0.25f} } },
        { "Vocals",   "Whisper Vowel",          { {"specType",Sp::Vowel}, {"specTone",0.7f}, {"specAmount",0.8f}, {"specMotion",0.5f}, {"revMix",0.3f} } },
        { "Ambient",  "Sustain Cloud",          { {"specType",Sp::Sustain}, {"specAmount",0.95f}, {"grainMix",0.4f}, {"grainSize",200}, {"grainWidth",1.0f}, {"revRoom",0.85f}, {"revMix",0.5f} } },
        { "Ambient",  "Mirror Mist",            { {"specType",Sp::Mirror}, {"specAmount",0.7f}, {"specMix",0.6f}, {"revRoom",0.8f}, {"revMix",0.45f} } },

        // ===== фильтр / ring / дисторшн / реверб =====
        { "Bass", "Auto Wah",             { {"filterType",Fi::BP}, {"cutoff",800}, {"res",4.0f}, {"filterSrc",Fs::Envelope}, {"filterMod",0.8f} } },
        { "Bass", "LFO Sweep 24 dB",      { {"filterType",Fi::LP}, {"filterSlope",1}, {"cutoff",700}, {"res",3.0f}, {"filterSrc",Fs::LFO}, {"filterMod",1.0f}, {"filterRate",0.5f} } },
        { "Bass", "Envelope Pump LP",     { {"filterType",Fi::LP}, {"cutoff",500}, {"res",2.0f}, {"filterSrc",Fs::Envelope}, {"filterMod",-0.8f} } },
        { "Bass", "Tube Warmth",          { {"distType",Di::Tube}, {"drive",0.35f}, {"distBias",0.15f}, {"distMix",0.6f} } },
        { "Bass", "Rectifier Octave",     { {"distType",Di::Rectify}, {"drive",0.4f}, {"distMix",0.5f}, {"filterType",Fi::LP}, {"cutoff",1500} } },
        { "Creative", "Sine Fold Fuzz",   { {"distType",Di::Sine}, {"drive",0.6f} } },
        { "Creative", "Square Ring",      { {"ringShape",Rs::Square}, {"ringFreq",180}, {"ringMix",0.6f} } },
        { "Creative", "FM Siren",         { {"ringFreq",400}, {"ringFM",0.8f}, {"ringFMRate",4}, {"ringMix",0.8f} } },
        { "Creative", "AM Tremolo Fast",  { {"ringMode",1}, {"ringFreq",20}, {"ringMix",1.0f} } },
        { "Creative", "Saw Ring Growl",   { {"ringShape",Rs::Saw}, {"ringFreq",70}, {"ringMix",0.7f}, {"distType",Di::Hard}, {"drive",0.4f}, {"distMix",0.5f} } },
        { "Ambient",  "Reverb Freeze Wash", { {"revFreeze",1}, {"revRoom",1.0f}, {"revMix",0.7f} } },
        { "Mixing",   "Pre-Delay Plate",  { {"revPre",80}, {"revRoom",0.5f}, {"revMix",0.3f} } },
        { "Mixing",   "Ducked Reverb",    { {"revRoom",0.8f}, {"revDuck",0.7f}, {"revMix",0.5f} } },
        { "Mixing",   "Tone-Shaped Sat",  { {"distType",Di::Soft}, {"drive",0.4f}, {"distTone",6000}, {"distMix",0.6f} } },

        // ===== гранулятор: новые возможности =====
        { "Melodies", "Pentatonic Cloud",    { {"grainMix",0.8f}, {"grainQuant",Qn::MajPent}, {"grainPitchRand",12}, {"grainSize",150}, {"grainDensity",14}, {"grainSpread",200}, {"grainWidth",1.0f} } },
        { "Melodies", "Minor Shimmer Arp",   { {"grainMix",0.7f}, {"grainQuant",Qn::MinPent}, {"grainPitchRand",12}, {"grainSize",200}, {"grainDensity",8}, {"grainWidth",1.0f}, {"revRoom",0.8f}, {"revMix",0.4f} } },
        { "Ambient",  "Scanning Freeze",     { {"grainFreeze",1}, {"grainScan",0.3f}, {"grainMix",1.0f}, {"grainSize",250}, {"grainDensity",18}, {"grainSpread",100}, {"grainWidth",1.0f} } },
        { "Creative", "Size Chaos",          { {"grainMix",0.8f}, {"grainSize",150}, {"grainSizeRand",1.0f}, {"grainDensity",25}, {"grainPitchRand",5}, {"grainWidth",1.0f} } },
        { "Creative", "Plucked Grains",      { {"grainMix",0.9f}, {"grainWindow",Wn::Pluck}, {"grainSize",120}, {"grainDensity",14}, {"grainPitchRand",12}, {"grainQuant",Qn::MajPent}, {"grainWidth",1.0f} } },
        { "Creative", "Swell Grains",        { {"grainMix",0.9f}, {"grainWindow",Wn::Swell}, {"grainSize",260}, {"grainDensity",10}, {"grainSpread",400}, {"grainWidth",1.0f}, {"revMix",0.3f} } },

        // ===== ритм: свинг, фаза, сглаживание =====
        { "Rhythmic", "Swung 16ths",          { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"swing",0.25f} } },
        { "Rhythmic", "Swung Pump",           { {"length",Ln::Beat1}, {"volType",Vo::Pump}, {"volDiv",Dv::s2}, {"swing",0.3f}, {"volDepth",0.85f} } },
        { "Rhythmic", "Shifted Gate",         { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s2}, {"shift",0.25f} } },
        { "Rhythmic", "Smooth Tremolo Pad",   { {"length",Ln::Beat2}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s2}, {"volDepth",0.5f}, {"volSmooth",20} } },
        { "Rhythmic", "Soft Gate",            { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"volSmooth",12}, {"volDepth",0.9f} } },

        // ============================== MOTION (LFO) ======================
        { "Motion", "Filter Wobble LFO",     { {"filterType",Fi::LP}, {"cutoff",500}, {"res",4.0f}, {"lfo1Shape",Ls::Sine}, {"lfo1Sync",Ly::Eighth}, {"m1.cutoff",0.35f} } },
        { "Motion", "Slow Filter Drift",     { {"filterType",Fi::LP}, {"cutoff",900}, {"res",2.0f}, {"lfo1Shape",Ls::SmoothRnd}, {"lfo1Rate",0.15f}, {"m1.cutoff",0.30f} } },
        { "Motion", "Grain Pitch Drift",     { {"grainMix",0.8f}, {"grainSize",140}, {"grainDensity",24}, {"grainWidth",1.0f}, {"lfo1Shape",Ls::SmoothRnd}, {"lfo1Rate",0.3f}, {"m1.grainPitch",0.15f} } },
        { "Motion", "Breathing Reverb",      { {"revRoom",0.8f}, {"revMix",0.35f}, {"lfo1Shape",Ls::Sine}, {"lfo1Sync",Ly::Bars2}, {"m1.revMix",0.25f} } },
        { "Motion", "Pulsing Echo",          { {"dlySync",Dy::s8D}, {"dlyFeedback",0.5f}, {"dlyMix",0.4f}, {"lfo1Shape",Ls::Triangle}, {"lfo1Sync",Ly::Bar1}, {"m1.dlyFeedback",0.30f} } },
        { "Motion", "Spectral Morph Comb",   { {"specType",Sp::Comb}, {"specAmount",0.9f}, {"lfo1Shape",Ls::Triangle}, {"lfo1Rate",0.2f}, {"m1.specTone",0.8f} } },
        { "Motion", "Vowel Auto-Talk",       { {"specType",Sp::Vowel}, {"specAmount",1.0f}, {"lfo1Shape",Ls::Sine}, {"lfo1Rate",0.6f}, {"m1.specTone",1.0f} } },
        { "Motion", "Drive Pulse",           { {"distType",Di::Soft}, {"drive",0.25f}, {"lfo1Shape",Ls::Square}, {"lfo1Sync",Ly::Quarter}, {"m1.drive",0.35f} } },
        { "Motion", "Siren Ring LFO",        { {"ringMix",0.8f}, {"ringFreq",500}, {"lfo1Shape",Ls::Triangle}, {"lfo1Rate",1.5f}, {"m1.ringFreq",0.25f} } },
        { "Motion", "Slicer Fade Mix",       { {"slType",Sl::Shuffle}, {"slSteps",St::n8}, {"slLength",Sn::b2}, {"lfo1Shape",Ls::Sine}, {"lfo1Sync",Ly::Bar1}, {"m1.slMix",0.5f} } },
        { "Motion", "Time Amount Sweep",     { {"length",Ln::Beat1}, {"timeType",Tm::Reverse}, {"timeDiv",Dv::s4}, {"lfo1Shape",Ls::Triangle}, {"lfo1Sync",Ly::Bars2}, {"m1.timeAmount",0.5f} } },
        { "Motion", "Grain Size Breathe",    { {"grainMix",0.8f}, {"grainDensity",22}, {"grainWidth",1.0f}, {"lfo1Shape",Ls::Sine}, {"lfo1Rate",0.25f}, {"m1.grainSize",0.30f} } },
        { "Motion", "Random Cutoff S&H",     { {"filterType",Fi::LP}, {"cutoff",700}, {"res",3.0f}, {"lfo1Shape",Ls::Random}, {"lfo1Sync",Ly::Eighth}, {"m1.cutoff",0.35f} } },
        { "Motion", "Swinging Gate",         { {"length",Ln::Beat1}, {"volType",Vo::Gate}, {"volDiv",Dv::s4}, {"lfo1Shape",Ls::Sine}, {"lfo1Sync",Ly::Bars4}, {"m1.swing",0.5f} } },
        { "Motion", "Reverb Width Sway",     { {"revRoom",0.8f}, {"revMix",0.4f}, {"lfo1Shape",Ls::Sine}, {"lfo1Rate",0.2f}, {"m1.revWidth",0.5f} } },
        { "Motion", "Tape Delay Wow",        { {"dlySync",Dy::Free}, {"dlyFree",350}, {"dlyMode",Dm::Tape}, {"dlyFeedback",0.55f}, {"dlyMix",0.4f}, {"lfo1Shape",Ls::Sine}, {"lfo1Rate",0.4f}, {"m1.dlyFree",0.03f} } },
        { "Motion", "Ring FM Drift",         { {"ringFreq",220}, {"ringMix",0.6f}, {"lfo1Shape",Ls::SmoothRnd}, {"lfo1Rate",0.5f}, {"m1.ringFM",0.5f} } },
        { "Motion", "Spectral Pitch Sweep",  { {"specType",Sp::Shift}, {"specPitch",5}, {"specMix",0.8f}, {"lfo1Shape",Ls::Sine}, {"lfo1Rate",0.15f}, {"m1.specPitch",0.4f} } },
        { "Motion", "Pumping Pad",           { {"length",Ln::Beat1}, {"volType",Vo::Tremolo}, {"volDiv",Dv::s1}, {"volDepth",0.0f}, {"revMix",0.3f}, {"lfo1Shape",Ls::Triangle}, {"lfo1Sync",Ly::Bar1}, {"m1.volDepth",0.5f} } },
    };
    return presets;
}

// Папки пресетов "v2" и "v3"
#include "PresetsV2.h"
#include "PresetsV3.h"

// Полный список: прежние пресеты, затем папка "v2", затем "v3"
inline const std::vector<Preset>& factoryPresets()
{
    static const std::vector<Preset> all = []
    {
        std::vector<Preset> v = presetsV1();
        for (const auto* extra : { &presetsV2(), &presetsV3() })
            v.insert (v.end(), extra->begin(), extra->end());
        return v;
    }();
    return all;
}
