#pragma once

/**
 * @file ice.hpp
 * @brief Unlinked External - Process memory protection helpers and frozen state management.
 */

#include <chrono>
#include <cstdio>

namespace ice {

inline void ComputeIceSeedParams( unsigned long long Tick, float& ShiftX, float& ShiftY, float& Twist, float& Grain ) noexcept {
    ShiftX = ( float )( Tick % 997ull ) * 0.041f;
    ShiftY = ( float )( ( Tick / 997ull ) % 991ull ) * 0.037f;
    Twist = ( float )( Tick % 628ull ) * 0.01f;
    Grain = 1.7f + ( float )( Tick % 80ull ) * 0.01f;
}

[[nodiscard]] inline bool FormatIceMetalShader( float ShiftX, float ShiftY, float Twist, float Grain, const float Deep[ 3 ], const float Mid[ 3 ], const float High[ 3 ], char* Out, size_t Cap ) noexcept {
    if ( !Deep || !Mid || !High || !Out || Cap == 0 )
        return false;
    int Res = snprintf( Out, Cap,
        "Float2 Uv = Screen * Float2( 0.0026, 0.0115 ) + Float2( %.4f, %.4f );\n"
        "float Time = Moment * 0.055;\n"
        "Float2 P = Uv;\n"
        "P.y += Time * 0.09;\n"
        "P.x += 0.05 * sin( P.y * 2.1 + Time * 0.28 + %.4f );\n"
        "float Rib = sin( P.x * 6.8 + 0.35 * sin( P.y * 1.6 ) );\n"
        "float Groove = pow( Saturate( 1.0 - abs( Rib ) ), 0.62 );\n"
        "float Pore = sin( P.x * 4.6 + 0.9 ) * sin( P.y * 5.4 - Time * 0.16 );\n"
        "float Hollow = pow( Saturate( 0.18 - Pore ), 1.6 );\n"
        "float Soft = 0.5 + 0.5 * sin( P.y * 1.7 + P.x * 1.1 + Time * 0.2 );\n"
        "float Speck = Fract( sin( dot( Uv, Float2( 127.1, 311.7 ) ) + %.4f ) * 43758.5453 );\n"
        "float Warp = 0.5 + 0.5 * sin( Uv.x * %.3f + Uv.y * 3.3 + Speck * 6.283 );\n"
        "float Mix = Saturate( Groove * 0.46 + Soft * 0.20 + Warp * 0.10 - Hollow * 0.38 - Speck * 0.08 );\n"
        "float Rim = pow( Groove, 3.6 );\n"
        "Float3 Deep = Float3( %.4f, %.4f, %.4f );\n"
        "Float3 Mid = Float3( %.4f, %.4f, %.4f );\n"
        "Float3 High = Float3( %.4f, %.4f, %.4f );\n"
        "Float3 Tint = Lerp( Deep, Mid, Mix );\n"
        "Tint = Lerp( Tint, High, Rim * 0.55 );\n"
        "Final.rgb = Saturate( Tint );\n",
        ( double )ShiftX, ( double )ShiftY, ( double )Twist, ( double )Twist, ( double )Grain,
        ( double )Deep[ 0 ], ( double )Deep[ 1 ], ( double )Deep[ 2 ],
        ( double )Mid[ 0 ], ( double )Mid[ 1 ], ( double )Mid[ 2 ],
        ( double )High[ 0 ], ( double )High[ 1 ], ( double )High[ 2 ] );
    return Res > 0 && ( size_t )Res < Cap;
}

}

inline const char* IceMetal( float Dr, float Dg, float Db, float Mr, float Mg, float Mb, float Hr, float Hg, float Hb ) {
    static char Body[ 2800 ] = { };
    static float ShiftX = 0.0f;
    static float ShiftY = 0.0f;
    static float Twist = 0.0f;
    static float Grain = 1.7f;
    static bool Seeded = false;
    if ( !Seeded ) {
        unsigned long long Tick = ( unsigned long long )std::chrono::high_resolution_clock::now( ).time_since_epoch( ).count( );
        ice::ComputeIceSeedParams( Tick, ShiftX, ShiftY, Twist, Grain );
        Seeded = true;
    }
    const float Deep[ 3 ] = { Dr, Dg, Db };
    const float Mid[ 3 ] = { Mr, Mg, Mb };
    const float High[ 3 ] = { Hr, Hg, Hb };
    ( void )ice::FormatIceMetalShader( ShiftX, ShiftY, Twist, Grain, Deep, Mid, High, Body, sizeof( Body ) );
    return Body;
}
