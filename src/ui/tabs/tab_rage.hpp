#pragma once

/**
 * @file tab_rage.hpp
 * @brief Unlinked External - Rage tab rendering and physics controls.
 */

static bool DrawRageJump( const CRectangle& Body, const CVector& Point, bool Click, bool Press, float Scale ) {
    move::Cfg& Move = move::Live( );
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    float Top = Body.Top + 12.0f * Scale;
    bool Busy = false;
    CRectangle First( Left, Top, Wide, 32.0f * Scale );
    Busy = DrawSwitch( First, "Enabled", "move.jump", Move.jump, Point, Click, Scale ) || Busy;
    Busy = DrawSlider( Left, First.Bottom( ) + 6.0f * Scale, Wide, "Power", "move.jump.power", Move.jumpPower, 1.0f, 500.0f, Point, Click, Press, Scale ) || Busy;
    CRectangle Inf( Left, First.Bottom( ) + 38.0f * Scale, Wide, 32.0f * Scale );
    Busy = DrawSwitch( Inf, "Inf Jump", "move.infjump", Move.infJump, Point, Click, Scale ) || Busy;
    return Busy;
}

static bool DrawRageNoclip( const CRectangle& Body, const CVector& Point, bool Click, float Scale ) {
    move::Cfg& Move = move::Live( );
    float Left = Body.Left + 14.0f * Scale;
    float Wide = Body.Width - 28.0f * Scale;
    CRectangle First( Left, Body.Top + 12.0f * Scale, Wide, 32.0f * Scale );
    return DrawSwitch( First, "Enabled", "move.noclip", Move.noclip, Point, Click, Scale );
}

static bool DrawRage( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, float Ease ) {
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
    float Right = Left + Wide + Gap;

    CRectangle Body;
    float OpenJump = 0.0f;
    float OpenClip = 0.0f;
    bool Busy = DrawFold( Left, Top, Wide, Head, Fit.rageJump, Round, Scale, "Jump", "fold.rage.jump", Rage.jump, Point, Click, Body, OpenJump );
    if ( OpenJump > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * OpenJump;
        Canvas->PushClip( CRectangle( Left, Top, Wide, Head + Fit.rageJump * OpenJump ) );
        Busy = DrawRageJump( Body, Point, Click, Press, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    Busy = DrawFold( Right, Top, Wide, Head, Fit.rageNoclip, Round, Scale, "Noclip", "fold.rage.noclip", Rage.noclip, Point, Click, Body, OpenClip ) || Busy;
    if ( OpenClip > 0.08f ) {
        float Amount = Canvas->Opacity;
        Canvas->Opacity = Amount * OpenClip;
        Canvas->PushClip( CRectangle( Right, Top, Wide, Head + Fit.rageNoclip * OpenClip ) );
        Busy = DrawRageNoclip( Body, Point, Click, Scale ) || Busy;
        Canvas->PopClip( );
        Canvas->Opacity = Amount;
    }

    Canvas->Opacity = Keep;
    return Busy;
}
