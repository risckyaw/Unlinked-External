#pragma once

/**
 * @file tab_settings.hpp
 * @brief Unlinked External - Settings tab rendering, performance caps, and environment customization.
 */

static bool DrawMiscBody( const CRectangle& Body, const CVector& Point, bool Click, bool Press, float Scale ) {
    float Left = Body.Left + 12.0f * Scale;
    float Wide = Body.Width - 24.0f * Scale;
    float Top = Body.Top + 8.0f * Scale;
    float Row = 28.0f * Scale;
    bool Busy = false;

    CRectangle LimitRow( Left, Top, Wide, Row );
    float TrackW = 44.0f * Scale;
    float TrackH = 22.0f * Scale;
    CRectangle Track( LimitRow.Right( ) - TrackW, LimitRow.Top + ( Row - TrackH ) * 0.5f, TrackW, TrackH );
    bool OverLimit = LimitRow.Contains( Point ) && !Moving( );
    if ( OverLimit && Click )
        Menu.limit = !Menu.limit;

    float On = ur::motion::toward( "set.limit", Menu.limit ? 1.0f : 0.0f, 28.0f );
    Canvas->Text( CVector( Left, LimitRow.Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Text, "Limit FPS" );
    if ( Menu.limit ) {
        char Live[ 24 ];
        snprintf( Live, sizeof( Live ), "%.0f", ( double )Context->Framerate );
        CVector LiveSize = Font->Measure( Live );
        Canvas->Text( CVector( Track.Left - 8.0f * Scale - LiveSize.Horizontal, LimitRow.Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Faint, Live );
    }
    Canvas->Rectangle( Track, Mix( Dress.trackOff, Mix( Dress.trackOn, Style->Accent, 0.4f ), On ), TrackH * 0.5f );
    float Knob = 18.0f * Scale;
    CRectangle Dot( Track.Left + 2.0f * Scale + ( TrackW - Knob - 4.0f * Scale ) * On, Track.Top + ( TrackH - Knob ) * 0.5f, Knob, Knob );
    Canvas->Rectangle( Dot, Dress.inkHot, Knob * 0.5f );
    Busy = Busy || OverLimit;

    float Slide = ur::motion::toward( "set.slider", Menu.limit ? 1.0f : 0.0f, 32.0f );
    if ( Slide > 0.02f ) {
        CRectangle SlideRow( Left, LimitRow.Bottom( ) + 4.0f * Scale, Wide, 26.0f * Scale );
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Slide;
        if ( Menu.fps < 60.0f )
            Menu.fps = 60.0f;
        if ( Menu.fps > 1000.0f )
            Menu.fps = 1000.0f;
        Menu.fps = ( float )( int )( Menu.fps + 0.5f );

        Busy = DrawSlider( Left, SlideRow.Top, Wide, nullptr, "set.fps", Menu.fps, 60.0f, 1000.0f, Point, Click, Press, Scale ) || Busy;
        Canvas->Opacity = Amount;
        Top = SlideRow.Bottom( ) + 6.0f * Scale;
    } else {
        Top = LimitRow.Bottom( ) + 6.0f * Scale;
    }

    CRectangle SyncRow( Left, Top, Wide, Row );
    Busy = DrawSwitch( SyncRow, "VSync", "set.vsync", Menu.vsync, Point, Click, Scale ) || Busy;
    Top = SyncRow.Bottom( ) + 6.0f * Scale;

    Canvas->Text( CVector( Left, Top ), Style->Faint, "Toggle menu" );
    CRectangle Field( Left, Top + Font->LineSpan + 4.0f * Scale, 156.0f * Scale, 28.0f * Scale );
    bool OverBind = Field.Contains( Point ) && !Moving( );
    if ( OverBind && Click ) {
        Menu.listen = true;
        SyncBindKeys( );
    }

    float Wait = ur::motion::toward( "set.listen", Menu.listen ? 1.0f : 0.0f, 26.0f );
    float BindHover = ur::motion::toward( "set.bind.hover", ( OverBind && !Menu.listen ) ? 1.0f : 0.0f, 26.0f );
    DrawIce( Field, Field, 6.0f * Scale, 1.0f );
    Canvas->Border( Field, Mix( CColor( 90, 110, 140, 160 ), Style->AccentSoft, Wait * 0.75f + BindHover * 0.4f ), 6.0f * Scale, 1.0f );
    Canvas->Text( CVector( Field.Left + 10.0f * Scale, Field.Top + ( Field.Height - Font->LineSpan ) * 0.5f ), Mix( Style->Text, Style->AccentSoft, Wait ), Menu.listen ? "Press a key..." : KeyLabel( Menu.menuKey ) );
    return Busy || OverBind;
}

static bool DrawGameBody( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 10.0f * Scale;
    float Row = 32.0f * Scale;
    bool Busy = false;

    float Gap = 2.0f * Scale;
    ui::RectBounds FirstB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 0 );
    CRectangle First( FirstB.left, FirstB.top, FirstB.width, FirstB.height );
    Busy = DrawSwitch( First, "Anti-AFK", "game.afk", Menu.afk, Point, Click, Scale ) || Busy;

    ui::RectBounds SecondB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 1 );
    CRectangle Second( SecondB.left, SecondB.top, SecondB.width, SecondB.height );
    Busy = DrawSwitch( Second, "Uncapped FPS", "game.uncap", Menu.uncap, Point, Click, Scale ) || Busy;

    ui::RectBounds ThirdB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 2 );
    CRectangle Third( ThirdB.left, ThirdB.top, ThirdB.width, ThirdB.height );
    Canvas->Text( CVector( Third.Left, Third.Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Text, "Game Explorer" );
    const char* Action = Tree.open ? "Close" : "Open";
    CVector OpenSize = Font->Measure( "Close" );
    CVector ActionSize = Font->Measure( Action );
    float ChipW = OpenSize.Horizontal + 16.0f * Scale;
    float ChipH = 22.0f * Scale;
    CRectangle Chip( Third.Right( ) - ChipW, Third.Top + ( Row - ChipH ) * 0.5f, ChipW, ChipH );
    bool OverOpen = Chip.Contains( Point ) && !Moving( ) && !Menu.slide;
    float Hover = ur::motion::toward( "game.explorer.open", OverOpen ? 1.0f : 0.0f, 26.0f );
    CColor Ink = Tree.open
        ? Mix( CColor( 214, 220, 232 ), CColor( 232, 64, 72 ), Hover )
        : Mix( Style->AccentSoft, CColor( 220, 236, 255 ), Hover );
    Canvas->Text( CVector( Chip.Left + ( ChipW - ActionSize.Horizontal ) * 0.5f, Chip.Top + ( ChipH - Font->LineSpan ) * 0.5f ), Ink, Action );
    if ( OverOpen && Click ) {
        if ( Tree.open ) {
            Tree.open = false;
            Tree.type = false;
            Tree.confirm = false;
            browse::Close( );
        } else {
            Tree.open = true;
            Tree.docked = true;
            Tree.ready = false;
            Tree.confirm = false;
            browse::Open( );
        }
    }
    Busy = Busy || OverOpen;

    ui::RectBounds FourthB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 3 );
    CRectangle Fourth( FourthB.left, FourthB.top, FourthB.width, FourthB.height );
    Canvas->Text( CVector( Fourth.Left, Fourth.Top + ( Row - Font->LineSpan ) * 0.5f ), Style->Text, "Offsets" );
    const char* Refresh = "Refresh";
    CVector RefreshSize = Font->Measure( Refresh );
    float RefreshW = RefreshSize.Horizontal + 16.0f * Scale;
    CRectangle RefreshChip( Fourth.Right( ) - RefreshW, Fourth.Top + ( Row - ChipH ) * 0.5f, RefreshW, ChipH );
    CRectangle RefreshHit( RefreshChip.Left - 6.0f * Scale, Fourth.Top, RefreshW + 6.0f * Scale, Row );
    bool Work = offsets::Busy( );
    bool OverRefresh = RefreshHit.Contains( Point ) && !Moving( ) && !Menu.slide && !Work;
    float Spin = ur::motion::toward( "game.offsets.spin", Work ? 1.0f : 0.0f, 22.0f );
    float RefreshHover = ur::motion::toward( "game.offsets.refresh", OverRefresh ? 1.0f : 0.0f, 26.0f );
    float Pulse = 0.55f + 0.45f * ( 0.5f + 0.5f * sinf( ( float )Context->Elapsed * 7.0f ) );
    if ( Spin > 0.02f )
        DrawIce( RefreshChip, RefreshChip, ChipH * 0.5f, Spin * ( 0.4f + Pulse * 0.6f ) );
    Canvas->Text(
        CVector( RefreshChip.Left + ( RefreshW - RefreshSize.Horizontal ) * 0.5f, RefreshChip.Top + ( ChipH - Font->LineSpan ) * 0.5f ),
        Mix( Mix( Style->AccentSoft, CColor( 220, 236, 255 ), RefreshHover ), Dress.inkHot, Spin ),
        Refresh
    );

    char Status[ 48 ] = { };
    CColor Note = Mix( Style->Faint, Style->AccentSoft, 0.45f );
    if ( Work || Spin > 0.08f ) {
        const char* Step = offsets::StageText( );
        if ( !Step[ 0 ] )
            Step = "updating";
        int Dots = ( ( int )( Context->Elapsed * 4.0 ) % 3 ) + 1;
        snprintf( Status, sizeof( Status ), "%s%.*s", Step, Dots, "..." );
        Note = Mix( Style->AccentSoft, Dress.inkHot, Pulse );
    } else if ( offsets::Fresh( ) ) {
        lstrcpynA( Status, offsets::Ready( ) ? "updated" : "failed", ( int )sizeof( Status ) );
        Note = offsets::Ready( ) ? Mix( Style->AccentSoft, CColor( 160, 220, 180 ), 0.55f ) : Mix( Style->Faint, CColor( 232, 96, 96 ), 0.8f );
    } else if ( offsets::Ready( ) ) {
        char Live[ 40 ] = { };
        offsets::CopyVersion( Live, ( int )sizeof( Live ) );
        const char* Hash = Live;
        if ( strncmp( Hash, "version-", 8 ) == 0 )
            Hash += 8;
        snprintf( Status, sizeof( Status ), "%s%s", offsets::Stale( ) ? "cached · " : "", Hash[ 0 ] ? Hash : "ready" );
        if ( offsets::Stale( ) )
            Note = Mix( Style->Faint, CColor( 232, 168, 96 ), 0.55f );
    } else {
        offsets::CopyError( Status, ( int )sizeof( Status ) );
        if ( !Status[ 0 ] )
            lstrcpynA( Status, "offline", ( int )sizeof( Status ) );
        Note = Mix( Style->Faint, CColor( 232, 168, 96 ), 0.75f );
    }

    float StatusLeft = Fourth.Left + Font->Measure( "Offsets" ).Horizontal + 10.0f * Scale;
    float StatusMax = RefreshChip.Left - 8.0f * Scale - StatusLeft;
    if ( StatusMax > 12.0f * Scale ) {
        CVector StatusSize = Font->Measure( Status );
        while ( Status[ 0 ] && StatusSize.Horizontal > StatusMax ) {
            size_t Len = strlen( Status );
            if ( Len < 2 )
                break;
            Status[ Len - 1 ] = 0;
            StatusSize = Font->Measure( Status );
        }
        Canvas->Text( CVector( StatusLeft, Fourth.Top + ( Row - Font->LineSpan ) * 0.5f ), Note, Status );
        if ( Spin > 0.02f && StatusMax > 20.0f * Scale ) {
            float Sweep = ( float )fmod( Context->Elapsed * 1.35f, 1.0 );
            float BarW = 28.0f * Scale;
            float Travel = StatusMax - BarW;
            if ( Travel < 1.0f )
                Travel = 1.0f;
            CRectangle Shine( StatusLeft + Travel * Sweep, Fourth.Top + Row - 3.0f * Scale, BarW, 2.0f * Scale );
            DrawIce( CRectangle( StatusLeft, Shine.Top, StatusMax, Shine.Height ), Shine, Shine.Height * 0.5f, Spin * Pulse );
        }
    }
    if ( OverRefresh && Click )
        offsets::Request( true );
    Busy = Busy || OverRefresh || Work;
    return Busy;
}

static bool DrawOverlayBody( const CRectangle& Body, const CVector& Point, bool Click, bool Press, float Scale ) {
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    float Row = 30.0f * Scale;
    float Gap = 4.0f * Scale;
    bool Busy = false;
    ui::RectBounds FirstB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 0 );
    CRectangle First( FirstB.left, FirstB.top, FirstB.width, FirstB.height );
    Busy = DrawSwitch( First, "Watermark", "set.watermark", Menu.watermark, Point, Click, Scale ) || Busy;

    ui::RectBounds SecondB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 1 );
    CRectangle Second( SecondB.left, SecondB.top, SecondB.width, SecondB.height );
    Busy = DrawSwitch( Second, "Show FPS", "set.showfps", Menu.showFps, Point, Click, Scale ) || Busy;

    ui::RectBounds ThirdB = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 2 );
    CRectangle Third( ThirdB.left, ThirdB.top, ThirdB.width, ThirdB.height );
    Busy = DrawSwitch( Third, "Streamproof", "set.stream", Menu.stream, Point, Click, Scale ) || Busy;
    Busy = DrawSlider( Left, Third.Bottom( ) + 8.0f * Scale, Wide, "Menu opacity", "set.fade", Menu.fade, 40.0f, 100.0f, Point, Click, Press, Scale ) || Busy;
    Busy = DrawSlider( Left, Third.Bottom( ) + 40.0f * Scale, Wide, "ESP range", "esp.range", Esp.range, 25.0f, 2000.0f, Point, Click, Press, Scale ) || Busy;
    return Busy;
}

static bool DrawThemeBody( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    static const char* Tones[ 3 ] = { "Dark Knight", "Coffee", "Matcha" };
    static const char* Looks[ 6 ] = {
        "Ice", "Thunder", "Ether", "Snow", "Bends", "Clouds"
    };
    static const char* Weather[ 4 ] = { "Off", "Snow", "Rain", "Thunder" };

    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 10.0f * Scale;
    bool Busy = false;
    Busy = DrawDrop( Left, Top, Wide, "Colors", "skin.tone", Tones, 3, skin::tone( ), Point, Click, Scale ) || Busy;
    Top += Font->LineSpan + 42.0f * Scale;
    Busy = DrawDrop( Left, Top, Wide, "Shader", "skin.look", Looks, 6, skin::look( ), Point, Click, Scale ) || Busy;
    Top += Font->LineSpan + 42.0f * Scale;
    Busy = DrawDrop( Left, Top, Wide, "Particles", "skin.weather", Weather, 4, weather::mode( ), Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawSettings( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, float Ease ) {
    float Keep = Canvas->Opacity;
    Canvas->Opacity = Keep * Ease;

    PageFit Fit = FitOf( Scale );
    float Inset = Fit.inset;
    float Gap = Fit.gap;
    float Left = Content.Left + Inset;
    float Top = Content.Top + Inset;
    float Wide = ( Content.Width - Inset * 2.0f - Gap ) * 0.5f;
    float Head = Fit.head;
    float Round = 8.0f * Scale;
    float MiscBody = Fit.misc;
    float GameBody = Fit.game;
    float OverlayBody = Fit.setOverlay;
    float ThemeBody = Fit.theme;

    Canvas->PushClip( Content );
    CRectangle Body;
    float Open = 0.0f;
    bool Busy = DrawFold( Left, Top, Wide, Head, MiscBody, Round, Scale, "Misc", "fold.misc", Menu.misc, Point, Click, Body, Open );
    float MiscOpen = Open;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( Left, Top, Wide, Head + MiscBody * Open ) );
        Busy = DrawMiscBody( Body, Point, Click, Press, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float GameLeft = Left + Wide + Gap;
    Busy = DrawFold( GameLeft, Top, Wide, Head, GameBody, Round, Scale, "Game", "fold.game", Menu.game, Point, Click, Body, Open ) || Busy;
    float GameOpen = Open;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( GameLeft, Top, Wide, Head + GameBody * Open ) );
        Busy = DrawGameBody( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float LeftStack = Top + Head + MiscBody * MiscOpen + Gap;
    Busy = DrawFold( Left, LeftStack, Wide, Head, OverlayBody, Round, Scale, "Overlay", "fold.overlay", Menu.overlay, Point, Click, Body, Open ) || Busy;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( Left, LeftStack, Wide, Head + OverlayBody * Open ) );
        Busy = DrawOverlayBody( Body, Point, Click, Press, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float RightStack = Top + Head + GameBody * GameOpen + Gap;
    Busy = DrawFold( GameLeft, RightStack, Wide, Head, ThemeBody, Round, Scale, "Theme", "fold.theme", Menu.theme, Point, Click, Body, Open ) || Busy;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( GameLeft, RightStack, Wide, Head + ThemeBody * Open ) );
        Busy = DrawThemeBody( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }
    Canvas->PopClip( );

    Canvas->Opacity = Keep;
    return Busy;
}
