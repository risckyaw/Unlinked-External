#pragma once

/**
 * @file layout.hpp
 * @brief Unlinked External - Pure UI layout geometry, bounding box clamping, and swatch grid calculations.
 */

#include <cmath>

namespace ui {

inline void ClampBox( float& X, float& Y, float Across, float Vertical, float Wide, float Tall, float Margin = 8.0f ) {
    float MaxX = Across - Wide - Margin;
    float MaxY = Vertical - Tall - Margin;
    if ( MaxX < Margin )
        MaxX = Margin;
    if ( MaxY < Margin )
        MaxY = Margin;

    if ( X < Margin )
        X = Margin;
    else if ( X > MaxX )
        X = MaxX;

    if ( Y < Margin )
        Y = Margin;
    else if ( Y > MaxY )
        Y = MaxY;
}

inline float SwatchSize( float Scale ) {
    return 16.0f * Scale;
}

inline float SwatchGap( float Scale ) {
    return 3.0f * Scale;
}

inline int SwatchColumns( float Wide, int Count, float Scale ) {
    float Size = SwatchSize( Scale );
    float Gap = SwatchGap( Scale );
    int Columns = Count;
    float Need = Size * ( float )Count + Gap * ( float )( Count - 1 );
    if ( Need > Wide )
        Columns = ( int )( ( Wide + Gap ) / ( Size + Gap ) );
    if ( Columns < 1 )
        Columns = 1;
    return Columns;
}

inline float SwatchTall( float Wide, int Count, float Scale ) {
    float Size = SwatchSize( Scale );
    float Gap = SwatchGap( Scale );
    int Columns = SwatchColumns( Wide, Count, Scale );
    int Rows = ( Count + Columns - 1 ) / Columns;
    if ( Rows < 1 )
        Rows = 1;
    return Size * ( float )Rows + Gap * ( float )( Rows - 1 );
}

struct RectBounds {
    float left = 0.0f;
    float top = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    float Right( ) const { return left + width; }
    float Bottom( ) const { return top + height; }
};

inline void ComputePanelChrome( float Left, float Top, float Width, float Height, float Scale, float HeaderHeight, float Pad,
                               RectBounds& OutHeader, RectBounds& OutPane ) {
    float Cap = HeaderHeight * Scale;
    float P = Pad * Scale;
    OutHeader.left = Left;
    OutHeader.top = Top;
    OutHeader.width = Width;
    OutHeader.height = Cap;

    OutPane.left = Left + P;
    OutPane.top = Top + Cap + P;
    OutPane.width = Width - P * 2.0f;
    OutPane.height = Height - Cap - P * 2.0f;
}

inline float ComputeDockOffset( float MenuX, float MenuWidth, float PanelWidth, float Gap, float ScreenAcross, float Margin = 8.0f ) {
    float Dock = MenuWidth + Gap;
    if ( MenuX + Dock + PanelWidth > ScreenAcross - Margin ) {
        Dock = -PanelWidth - Gap;
    }
    return Dock;
}

inline void UpdateDragState( bool Press, bool CanStart, float PointX, float PointY,
                             float& OriginX, float& OriginY, float& GrabX, float& GrabY, bool& Held ) {
    if ( Press && CanStart ) {
        Held = true;
        GrabX = PointX - OriginX;
        GrabY = PointY - OriginY;
    }
    if ( Held ) {
        if ( Press ) {
            OriginX = PointX - GrabX;
            OriginY = PointY - GrabY;
        } else {
            Held = false;
        }
    }
}

inline void ComputeBadgeSize( float TextW, float TextH, float Scale, float& OutW, float& OutH, float PadX = 12.0f, float PadY = 7.0f ) {
    OutW = TextW + PadX * 2.0f * Scale;
    OutH = TextH + PadY * 2.0f * Scale;
}

inline float ClampSliderValue( float Value, float Lo, float Hi ) {
    if ( Value < Lo )
        Value = Lo;
    if ( Value > Hi )
        Value = Hi;
    return ( float )( int )( Value + 0.5f );
}

inline float ComputeSliderRatio( float Value, float Lo, float Hi ) {
    if ( Hi <= Lo )
        return 0.0f;
    float Portion = ( Value - Lo ) / ( Hi - Lo );
    if ( Portion < 0.0f )
        return 0.0f;
    if ( Portion > 1.0f )
        return 1.0f;
    return Portion;
}

inline float ComputeSliderValueFromPoint( float PointX, float GrooveLeft, float GrooveWidth, float Lo, float Hi ) {
    if ( GrooveWidth <= 1.0f )
        return Lo;
    float Ratio = ( PointX - GrooveLeft ) / GrooveWidth;
    if ( Ratio < 0.0f )
        Ratio = 0.0f;
    if ( Ratio > 1.0f )
        Ratio = 1.0f;
    return ( float )( int )( Lo + Ratio * ( Hi - Lo ) + 0.5f );
}

inline float ComputeGrooveWidth( float Wide, float TextW, float ValueW, float Thumb, float Scale ) {
    float GrooveW = Wide - TextW - ValueW - Thumb - 18.0f * Scale;
    if ( GrooveW < 48.0f * Scale )
        GrooveW = 48.0f * Scale;
    return GrooveW;
}

inline RectBounds ComputeTabBounds( float RailLeft, float RailTop, float RailWidth, float TabHeight, float TabGap, float Scale, int Index ) {
    float Step = TabHeight * Scale + TabGap * Scale;
    return { RailLeft, RailTop + Step * ( float )Index, RailWidth, TabHeight * Scale };
}

inline RectBounds ComputeTabPlate( const RectBounds& Tab, bool Top, bool Bot, float Round ) {
    if ( Top && !Bot )
        return { Tab.left, Tab.top, Tab.width, Tab.height + Round };
    if ( Bot && !Top )
        return { Tab.left, Tab.top - Round, Tab.width, Tab.height + Round };
    return Tab;
}

inline RectBounds ComputeCloseBounds( float HeaderRight, float HeaderTop, float HeaderHeight, float Scale, float Size = 32.0f, float Margin = 8.0f ) {
    float S = Size * Scale;
    return { HeaderRight - S - Margin * Scale, HeaderTop + ( HeaderHeight - S ) * 0.5f, S, S };
}

inline void ComputeCaretTips( float AtX, float AtY, bool Down, float Scale, float OutX[ 3 ], float OutY[ 3 ] ) {
    float Span = 3.6f * Scale;
    if ( Down ) {
        OutX[ 0 ] = AtX - Span;         OutY[ 0 ] = AtY - Span * 0.45f;
        OutX[ 1 ] = AtX + Span;         OutY[ 1 ] = AtY - Span * 0.45f;
        OutX[ 2 ] = AtX;                OutY[ 2 ] = AtY + Span * 0.75f;
    } else {
        OutX[ 0 ] = AtX - Span * 0.35f; OutY[ 0 ] = AtY - Span;
        OutX[ 1 ] = AtX + Span * 0.8f;  OutY[ 1 ] = AtY;
        OutX[ 2 ] = AtX - Span * 0.35f; OutY[ 2 ] = AtY + Span;
    }
}

inline void ComputeSwitchTrack( float RowRight, float RowTop, float RowHeight, float Scale, RectBounds& OutTrack, float TrackW = 44.0f, float TrackH = 22.0f ) {
    float W = TrackW * Scale;
    float H = TrackH * Scale;
    OutTrack = { RowRight - W, RowTop + ( RowHeight - H ) * 0.5f, W, H };
}

inline void ComputeSwitchKnob( const RectBounds& Track, float Scale, float OnProgress, RectBounds& OutKnob, float KnobSize = 18.0f ) {
    float Knob = KnobSize * Scale;
    float Pad = 2.0f * Scale;
    float Travel = Track.width - Knob - Pad * 2.0f;
    OutKnob = { Track.left + Pad + Travel * OnProgress, Track.top + ( Track.height - Knob ) * 0.5f, Knob, Knob };
}

inline bool ComputeDropListBox( float FieldLeft, float FieldTop, float FieldBottom, float FieldWidth, int ItemCount, float ScreenHeight, float Scale, RectBounds& OutBounds, float ItemHeight = 26.0f, float PadY = 6.0f, float Gap = 4.0f, float Margin = 8.0f ) {
    if ( ItemCount <= 0 )
        return false;
    float Item = ItemHeight * Scale;
    float Tall = Item * ( float )ItemCount + PadY * Scale;
    float Top = FieldBottom + Gap * Scale;
    float Limit = ScreenHeight - Margin * Scale;
    if ( Top + Tall > Limit )
        Top = FieldTop - Gap * Scale - Tall;
    OutBounds = { FieldLeft, Top, FieldWidth, Tall };
    return true;
}

inline int ValidatePickIndex( int Pick, int Count ) {
    if ( Pick < 0 || Pick >= Count )
        return 0;
    return Pick;
}

inline RectBounds ComputeStackedRow( float Left, float Top, float Wide, float RowHeight, float Gap, int Index ) {
    float Step = RowHeight + Gap;
    return { Left, Top + Step * ( float )Index, Wide, RowHeight };
}

inline void ComputeSplitPair( float Left, float Top, float AvailableWidth, float Gap, float Height, RectBounds& OutLeft, RectBounds& OutRight ) {
    float Half = ( AvailableWidth - Gap ) * 0.5f;
    if ( Half < 0.0f )
        Half = 0.0f;
    OutLeft = { Left, Top, Half, Height };
    OutRight = { Left + Half + Gap, Top, Half, Height };
}

inline void ComputeCardContainer( float Left, float Top, float Wide, float HeaderHeight, float BodyHeight, RectBounds& OutCard, RectBounds& OutBar, RectBounds& OutBody ) {
    OutCard = { Left, Top, Wide, HeaderHeight + BodyHeight };
    OutBar = { Left, Top, Wide, HeaderHeight };
    OutBody = { Left, Top + HeaderHeight, Wide, BodyHeight };
}

inline void ComputeFoldCard( float Left, float Top, float Wide, float Head, float BodyNeed, float OpenProgress, RectBounds& OutCard, RectBounds& OutBar, RectBounds& OutBody ) {
    float ClampedOpen = OpenProgress < 0.0f ? 0.0f : ( OpenProgress > 1.0f ? 1.0f : OpenProgress );
    OutCard = { Left, Top, Wide, Head + BodyNeed * ClampedOpen };
    OutBar = { Left, Top, Wide, Head };
    OutBody = { Left, Top + Head, Wide, BodyNeed };
}

inline RectBounds ComputeFoldArrow( const RectBounds& Bar, float Scale, float MarkSize = 15.0f, float Margin = 14.0f ) {
    float Mark = MarkSize * Scale;
    return { ( Bar.left + Bar.width ) - Mark - Margin * Scale, Bar.top + ( Bar.height - Mark ) * 0.5f, Mark, Mark };
}

inline RectBounds ComputeCenteredBounds( float ScreenW, float ScreenH, float Width, float Height ) {
    return { ( ScreenW - Width ) * 0.5f, ( ScreenH - Height ) * 0.5f, Width, Height };
}

inline float ComputeModalTall( float HeadH, float Line, float StepGap, int StepCount, float AfterSteps, float ActH, float Pad, float Scale ) {
    float Tall = HeadH + 14.0f * Scale + Line + 6.0f * Scale + Line + 12.0f * Scale + Line + 10.0f * Scale;
    for ( int Index = 0; Index < StepCount; Index++ )
        Tall += Line + StepGap;
    Tall += AfterSteps + ActH + Pad;
    return Tall;
}

inline float EaseOutQuint( float T ) {
    if ( T <= 0.0f )
        return 0.0f;
    if ( T >= 1.0f )
        return 1.0f;
    float Remain = 1.0f - T;
    return 1.0f - Remain * Remain * Remain * Remain * Remain;
}

inline float ComputePageSlide( float PageIn, float Scale, float PageDir, float Distance = 36.0f ) {
    float Ease = EaseOutQuint( PageIn );
    return ( 1.0f - Ease ) * Distance * Scale * PageDir;
}

inline RectBounds ComputeCenteredIcon( const RectBounds& Container, float IconSize ) {
    return { Container.left + ( Container.width - IconSize ) * 0.5f,
             Container.top + ( Container.height - IconSize ) * 0.5f,
             IconSize, IconSize };
}

inline void ComputeTitleLayout( float HeaderLeft, float HeaderTop, float HeaderWidth, float HeaderHeight,
                                float TextWidth, float TextHeight, bool HasLogo, float Scale,
                                RectBounds& OutLogo, float& OutTextX, float& OutTextY ) {
    float LogoSize = 25.0f * Scale;
    float Gap = 8.0f * Scale;
    float Total = TextWidth + ( HasLogo ? LogoSize + Gap : 0.0f );
    float Left = HeaderLeft + ( HeaderWidth - Total ) * 0.5f;
    float Top = HeaderTop + ( HeaderHeight - TextHeight ) * 0.5f;
    OutLogo = { Left, HeaderTop + ( HeaderHeight - LogoSize ) * 0.5f, LogoSize, LogoSize };
    OutTextX = Left + ( HasLogo ? LogoSize + Gap : 0.0f );
    OutTextY = Top;
}

inline float UpdateTabSlide( float CurrentAt, int TargetTab, float DeltaTime, float Speed = 20.0f ) {
    float Want = ( float )TargetTab;
    float Step = Speed * DeltaTime;
    if ( Step > 1.0f )
        Step = 1.0f;
    return CurrentAt + ( Want - CurrentAt ) * Step;
}

inline float ComputeTabActiveWeight( float TabAt, int Index ) {
    float Dist = fabsf( TabAt - ( float )Index );
    return Dist < 1.0f ? 1.0f - Dist : 0.0f;
}

struct TabSwipeGeometry {
    RectBounds stack;
    RectBounds fill;
    bool capTop = false;
    bool capBot = false;
    float round = 0.0f;
};

inline TabSwipeGeometry ComputeTabSwipeGeometry( float RailLeft, float RailTop, float RailWidth,
                                                float TabHeight, float TabGap, float Scale,
                                                int TabCount, float TabAt ) {
    float Stride = TabHeight * Scale + TabGap * Scale;
    float Tall = TabHeight * Scale;
    float Round = 12.0f * Scale;
    RectBounds Stack{ RailLeft, RailTop, RailWidth, Stride * ( float )TabCount };
    RectBounds Fill{ RailLeft, RailTop + Stride * TabAt, RailWidth, Tall };
    bool Top = Fill.top <= Stack.top + 0.75f;
    bool Bot = ( Fill.top + Fill.height ) >= ( Stack.top + Stack.height ) - 0.75f;
    return TabSwipeGeometry{ Stack, Fill, Top, Bot, ( Top || Bot ) ? Round : 0.0f };
}

inline void ComputeTabItemGeometry( float TabLeft, float TabTop, float TabWidth,
                                   float TextWidth, float Scale,
                                   RectBounds& OutGlyph, float& OutLabelX, float& OutLabelY ) {
    float Mark = 24.0f * Scale;
    OutGlyph = { TabLeft + ( TabWidth - Mark ) * 0.5f, TabTop + 13.0f * Scale, Mark, Mark };
    OutLabelX = TabLeft + ( TabWidth - TextWidth ) * 0.5f;
    OutLabelY = ( OutGlyph.top + OutGlyph.height ) + 7.0f * Scale;
}

inline int ValidateBitmask( int Bits, int TotalCount, int DefaultBits = 1 ) {
    int Mask = ( 1 << TotalCount ) - 1;
    if ( ( Bits & Mask ) == 0 )
        return DefaultBits;
    return Bits;
}

inline int ToggleBitmaskOption( int CurrentBits, int Index, int TotalCount ) {
    int Mask = ( 1 << TotalCount ) - 1;
    int NewBits = CurrentBits ^ ( 1 << Index );
    if ( ( NewBits & Mask ) == 0 )
        NewBits = 1 << Index;
    return NewBits;
}

inline RectBounds ComputeGridItemBounds( float Left, float Top, float ItemW, float ItemH, float GapX, float GapY, int Columns, int Index ) {
    if ( Columns < 1 )
        Columns = 1;
    int Col = Index % Columns;
    int Row = Index / Columns;
    return { Left + ( ItemW + GapX ) * ( float )Col,
             Top + ( ItemH + GapY ) * ( float )Row,
             ItemW, ItemH };
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

inline PageFit ComputePageFit( float Scale, bool DrawFov, bool LimitFps ) {
    PageFit Fit;
    Fit.inset = 10.0f * Scale;
    Fit.gap = 8.0f * Scale;
    Fit.head = 36.0f * Scale;
    Fit.general = 200.0f * Scale;
    Fit.silent = 220.0f * Scale;
    Fit.target = ( 276.0f + ( DrawFov ? 32.0f : 0.0f ) ) * Scale;
    Fit.rageJump = 128.0f * Scale;
    Fit.rageNoclip = 58.0f * Scale;
    Fit.overlay = 232.0f * Scale;
    Fit.visual = 88.0f * Scale;
    Fit.theme = 228.0f * Scale;
    Fit.custom = 236.0f * Scale;
    Fit.misc = ( 138.0f + ( LimitFps ? 28.0f : 0.0f ) ) * Scale;
    Fit.game = 160.0f * Scale;
    Fit.setOverlay = 204.0f * Scale;
    return Fit;
}

inline float ComputeClampedScroll( float CurrentScroll, float WheelDelta, float ScrollStep, int ItemCount, float RowHeight, float ViewportHeight ) {
    float Need = ( float )ItemCount * RowHeight;
    float Most = Need - ViewportHeight;
    if ( Most < 0.0f )
        Most = 0.0f;
    float Scroll = CurrentScroll - WheelDelta * ScrollStep;
    if ( Scroll > Most )
        Scroll = Most;
    if ( Scroll < 0.0f )
        Scroll = 0.0f;
    return Scroll;
}

inline bool IsRowVisible( float RowTop, float RowHeight, float ViewportTop, float ViewportBottom ) {
    return ( RowTop + RowHeight > ViewportTop ) && ( RowTop < ViewportBottom );
}

inline RectBounds ComputeTreeRowBounds( float PaneLeft, float PaneTop, float PaneWidth, int RowIndex, float RowHeight, float Scroll ) {
    float Top = PaneTop + ( float )RowIndex * RowHeight - Scroll;
    return RectBounds{ PaneLeft, Top, PaneWidth, RowHeight };
}

struct TreeItemElements {
    RectBounds arm;
    RectBounds mark;
    RectBounds highlight;
    float caretCenterX = 0.0f;
    float caretCenterY = 0.0f;
    float textX = 0.0f;
    float textY = 0.0f;
};

inline TreeItemElements ComputeTreeItemElements( float PaneLeft, float PaneWidth, float LineTop, float RowH, int Depth, float Scale, float IconSize, float LineSpan ) {
    TreeItemElements E;
    float Indent = 12.0f * Scale;
    float ArmLeft = PaneLeft + 4.0f * Scale + Indent * ( float )Depth;
    float ArmWidth = 14.0f * Scale;
    E.arm = RectBounds{ ArmLeft, LineTop, ArmWidth, RowH };
    E.caretCenterX = ArmLeft + ArmWidth * 0.5f;
    E.caretCenterY = LineTop + RowH * 0.5f;

    float MarkLeft = ArmLeft + ArmWidth + 2.0f * Scale;
    float MarkTop = LineTop + ( RowH - IconSize ) * 0.5f;
    E.mark = RectBounds{ MarkLeft, MarkTop, IconSize, IconSize };

    E.textX = MarkLeft + IconSize + 6.0f * Scale;
    E.textY = LineTop + ( RowH - LineSpan ) * 0.5f;

    E.highlight = RectBounds{ PaneLeft + 2.0f * Scale, LineTop + 1.0f * Scale, PaneWidth - 4.0f * Scale, RowH - 2.0f * Scale };
    return E;
}

inline bool FormatTreeCaption( char* Out, size_t Cap, const char* Name, const char* Klass, int Extra, bool Open ) {
    if ( !Out || Cap == 0 )
        return false;
    const char* SafeName = ( Name && Name[ 0 ] ) ? Name : "";
    const char* SafeKlass = ( Klass && Klass[ 0 ] ) ? Klass : "";
    if ( Extra > 0 && Open )
        snprintf( Out, Cap, "%s [%s] +%d", SafeName, SafeKlass, Extra );
    else
        snprintf( Out, Cap, "%s [%s]", SafeName, SafeKlass );
    return true;
}

struct ExplorerLayout {
    RectBounds search;
    RectBounds treePane;
    RectBounds sidePane;
};

inline ExplorerLayout ComputeExplorerLayout( float BodyLeft, float BodyTop, float BodyWidth, float BodyHeight, float Scale ) {
    ExplorerLayout L;
    float SearchH = 28.0f * Scale;
    L.search = RectBounds{ BodyLeft, BodyTop, BodyWidth, SearchH };

    float Gap = 8.0f * Scale;
    float TreeW = BodyWidth * 0.56f;
    float SideW = BodyWidth - TreeW - Gap;
    float WorkTop = BodyTop + SearchH + 6.0f * Scale;
    float WorkH = ( BodyTop + BodyHeight ) - WorkTop;

    L.treePane = RectBounds{ BodyLeft, WorkTop, TreeW, WorkH };
    L.sidePane = RectBounds{ BodyLeft + TreeW + Gap, WorkTop, SideW, WorkH };
    return L;
}

}

