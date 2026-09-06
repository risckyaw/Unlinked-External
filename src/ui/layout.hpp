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

}
