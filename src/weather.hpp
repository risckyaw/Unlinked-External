#pragma once

/**
 * @file weather.hpp
 * @brief Unlinked External - Menu background shader effects, particle snow simulation, and visual styling.
 */

#if __has_include("Shaders.h")
#include "Shaders.h"
#endif

#include <cmath>
#include <cstdint>

namespace weather {

inline constexpr int ModeCount = 4;
inline constexpr int DropMax = 280;

enum Mode : int {
    Off = 0,
    Snow = 1,
    Rain = 2,
    Storm = 3
};

struct Drop {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float size = 1.0f;
    float life = 1.0f;
};

struct State {
    int mode = Snow;
    Drop list[ DropMax ];
    int used = 0;
    float wide = 1920.0f;
    float tall = 1080.0f;
    unsigned thunder = 0;
    uint32_t seed = 1;
};

inline State& Live( ) {
    static State Store;
    return Store;
}

inline int& mode( ) {
    return Live( ).mode;
}

inline const char* Name( int Index ) {
    static const char* Names[ ModeCount ] = { "Off", "Snow", "Rain", "Thunder" };
    if ( Index < 0 || Index >= ModeCount )
        Index = 0;
    return Names[ Index ];
}

inline float NextRand( uint32_t& Seed ) {
    Seed = Seed * 1664525u + 1013904223u;
    return ( float )( Seed >> 8 ) * ( 1.0f / 16777215.0f );
}

inline float Rand( ) {
    return NextRand( Live( ).seed );
}

inline int TargetDropCount( int Mode ) {
    if ( Mode == Snow )
        return 80;
    if ( Mode == Rain )
        return 70;
    return 0;
}

inline float ComputeSnowDrift( float Y, float Size ) {
    return sinf( Y * 0.02f + Size ) * 18.0f;
}

inline bool IsDropOutOfBounds( float X, float Y, float Wide, float Tall, float Margin = 30.0f, float BottomMargin = 20.0f ) {
    return Y > Tall + BottomMargin || X < -Margin || X > Wide + Margin;
}

inline void UpdateDrop( Drop& Item, int Mode, float Dt ) {
    Item.x += Item.vx * Dt;
    Item.y += Item.vy * Dt;
    if ( Mode == Snow )
        Item.x += ComputeSnowDrift( Item.y, Item.size ) * Dt;
}

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
};

inline Point2D ComputeRainStreakEnd( float X, float Y, float Vx, float Size, float ScaleTime = 0.018f ) {
    return Point2D{ X + Vx * ScaleTime, Y + Size };
}

inline void Spawn( Drop& Item, int Kind, float Wide, float Tall, bool Fresh ) {
    Item.x = Rand( ) * Wide;
    Item.y = Fresh ? Rand( ) * Tall : -8.0f - Rand( ) * 40.0f;
    if ( Kind == Rain ) {
        Item.vx = -40.0f - Rand( ) * 30.0f;
        Item.vy = 980.0f + Rand( ) * 420.0f;
        Item.size = 8.0f + Rand( ) * 14.0f;
    } else {
        Item.vx = -18.0f + Rand( ) * 36.0f;
        Item.vy = 42.0f + Rand( ) * 55.0f;
        Item.size = 1.4f + Rand( ) * 2.2f;
        if ( Fresh )
            Item.y = Rand( ) * Tall;
    }
    Item.life = 1.0f;
}

inline void Tick( float Dt, float Wide, float Tall ) {
    State& S = Live( );
    S.wide = Wide;
    S.tall = Tall;
    if ( S.mode == Off || S.mode == Storm ) {
        S.used = 0;
        return;
    }

    int Want = TargetDropCount( S.mode );
    if ( S.used > Want )
        S.used = Want;
    while ( S.used < Want )
        Spawn( S.list[ S.used++ ], S.mode, Wide, Tall, true );

    for ( int Index = 0; Index < S.used; Index++ ) {
        Drop& Item = S.list[ Index ];
        UpdateDrop( Item, S.mode, Dt );
        if ( IsDropOutOfBounds( Item.x, Item.y, Wide, Tall ) )
            Spawn( Item, S.mode, Wide, Tall, false );
    }
}

inline unsigned ThunderFx( ) {
    State& S = Live( );
    if ( S.thunder )
        return S.thunder;
#if __has_include("Shaders.h")
    S.thunder = Shaders->Compose( "weather.pack.lightning2",
        "Float2 Uv = Local / max( Extent, Float2( 0.001, 0.001 ) );\n"
        "float N = 0.0;\n"
        "float Amp = 0.5;\n"
        "Float2 P = Uv * 2.5 + Float2( Moment * 0.18, 0.0 );\n"
        "for ( int I = 0; I < 8; I++ )\n"
        "{\n"
        "    Float2 Ip = floor( P );\n"
        "    Float2 Fp = Fract( P );\n"
        "    float Ha = Fract( sin( dot( Ip, Float2( 12.9898, 78.233 ) ) ) * 43758.5453 );\n"
        "    float Hb = Fract( sin( dot( Ip + Float2( 1.0, 0.0 ), Float2( 12.9898, 78.233 ) ) ) * 43758.5453 );\n"
        "    float Hc = Fract( sin( dot( Ip + Float2( 0.0, 1.0 ), Float2( 12.9898, 78.233 ) ) ) * 43758.5453 );\n"
        "    float Hd = Fract( sin( dot( Ip + Float2( 1.0, 1.0 ), Float2( 12.9898, 78.233 ) ) ) * 43758.5453 );\n"
        "    Float2 Smp = Fp * Fp * ( 3.0 - 2.0 * Fp );\n"
        "    N += Amp * Lerp( Lerp( Ha, Hb, Smp.x ), Lerp( Hc, Hd, Smp.x ), Smp.y );\n"
        "    float Rot = 0.45;\n"
        "    P = Float2( P.x * cos( Rot ) - P.y * sin( Rot ), P.x * sin( Rot ) + P.y * cos( Rot ) ) * 2.0;\n"
        "    Amp *= 0.5;\n"
        "}\n"
        "Float3 Col = Float3( 0.0, 0.0, 0.0 );\n"
        "for ( int Bolt = 0; Bolt < 4; Bolt++ )\n"
        "{\n"
        "    float Bi = float( Bolt );\n"
        "    float Seed = Fract( sin( Bi * 19.17 + 2.3 ) * 43758.5453 );\n"
        "    float Speed = 0.28 + Seed * 0.22;\n"
        "    float Tick = Moment * Speed + Seed * 4.1;\n"
        "    float Phase = Fract( Tick );\n"
        "    float Roll = Fract( sin( floor( Tick ) * 51.13 + Bi * 9.7 ) * 43758.5453 );\n"
        "    float LiveBolt = step( Lerp( 0.12, 0.58, Saturate( Bi * 0.34 ) ), Roll );\n"
        "    float Front = Lerp( -1.30, 1.40, Saturate( Phase / 0.17 ) );\n"
        "    float Reveal = smoothstep( Front + 0.12, Front - 0.03, Uv.y );\n"
        "    float Hold = 1.0 - smoothstep( 0.16, 0.40, Phase );\n"
        "    float Flick = 0.50 + 0.50 * Fract( sin( Moment * 21.0 + Seed * 8.0 ) * 43758.5453 );\n"
        "    float Lane = -0.78 + Bi * 0.52;\n"
        "    float Wander = ( Fract( sin( floor( Moment * 0.22 + Bi * 3.1 ) * 91.7 ) * 43758.5453 ) - 0.5 ) * 0.34;\n"
        "    float Shift = Lane + Wander;\n"
        "    float Twist = ( 2.0 * N - 1.0 ) * ( 0.20 + 0.12 * Seed ) + 0.07 * sin( Uv.y * 3.4 + Bi * 1.7 );\n"
        "    float Dist = abs( Uv.x - Shift + Twist );\n"
        "    float Core = pow( 0.016 / max( Dist, 0.0011 ), 1.28 );\n"
        "    float Halo = pow( 0.050 / max( Dist, 0.0022 ), 0.82 );\n"
        "    float Gain = Reveal * Hold * Flick * LiveBolt;\n"
        "    Col += Float3( 0.78, 0.88, 1.0 ) * Saturate( Core * 1.15 * Gain );\n"
        "    Col += Float3( 0.24, 0.38, 0.82 ) * Saturate( Halo * 0.28 * Gain );\n"
        "}\n"
        "float Glow = Saturate( max( Col.x, max( Col.y, Col.z ) ) );\n"
        "Final = Float4( Saturate( Col ), Glow );\n"
    );
#endif
    return S.thunder;
}

}
