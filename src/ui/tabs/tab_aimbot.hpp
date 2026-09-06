#pragma once

/**
 * @file tab_aimbot.hpp
 * @brief Unlinked External - Aimbot tab rendering and controls.
 */

static bool DrawAimGeneral( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    float Row = 32.0f * Scale;
    float Gap = 2.0f * Scale;
    bool Busy = false;
    ui::RectBounds B0 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 0 );
    Busy = DrawSwitch( CRectangle( B0.left, B0.top, B0.width, B0.height ), "Enabled", "aim.on", Aim.on, Point, Click, Scale ) || Busy;
    ui::RectBounds B1 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 1 );
    Busy = DrawSwitch( CRectangle( B1.left, B1.top, B1.width, B1.height ), "Team check", "aim.team", Aim.team, Point, Click, Scale ) || Busy;
    ui::RectBounds B2 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 2 );
    Busy = DrawSwitch( CRectangle( B2.left, B2.top, B2.width, B2.height ), "Visible only", "aim.vis", Aim.vis, Point, Click, Scale ) || Busy;
    ui::RectBounds B3 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 3 );
    Busy = DrawSwitch( CRectangle( B3.left, B3.top, B3.width, B3.height ), "Sticky aim", "aim.sticky", Aim.sticky, Point, Click, Scale ) || Busy;
    ui::RectBounds B4 = ui::ComputeStackedRow( Left, Top, Wide, Row, Gap, 4 );
    Busy = DrawSwitch( CRectangle( B4.left, B4.top, B4.width, B4.height ), "Prediction", "aim.pred", Aim.pred, Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawAimSilent( const CRectangle& Body, const CVector& Point, bool Click, bool Press, float Scale ) {
    ( void )Press;
    static const char* Bones[ ] = { "Head", "Neck", "Chest", "Stomach", "Body", "Legs" };
    float Pad = 14.0f * Scale;
    float Gap = 16.0f * Scale;
    float Left = 0.0f, Right = 0.0f, Col = 0.0f;
    ui::ComputeTwoColumnPartition( Body.Left, Body.Width, Pad, Gap, Left, Right, Col );
    float Top = Body.Top + 12.0f * Scale;
    float Row = 30.0f * Scale;
    bool Busy = false;

    CRectangle First( Left, Top, Col, Row );
    Busy = DrawSwitch( First, "Enabled", "silent.on", Mute.on, Point, Click, Scale ) || Busy;
    CRectangle Second( Left, First.Bottom( ) + 2.0f * Scale, Col, Row );
    Busy = DrawSwitch( Second, "Team check", "silent.team", Mute.team, Point, Click, Scale ) || Busy;
    CRectangle Third( Left, Second.Bottom( ) + 2.0f * Scale, Col, Row );
    Busy = DrawSwitch( Third, "Visible only", "silent.vis", Mute.vis, Point, Click, Scale ) || Busy;
    CRectangle Fourth( Left, Third.Bottom( ) + 2.0f * Scale, Col, Row );
    Busy = DrawSwitch( Fourth, "Prediction", "silent.pred", Mute.pred, Point, Click, Scale ) || Busy;
    Busy = DrawBind( Left, Fourth.Bottom( ) + 10.0f * Scale, "Silent key", "silent.listen", Mute.key, Mute.listen, Point, Click, Scale ) || Busy;

    float Slide = Top;
    static const char* Sorts[ ] = { "FOV", "Distance", "Combine" };
    Busy = DrawDrop( Right, Slide, Col, "Priority", "silent.sort", Sorts, 3, Mute.sort, Point, Click, Scale ) || Busy;
    Slide += Font->LineSpan + 38.0f * Scale;
    Busy = DrawDropBits( Right, Slide, Col, "Target", "silent.bone", Bones, 6, Mute.bones, Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawAimTarget( const CRectangle& Body, const CVector& Point, bool Click, bool Press, float Scale ) {
    static const char* Bones[ ] = { "Head", "Neck", "Chest", "Stomach", "Body", "Legs" };
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    float Row = 30.0f * Scale;
    bool Busy = false;

    CRectangle Draw( Left, Top, Wide, Row );
    Busy = DrawSwitch( Draw, "Draw FOV", "aim.drawfov", Aim.drawFov, Point, Click, Scale ) || Busy;
    Top = Draw.Bottom( ) + 8.0f * Scale;

    float ShowFov = ur::motion::toward( "aim.fov.show", Aim.drawFov ? 1.0f : 0.0f, 32.0f );
    if ( ShowFov > 0.02f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * ShowFov;
        Busy = DrawSlider( Left, Top, Wide, "FOV", "aim.fov", Aim.fov, 10.0f, 360.0f, Point, Click, Press, Scale ) || Busy;
        Canvas->Opacity = Amount;
        Top += 30.0f * Scale * ShowFov;
    }

    Busy = DrawSlider( Left, Top, Wide, "Smooth", "aim.smooth", Aim.smooth, 0.0f, 100.0f, Point, Click, Press, Scale ) || Busy;
    Top += 32.0f * Scale;
    static const char* Sorts[ ] = { "FOV", "Distance", "Combine" };
    Busy = DrawDrop( Left, Top, Wide, "Priority", "aim.sort", Sorts, 3, Aim.sort, Point, Click, Scale ) || Busy;
    Top += Font->LineSpan + 38.0f * Scale;
    Busy = DrawDropBits( Left, Top, Wide, "Target", "aim.bone", Bones, 6, Aim.bones, Point, Click, Scale ) || Busy;
    Top += Font->LineSpan + 38.0f * Scale;
    Busy = DrawBind( Left, Top, "Aim key", "aim.listen", Aim.key, Aim.listen, Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawAimbot( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, float Ease ) {
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
    float GeneralBody = Fit.general;
    float TargetBody = Fit.target;

    CRectangle Body;
    float OpenGen = 0.0f;
    float OpenTgt = 0.0f;
    float OpenMute = 0.0f;
    bool Busy = DrawFold( Left, Top, Wide, Head, GeneralBody, Round, Scale, "General", "fold.aim.general", Aim.general, Point, Click, Body, OpenGen );
    if ( OpenGen > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * OpenGen;
        Canvas->PushClip( CRectangle( Left, Top, Wide, Head + GeneralBody * OpenGen ) );
        Busy = DrawAimGeneral( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float Right = Left + Wide + Gap;
    Busy = DrawFold( Right, Top, Wide, Head, TargetBody, Round, Scale, "Targeting", "fold.aim.target", Aim.targeting, Point, Click, Body, OpenTgt ) || Busy;
    if ( OpenTgt > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * OpenTgt;
        Canvas->PushClip( CRectangle( Right, Top, Wide, Head + TargetBody * OpenTgt ) );
        Busy = DrawAimTarget( Body, Point, Click, Press, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    float LeftH = Head + GeneralBody * OpenGen;
    float RightH = Head + TargetBody * OpenTgt;
    float Stack = LeftH > RightH ? LeftH : RightH;
    float SilentTop = Top + Stack + Gap;
    float SilentWide = Wide * 2.0f + Gap;
    float SilentBody = Fit.silent;
    float SilentRoom = Content.Bottom( ) - SilentTop - Head - 8.0f * Scale;
    if ( SilentRoom < 0.0f )
        SilentRoom = 0.0f;
    if ( SilentBody > SilentRoom )
        SilentBody = SilentRoom;
    Canvas->PushClip( Content );
    Busy = DrawFold( Left, SilentTop, SilentWide, Head, SilentBody, Round, Scale, "Silent Aim", "fold.aim.silent", Mute.fold, Point, Click, Body, OpenMute ) || Busy;
    if ( OpenMute > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * OpenMute;
        Canvas->PushClip( CRectangle( Left, SilentTop, SilentWide, Head + SilentBody * OpenMute ) );
        Busy = DrawAimSilent( Body, Point, Click, Press, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }
    Canvas->PopClip( );

    Canvas->Opacity = Keep;
    return Busy;
}
