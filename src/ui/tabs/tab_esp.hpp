#pragma once

/**
 * @file tab_esp.hpp
 * @brief Unlinked External - ESP tab rendering, visual customization, and palette pickers.
 */

static bool DrawEspOverlay( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    float Row = 32.0f * Scale;
    float Gap = 2.0f * Scale;
    bool Busy = false;
    struct ToggleDef { const char* label; const char* motion; bool& val; };
    ToggleDef defs[] = {
        { "Enabled", "esp.on", Esp.on },
        { "Box", "esp.box", Esp.box },
        { "Name", "esp.name", Esp.name },
        { "Health", "esp.health", Esp.health },
        { "Distance", "esp.dist", Esp.dist },
        { "Team check", "esp.team", Esp.team }
    };
    for ( int i = 0; i < 6; ++i ) {
        ui::RectBounds B = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, i );
        Busy = DrawSwitch( CRectangle( B.left, B.top, B.width, B.height ), defs[ i ].label, defs[ i ].motion, defs[ i ].val, Point, Click, Scale ) || Busy;
    }
    return Busy;
}

static bool DrawEspVisual( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    float Row = 32.0f * Scale;
    float Gap = 2.0f * Scale;
    bool Busy = false;
    ui::RectBounds B0 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 0 );
    Busy = DrawSwitch( CRectangle( B0.left, B0.top, B0.width, B0.height ), "Skeleton", "esp.skel", Esp.skeleton, Point, Click, Scale ) || Busy;
    ui::RectBounds B1 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 1 );
    Busy = DrawSwitch( CRectangle( B1.left, B1.top, B1.width, B1.height ), "Snaplines", "esp.snap", Esp.snap, Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawEspCustom( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    static const char* Feats[ ] = { "Box", "Name", "Health", "Distance", "Skeleton", "Snaplines", "All" };
    float Left = Body.Left + 16.0f * Scale;
    float Wide = Body.Width - 32.0f * Scale;
    float Top = Body.Top + 14.0f * Scale;
    bool Busy = false;
    Busy = DrawDrop( Left, Top, Wide, "Feature", "esp.feat", Feats, FeatCount + 1, Dye.feat, Point, Click, Scale ) || Busy;
    Top += Font->LineSpan + 42.0f * Scale;
    if ( Dye.feat < 0 || Dye.feat > FeatCount )
        Dye.feat = 0;
    bool All = Dye.feat == FeatCount;
    int& VisPick = All ? Dye.globVis : Dye.vis[ Dye.feat ];
    int& HidPick = All ? Dye.globHid : Dye.hid[ Dye.feat ];
    Canvas->Text( CVector( Left, Top ), Style->Faint, All ? "Visible (all)" : "Visible" );
    Top += Font->LineSpan + 8.0f * Scale;
    int WasVis = VisPick;
    Busy = DrawSwatches( Left, Top, Wide, 13, EspTints, VisPick, "esp.vis", Point, Click, Scale ) || Busy;
    if ( All && VisPick != WasVis ) {
        for ( int Index = 0; Index < FeatCount; Index++ )
            Dye.vis[ Index ] = VisPick;
    }
    Top += SwatchTall( Wide, 13, Scale ) + 14.0f * Scale;
    Canvas->Text( CVector( Left, Top ), Style->Faint, All ? "Hidden (all)" : "Hidden" );
    Top += Font->LineSpan + 8.0f * Scale;
    int WasHid = HidPick;
    Busy = DrawSwatches( Left, Top, Wide, 13, EspTints, HidPick, "esp.hid", Point, Click, Scale ) || Busy;
    if ( All && HidPick != WasHid ) {
        for ( int Index = 0; Index < FeatCount; Index++ )
            Dye.hid[ Index ] = HidPick;
    }
    return Busy;
}

static bool DrawEsp( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, float Ease ) {
    ( void )Press;
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
    float OverlayBody = Fit.overlay;
    float VisualBody = Fit.visual;
    float CustomBody = Fit.custom;
    float CustomRoom = Content.Bottom( ) - Inset - Top - Head;
    if ( CustomRoom < 0.0f )
        CustomRoom = 0.0f;
    if ( CustomBody > CustomRoom )
        CustomBody = CustomRoom;

    CRectangle Body;
    float Open = 0.0f;
    bool Busy = DrawFold( Left, Top, Wide, Head, OverlayBody, Round, Scale, "Overlay", "fold.esp.overlay", Esp.overlay, Point, Click, Body, Open );
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( Left, Top, Wide, Head + OverlayBody * Open ) );
        Busy = DrawEspOverlay( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float VisualTop = Top + Head + OverlayBody * Open + Gap;
    Busy = DrawFold( Left, VisualTop, Wide, Head, VisualBody, Round, Scale, "Visuals", "fold.esp.visual", Esp.visual, Point, Click, Body, Open ) || Busy;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( Left, VisualTop, Wide, Head + VisualBody * Open ) );
        Busy = DrawEspVisual( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float Right = Left + Wide + Gap;
    Busy = DrawFold( Right, Top, Wide, Head, CustomBody, Round, Scale, "Customization", "fold.esp.custom", Esp.custom, Point, Click, Body, Open ) || Busy;
    if ( Open > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * Open;
        Canvas->PushClip( CRectangle( Right, Top, Wide, Head + CustomBody * Open ) );
        Busy = DrawEspCustom( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    Canvas->Opacity = Keep;
    return Busy;
}
