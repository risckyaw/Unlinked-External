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

}
