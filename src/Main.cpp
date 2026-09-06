/**
 * @file Main.cpp
 * @brief Unlinked External - Main application entry point, window management, overlay rendering loop, and UI composition.
 */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <Shellapi.h>
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_NONE 0x00000000
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "catalog.hpp"
#include "store.hpp"
#include "offsets.hpp"
#include "world.hpp"
#include "move.hpp"
#include "silent.hpp"
#include "weather.hpp"
#include "gameplay.hpp"
#include "aim.hpp"
#include "esp.hpp"
#include "ur/ur.hpp"
#include "explorer.hpp"
#include "browse.hpp"
#include "ui/key_labels.hpp"
#include "ui/layout.hpp"

namespace {

constexpr float MenuWidth = 700.0f;
constexpr float MenuHeight = 610.0f;
constexpr float HeaderHeight = 52.0f;
constexpr float RailWidth = 80.0f;
constexpr float TabHeight = 70.0f;
constexpr float TabGap = 0.0f;
constexpr float ExploreWidth = 500.0f;
constexpr float ExploreHeight = 600.0f;
constexpr int TabCount = 5;
constexpr int TabAimbot = 0;
constexpr int TabEsp = 1;
constexpr int TabRage = 2;
constexpr int TabConfigs = 3;
constexpr int TabSettings = 4;

static const char* Fonts[ ] = {
    "Inter",
    "Segoe UI",
    "Segoe UI Symbol"
};

struct TabSpec {
    const char* name;
    const char* id;
    ur::icons::Icon icon;
};

static const TabSpec Tabs[ TabCount ] = {
    { "Aimbot", "tab.aimbot", ur::icons::Icon::Crosshairs },
    { "ESP", "tab.esp", ur::icons::Icon::Eye },
    { "Rage", "tab.rage", ur::icons::Icon::FireFlame },
    { "Configs", "tab.configs", ur::icons::Icon::Folder },
    { "Settings", "tab.settings", ur::icons::Icon::Gear }
};

struct Shell {
    CVector origin;
    CVector grab;
    bool ready = false;
    bool painted = false;
    bool visible = true;
    bool held = false;
    bool mouse = false;
    bool escape = false;
    bool insert = false;
    int tab = 0;
    float tabAt = 0.0f;
    float pageIn = 1.0f;
    float pageDir = 1.0f;
    int menuKey = VK_INSERT;
    bool listen = false;
    bool limit = false;
    float fps = 240.0f;
    bool capped = false;
    bool misc = false;
    bool game = false;
    bool overlay = false;
    bool theme = false;
    bool slide = false;
    const char* knob = nullptr;
    bool afk = false;
    bool uncap = false;
    bool vsync = false;
    bool watermark = true;
    bool showFps = true;
    bool stream = false;
    bool streamed = false;
    float fade = 100.0f;
};

struct Combat {
    bool on = false;
    bool team = true;
    bool vis = true;
    bool sticky = false;
    bool pred = false;
    bool drawFov = true;
    float fov = 72.0f;
    float smooth = 40.0f;
    int key = VK_RBUTTON;
    int bones = 1;
    int sort = 0;
    bool listen = false;
    bool general = false;
    bool targeting = false;
};

struct Fury {
    bool jump = false;
    bool noclip = false;
};

struct Quiet {
    bool on = false;
    bool team = true;
    bool vis = true;
    bool pred = false;
    int key = 'M';
    int bones = 1;
    int sort = 0;
    bool listen = false;
    bool fold = false;
};

struct Vision {
    bool on = false;
    bool box = true;
    bool name = true;
    bool health = true;
    bool dist = true;
    bool skeleton = false;
    bool snap = false;
    bool team = true;
    bool overlay = false;
    bool visual = false;
    bool custom = false;
    float range = 500.0f;
};

using EspFeat = esp::EspFeat;
inline constexpr auto FeatBox = esp::FeatBox;
inline constexpr auto FeatName = esp::FeatName;
inline constexpr auto FeatHealth = esp::FeatHealth;
inline constexpr auto FeatDist = esp::FeatDist;
inline constexpr auto FeatSkel = esp::FeatSkel;
inline constexpr auto FeatSnap = esp::FeatSnap;
inline constexpr auto FeatCount = esp::FeatCount;

using Coat = esp::Coat;

struct Vault {
    char names[ store::SlotMax ][ store::NameCap ] = { };
    int count = 0;
    int pick = 0;
    char live[ store::NameCap ] = "Default";
    char draft[ store::NameCap ] = { };
    bool type = false;
    bool confirm = false;
    bool ready = false;
    float scroll = 0.0f;
    float noteAge = 0.0f;
    char note[ 64 ] = { };
};

struct Browse {
    CVector origin;
    CVector grab;
    bool ready = false;
    bool open = false;
    bool held = false;
    bool docked = true;
    CVector dock;
    uintptr_t pick = 0;
    float scroll = 0.0f;
    float propScroll = 0.0f;
    char find[ 48 ] = { };
    bool type = false;
    bool confirm = false;
    bool mark = false;
    char note[ 64 ] = { };
    float noteAge = 0.0f;
};

static Shell Menu;
static Browse Tree;
static Combat Aim;
static Quiet Mute;
static Fury Rage;
static Vision Esp;
static Coat Dye;
static const CColor EspTints[ ] = {
    CColor( 72, 220, 118 ),
    CColor( 232, 72, 72 ),
    CColor( 64, 220, 230 ),
    CColor( 64, 132, 255 ),
    CColor( 168, 88, 255 ),
    CColor( 255, 96, 180 ),
    CColor( 255, 148, 48 ),
    CColor( 255, 220, 64 ),
    CColor( 160, 255, 64 ),
    CColor( 244, 244, 248 ),
    CColor( 232, 188, 72 ),
    CColor( 176, 24, 48 ),
    CColor( 18, 18, 22 )
};

static CColor FeatColor( int Feat, bool Seen ) {
    int Pick = esp::PickFeatTint( Dye, Feat, Seen, 3 );
    return EspTints[ Pick ];
}

static Vault Packs;

struct Channel {
    bool open = false;
    bool dismissed = false;
    bool mismatch = false;
    unsigned nextScan = 0;
    char client[ 48 ] = { };
    char dump[ 48 ] = { };
};

static Channel LiveCh;
static bool ChanMouse = false;

static bool KeyWas[ 256 ] = { };
static char LoadedFaces[ 8 ][ MAX_PATH ] = { };
static int LoadedFaceCount = 0;
static CFont TitleFace;
static float TitleScale = 0.0f;
static std::string LogoPath;

struct Tone {
    CColor surface;
    CColor elevated;
    CColor header;
    CColor outline;
    CColor highlight;
    CColor text;
    CColor faint;
    CColor accent;
    CColor accentSoft;
    CColor shade;
    CColor card;
    CColor rail;
    CColor groove;
    CColor trackOff;
    CColor trackOn;
    CColor foldLine;
    CColor ink;
    CColor inkHot;
};

static Tone Dress;
static const char* DropId = nullptr;
static CRectangle DropField;
static const char* const* DropOpts = nullptr;
static int DropCount = 0;
static int* DropPick = nullptr;
static int* DropBits = nullptr;
static bool DropMany = false;
static bool DropFresh = false;

static bool Held( int Key ) {
    return ( GetAsyncKeyState( Key ) & 0x8000 ) != 0;
}

struct Stamp {
    CVector origin;
    CVector grab;
    bool ready = false;
    bool held = false;
};

static Stamp Badge;

static bool Moving( ) {
    return Menu.held || Tree.held || Badge.held;
}

static bool Edge( int Key, bool& Prior ) {
    return ui::DetectRisingEdge( Held( Key ), Prior );
}

static CVector Cursor( ) {
    HWND Handle = ( HWND )ur::app::window( );
    POINT Point = { };
    GetCursorPos( &Point );
    if ( Handle )
        ScreenToClient( Handle, &Point );
    return CVector( ( float )Point.x, ( float )Point.y );
}

static bool CursorVisible( ) {
    if ( Menu.visible )
        return true;
    CURSORINFO Info = { };
    Info.cbSize = sizeof( Info );
    if ( !GetCursorInfo( &Info ) )
        return false;
    return ( Info.flags & CURSOR_SHOWING ) != 0;
}

static CVector ScreenMid( ) {
    const world::Snap& Live = world::View( );
    if ( Live.clientW > 64 && Live.clientH > 64 ) {
        POINT Mid{ Live.clientX + Live.clientW / 2, Live.clientY + Live.clientH / 2 };
        HWND Overlay = ( HWND )ur::app::window( );
        if ( Overlay )
            ScreenToClient( Overlay, &Mid );
        return CVector( ( float )Mid.x, ( float )Mid.y );
    }
    return CVector( ( float )ur::app::width( ) * 0.5f, ( float )ur::app::height( ) * 0.5f );
}

static void Tokens( ) {
    if ( !Menu.painted ) {
        ur::theme::apply( 4 );
        Style->Rounding = 14.0f;
        Style->FadeSpeed = 28.0f;
        Style->Glass = true;
        Style->Shadows = false;
        Menu.painted = true;
    }

    static const Tone Set[ 3 ] = {
        {
            CColor( 10, 28, 58, 170 ), CColor( 13, 39, 78, 145 ), CColor( 12, 34, 70, 190 ),
            CColor( 72, 146, 230, 100 ), CColor( 130, 200, 255, 30 ), CColor( 230, 242, 255 ),
            CColor( 145, 180, 220 ), CColor( 70, 160, 255 ), CColor( 140, 205, 255 ),
            CColor( 5, 18, 40, 90 ), CColor( 12, 35, 70, 150 ), CColor( 8, 22, 48, 95 ),
            CColor( 11, 30, 60, 135 ), CColor( 28, 70, 125, 180 ), CColor( 55, 145, 240, 230 ),
            CColor( 70, 100, 140, 120 ), CColor( 220, 236, 255 ), CColor( 240, 248, 255 )
        },
        {
            CColor( 22, 16, 12, 208 ), CColor( 28, 20, 14, 168 ), CColor( 24, 18, 14, 230 ),
            CColor( 140, 100, 60, 70 ), CColor( 220, 170, 110, 22 ), CColor( 240, 226, 208 ),
            CColor( 150, 122, 96 ), CColor( 196, 132, 72 ), CColor( 220, 176, 120 ),
            CColor( 16, 10, 6, 120 ), CColor( 48, 34, 24, 242 ), CColor( 16, 10, 8, 140 ),
            CColor( 32, 22, 16, 230 ), CColor( 36, 26, 20, 230 ), CColor( 140, 88, 48, 230 ),
            CColor( 130, 96, 64, 110 ), CColor( 226, 208, 186 ), CColor( 244, 232, 214 )
        },
        {
            CColor( 14, 18, 14, 208 ), CColor( 16, 22, 16, 168 ), CColor( 16, 22, 16, 230 ),
            CColor( 90, 130, 80, 70 ), CColor( 160, 210, 140, 22 ), CColor( 228, 236, 220 ),
            CColor( 118, 140, 112 ), CColor( 112, 168, 86 ), CColor( 160, 200, 130 ),
            CColor( 8, 14, 8, 120 ), CColor( 32, 42, 32, 242 ), CColor( 10, 14, 10, 140 ),
            CColor( 22, 30, 22, 230 ), CColor( 26, 34, 26, 230 ), CColor( 72, 120, 64, 230 ),
            CColor( 80, 112, 78, 110 ), CColor( 210, 224, 206 ), CColor( 232, 240, 226 )
        }
    };

    int Index = skin::tone( );
    if ( Index < 0 || Index >= 3 )
        Index = 0;
    Dress = Set[ Index ];
    Style->Backdrop = CColor( 0, 0, 0, 0 );
    Style->Surface = Dress.surface;
    Style->Elevated = Dress.elevated;
    Style->Header = Dress.header;
    Style->Outline = Dress.outline;
    Style->Highlight = Dress.highlight;
    Style->Text = Dress.text;
    Style->Faint = Dress.faint;
    Style->Accent = Dress.accent;
    Style->AccentSoft = Dress.accentSoft;
    Style->Shade = Dress.shade;
}

static void Center( float Across, float Vertical, float Scale ) {
    Menu.origin = CVector( ( Across - MenuWidth * Scale ) * 0.5f, ( Vertical - MenuHeight * Scale ) * 0.5f );
}

static void OpenLiveFolds( ) {
    Aim.general = Aim.on;
    Aim.targeting = Aim.on;
    Rage.jump = move::Live( ).jump || move::Live( ).infJump;
    Rage.noclip = move::Live( ).noclip;
    Mute.fold = Mute.on;
    Esp.overlay = Esp.on;
    Esp.visual = Esp.skeleton || Esp.snap;
    Esp.custom = false;
    Menu.misc = false;
    Menu.game = false;
    Menu.overlay = false;
    Menu.theme = false;
}

static void ClampBox( CVector& Origin, float Across, float Vertical, float Wide, float Tall ) {
    ui::ClampBox( Origin.Horizontal, Origin.Vertical, Across, Vertical, Wide, Tall );
}

static void Clamp( float Across, float Vertical, float Wide, float Tall ) {
    ClampBox( Menu.origin, Across, Vertical, Wide, Tall );
}

static void Gate( bool Over, const CVector& Point ) {
    ur::overlay::Options& Overlay = ur::app::overlay_options( );
    bool Through = !Over && !Moving( ) && !Menu.slide;
    if ( Overlay.click_through != Through )
        Overlay.click_through = Through;

    Input->ApplyPosition( Point.Horizontal, Point.Vertical );
    if ( Menu.held )
        Input->Pointer = PointerMove;
    else if ( Over )
        Input->Pointer = PointerHand;
}

static void Drag( const CRectangle& Bounds, const CVector& Point, float Across, float Vertical, float Wide, float Tall, bool AllowStart ) {
    bool Press = Held( VK_LBUTTON );

    bool CanStart = AllowStart && !Menu.mouse && Bounds.Contains( Point ) && !Menu.slide;
    ui::UpdateDragState( Press, CanStart, Point.Horizontal, Point.Vertical,
                         Menu.origin.Horizontal, Menu.origin.Vertical,
                         Menu.grab.Horizontal, Menu.grab.Vertical, Menu.held );

    if ( !Press ) {
        Menu.slide = false;
        Menu.knob = nullptr;
    }

    Menu.mouse = Press;
    Clamp( Across, Vertical, Wide, Tall );
}

static const char* KeyLabel( int Code ) {
    return ui::KeyLabel( Code );
}

static int PollBind( bool Mouse1 ) {
    return ui::PollKeyBind( Mouse1, Held, KeyWas );
}

static void SyncBindKeys( ) {
    ui::SyncKeyHistory( Held, KeyWas );
}

static void Pace( ) {
    static LARGE_INTEGER Freq = { };
    static LARGE_INTEGER Last = { };
    if ( Freq.QuadPart == 0 )
        QueryPerformanceFrequency( &Freq );

    bool WantSync = Menu.vsync && !Menu.limit;
    if ( ur::app::vsync( ) != WantSync ) {
        ur::app::set_vsync( WantSync );
        Last.QuadPart = 0;
    }
    Menu.capped = Menu.limit;

    if ( !Menu.limit )
        return;

    double Goal = play::ComputeFrameGoalTime( Menu.fps );

    LARGE_INTEGER Now = { };
    QueryPerformanceCounter( &Now );
    if ( Last.QuadPart != 0 ) {
        for ( ;; ) {
            QueryPerformanceCounter( &Now );
            double Spent = ( double )( Now.QuadPart - Last.QuadPart ) / ( double )Freq.QuadPart;
            int Action = play::ComputeFramePacingAction( Goal, Spent );
            if ( Action == 0 )
                break;
            if ( Action == 1 )
                Sleep( 1 );
        }
    }
    QueryPerformanceCounter( &Last );
}

static CColor Mix( CColor From, CColor Till, float Amount ) {
    return From.Blend( Till, Amount );
}

static void EnsureTitle( float Scale ) {
    if ( TitleFace.LineSpan < 1.0f )
        TitleFace.Create( Fonts, 3, 22.0f, 600 );
    if ( TitleScale != Scale ) {
        TitleFace.Rescale( Scale );
        TitleScale = Scale;
    }
}

static void DrawIce( const CRectangle& Clip, const CRectangle& Fill, float Round, float Amount ) {
    if ( Amount < 0.01f )
        return;

    float Keep = Canvas->Opacity;
    Canvas->Opacity = Keep * Amount;
    Canvas->PushClip( Clip );
    unsigned int Former = Canvas->Effect( skin::effect( ) );
    Canvas->Rectangle( Fill, CColor( 255, 255, 255 ), Round );
    Canvas->Effect( Former );
    Canvas->PopClip( );
    Canvas->Opacity = Keep;
}

static void DrawTitle( const CRectangle& Header, float Scale, const char* Title ) {
    EnsureTitle( Scale );
    CVector Size = TitleFace.Measure( Title );
    float LogoSize = 25.0f * Scale;
    float Gap = 8.0f * Scale;
    unsigned long long Logo = LogoPath.empty( ) ? 0 : ur::image::file( LogoPath.c_str( ), 64 );
    float Total = Size.Horizontal + ( Logo ? LogoSize + Gap : 0.0f );
    float Left = Header.Left + ( Header.Width - Total ) * 0.5f;
    float Top = Header.Top + ( Header.Height - TitleFace.LineSpan ) * 0.5f;
    if ( Logo )
        Canvas->Image( CRectangle( Left, Header.Top + ( Header.Height - LogoSize ) * 0.5f, LogoSize, LogoSize ), Logo, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), CColor( 255, 255, 255 ), LogoSize * 0.18f );
    Left += Logo ? LogoSize + Gap : 0.0f;
    Canvas->Write( &TitleFace, CVector( Left, Top ), Dress.inkHot.Fade( 0.93f ), Title );
}

static CRectangle TabBounds( const CRectangle& Rail, float Scale, int Index ) {
    ui::RectBounds B = ui::ComputeTabBounds( Rail.Left, Rail.Top, Rail.Width, TabHeight, TabGap, Scale, Index );
    return CRectangle( B.left, B.top, B.width, B.height );
}

static CRectangle TabPlate( const CRectangle& Tab, bool Top, bool Bot, float Round ) {
    ui::RectBounds B = ui::ComputeTabPlate( { Tab.Left, Tab.Top, Tab.Width, Tab.Height }, Top, Bot, Round );
    return CRectangle( B.left, B.top, B.width, B.height );
}

static void DrawTabPlate( const CRectangle& Tab, int Index, float Round, CColor Fill, unsigned int Effect, float Amount ) {
    if ( Amount < 0.01f )
        return;

    bool Top = Index == 0;
    bool Bot = Index == TabCount - 1;
    bool Cap = Top || Bot;
    float Use = Cap ? Round : 0.0f;
    float Keep = Canvas->Opacity;
    Canvas->Opacity = Keep * Amount;
    if ( Cap )
        Canvas->PushClip( Tab );
    unsigned int Former = Effect ? Canvas->Effect( Effect ) : 0;
    Canvas->Rectangle( TabPlate( Tab, Top, Bot, Round ), Fill, Use );
    if ( Effect )
        Canvas->Effect( Former );
    if ( Cap )
        Canvas->PopClip( );
    Canvas->Opacity = Keep;
}

static void DrawTabSwipe( const CRectangle& Rail, float Scale ) {
    float Want = ( float )Menu.tab;
    float Step = 20.0f * Context->DeltaTime;
    if ( Step > 1.0f )
        Step = 1.0f;
    Menu.tabAt += ( Want - Menu.tabAt ) * Step;

    float Stride = TabHeight * Scale + TabGap * Scale;
    float Tall = TabHeight * Scale;
    float Round = 12.0f * Scale;
    CRectangle Stack( Rail.Left, Rail.Top, Rail.Width, Stride * ( float )TabCount );
    CRectangle Fill( Rail.Left, Rail.Top + Stride * Menu.tabAt, Rail.Width, Tall );
    bool Top = Fill.Top <= Stack.Top + 0.75f;
    bool Bot = Fill.Bottom( ) >= Stack.Bottom( ) - 0.75f;
    float Use = ( Top || Bot ) ? Round : 0.0f;
    Canvas->PushClip( Stack );
    DrawIce( Fill, TabPlate( Fill, Top, Bot, Round ), Use, 1.0f );
    Canvas->PopClip( );
}

static void DrawTab( const CRectangle& Tab, const TabSpec& Spec, int Index, float Scale, bool Hovered ) {
    char HoverId[ 48 ];
    snprintf( HoverId, sizeof( HoverId ), "%s.hover", Spec.id );

    bool Selected = Menu.tab == Index;
    float Dist = Menu.tabAt - ( float )Index;
    if ( Dist < 0.0f )
        Dist = -Dist;
    float Active = Dist < 1.0f ? 1.0f - Dist : 0.0f;
    float Hover = ur::motion::toward( HoverId, ( Hovered && !Selected ) ? 1.0f : 0.0f, 26.0f );
    float Round = 12.0f * Scale;
    float Mark = 24.0f * Scale;

    DrawTabPlate( Tab, Index, Round, CColor( 255, 255, 255, 18 ), 0, Hover * ( 1.0f - Active ) );

    unsigned long long Icon = ur::glyphs::image( Spec.icon, ( int )( 26.0f * Scale + 0.5f ), ur::glyphs::Weight::Solid );
    CRectangle Glyph( Tab.Left + ( Tab.Width - Mark ) * 0.5f, Tab.Top + 13.0f * Scale, Mark, Mark );
    CColor Ink = Mix( Mix( Style->Faint, Dress.ink, Hover ), Dress.inkHot, Active );
    if ( Icon )
        Canvas->Image( Glyph, Icon, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), Ink, 0.0f );

    CVector Size = Font->Measure( Spec.name );
    float LabelTop = Glyph.Bottom( ) + 7.0f * Scale;
    Canvas->Text( CVector( Tab.Left + ( Tab.Width - Size.Horizontal ) * 0.5f, LabelTop ), Ink, Spec.name );
}

static CRectangle CloseBounds( const CRectangle& Header, float Scale ) {
    ui::RectBounds B = ui::ComputeCloseBounds( Header.Right( ), Header.Top, Header.Height, Scale, 32.0f, 8.0f );
    return CRectangle( B.left, B.top, B.width, B.height );
}

static bool DrawClose( const CRectangle& Header, const CVector& Point, bool Click, float Scale, const char* Motion, bool Exit ) {
    CRectangle Close = CloseBounds( Header, Scale );
    bool Over = Close.Contains( Point ) && !Moving( );
    float Hover = ur::motion::toward( Motion, Over ? 1.0f : 0.0f, 26.0f );
    float Round = 9.0f * Scale;

    if ( Hover > 0.02f )
        Canvas->Rectangle( Close, CColor( 210, 64, 72, ( int )( 200.0f * Hover ) ), Round );

    unsigned long long Icon = ur::glyphs::image( ur::icons::Icon::Xmark, ( int )( 14.0f * Scale + 0.5f ), ur::glyphs::Weight::Solid );
    CColor Ink = Mix( CColor( 220, 226, 236 ), CColor( 255, 246, 246 ), Hover );
    float Mark = 14.0f * Scale;
    if ( Icon )
        Canvas->Image( CRectangle( Close.Left + ( Close.Width - Mark ) * 0.5f, Close.Top + ( Close.Height - Mark ) * 0.5f, Mark, Mark ), Icon, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), Ink, 0.0f );

    if ( Over && Click && Exit )
        ur::app::quit( );

    return Over;
}

static bool Listening( ) {
    return Menu.listen || Aim.listen || Mute.listen || Packs.type || Tree.type;
}

static bool DrawSlider( float Left, float Top, float Wide, const char* Label, const char* Id, float& Value, float Lo, float Hi, const CVector& Point, bool Click, bool Press, float Scale ) {
    Value = ui::ClampSliderValue( Value, Lo, Hi );

    float Row = 26.0f * Scale;
    float TextW = 0.0f;
    if ( Label ) {
        Canvas->Text( CVector( Left, Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Text, Label );
        TextW = Font->Measure( Label ).Horizontal + 10.0f * Scale;
    }

    char Stamp[ 24 ];
    snprintf( Stamp, sizeof( Stamp ), "%d", ( int )Value );
    CVector Size = Font->Measure( Stamp );
    float Thumb = 14.0f * Scale;
    float GrooveW = ui::ComputeGrooveWidth( Wide, TextW, Size.Horizontal, Thumb, Scale );
    CRectangle Groove( Left + TextW, Top + 10.0f * Scale, GrooveW, 6.0f * Scale );
    CRectangle Hit( Left, Top, Wide, Row );
    bool Mine = Menu.slide && Menu.knob == Id;
    bool Over = Hit.Contains( Point ) && !Moving( ) && !Listening( );
    if ( Over && Click ) {
        Menu.slide = true;
        Menu.knob = Id;
        Mine = true;
    }
    if ( !Press ) {
        Menu.slide = false;
        Menu.knob = nullptr;
        Mine = false;
    }
    if ( Mine ) {
        Value = ui::ComputeSliderValueFromPoint( Point.Horizontal, Groove.Left, Groove.Width, Lo, Hi );
    }

    float Portion = ui::ComputeSliderRatio( Value, Lo, Hi );
    Canvas->Text( CVector( Groove.Right( ) + Thumb * 0.5f + 10.0f * Scale, Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Text, Stamp );
    Canvas->Rectangle( Groove, Dress.groove, 2.5f * Scale );
    if ( Portion > 0.0f )
        Canvas->Rectangle( CRectangle( Groove.Left, Groove.Top, Groove.Width * Portion, Groove.Height ), Mix( Style->Accent, Style->AccentSoft, 0.35f ), 2.5f * Scale );
    Canvas->Rectangle( CRectangle( Groove.Left + Groove.Width * Portion - Thumb * 0.5f, Groove.Top + ( Groove.Height - Thumb ) * 0.5f, Thumb, Thumb ), CColor( 236, 242, 252 ), Thumb * 0.5f );
    return Over || Mine;
}

static bool DrawBind( float Left, float Top, const char* Label, const char* Motion, int& Code, bool& Listen, const CVector& Point, bool Click, float Scale ) {
    Canvas->Text( CVector( Left, Top ), Style->Faint, Label );
    CRectangle Field( Left, Top + Font->LineSpan + 6.0f * Scale, 156.0f * Scale, 30.0f * Scale );
    bool Over = Field.Contains( Point ) && !Moving( ) && !Menu.slide;
    if ( Over && Click ) {
        Listen = true;
        SyncBindKeys( );
    }

    float Wait = ur::motion::toward( Motion, Listen ? 1.0f : 0.0f, 26.0f );
    char HoverId[ 48 ];
    snprintf( HoverId, sizeof( HoverId ), "%s.hover", Motion );
    float Hover = ur::motion::toward( HoverId, ( Over && !Listen ) ? 1.0f : 0.0f, 26.0f );
    DrawIce( Field, Field, 6.0f * Scale, 1.0f );
    Canvas->Border( Field, Mix( CColor( 90, 110, 140, 160 ), Style->AccentSoft, Wait * 0.75f + Hover * 0.4f ), 6.0f * Scale, 1.0f );
    Canvas->Text( CVector( Field.Left + 10.0f * Scale, Field.Top + ( Field.Height - Font->LineSpan ) * 0.5f ), Mix( Style->Text, Style->AccentSoft, Wait ), Listen ? "Press a key..." : KeyLabel( Code ) );
    return Over;
}

static bool DrawSwitch( const CRectangle& Row, const char* Label, const char* Id, bool& Value, const CVector& Point, bool Click, float Scale ) {
    ui::RectBounds TrackB;
    ui::ComputeSwitchTrack( Row.Right( ), Row.Top, Row.Height, Scale, TrackB );
    CRectangle Track( TrackB.left, TrackB.top, TrackB.width, TrackB.height );
    bool Over = Row.Contains( Point ) && !Moving( ) && !Menu.slide;
    if ( Over && Click )
        Value = !Value;

    float On = ur::motion::toward( Id, Value ? 1.0f : 0.0f, 28.0f );
    Canvas->Text( CVector( Row.Left, Row.Top + ( Row.Height - Font->LineSpan ) * 0.5f ), Style->Text, Label );
    Canvas->Rectangle( Track, Mix( Dress.trackOff, Mix( Dress.trackOn, Style->Accent, 0.4f ), On ), TrackB.height * 0.5f );
    ui::RectBounds KnobB;
    ui::ComputeSwitchKnob( TrackB, Scale, On, KnobB );
    Canvas->Rectangle( CRectangle( KnobB.left, KnobB.top, KnobB.width, KnobB.height ), Dress.inkHot, KnobB.width * 0.5f );
    return Over;
}


static void TickAfk( ) {
    play::TickAfk( Menu.afk, Context->DeltaTime );
    play::TickUncap( Menu.uncap );
}

static bool DrawFold( float Left, float Top, float Wide, float Head, float BodyNeed, float Round, float Scale, const char* Name, const char* Motion, bool& OpenFlag, const CVector& Point, bool Click, CRectangle& Body, float& Open ) {
    Open = ur::motion::toward( Motion, OpenFlag ? 1.0f : 0.0f, 32.0f );
    ui::RectBounds CardB, BarB, BodyB;
    ui::ComputeFoldCard( Left, Top, Wide, Head, BodyNeed, Open, CardB, BarB, BodyB );
    CRectangle Card( CardB.left, CardB.top, CardB.width, CardB.height );
    CRectangle Bar( BarB.left, BarB.top, BarB.width, BarB.height );
    bool OverBar = Bar.Contains( Point ) && !Moving( );
    if ( OverBar && Click )
        OpenFlag = !OpenFlag;

    Canvas->Rectangle( Card, Dress.card, Round );
    if ( Open > 0.02f )
        DrawIce( Bar, CRectangle( Left, Top, Wide, Head + Round ), Round, Open );
    Canvas->Border( Card, Dress.foldLine, Round, 1.0f );

    CColor Title = Mix( Dress.ink, Dress.inkHot, Open );
    Canvas->Write( Heading.get( ), CVector( Left + 14.0f * Scale, Bar.Top + ( Head - Heading->LineSpan ) * 0.5f ), Title, Name );

    ur::icons::Icon Arrow = Open > 0.5f ? ur::icons::Icon::ChevronUp : ur::icons::Icon::ChevronDown;
    unsigned long long Icon = ur::glyphs::image( Arrow, ( int )( 15.0f * Scale + 0.5f ), ur::glyphs::Weight::Solid );
    if ( Icon ) {
        ui::RectBounds ArrowB = ui::ComputeFoldArrow( BarB, Scale, 15.0f, 14.0f );
        Canvas->Image( CRectangle( ArrowB.left, ArrowB.top, ArrowB.width, ArrowB.height ), Icon, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), Mix( Style->Faint, Dress.inkHot, Open ), 0.0f );
    }

    Body = CRectangle( BodyB.left, BodyB.top, BodyB.width, BodyB.height );
    return OverBar;
}

static CRectangle DropListBox( float Scale ) {
    if ( !DropId || DropCount <= 0 )
        return CRectangle( );
    ui::RectBounds B;
    if ( !ui::ComputeDropListBox( DropField.Left, DropField.Top, DropField.Bottom( ), DropField.Width, DropCount, ( float )ur::app::height( ), Scale, B ) )
        return CRectangle( );
    return CRectangle( B.left, B.top, B.width, B.height );
}

static bool DropHit( const CVector& Point, float Scale ) {
    if ( !DropId )
        return false;
    return DropListBox( Scale ).Contains( Point );
}

static bool DrawDrop( float Left, float Top, float Wide, const char* Label, const char* Id, const char* const* Options, int Count, int& Pick, const CVector& Point, bool Click, float Scale ) {
    Pick = ui::ValidatePickIndex( Pick, Count );
    if ( Label ) {
        Canvas->Text( CVector( Left, Top ), Style->Faint, Label );
        Top += Font->LineSpan + 4.0f * Scale;
    }

    CRectangle Field( Left, Top, Wide, 28.0f * Scale );
    bool Over = Field.Contains( Point ) && !Moving( ) && !Menu.slide;
    bool Open = DropId && strcmp( DropId, Id ) == 0;
    if ( Over && Click ) {
        DropId = Open ? nullptr : Id;
        DropFresh = DropId != nullptr;
    }
    Open = DropId && strcmp( DropId, Id ) == 0;

    float Tone = ur::motion::toward( Id, Open ? 1.0f : ( Over ? 0.45f : 0.0f ), 26.0f );
    DrawIce( Field, Field, 6.0f * Scale, 0.55f + Tone * 0.45f );
    Canvas->Border( Field, Mix( Dress.foldLine, Style->AccentSoft, Tone ), 6.0f * Scale, 1.0f );
    Canvas->Text( CVector( Field.Left + 10.0f * Scale, Field.Top + ( Field.Height - Font->LineSpan ) * 0.5f ), Style->Text, Options[ Pick ] );

    unsigned long long Icon = ur::glyphs::image( Open ? ur::icons::Icon::ChevronUp : ur::icons::Icon::ChevronDown, ( int )( 11.0f * Scale + 0.5f ), ur::glyphs::Weight::Solid );
    float Mark = 11.0f * Scale;
    if ( Icon )
        Canvas->Image( CRectangle( Field.Right( ) - Mark - 10.0f * Scale, Field.Top + ( Field.Height - Mark ) * 0.5f, Mark, Mark ), Icon, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), Style->Faint, 0.0f );

    if ( Open ) {
        DropField = Field;
        DropOpts = Options;
        DropCount = Count;
        DropPick = &Pick;
        DropBits = nullptr;
        DropMany = false;
    }
    return Over;
}

static const char* BitLabel( const char* const* Options, int Count, int Bits ) {
    return ui::BitLabel( Options, Count, Bits );
}

static bool DrawDropBits( float Left, float Top, float Wide, const char* Label, const char* Id, const char* const* Options, int Count, int& Bits, const CVector& Point, bool Click, float Scale ) {
    if ( ( Bits & ( ( 1 << Count ) - 1 ) ) == 0 )
        Bits = 1;
    if ( Label ) {
        Canvas->Text( CVector( Left, Top ), Style->Faint, Label );
        Top += Font->LineSpan + 4.0f * Scale;
    }

    CRectangle Field( Left, Top, Wide, 28.0f * Scale );
    bool Over = Field.Contains( Point ) && !Moving( ) && !Menu.slide;
    bool Open = DropId && strcmp( DropId, Id ) == 0;
    if ( Over && Click ) {
        DropId = Open ? nullptr : Id;
        DropFresh = DropId != nullptr;
    }
    Open = DropId && strcmp( DropId, Id ) == 0;

    float Tone = ur::motion::toward( Id, Open ? 1.0f : ( Over ? 0.45f : 0.0f ), 26.0f );
    DrawIce( Field, Field, 6.0f * Scale, 0.55f + Tone * 0.45f );
    Canvas->Border( Field, Mix( Dress.foldLine, Style->AccentSoft, Tone ), 6.0f * Scale, 1.0f );
    Canvas->Text( CVector( Field.Left + 10.0f * Scale, Field.Top + ( Field.Height - Font->LineSpan ) * 0.5f ), Style->Text, BitLabel( Options, Count, Bits ) );

    unsigned long long Icon = ur::glyphs::image( Open ? ur::icons::Icon::ChevronUp : ur::icons::Icon::ChevronDown, ( int )( 11.0f * Scale + 0.5f ), ur::glyphs::Weight::Solid );
    float Mark = 11.0f * Scale;
    if ( Icon )
        Canvas->Image( CRectangle( Field.Right( ) - Mark - 10.0f * Scale, Field.Top + ( Field.Height - Mark ) * 0.5f, Mark, Mark ), Icon, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), Style->Faint, 0.0f );

    if ( Open ) {
        DropField = Field;
        DropOpts = Options;
        DropCount = Count;
        DropPick = nullptr;
        DropBits = &Bits;
        DropMany = true;
    }
    return Over;
}

static bool DrawDropList( const CVector& Point, bool Click, float Scale ) {
    if ( !DropId || !DropOpts || DropCount <= 0 )
        return false;
    if ( !DropMany && !DropPick )
        return false;
    if ( DropMany && !DropBits )
        return false;

    CRectangle List = DropListBox( Scale );
    float Item = 26.0f * Scale;
    bool Over = List.Contains( Point );
    Canvas->Rectangle( List, Dress.card, 6.0f * Scale );
    Canvas->Border( List, Dress.foldLine, 6.0f * Scale, 1.0f );
    for ( int Index = 0; Index < DropCount; Index++ ) {
        CRectangle Row( List.Left + 3.0f * Scale, List.Top + 3.0f * Scale + Item * ( float )Index, List.Width - 6.0f * Scale, Item );
        bool Hit = Row.Contains( Point );
        bool On = DropMany ? ( ( *DropBits & ( 1 << Index ) ) != 0 ) : ( *DropPick == Index );
        if ( On || Hit )
            Canvas->Rectangle( Row, On ? Style->Accent.Fade( 0.28f ) : CColor( 255, 255, 255, 16 ), 4.0f * Scale );
        Canvas->Text( CVector( Row.Left + 8.0f * Scale, Row.Top + ( Row.Height - Font->LineSpan ) * 0.5f ), On ? Dress.inkHot : Style->Text, DropOpts[ Index ] );
        if ( Hit && Click && !DropFresh ) {
            if ( DropMany ) {
                *DropBits ^= ( 1 << Index );
                if ( ( *DropBits & ( ( 1 << DropCount ) - 1 ) ) == 0 )
                    *DropBits = 1 << Index;
            } else {
                *DropPick = Index;
                DropId = nullptr;
            }
        }
    }
    if ( Click && !DropFresh && !Over && !DropField.Contains( Point ) )
        DropId = nullptr;
    DropFresh = false;
    return Over;
}

static float SwatchSize( float Scale ) {
    return ui::SwatchSize( Scale );
}

static float SwatchGap( float Scale ) {
    return ui::SwatchGap( Scale );
}

static int SwatchColumns( float Wide, int Count, float Scale ) {
    return ui::SwatchColumns( Wide, Count, Scale );
}

static float SwatchTall( float Wide, int Count, float Scale ) {
    return ui::SwatchTall( Wide, Count, Scale );
}

static bool DrawSwatches( float Left, float Top, float Wide, int Count, const CColor* Colors, int& Pick, const char* Prefix, const CVector& Point, bool Click, float Scale ) {
    float Size = SwatchSize( Scale );
    float Gap = SwatchGap( Scale );
    int Columns = SwatchColumns( Wide, Count, Scale );
    bool Busy = false;
    for ( int Index = 0; Index < Count; Index++ ) {
        int Col = Index % Columns;
        int Row = Index / Columns;
        CRectangle Chip( Left + ( Size + Gap ) * ( float )Col, Top + ( Size + Gap ) * ( float )Row, Size, Size );
        bool Over = Chip.Contains( Point ) && !Moving( ) && !Menu.slide;
        char Id[ 64 ];
        snprintf( Id, sizeof( Id ), "%s.%d", Prefix, Index );
        float Tone = ur::motion::toward( Id, Pick == Index ? 1.0f : ( Over ? 0.5f : 0.0f ), 26.0f );
        Canvas->Rectangle( Chip, Colors[ Index ], 4.0f * Scale );
        Canvas->Border( Chip, Mix( CColor( 80, 96, 120, 140 ), CColor( 236, 242, 252 ), Tone ), 4.0f * Scale, Pick == Index ? 1.6f * Scale : 1.0f * Scale );
        if ( Over && Click )
            Pick = Index;
        Busy = Busy || Over;
    }
    return Busy;
}

static bool DrawAction( const CRectangle& Row, const char* Label, const CVector& Point, bool Click, float Scale, bool Danger ) {
    bool Over = Row.Contains( Point ) && !Moving( ) && !Menu.slide;
    float Tone = ur::motion::toward( Label, Over ? 1.0f : 0.0f, 26.0f );
    float Round = 6.0f * Scale;
    if ( Over )
        DrawIce( Row, Row, Round, 0.45f + Tone * 0.35f );
    else
        Canvas->Rectangle( Row, Dress.elevated, Round );
    Canvas->Border( Row, Mix( Dress.foldLine, Danger ? CColor( 232, 64, 72 ) : Style->AccentSoft, Tone ), Round, 1.0f );
    CVector Size = Font->Measure( Label );
    CColor Ink = Danger
        ? Mix( CColor( 214, 220, 232 ), CColor( 232, 64, 72 ), Tone )
        : Mix( Style->Text, Dress.inkHot, Tone );
    Canvas->Text( CVector( Row.Left + ( Row.Width - Size.Horizontal ) * 0.5f, Row.Top + ( Row.Height - Font->LineSpan ) * 0.5f ), Ink, Label );
    return Over && Click;
}

struct PageFit {
    float inset;
    float gap;
    float head;
    float general;
    float target;
    float silent;
    float rageJump;
    float rageNoclip;
    float overlay;
    float visual;
    float theme;
    float custom;
    float misc;
    float game;
    float setOverlay;
};

static PageFit FitOf( float Scale ) {
    PageFit Fit;
    Fit.inset = 10.0f * Scale;
    Fit.gap = 8.0f * Scale;
    Fit.head = 36.0f * Scale;
    Fit.general = 200.0f * Scale;
    Fit.silent = 220.0f * Scale;
    Fit.target = ( 276.0f + ( Aim.drawFov ? 32.0f : 0.0f ) ) * Scale;
    Fit.rageJump = 128.0f * Scale;
    Fit.rageNoclip = 58.0f * Scale;
    Fit.overlay = 232.0f * Scale;
    Fit.visual = 88.0f * Scale;
    Fit.theme = 228.0f * Scale;
    Fit.custom = 236.0f * Scale;
    Fit.misc = ( 138.0f + ( Menu.limit ? 28.0f : 0.0f ) ) * Scale;
    Fit.game = 160.0f * Scale;
    Fit.setOverlay = 204.0f * Scale;
    return Fit;
}

#include "ui/tabs/tab_aimbot.hpp"
#include "ui/tabs/tab_rage.hpp"
#include "ui/tabs/tab_esp.hpp"
#include "ui/tabs/tab_settings.hpp"
#include "ui/tabs/tab_configs.hpp"

static bool ReadClientVer( char* Out, int Cap ) {
    static const wchar_t* Names[ ] = { L"RobloxPlayerBeta.exe", L"RobloxPlayer.exe", L"Windows10Universal.exe" };
    if ( !Out || Cap < 8 )
        return false;
    Out[ 0 ] = 0;
    for ( const wchar_t* Name : Names ) {
        DWORD Pid = world::FindPid( Name );
        if ( !Pid )
            continue;
        unlinked::UniqueHandle Handle( OpenProcess( PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Pid ) );
        if ( !Handle )
            continue;
        char Path[ MAX_PATH ] = { };
        DWORD Size = ( DWORD )sizeof( Path );
        BOOL Ok = QueryFullProcessImageNameA( Handle.get( ), 0, Path, &Size );
        if ( !Ok )
            continue;
        if ( offsets::ExtractVersion( Path, Out, Cap ) )
            return true;
    }
    return false;
}

static void TickChannel( ) {
    unsigned Now = GetTickCount( );
    if ( Now < LiveCh.nextScan )
        return;
    LiveCh.nextScan = Now + 2000;

    char Client[ 48 ] = { };
    bool HaveClient = ReadClientVer( Client, ( int )sizeof( Client ) );
    char Dump[ 48 ] = { };
    if ( offsets::Ready( ) )
        offsets::CopyVersion( Dump, ( int )sizeof( Dump ) );

    if ( HaveClient )
        lstrcpynA( LiveCh.client, Client, ( int )sizeof( LiveCh.client ) );
    else
        LiveCh.client[ 0 ] = 0;
    lstrcpynA( LiveCh.dump, Dump, ( int )sizeof( LiveCh.dump ) );

    LiveCh.mismatch = HaveClient && Dump[ 0 ] && !offsets::IsVersionMatch( Client, Dump );
    if ( !LiveCh.mismatch ) {
        LiveCh.open = false;
        LiveCh.dismissed = false;
        return;
    }
    if ( !LiveCh.dismissed )
        LiveCh.open = true;
}

static void DrawChannelNotice( float Across, float Vertical, const CVector& Point, bool Click, float Scale ) {
    if ( !LiveCh.open || !Font )
        return;

    float Line = Font->LineSpan;
    float Pad = 18.0f * Scale;
    float HeadH = 42.0f * Scale;
    float ActH = 32.0f * Scale;
    float AfterSteps = 18.0f * Scale;
    float StepGap = 6.0f * Scale;
    float Wide = 448.0f * Scale;
    float Tall = ui::ComputeModalTall( HeadH, Line, StepGap, 7, AfterSteps, ActH, Pad, Scale );
    CRectangle Shade( 0.0f, 0.0f, Across, Vertical );
    ui::RectBounds CardB = ui::ComputeCenteredBounds( Across, Vertical, Wide, Tall );
    CRectangle Card( CardB.left, CardB.top, CardB.width, CardB.height );
    float Keep = Canvas->Opacity;
    Canvas->Opacity = 1.0f;
    Canvas->Rectangle( Shade, CColor( 6, 8, 12, 186 ), 0.0f );
    Canvas->Shadow( Card, CColor( 6, 10, 18, 130 ), 10.0f * Scale, 22.0f * Scale );
    Canvas->Rectangle( Card, Style->Surface, 10.0f * Scale );
    CRectangle Head( Card.Left, Card.Top, Card.Width, HeadH );
    DrawIce( Head, Card, 10.0f * Scale, 1.0f );

    CFont* Title = Heading.get( );
    if ( Title && Title->LineSpan > 1.0f )
        Canvas->Write( Title, CVector( Card.Left + Pad, Head.Top + ( Head.Height - Title->LineSpan ) * 0.5f ), Dress.inkHot, "Wrong Roblox channel" );
    else
        Canvas->Text( CVector( Card.Left + Pad, Head.Top + 12.0f * Scale ), Dress.inkHot, "Wrong Roblox channel" );

    float Y = Head.Bottom( ) + 14.0f * Scale;
    char LineText[ 96 ] = { };
    snprintf( LineText, sizeof( LineText ), "Your client  %s", LiveCh.client[ 0 ] ? LiveCh.client : "unknown" );
    Canvas->Text( CVector( Card.Left + Pad, Y ), Style->Text, LineText );
    Y += Line + 6.0f * Scale;
    snprintf( LineText, sizeof( LineText ), "LIVE dump    %s", LiveCh.dump[ 0 ] ? LiveCh.dump : "unknown" );
    Canvas->Text( CVector( Card.Left + Pad, Y ), Style->Faint, LineText );
    Y += Line + 12.0f * Scale;
    Canvas->Text( CVector( Card.Left + Pad, Y ), Style->Faint, "Offsets are dumped for the LIVE channel only." );
    Y += Line + 10.0f * Scale;

    static const char* Steps[ ] = {
        "1. Download Fishstrap from fishstrap.app",
        "2. Install it, then open Fishstrap from search",
        "3. Click Configure Settings",
        "4. Open the Deployment tab",
        "5. Set Channel to production and press Enter",
        "6. Set Automatic channel change to Never change",
        "7. Press Save and Launch"
    };
    for ( const char* Step : Steps ) {
        Canvas->Text( CVector( Card.Left + Pad, Y ), Style->Text, Step );
        Y += Line + StepGap;
    }

    Y += AfterSteps;
    float Gap = 8.0f * Scale;
    ui::RectBounds GetB, OkB;
    ui::ComputeSplitPair( Card.Left + Pad, Y, Card.Width - Pad * 2.0f, Gap, ActH, GetB, OkB );
    CRectangle Get( GetB.left, GetB.top, GetB.width, GetB.height );
    CRectangle Ok( OkB.left, OkB.top, OkB.width, OkB.height );
    if ( DrawAction( Get, "Get Fishstrap", Point, Click, Scale, false ) )
        ShellExecuteA( nullptr, "open", "https://www.fishstrap.app/Fishstrap.exe", nullptr, nullptr, SW_SHOWNORMAL );
    if ( DrawAction( Ok, "Got it", Point, Click, Scale, false ) ) {
        LiveCh.open = false;
        LiveCh.dismissed = true;
    }

    ur::overlay::Options& Overlay = ur::app::overlay_options( );
    Overlay.click_through = false;
    Input->ApplyPosition( Point.Horizontal, Point.Vertical );
    Canvas->Opacity = Keep;
}

static void DrawPage( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, bool& Busy ) {
    Menu.pageIn += Context->DeltaTime * 5.2f;
    if ( Menu.pageIn > 1.0f )
        Menu.pageIn = 1.0f;

    float Ease = ui::EaseOutQuint( Menu.pageIn );
    float Slide = ui::ComputePageSlide( Menu.pageIn, Scale, Menu.pageDir, 36.0f );
    bool Live = Menu.pageIn > 0.82f;

    CRectangle Shifted = Content;
    Shifted.Top += Slide;
    CVector Hit( Point.Horizontal, Point.Vertical - Slide );

    bool Block = DropHit( Hit, Scale );
    Canvas->PushClip( Content );
    float Keep = Canvas->Opacity;
    Canvas->Opacity = Keep * Ease;
    if ( Menu.tab == TabAimbot )
        Busy = DrawAimbot( Shifted, Hit, Click && Live && !Block, Press && Live && !Block, Scale, 1.0f ) || Busy;
    else if ( Menu.tab == TabRage )
        Busy = DrawRage( Shifted, Hit, Click && Live && !Block, Press && Live && !Block, Scale, 1.0f ) || Busy;
    else if ( Menu.tab == TabEsp )
        Busy = DrawEsp( Shifted, Hit, Click && Live && !Block, Press && Live && !Block, Scale, 1.0f ) || Busy;
    else if ( Menu.tab == TabConfigs )
        Busy = DrawConfigs( Shifted, Hit, Click && Live && !Block, Press && Live && !Block, Scale, 1.0f ) || Busy;
    else if ( Menu.tab == TabSettings )
        Busy = DrawSettings( Shifted, Hit, Click && Live && !Block, Press && Live && !Block, Scale, 1.0f ) || Busy;
    Canvas->Opacity = Keep;

    float Wash = ( 1.0f - Ease ) * 0.55f;
    if ( Wash > 0.02f ) {
        float Edge = Content.Top + ( Menu.pageDir > 0.0f ? 0.0f : Content.Height - 3.0f * Scale );
        DrawIce( Content, CRectangle( Content.Left, Edge, Content.Width, 3.0f * Scale ), 0.0f, Wash );
    }
    Canvas->PopClip( );
    Busy = DrawDropList( Hit, Click && Live, Scale ) || Busy;
}

static void PlaceExplore( float Across, float Vertical, float Scale ) {
    float Wide = ExploreWidth * Scale;
    float Tall = ExploreHeight * Scale;
    float Gap = 16.0f * Scale;
    if ( !Tree.ready ) {
        float DockX = ui::ComputeDockOffset( Menu.origin.Horizontal, MenuWidth * Scale, Wide, Gap, Across, 8.0f );
        Tree.dock = CVector( DockX, 0.0f );
        Tree.docked = true;
        Tree.ready = true;
    }
    if ( Tree.docked )
        Tree.origin = Menu.origin + Tree.dock;
    ClampBox( Tree.origin, Across, Vertical, Wide, Tall );
}

static void DragExplore( const CRectangle& Bounds, const CVector& Point, float Across, float Vertical, float Wide, float Tall, bool AllowStart ) {
    bool Press = Held( VK_LBUTTON );
    bool CanStart = AllowStart && !Menu.mouse && !Menu.held && Bounds.Contains( Point ) && !Menu.slide;
    if ( Press && CanStart )
        Tree.docked = false;

    ui::UpdateDragState( Press, CanStart, Point.Horizontal, Point.Vertical,
                         Tree.origin.Horizontal, Tree.origin.Vertical,
                         Tree.grab.Horizontal, Tree.grab.Vertical, Tree.held );

    ClampBox( Tree.origin, Across, Vertical, Wide, Tall );
}

static void ExploreChrome( const CRectangle& Bounds, float Scale, CRectangle& Header, CRectangle& Pane ) {
    ui::RectBounds H, P;
    ui::ComputePanelChrome( Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height, Scale, HeaderHeight, 10.0f, H, P );
    Header = CRectangle( H.left, H.top, H.width, H.height );
    Pane = CRectangle( P.left, P.top, P.width, P.height );
}

static void DrawCaret( CVector At, bool Down, float Scale, CColor Tint ) {
    float TipsX[ 3 ] = { };
    float TipsY[ 3 ] = { };
    ui::ComputeCaretTips( At.Horizontal, At.Vertical, Down, Scale, TipsX, TipsY );
    CVector Tips[ 3 ] = {
        CVector( TipsX[ 0 ], TipsY[ 0 ] ),
        CVector( TipsX[ 1 ], TipsY[ 1 ] ),
        CVector( TipsX[ 2 ], TipsY[ 2 ] )
    };
    Canvas->Polygon( Tips, 3, Tint );
}

static void CopyText( const char* Text ) {
    if ( !Text || !Text[ 0 ] )
        return;
    size_t Bytes = strlen( Text ) + 1;
    HGLOBAL Block = GlobalAlloc( GMEM_MOVEABLE, Bytes );
    if ( !Block )
        return;
    void* Dest = GlobalLock( Block );
    if ( !Dest ) {
        GlobalFree( Block );
        return;
    }
    memcpy( Dest, Text, Bytes );
    GlobalUnlock( Block );
    if ( OpenClipboard( ( HWND )ur::app::window( ) ) ) {
        EmptyClipboard( );
        SetClipboardData( CF_TEXT, Block );
        CloseClipboard( );
    } else {
        GlobalFree( Block );
    }
}

static void TreeDraft( ) {
    if ( !Tree.type )
        return;
    if ( Edge( VK_BACK, KeyWas[ VK_BACK ] ) ) {
        size_t Len = strlen( Tree.find );
        if ( Len )
            Tree.find[ Len - 1 ] = 0;
        return;
    }
    if ( Edge( VK_RETURN, KeyWas[ VK_RETURN ] ) ) {
        Tree.type = false;
        return;
    }
    bool Shift = Held( VK_SHIFT );
    for ( int Code = 'A'; Code <= 'Z'; Code++ ) {
        if ( !Edge( Code, KeyWas[ Code ] ) )
            continue;
        size_t Len = strlen( Tree.find );
        if ( Len >= ( size_t )sizeof( Tree.find ) - 1 )
            return;
        Tree.find[ Len ] = ( char )( Shift ? Code : Code + 32 );
        Tree.find[ Len + 1 ] = 0;
        return;
    }
    for ( int Code = '0'; Code <= '9'; Code++ ) {
        if ( !Edge( Code, KeyWas[ Code ] ) )
            continue;
        size_t Len = strlen( Tree.find );
        if ( Len >= ( size_t )sizeof( Tree.find ) - 1 )
            return;
        Tree.find[ Len ] = ( char )Code;
        Tree.find[ Len + 1 ] = 0;
        return;
    }
    char Extra = 0;
    if ( Edge( VK_SPACE, KeyWas[ VK_SPACE ] ) )
        Extra = ' ';
    else if ( Edge( VK_OEM_PERIOD, KeyWas[ VK_OEM_PERIOD ] ) )
        Extra = '.';
    else if ( Edge( VK_OEM_MINUS, KeyWas[ VK_OEM_MINUS ] ) )
        Extra = '-';
    if ( Extra ) {
        size_t Len = strlen( Tree.find );
        if ( Len >= ( size_t )sizeof( Tree.find ) - 1 )
            return;
        Tree.find[ Len ] = Extra;
        Tree.find[ Len + 1 ] = 0;
    }
}

static int WalkLive( uintptr_t Parent, int Depth, float Scale, const CRectangle& Pane, const CVector& Point, bool Click, bool Locked, int& Row, bool Paint, bool& Busy ) {
    uintptr_t List[ browse::KidCap ];
    int Count = 0;
    if ( Parent == 0 ) {
        if ( browse::Root( ) )
            List[ Count++ ] = browse::Root( );
    } else {
        Count = browse::Children( Parent, List, browse::KidCap );
    }
    int Shown = 0;
    float RowH = 24.0f * Scale;
    float Indent = 12.0f * Scale;
    float Icon = 15.0f * Scale;
    for ( int Index = 0; Index < Count; Index++ ) {
        uintptr_t Addr = List[ Index ];
        if ( !browse::Visible( Addr ) )
            continue;
        browse::Node* Item = browse::Get( Addr );
        if ( !Item )
            continue;
        Shown += 1;
        float Top = Pane.Top + ( float )Row * RowH - Tree.scroll;
        CRectangle Line( Pane.Left, Top, Pane.Width, RowH );
        bool See = Top + RowH > Pane.Top && Top < Pane.Bottom( );
        bool Kids = browse::HasKids( Addr );
        if ( Paint && See ) {
            bool Over = Line.Contains( Point ) && Pane.Contains( Point ) && !Locked && !Tree.type;
            CRectangle Arm( Pane.Left + 4.0f * Scale + Indent * ( float )Depth, Line.Top, 14.0f * Scale, RowH );
            if ( Over && Click ) {
                if ( Kids && Arm.Contains( Point ) )
                    browse::Toggle( Addr );
                else {
                    browse::Select( Addr );
                    Tree.pick = Addr;
                    Tree.confirm = false;
                }
            }
            if ( Tree.pick == Addr )
                Canvas->Rectangle( CRectangle( Pane.Left + 2.0f * Scale, Line.Top + 1.0f * Scale, Pane.Width - 4.0f * Scale, RowH - 2.0f * Scale ), Dress.trackOn, 4.0f * Scale );
            else if ( Over )
                Canvas->Rectangle( CRectangle( Pane.Left + 2.0f * Scale, Line.Top + 1.0f * Scale, Pane.Width - 4.0f * Scale, RowH - 2.0f * Scale ), CColor( 255, 255, 255, 14 ), 4.0f * Scale );
            if ( Kids )
                DrawCaret( CVector( Arm.Left + Arm.Width * 0.5f, Line.Top + RowH * 0.5f ), Item->open, Scale, Mix( CColor( 168, 178, 194 ), CColor( 230, 236, 246 ), Tree.pick == Addr ? 1.0f : 0.0f ) );
            unsigned long long Glyph = TreeGlyph( browse::Glyph( Addr ) );
            CRectangle Mark( Arm.Right( ) + 2.0f * Scale, Line.Top + ( RowH - Icon ) * 0.5f, Icon, Icon );
            if ( Glyph )
                Canvas->Image( Mark, Glyph, CRectangle( 0.0f, 0.0f, 1.0f, 1.0f ), CColor( 255, 255, 255 ), 0.0f );
            char Caption[ 96 ];
            if ( Item->extra > 0 && Item->open )
                snprintf( Caption, sizeof( Caption ), "%s [%s] +%d", Item->name, Item->klass, Item->extra );
            else
                snprintf( Caption, sizeof( Caption ), "%s [%s]", Item->name, Item->klass );
            CColor Ink = Tree.pick == Addr ? CColor( 240, 246, 255 ) : Style->Text;
            Canvas->Text( CVector( Mark.Right( ) + 6.0f * Scale, Line.Top + ( RowH - Font->LineSpan ) * 0.5f ), Ink, Caption );
            Busy = Busy || Over;
        }
        Row += 1;
        if ( Item->open && Kids )
            Shown += WalkLive( Addr, Depth + 1, Scale, Pane, Point, Click, Locked, Row, Paint, Busy );
    }
    return Shown;
}

static bool DrawExplorer( float Across, float Vertical, const CVector& Point, bool Click, bool Press, float Scale ) {
    ( void )Press;
    PlaceExplore( Across, Vertical, Scale );

    float Wide = ExploreWidth * Scale;
    float Tall = ExploreHeight * Scale;
    float Round = Style->Rounding * Scale;
    CRectangle Bounds( Tree.origin, CVector( Wide, Tall ) );
    CRectangle Header;
    CRectangle Body;
    ExploreChrome( Bounds, Scale, Header, Body );

    float SearchH = 28.0f * Scale;
    CRectangle Search( Body.Left, Body.Top, Body.Width, SearchH );
    float Gap = 8.0f * Scale;
    float TreeW = Body.Width * 0.56f;
    float SideW = Body.Width - TreeW - Gap;
    float WorkTop = Search.Bottom( ) + 6.0f * Scale;
    float WorkH = Body.Bottom( ) - WorkTop;
    CRectangle Pane( Body.Left, WorkTop, TreeW, WorkH );
    CRectangle Side( Body.Left + TreeW + Gap, WorkTop, SideW, WorkH );

    bool OverTree = Pane.Contains( Point );
    bool OverSide = Side.Contains( Point );
    bool Locked = Tree.held;

    uintptr_t Root = browse::Root( );
    int Count = 0;
    bool Busy = false;
    if ( Root )
        Count = WalkLive( 0, 0, Scale, Pane, Point, false, true, Count, false, Busy );
    float RowH = 24.0f * Scale;
    float Need = ( float )Count * RowH;
    float Most = Need - Pane.Height;
    if ( Most < 0.0f )
        Most = 0.0f;
    if ( OverTree && !Locked && !Tree.type && Input->WheelDelta != 0.0f ) {
        Tree.scroll -= Input->WheelDelta * 42.0f * Scale;
        Input->WheelDelta = 0.0f;
    }
    if ( Tree.scroll > Most )
        Tree.scroll = Most;
    if ( Tree.scroll < 0.0f )
        Tree.scroll = 0.0f;

    Canvas->Shadow( Bounds, CColor( 6, 10, 18, 130 ), Round, 24.0f * Scale );
    Canvas->Rectangle( Bounds, Style->Surface, Round );
    DrawIce( Header, Bounds, Round, 1.0f );
    DrawTitle( Header, Scale, "Explorer" );

    bool OverFind = Search.Contains( Point ) && !Locked;
    if ( OverFind && Click )
        Tree.type = true;
    else if ( Click && !OverFind && Tree.type )
        Tree.type = false;
    Canvas->Rectangle( Search, Style->Elevated, 8.0f * Scale );
    if ( Tree.type )
        DrawIce( Search, Search, 8.0f * Scale, 0.45f );
    Canvas->Border( Search, Mix( Dress.foldLine, Style->AccentSoft, Tree.type ? 1.0f : 0.0f ), 8.0f * Scale, 1.0f );
    const char* Shown = Tree.find[ 0 ] ? Tree.find : ( Tree.type ? "" : "Search name or class" );
    Canvas->Text( CVector( Search.Left + 10.0f * Scale, Search.Top + ( SearchH - Font->LineSpan ) * 0.5f ), Tree.find[ 0 ] ? Style->Text : Style->Faint, Shown );
    if ( Tree.type && ( ( int )( Context->Elapsed * 2.0 ) & 1 ) ) {
        CVector Caret = Font->Measure( Tree.find );
        Canvas->Rectangle( CRectangle( Search.Left + 10.0f * Scale + Caret.Horizontal, Search.Top + 6.0f * Scale, 1.0f * Scale, SearchH - 12.0f * Scale ), Style->AccentSoft, 0.0f );
    }

    Canvas->Rectangle( Pane, Style->Elevated, 10.0f * Scale );
    Canvas->PushClip( Pane );
    int Row = 0;
    Busy = false;
    if ( Root )
        WalkLive( 0, 0, Scale, Pane, Point, Click && !Tree.held && !Tree.type, Tree.held, Row, true, Busy );
    else
        Canvas->Text( CVector( Pane.Left + 8.0f * Scale, Pane.Top + 8.0f * Scale ), Style->Faint, browse::Core( ).note[ 0 ] ? browse::Core( ).note : "Not attached" );
    Canvas->PopClip( );

    Canvas->Rectangle( Side, Style->Elevated, 10.0f * Scale );
    float In = 8.0f * Scale;
    float Y = Side.Top + In;
    const char* Path = browse::Core( ).path;
    CRectangle PathRow( Side.Left + In, Y, Side.Width - In * 2.0f, 22.0f * Scale );
    bool OverPath = PathRow.Contains( Point ) && !Locked;
    Canvas->Text( CVector( PathRow.Left, PathRow.Top + ( PathRow.Height - Font->LineSpan ) * 0.5f ), OverPath ? Dress.inkHot : Style->Faint, Path[ 0 ] ? Path : "—" );
    if ( OverPath && Click && Path[ 0 ] ) {
        CopyText( Path );
        lstrcpynA( Tree.note, "Copied path", ( int )sizeof( Tree.note ) );
        Tree.noteAge = 1.6f;
    }
    Y = PathRow.Bottom( ) + 6.0f * Scale;

    float ActH = 22.0f * Scale;
    float ActW = ( Side.Width - In * 2.0f - 6.0f * Scale ) * 0.5f;
    CRectangle A1( Side.Left + In, Y, ActW, ActH );
    CRectangle A2( A1.Right( ) + 6.0f * Scale, Y, ActW, ActH );
    Y = A1.Bottom( ) + 4.0f * Scale;
    CRectangle A3( Side.Left + In, Y, ActW, ActH );
    CRectangle A4( A3.Right( ) + 6.0f * Scale, Y, ActW, ActH );
    if ( DrawAction( A1, "Copy Path", Point, Click && !Locked, Scale, false ) && Path[ 0 ] ) {
        CopyText( Path );
        lstrcpynA( Tree.note, "Copied path", ( int )sizeof( Tree.note ) );
        Tree.noteAge = 1.6f;
    }
    if ( DrawAction( A2, "Goto", Point, Click && !Locked, Scale, false ) ) {
        browse::Goto( Tree.pick );
        lstrcpynA( Tree.note, browse::Core( ).note, ( int )sizeof( Tree.note ) );
        Tree.noteAge = 1.6f;
    }
    const char* Kill = Tree.confirm ? "Confirm" : "Destroy";
    if ( DrawAction( A3, Kill, Point, Click && !Locked, Scale, true ) ) {
        if ( !Tree.confirm )
            Tree.confirm = true;
        else {
            browse::Destroy( Tree.pick );
            Tree.pick = browse::Pick( );
            Tree.confirm = false;
            lstrcpynA( Tree.note, browse::Core( ).note, ( int )sizeof( Tree.note ) );
            Tree.noteAge = 1.6f;
        }
    }
    if ( DrawAction( A4, Tree.mark ? "Unmark" : "Highlight", Point, Click && !Locked, Scale, false ) )
        Tree.mark = !Tree.mark;
    Y = A4.Bottom( ) + 8.0f * Scale;

    CRectangle PropPane( Side.Left + In, Y, Side.Width - In * 2.0f, Side.Bottom( ) - In - Y - 18.0f * Scale );
    if ( PropPane.Height < 20.0f * Scale )
        PropPane.Height = 20.0f * Scale;
    float PropH = 18.0f * Scale;
    int PropN = browse::Core( ).propN;
    if ( OverSide && !OverTree && Input->WheelDelta != 0.0f ) {
        Tree.propScroll -= Input->WheelDelta * 28.0f * Scale;
        Input->WheelDelta = 0.0f;
    }
    float PropNeed = ( float )PropN * PropH;
    float PropMost = PropNeed - PropPane.Height;
    if ( PropMost < 0.0f )
        PropMost = 0.0f;
    if ( Tree.propScroll > PropMost )
        Tree.propScroll = PropMost;
    if ( Tree.propScroll < 0.0f )
        Tree.propScroll = 0.0f;
    Canvas->PushClip( PropPane );
    for ( int Index = 0; Index < PropN; Index++ ) {
        const browse::Prop& Item = browse::Core( ).props[ Index ];
        float Top = PropPane.Top + ( float )Index * PropH - Tree.propScroll;
        CRectangle Line( PropPane.Left, Top, PropPane.Width, PropH );
        if ( Top + PropH < PropPane.Top || Top > PropPane.Bottom( ) )
            continue;
        Canvas->Text( CVector( Line.Left, Line.Top + 1.0f * Scale ), Style->Faint, Item.label );
        CVector Size = Font->Measure( Item.text );
        float TextX = Line.Right( ) - Size.Horizontal;
        if ( TextX < Line.Left + 64.0f * Scale )
            TextX = Line.Left + 64.0f * Scale;
        Canvas->Text( CVector( TextX, Line.Top + 1.0f * Scale ), Style->Text, Item.text );
        if ( Item.write && Line.Contains( Point ) && Click && !Locked ) {
            browse::Nudge( Tree.pick, Item.write, Held( VK_SHIFT ) ? -2.0f : 2.0f );
            lstrcpynA( Tree.note, browse::Core( ).note, ( int )sizeof( Tree.note ) );
            Tree.noteAge = 1.2f;
        }
    }
    Canvas->PopClip( );

    if ( Tree.noteAge > 0.0f ) {
        Tree.noteAge -= Context->DeltaTime;
        Canvas->Text( CVector( Side.Left + In, Side.Bottom( ) - 16.0f * Scale ), Style->Faint, Tree.note );
    }

    Canvas->Border( Bounds, Style->Outline.Blend( Style->Accent, 0.22f ), Round, Style->Thickness );
    if ( ( Busy || OverTree || OverFind || OverPath || OverSide ) && !Moving( ) )
        Input->Pointer = PointerHand;
    if ( Tree.held )
        Input->Pointer = PointerMove;
    return Bounds.Contains( Point ) || Tree.held;
}

static float LiveFps( ) {
    static play::FpsFilter Filter;
    return Filter.Update( Context->DeltaTime, Context->Framerate );
}

static void MarkLine( char* Line, size_t Cap ) {
    float Fps = LiveFps( );
    play::FormatWatermark( Menu.watermark, Menu.showFps, Fps, Line, Cap );
}

static CRectangle PlaceMark( float Across, float Vertical, float Scale, CFont* Face, const char* Line ) {
    CVector Size = Face->Measure( Line );
    float Wide = 0.0f;
    float Tall = 0.0f;
    ui::ComputeBadgeSize( Size.Horizontal, Size.Vertical, Scale, Wide, Tall );
    if ( !Badge.ready ) {
        Badge.origin = CVector( 14.0f * Scale, Vertical - Tall - 14.0f * Scale );
        Badge.ready = true;
    }
    ClampBox( Badge.origin, Across, Vertical, Wide, Tall );
    return CRectangle( Badge.origin, CVector( Wide, Tall ) );
}

static bool DragMark( const CRectangle& Chip, const CVector& Point, float Across, float Vertical, bool AllowStart ) {
    bool Press = Held( VK_LBUTTON );
    bool CanStart = AllowStart && !Menu.mouse && !Menu.held && !Tree.held && Chip.Contains( Point ) && !Menu.slide && !Listening( );
    ui::UpdateDragState( Press, CanStart, Point.Horizontal, Point.Vertical,
                         Badge.origin.Horizontal, Badge.origin.Vertical,
                         Badge.grab.Horizontal, Badge.grab.Vertical, Badge.held );
    ClampBox( Badge.origin, Across, Vertical, Chip.Width, Chip.Height );
    return Chip.Contains( Point ) || Badge.held;
}

static CVector OverlayOf( float Across, float Down ) {
    HWND Handle = ( HWND )ur::app::window( );
    POINT Point{ ( LONG )( Across + 0.5f ), ( LONG )( Down + 0.5f ) };
    if ( Handle )
        ScreenToClient( Handle, &Point );
    return CVector( ( float )Point.x, ( float )Point.y );
}

static bool OverlayToView( const CVector& Overlay, float& X, float& Y ) {
    POINT Point{ ( LONG )( Overlay.Horizontal + 0.5f ), ( LONG )( Overlay.Vertical + 0.5f ) };
    HWND OverlayHwnd = ( HWND )ur::app::window( );
    if ( OverlayHwnd )
        ClientToScreen( OverlayHwnd, &Point );
    HWND Game = world::GameWindow( );
    if ( Game && IsWindow( Game ) ) {
        ScreenToClient( Game, &Point );
        X = ( float )Point.x;
        Y = ( float )Point.y;
        return X > 1.0f && Y > 1.0f;
    }
    const world::Snap& Live = world::View( );
    X = Overlay.Horizontal - ( float )Live.clientX;
    Y = Overlay.Vertical - ( float )Live.clientY;
    return X > 1.0f && Y > 1.0f;
}

static bool EspDot( const world::Vec3& World, CVector& Out );

static world::Vec3 AimPoint( const world::Actor& Item, bool UsePred, int Bones, bool AllowPred ) {
    return aim::AimPoint( Item, UsePred, Bones, AllowPred, world::View( ).localPing );
}

static world::Vec3 AimPoint( const world::Actor& Item, bool UsePred = true ) {
    return AimPoint( Item, UsePred, Aim.bones, Aim.pred );
}

static bool AimDot( const world::Vec3& World, CVector& Out ) {
    world::Dot View;
    if ( !world::ToView( World, View ) )
        return false;
    const world::Snap& Live = world::View( );
    float Wide = ( float )Live.viewW;
    float Tall = ( float )Live.viewH;
    if ( Wide < 8.0f || Tall < 8.0f )
        return false;
    if ( View.x < -48.0f || View.y < -48.0f || View.x > Wide + 48.0f || View.y > Tall + 48.0f )
        return false;
    world::Dot Hit;
    if ( !world::ToScreen( World, Hit ) )
        return false;
    Out = OverlayOf( Hit.x, Hit.y );
    return true;
}

static CVector AimMid( ) {
    if ( CursorVisible( ) )
        return Cursor( );
    return ScreenMid( );
}

static float AimRadius( float Scale, float Fov ) {
    const world::Snap& Snap = world::View( );
    float Wide = ( float )Snap.clientW;
    float Tall = ( float )Snap.clientH;
    HWND Overlay = ( HWND )ur::app::window( );
    if ( Overlay && Snap.clientW > 64 && Snap.clientH > 64 ) {
        POINT A{ Snap.clientX, Snap.clientY };
        POINT B{ Snap.clientX + Snap.clientW, Snap.clientY + Snap.clientH };
        ScreenToClient( Overlay, &A );
        ScreenToClient( Overlay, &B );
        Wide = ( float )( B.x - A.x );
        Tall = ( float )( B.y - A.y );
        if ( Wide < 0.0f )
            Wide = -Wide;
        if ( Tall < 0.0f )
            Tall = -Tall;
    }
    if ( Wide < 64.0f )
        Wide = ( float )ur::app::width( );
    if ( Tall < 64.0f )
        Tall = ( float )ur::app::height( );
    return aim::ComputeAimRadius( Wide, Tall, Scale, Fov );
}

static world::Vec3 SilentBone( const world::Actor& Item ) {
    return aim::SilentBone( Item, Mute.bones );
}

static void TickAim( float Scale ) {
    static float RestX = 0.0f;
    static float RestY = 0.0f;
    static uintptr_t Hold = 0;
    static LARGE_INTEGER Freq = { };
    static LARGE_INTEGER Last = { };
    if ( !Freq.QuadPart ) {
        QueryPerformanceFrequency( &Freq );
        QueryPerformanceCounter( &Last );
    }
    LARGE_INTEGER Now = { };
    QueryPerformanceCounter( &Now );
    float Dt = ( float )( Now.QuadPart - Last.QuadPart ) / ( float )Freq.QuadPart;
    Last = Now;
    Dt = aim::ClampAimDt( Dt );

    bool ListenBusy = Mute.listen || Aim.listen || Menu.listen || Menu.slide;
    bool MuteHeld = Held( Mute.key );
    bool SilentOk = Mute.on && MuteHeld && !ListenBusy;
    bool MouseOk = Aim.on && Held( Aim.key ) && !ListenBusy;
    if ( !Mute.on )
        silent::Remove( );
    if ( !SilentOk && !MouseOk ) {
        RestX = 0.0f;
        RestY = 0.0f;
        if ( !Aim.sticky || !Held( Aim.key ) )
            Hold = 0;
        if ( Mute.on )
            silent::Off( );
        return;
    }

    const world::Snap& Snap = world::View( );
    if ( !Snap.ready || Snap.count <= 0 ) {
        silent::Off( );
        return;
    }

    CVector Mid = AimMid( );
    auto Pick = [ & ]( float Fov, bool Team, int Bones, bool Pred, bool NeedVis, int Sort, CVector* OutAt ) -> const world::Actor* {
        const world::Actor* Best = nullptr;
        float Limit = AimRadius( Scale, Fov );
        float BestScore = 1.0e9f;
        float Far = 1.0f;
        for ( int Index = 0; Index < Snap.count; Index++ ) {
            if ( Snap.list[ Index ].dist > Far )
                Far = Snap.list[ Index ].dist;
        }
        CVector Chosen;
        for ( int Index = 0; Index < Snap.count; Index++ ) {
            const world::Actor& Item = Snap.list[ Index ];
            if ( Team && Item.mate )
                continue;
            if ( NeedVis && !Item.vis )
                continue;
            CVector At;
            float Screen = 1.0e9f;
            bool OnScreen = false;
            auto Consider = [ & ]( const world::Vec3& World ) {
                CVector Point;
                if ( !EspDot( World, Point ) )
                    return;
                float Dx = Point.Horizontal - Mid.Horizontal;
                float Dy = Point.Vertical - Mid.Vertical;
                float Dist = sqrtf( Dx * Dx + Dy * Dy );
                if ( !OnScreen || Dist < Screen ) {
                    OnScreen = true;
                    Screen = Dist;
                    At = Point;
                }
            };
            Consider( AimPoint( Item, Pred, Bones, Pred ) );
            if ( !OnScreen )
                continue;
            float Score = aim::ScoreTarget( Screen, Limit, Item.dist, Far, Sort );
            if ( Score >= 1.0e9f )
                continue;
            if ( Score < BestScore ) {
                BestScore = Score;
                Best = &Item;
                Chosen = At;
            }
        }
        if ( Best && OutAt )
            *OutAt = Chosen;
        return Best;
    };

    if ( SilentOk ) {
        CVector GhostAt;
        CVector Cross = ScreenMid( );
        CVector KeepMid = Mid;
        float KeepScale = Scale;
        Mid = Cross;
        Scale = 1.0f;
        const world::Actor* Ghost = Pick( 360.0f, Mute.team, Mute.bones, Mute.pred, Mute.vis, Mute.sort, &GhostAt );
        Mid = KeepMid;
        Scale = KeepScale;
        if ( Ghost ) {
            world::Vec3 AimAt = SilentBone( *Ghost );
            if ( Mute.pred ) {
                AimAt = aim::PredictLeadSilent( AimAt, Ghost->vel, Ghost->dist, Snap.localPing, Ghost->ping );
            }
            world::Dot View;
            float Sx = 0.0f;
            float Sy = 0.0f;
            CVector At;
            if ( world::ToView( AimAt, View ) ) {
                Sx = View.x;
                Sy = View.y;
            } else if ( EspDot( AimAt, At ) ) {
                OverlayToView( At, Sx, Sy );
            } else {
                OverlayToView( GhostAt, Sx, Sy );
            }
            silent::On( AimAt, Sx, Sy, true );
        } else {
            silent::Off( );
        }
        if ( !MouseOk )
            return;
    } else if ( Mute.on ) {
        silent::Off( );
    }
    if ( !MouseOk )
        return;

    CVector BestAt;
    const world::Actor* Best = nullptr;
    if ( Aim.sticky && Hold ) {
        for ( int Index = 0; Index < Snap.count; Index++ ) {
            const world::Actor& Item = Snap.list[ Index ];
            if ( Item.player != Hold )
                continue;
            if ( !aim::IsTargetValid( Item.mate, Item.vis, Aim.team, Aim.vis ) )
                break;
            if ( AimDot( AimPoint( Item ), BestAt ) )
                Best = &Item;
            break;
        }
    }
    if ( !Best )
        Best = Pick( Aim.fov, Aim.team, Aim.bones, Aim.pred, Aim.vis, Aim.sort, &BestAt );
    if ( !Best ) {
        Hold = 0;
        return;
    }
    Hold = Best->player;
    float Dx = BestAt.Horizontal - Mid.Horizontal;
    float Dy = BestAt.Vertical - Mid.Vertical;
    aim::MouseStep Mouse = aim::ComputeSmoothMouseStep( Dx, Dy, Aim.smooth, Dt, RestX, RestY );
    if ( !Mouse.moved )
        return;
    INPUT Step{ };
    Step.type = INPUT_MOUSE;
    Step.mi.dx = Mouse.moveX;
    Step.mi.dy = Mouse.moveY;
    Step.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput( 1, &Step, sizeof( Step ) );
}

static bool EspDot( const world::Vec3& World, CVector& Out ) {
    world::Dot Hit;
    if ( !world::ToScreen( World, Hit ) )
        return false;
    Out = OverlayOf( Hit.x, Hit.y );
    return true;
}

static void DrawFovRings( float Scale ) {
    CVector Mid = AimMid( );
    float Pulse = aim::ComputeFovPulse( Context->Elapsed );
    float Keep = Canvas->Opacity;
    if ( Aim.on && Aim.drawFov ) {
        float Ring = ur::motion::toward( "aim.fov.ring", 1.0f, 18.0f );
        float Radius = AimRadius( Scale, Aim.fov );
        Canvas->Opacity = Keep * Ring * Pulse;
        Canvas->Border( CRectangle( Mid.Horizontal - Radius, Mid.Vertical - Radius, Radius * 2.0f, Radius * 2.0f ), Mix( Style->Accent, Style->AccentSoft, 0.3f ), Radius, 1.6f * Scale );
    }
    Canvas->Opacity = Keep;
}

static void DrawWeather( float Across, float Vertical, float Scale ) {
    weather::Tick( ( float )Context->DeltaTime, Across, Vertical );
    if ( weather::mode( ) == weather::Off )
        return;

    float Keep = Canvas->Opacity;
    Canvas->Opacity = 1.0f;
    weather::State& Storm = weather::Live( );
    CColor Flake = Mix( CColor( 230, 236, 246 ), Style->AccentSoft, 0.15f );
    CColor Streak = Mix( CColor( 170, 190, 220 ), Style->AccentSoft, 0.25f );

    if ( weather::mode( ) == weather::Storm ) {
        unsigned int Former = Canvas->Effect( weather::ThunderFx( ) );
        Canvas->Rectangle( CRectangle( 0.0f, 0.0f, Across, Vertical ), CColor( 255, 255, 255, 210 ), 0.0f );
        Canvas->Effect( Former );
        Canvas->Opacity = Keep;
        return;
    }

    for ( int Index = 0; Index < Storm.used; Index++ ) {
        const weather::Drop& Item = Storm.list[ Index ];
        if ( weather::mode( ) == weather::Snow ) {
            Canvas->Circle( CVector( Item.x, Item.y ), Item.size * Scale, Flake.Fade( 0.72f ) );
        } else {
            weather::Point2D End = weather::ComputeRainStreakEnd( Item.x, Item.y, Item.vx, Item.size );
            Canvas->Line( CVector( Item.x, Item.y ), CVector( End.x, End.y ), Streak.Fade( 0.45f ), 1.1f * Scale );
        }
    }
    Canvas->Opacity = Keep;
}

static void DrawEspWorld( float Scale ) {
    if ( !Esp.on )
        return;
    const world::Snap& Snap = world::View( );
    if ( !Snap.ready || Snap.count <= 0 )
        return;

    float Keep = Canvas->Opacity;
    Canvas->Opacity = 1.0f;
    CColor Edge = CColor( 8, 10, 14, 210 );
    float Thick = 1.5f * Scale;
    CVector Foot( ( float )ur::app::width( ) * 0.5f, ( float )ur::app::height( ) - 4.0f * Scale );

    for ( int Index = 0; Index < Snap.count; Index++ ) {
        const world::Actor& Item = Snap.list[ Index ];
        if ( Item.dist > Esp.range )
            continue;
        if ( Esp.team && Item.mate )
            continue;
        CVector Dots[ world::BoneMax ];
        bool On[ world::BoneMax ] = { };
        esp::BBox2D BBox;
        auto Push = [ & ]( const world::Vec3& World ) {
            CVector At;
            if ( !EspDot( World, At ) )
                return;
            BBox.Push( At.Horizontal, At.Vertical );
        };
        auto PushOff = [ & ]( world::Vec3 Point, float Side, float Lift ) {
            Point.x += Snap.right.x * Side;
            Point.y += Lift;
            Point.z += Snap.right.z * Side;
            Push( Point );
        };
        Push( Item.head );
        Push( Item.low );
        Push( Item.root );
        PushOff( Item.head, 0.70f, 0.20f );
        PushOff( Item.head, -0.70f, 0.20f );
        PushOff( Item.low, 1.05f, 0.0f );
        PushOff( Item.low, -1.05f, 0.0f );
        for ( int Slot = 0; Slot < world::BoneMax; Slot++ ) {
            if ( !Item.boneOk[ Slot ] )
                continue;
            CVector At;
            if ( !EspDot( Item.world[ Slot ], At ) )
                continue;
            Dots[ Slot ] = At;
            On[ Slot ] = true;
            Push( Item.world[ Slot ] );
        }
        if ( !BBox.IsValid( 2.0f, 2 ) )
            continue;

        CRectangle Box( BBox.minX, BBox.minY, BBox.Width( ), BBox.Height( ) );

        if ( Esp.snap ) {
            float SnapX = 0.0f, SnapY = 0.0f;
            esp::ComputeSnaplineTarget( Box.Left, Box.Bottom( ), Box.Width, SnapX, SnapY );
            Canvas->Line( Foot, CVector( SnapX, SnapY ), FeatColor( FeatSnap, Item.vis ).Fade( 0.55f ), Thick );
        }

        if ( Esp.skeleton ) {
            CColor Joint = FeatColor( FeatSkel, Item.vis );
            int LinkCount = 0;
            const esp::BoneLink* Skeleton = esp::GetSkeletonLinks( Item.r15, LinkCount );
            for ( int Link = 0; Link < LinkCount; Link++ ) {
                int A = Skeleton[ Link ].from;
                int B = Skeleton[ Link ].to;
                if ( !On[ A ] || !On[ B ] )
                    continue;
                Canvas->Line( Dots[ A ], Dots[ B ], Joint, Thick );
            }
        }

        if ( Esp.box )
            Canvas->Border( Box, FeatColor( FeatBox, Item.vis ), 0.0f, Thick );

        if ( Esp.health ) {
            float Ratio = esp::ComputeHealthRatio( Item.health, Item.maxHealth );
            float RailLeft = 0.0f, RailTop = 0.0f, RailW = 0.0f, RailH = 0.0f;
            float FillTop = 0.0f, FillH = 0.0f;
            esp::ComputeHealthBar( Box.Left, Box.Top, Box.Height, Scale, Ratio,
                                  RailLeft, RailTop, RailW, RailH, FillTop, FillH );
            Canvas->Rectangle( CRectangle( RailLeft, RailTop, RailW, RailH ), CColor( 10, 12, 16, 190 ), 0.0f );
            Canvas->Rectangle( CRectangle( RailLeft, FillTop, RailW, FillH ), FeatColor( FeatHealth, Item.vis ), 0.0f );
        }

        if ( Font && Esp.name ) {
            CVector Size = Font->Measure( Item.name );
            float AtX = 0.0f, AtY = 0.0f;
            esp::ComputeTopCenteredText( Box.Left, Box.Top, Box.Width, Size.Horizontal, Size.Vertical, Scale, AtX, AtY );
            Canvas->Outlined( CVector( AtX, AtY ), FeatColor( FeatName, Item.vis ), Edge, 1.0f, Item.name );
        }
        if ( Font && Esp.dist ) {
            char Line[ 24 ];
            esp::FormatDistance( Item.dist, Line, sizeof( Line ) );
            CVector Size = Font->Measure( Line );
            float AtX = 0.0f, AtY = 0.0f;
            esp::ComputeBottomCenteredText( Box.Left, Box.Bottom( ), Box.Width, Size.Horizontal, Scale, AtX, AtY );
            Canvas->Outlined( CVector( AtX, AtY ), FeatColor( FeatDist, Item.vis ), Edge, 1.0f, Line );
        }
    }
    Canvas->Opacity = Keep;
}

static void DrawExploreMark( float Scale ) {
    if ( !Tree.mark || !Tree.pick )
        return;
    uintptr_t Part = browse::AimPart( Tree.pick );
    world::Vec3 Pos;
    if ( !Part || !world::PartPos( Part, Pos ) )
        return;
    world::Vec3 Size{ };
    if ( !world::PartSize( Part, Size ) ) {
        Size.x = 1.0f;
        Size.y = 2.0f;
        Size.z = 1.0f;
    }
    world::Vec3 Hi{ }, Lo{ }, Right{ }, Left{ };
    esp::ComputePartExtents( Pos, Size, Hi, Lo, Right, Left );
    CVector A, B, C, D;
    if ( !EspDot( Hi, A ) || !EspDot( Lo, B ) )
        return;
    EspDot( Right, C );
    EspDot( Left, D );
    esp::BBox2D BBox;
    BBox.Push( A.Horizontal, A.Vertical );
    BBox.Push( B.Horizontal, B.Vertical );
    BBox.Push( C.Horizontal, C.Vertical );
    BBox.Push( D.Horizontal, D.Vertical );
    if ( !BBox.IsValid( 2.0f, 2 ) )
        return;
    float Keep = Canvas->Opacity;
    Canvas->Opacity = 1.0f;
    Canvas->Border( CRectangle( BBox.minX, BBox.minY, BBox.Width( ), BBox.Height( ) ), CColor( 255, 80, 200, 230 ), 0.0f, 2.0f * Scale );
    Canvas->Opacity = Keep;
}

static bool DrawMarks( float Across, float Vertical, float Scale, const CVector& Point, bool AllowDrag ) {
    if ( !Menu.watermark && !Menu.showFps ) {
        Badge.held = false;
        return false;
    }

    EnsureTitle( Scale );
    CFont* Face = ( Font && Font->LineSpan > 1.0f ) ? Font.get( ) : &TitleFace;
    if ( !Face || Face->LineSpan < 1.0f )
        return false;

    char Line[ 48 ] = { };
    MarkLine( Line, sizeof( Line ) );
    CRectangle Chip = PlaceMark( Across, Vertical, Scale, Face, Line );
    bool Over = DragMark( Chip, Point, Across, Vertical, AllowDrag );
    Chip = PlaceMark( Across, Vertical, Scale, Face, Line );

    float PadX = 12.0f * Scale;
    float Keep = Canvas->Opacity;
    Canvas->Opacity = 1.0f;
    Canvas->Rectangle( Chip, Dress.card, 7.0f * Scale );
    DrawIce( Chip, Chip, 7.0f * Scale, 0.55f );
    Canvas->Border( Chip, Dress.foldLine, 7.0f * Scale, 1.0f );
    Canvas->Write( Face, CVector( Chip.Left + PadX, Chip.Top + ( Chip.Height - Face->LineSpan ) * 0.5f ), Dress.inkHot, Line );
    Canvas->Opacity = Keep;
    if ( Over && !Moving( ) )
        Input->Pointer = PointerHand;
    if ( Badge.held )
        Input->Pointer = PointerMove;
    return Over;
}

static void TickStream( ) {
    HWND Handle = ( HWND )ur::app::window( );
    if ( !Handle )
        return;

    DWORD Want = Menu.stream ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE;
    DWORD Now = WDA_NONE;
    if ( GetWindowDisplayAffinity( Handle, &Now ) && Now == Want ) {
        Menu.streamed = Menu.stream;
        return;
    }

    if ( SetWindowDisplayAffinity( Handle, Want ) ) {
        Menu.streamed = Menu.stream;
        return;
    }

    if ( Menu.stream )
        SetWindowDisplayAffinity( Handle, 0x00000001 );
}

static void Draw( float Across, float Vertical ) {
    float Scale = Style->Scale > 0.0f ? Style->Scale : 1.0f;
    float Wide = MenuWidth * Scale;
    float Tall = MenuHeight * Scale;
    float Cap = HeaderHeight * Scale;
    float Round = Style->Rounding * Scale;
    float Pad = 10.0f * Scale;
    float RailW = RailWidth * Scale;

    if ( !Menu.ready ) {
        Center( Across, Vertical, Scale );
        Menu.ready = true;
    }

    CVector Point = Cursor( );
    CRectangle Bounds( Menu.origin, CVector( Wide, Tall ) );
    CRectangle Header( Bounds.Left, Bounds.Top, Bounds.Width, Cap );
    CRectangle Rail( Bounds.Left + Pad, Header.Bottom( ) + Pad, RailW, Bounds.Height - Cap - Pad * 2.0f );
    CRectangle Stack( Rail.Left, Rail.Top, Rail.Width, TabHeight * Scale * ( float )TabCount + TabGap * Scale * ( float )( TabCount - 1 ) );
    CRectangle Content( Rail.Right( ) + Pad, Header.Bottom( ) + Pad, Bounds.Right( ) - Rail.Right( ) - Pad * 2.0f, Bounds.Height - Cap - Pad * 2.0f );
    CRectangle Shut = CloseBounds( Header, Scale );

    bool Press = Held( VK_LBUTTON );
    bool Click = Press && !Menu.mouse && !LiveCh.open;
    bool OverExplore = false;
    if ( Tree.open ) {
        PlaceExplore( Across, Vertical, Scale );
        CRectangle ExploreBox( Tree.origin, CVector( ExploreWidth * Scale, ExploreHeight * Scale ) );
        CRectangle ExploreHead;
        CRectangle ExplorePane;
        ExploreChrome( ExploreBox, Scale, ExploreHead, ExplorePane );
        OverExplore = ExploreBox.Contains( Point );
        DragExplore( ExploreBox, Point, Across, Vertical, ExploreWidth * Scale, ExploreHeight * Scale, OverExplore && !ExplorePane.Contains( Point ) && !Listening( ) );
        OverExplore = CRectangle( Tree.origin, CVector( ExploreWidth * Scale, ExploreHeight * Scale ) ).Contains( Point ) || Tree.held;
    }

    if ( Click && !Moving( ) && !OverExplore ) {
        for ( int Index = 0; Index < TabCount; Index++ ) {
            if ( TabBounds( Rail, Scale, Index ).Contains( Point ) && Menu.tab != Index ) {
                Menu.pageDir = Index > Menu.tab ? 1.0f : -1.0f;
                Menu.tab = Index;
                Menu.pageIn = 0.0f;
                DropId = nullptr;
            }
        }
    }

    bool OverMark = false;
    if ( Menu.watermark || Menu.showFps ) {
        EnsureTitle( Scale );
        CFont* Face = ( Font && Font->LineSpan > 1.0f ) ? Font.get( ) : &TitleFace;
        if ( Face && Face->LineSpan > 1.0f ) {
            char Line[ 48 ] = { };
            MarkLine( Line, sizeof( Line ) );
            CRectangle Chip = PlaceMark( Across, Vertical, Scale, Face, Line );
            OverMark = DragMark( Chip, Point, Across, Vertical, !OverExplore && !Listening( ) );
        }
    }

    bool OverTab = Stack.Contains( Point );
    bool OverClose = Shut.Contains( Point );
    bool OverPage = ( ( Menu.tab == TabAimbot || Menu.tab == TabRage || Menu.tab == TabEsp || Menu.tab == TabConfigs || Menu.tab == TabSettings ) && Content.Contains( Point ) ) || DropHit( Point, Scale );
    Drag( Bounds, Point, Across, Vertical, Wide, Tall, !OverTab && !OverClose && !OverPage && !Listening( ) && !OverExplore && !Tree.held && !OverMark && !Badge.held );
    Bounds = CRectangle( Menu.origin, CVector( Wide, Tall ) );
    Header = CRectangle( Bounds.Left, Bounds.Top, Bounds.Width, Cap );
    Rail = CRectangle( Bounds.Left + Pad, Header.Bottom( ) + Pad, RailW, Bounds.Height - Cap - Pad * 2.0f );
    Stack = CRectangle( Rail.Left, Rail.Top, Rail.Width, TabHeight * Scale * ( float )TabCount + TabGap * Scale * ( float )( TabCount - 1 ) );
    Content = CRectangle( Rail.Right( ) + Pad, Header.Bottom( ) + Pad, Bounds.Right( ) - Rail.Right( ) - Pad * 2.0f, Bounds.Height - Cap - Pad * 2.0f );
    Shut = CloseBounds( Header, Scale );

    CRectangle Drawn[ TabCount ];
    OverTab = false;
    for ( int Index = 0; Index < TabCount; Index++ ) {
        Drawn[ Index ] = TabBounds( Rail, Scale, Index );
        if ( Drawn[ Index ].Contains( Point ) )
            OverTab = true;
    }

    Gate( Bounds.Contains( Point ) || OverExplore || OverMark || DropHit( Point, Scale ), Point );
    if ( ( OverTab || OverClose ) && !Moving( ) )
        Input->Pointer = PointerHand;

    Canvas->Route( 0 );
    DrawWeather( Across, Vertical, Scale );
    DrawEspWorld( Scale );
    DrawExploreMark( Scale );
    float Fade = Menu.fade / 100.0f;
    if ( Fade < 0.4f )
        Fade = 0.4f;
    if ( Fade > 1.0f )
        Fade = 1.0f;
    DrawFovRings( Scale );
    float Shell = Canvas->Opacity;
    Canvas->Opacity = Shell * Fade;
    Canvas->Shadow( Bounds, CColor( 6, 10, 18, 130 ), Round, 24.0f * Scale );
    Canvas->Rectangle( Bounds, Style->Surface, Round );

    DrawIce( Header, Bounds, Round, 1.0f );
    DrawTitle( Header, Scale, "Unlinked" );
    bool CloseBusy = DrawClose( Header, Point, Click && !OverExplore, Scale, "close.hover", true );

    Canvas->Rectangle( Stack, Dress.rail, 12.0f * Scale );
    DrawTabSwipe( Rail, Scale );
    Canvas->Rectangle( Content, Style->Elevated, 10.0f * Scale );
    for ( int Index = 0; Index < TabCount; Index++ )
        DrawTab( Drawn[ Index ], Tabs[ Index ], Index, Scale, Drawn[ Index ].Contains( Point ) && !Moving( ) && !OverExplore );
    bool PageBusy = false;
    DrawPage( Content, Point, Click && !OverExplore, Press, Scale, PageBusy );
    if ( ( PageBusy || CloseBusy ) && !Moving( ) )
        Input->Pointer = PointerHand;
    Canvas->Border( Bounds, Style->Outline.Blend( Style->Accent, 0.22f ), Round, Style->Thickness );

    if ( Tree.open )
        DrawExplorer( Across, Vertical, Point, Click, Press, Scale );
    Canvas->Opacity = Shell;
    DrawMarks( Across, Vertical, Scale, Point, false );
}

static void TickMenuMouse( ) {
    static bool Freed = false;
    if ( !Freed ) {
        ClipCursor( nullptr );
        Freed = true;
    }
}

static void Tick( ) {
    if ( !Menu.vsync )
        ur::app::set_vsync( false );
    Tokens( );
    PackBoot( );
    offsets::Boot( );
    TickChannel( );
    world::Pulse( Esp.on || Aim.on || Mute.on, Aim.on || Mute.on, Esp.skeleton, Esp.range, Esp.on || Aim.vis || ( Mute.on && Mute.vis ) );
    TickMenuMouse( );
    TickAim( Style->Scale > 0.0f ? Style->Scale : 1.0f );
    move::Tick( Context->DeltaTime, Menu.listen || Aim.listen || Mute.listen );
    if ( !Mute.on ) {
        silent::Off( );
        silent::Remove( );
    }
    if ( Tree.open ) {
        if ( !browse::Live( ) )
            browse::Open( );
        if ( !Esp.on && !Aim.on )
            world::FrameView( );
        lstrcpynA( browse::Core( ).query, Tree.find, ( int )sizeof( browse::Core( ).query ) );
        browse::Tick( );
        Tree.pick = browse::Pick( );
        if ( Tree.type )
            TreeDraft( );
    }

    static bool Quiet = false;
    if ( !Quiet ) {
        ur::toast::clear( );
        Quiet = true;
    }

    if ( Edge( VK_ESCAPE, Menu.escape ) ) {
        if ( LiveCh.open ) {
            LiveCh.open = false;
            LiveCh.dismissed = true;
        } else if ( Packs.type )
            Packs.type = false;
        else if ( Tree.type )
            Tree.type = false;
        else if ( Tree.confirm )
            Tree.confirm = false;
        else if ( Menu.listen )
            Menu.listen = false;
        else if ( DropId )
            DropId = nullptr;
        else if ( Aim.listen )
            Aim.listen = false;
        else if ( Mute.listen )
            Mute.listen = false;
        else if ( Tree.open ) {
            Tree.open = false;
            browse::Close( );
        }
    }

    if ( Packs.type ) {
        PackDraft( );
    } else if ( Menu.listen || Aim.listen || Mute.listen ) {
        int Next = PollBind( ( Aim.listen || Mute.listen ) && !Menu.listen );
        if ( Next ) {
            if ( Menu.listen ) {
                Menu.menuKey = Next;
                Menu.listen = false;
                Menu.insert = true;
            } else if ( Mute.listen ) {
                Mute.key = Next;
                Mute.listen = false;
            } else {
                Aim.key = Next;
                Aim.listen = false;
            }
        }
    } else if ( Edge( Menu.menuKey, Menu.insert ) ) {
        Menu.visible = !Menu.visible;
        if ( Menu.visible )
            OpenLiveFolds( );
    }

    TickAfk( );
    TickStream( );

    bool Press = Held( VK_LBUTTON );
    bool ChanClick = Press && !ChanMouse;
    ChanMouse = Press;

    if ( !Menu.visible ) {
        Tree.held = false;
        float Across = ( float )ur::app::width( );
        float Vertical = ( float )ur::app::height( );
        float Scale = Style->Scale > 0.0f ? Style->Scale : 1.0f;
        CVector Point = Cursor( );
        Canvas->Route( 0 );
        DrawEspWorld( Scale );
        DrawFovRings( Scale );
        DrawExploreMark( Scale );
        bool OverMark = DrawMarks( Across, Vertical, Scale, Point, true );
        DrawChannelNotice( Across, Vertical, Point, ChanClick, Scale );
        ur::overlay::Options& Overlay = ur::app::overlay_options( );
        Overlay.click_through = !LiveCh.open && !OverMark && !Badge.held;
        Menu.mouse = Press;
        Pace( );
        return;
    }

    Draw( ( float )ur::app::width( ), ( float )ur::app::height( ) );
    DrawChannelNotice( ( float )ur::app::width( ), ( float )ur::app::height( ), Cursor( ), ChanClick, Style->Scale > 0.0f ? Style->Scale : 1.0f );
    if ( LiveCh.open )
        ur::app::overlay_options( ).click_through = false;
    Pace( );
}

static bool LoadFace( const char* Path ) {
    if ( AddFontResourceExA( Path, FR_PRIVATE, nullptr ) <= 0 )
        return false;
    if ( LoadedFaceCount < 8 )
        lstrcpynA( LoadedFaces[ LoadedFaceCount++ ], Path, MAX_PATH );
    return true;
}

static void UnloadFaces( ) {
    for ( int Index = 0; Index < LoadedFaceCount; Index++ ) {
        if ( LoadedFaces[ Index ][ 0 ] )
            RemoveFontResourceExA( LoadedFaces[ Index ], FR_PRIVATE, nullptr );
    }
    LoadedFaceCount = 0;
}

static void BindFace( ) {
    char Module[ MAX_PATH ] = { };
    GetModuleFileNameA( nullptr, Module, MAX_PATH );
    std::string Folder = Module;
    size_t Slash = Folder.find_last_of( "\\/" );
    if ( Slash != std::string::npos )
        Folder = Folder.substr( 0, Slash ) + "\\assets\\fonts\\";
    else
        Folder = "assets\\fonts\\";

    const char* Local[ ] = {
        "Inter-Regular.ttf",
        "Inter-Medium.ttf",
        "Inter-SemiBold.ttf"
    };

    for ( const char* Name : Local )
        LoadFace( ( Folder + Name ).c_str( ) );
}

}

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int ) {
    BindFace( );

    char Module[ MAX_PATH ] = { };
    GetModuleFileNameA( nullptr, Module, MAX_PATH );
    LogoPath = Module;
    size_t Slash = LogoPath.find_last_of( "\\/" );
    if ( Slash != std::string::npos )
        LogoPath = LogoPath.substr( 0, Slash ) + "\\assets\\Unlinked.webp";
    else
        LogoPath = "assets\\Unlinked.webp";

    ur::overlay::Options& Overlay = ur::app::overlay_options( );
    Overlay.topmost = true;
    Overlay.borderless = true;
    Overlay.transparent = true;
    Overlay.layered = true;
    Overlay.click_through = true;
    Overlay.alpha = 255;

    ur::app::Config Config;
    Config.title = "Unlinked";
    Config.width = 1280;
    Config.height = 720;
    Config.backend = ur::Backend::DX11;
    Config.overlay = true;
    Config.persist = false;
    Config.docking = false;
    Config.vsync = false;
    Config.fonts = Fonts;
    Config.font_count = 3;
    Config.font_size = 13.0f;

    offsets::Boot( );
    world::Boot( );
    int Code = ur::app::run( Config, Tick );
    TitleFace.Destroy( );
    UnloadFaces( );
    return Code;
}
