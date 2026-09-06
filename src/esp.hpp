#pragma once

/**
 * @file esp.hpp
 * @brief Unlinked External - Pure ESP calculation helpers, skeleton topologies, 2D bounding boxes, and health metrics.
 */

#include <cmath>
#include <cstdio>
#include <cstdint>

namespace esp {

struct BoneLink {
    int from;
    int to;
};

inline constexpr BoneLink R15Skeleton[ 14 ] = {
    { 0, 2 },   // Head -> UpperTorso
    { 2, 3 },   // UpperTorso -> LowerTorso
    { 2, 4 },   // UpperTorso -> LeftUpperArm
    { 4, 5 },   // LeftUpperArm -> LeftLowerArm
    { 5, 6 },   // LeftLowerArm -> LeftHand
    { 2, 7 },   // UpperTorso -> RightUpperArm
    { 7, 8 },   // RightUpperArm -> RightLowerArm
    { 8, 9 },   // RightLowerArm -> RightHand
    { 3, 10 },  // LowerTorso -> LeftUpperLeg
    { 10, 11 }, // LeftUpperLeg -> LeftLowerLeg
    { 11, 12 }, // LeftLowerLeg -> LeftFoot
    { 3, 13 },  // LowerTorso -> RightUpperLeg
    { 13, 14 }, // RightUpperLeg -> RightLowerLeg
    { 14, 15 }  // RightLowerLeg -> RightFoot
};

inline constexpr BoneLink R6Skeleton[ 5 ] = {
    { 0, 2 },   // Head -> Torso
    { 2, 4 },   // Torso -> Left Arm
    { 2, 7 },   // Torso -> Right Arm
    { 2, 10 },  // Torso -> Left Leg
    { 2, 13 }   // Torso -> Right Leg
};

inline const BoneLink* GetSkeletonLinks( bool R15, int& OutCount ) {
    if ( R15 ) {
        OutCount = 14;
        return R15Skeleton;
    }
    OutCount = 5;
    return R6Skeleton;
}

struct BBox2D {
    float minX = 1.0e9f;
    float maxX = -1.0e9f;
    float minY = 1.0e9f;
    float maxY = -1.0e9f;
    int hits = 0;

    void Reset( ) {
        minX = 1.0e9f;
        maxX = -1.0e9f;
        minY = 1.0e9f;
        maxY = -1.0e9f;
        hits = 0;
    }

    void Push( float X, float Y ) {
        if ( X < minX )
            minX = X;
        if ( X > maxX )
            maxX = X;
        if ( Y < minY )
            minY = Y;
        if ( Y > maxY )
            maxY = Y;
        hits++;
    }

    bool IsValid( float MinSize = 2.0f, int MinHits = 2 ) const {
        if ( hits < MinHits )
            return false;
        float W = Width( );
        float H = Height( );
        return W >= MinSize && H >= MinSize;
    }

    float Width( ) const {
        return maxX - minX;
    }

    float Height( ) const {
        return maxY - minY;
    }
};

inline float ComputeHealthRatio( float Health, float MaxHealth ) {
    if ( MaxHealth <= 0.0f || std::isnan( Health ) || std::isnan( MaxHealth ) )
        return 0.0f;
    float Ratio = Health / MaxHealth;
    if ( Ratio < 0.0f )
        return 0.0f;
    if ( Ratio > 1.0f )
        return 1.0f;
    return Ratio;
}

inline bool FormatDistance( float Dist, char* Out, size_t MaxLen ) {
    if ( !Out || MaxLen == 0 )
        return false;
    if ( Dist < 0.0f )
        Dist = 0.0f;
    int Written = snprintf( Out, MaxLen, "%.0fm", ( double )Dist );
    return Written > 0 && ( size_t )Written < MaxLen;
}

enum EspFeat {
    FeatBox = 0,
    FeatName,
    FeatHealth,
    FeatDist,
    FeatSkel,
    FeatSnap,
    FeatCount
};

struct Coat {
    int feat = 0;
    int vis[ FeatCount ] = { 3, 9, 0, 9, 3, 3 };
    int hid[ FeatCount ] = { 12, 12, 11, 12, 12, 12 };
    int globVis = 3;
    int globHid = 12;
};

struct RgbColor {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

inline constexpr int EspTintCount = 13;
inline constexpr RgbColor EspTintPalette[ EspTintCount ] = {
    { 72, 220, 118 },
    { 232, 72, 72 },
    { 64, 220, 230 },
    { 64, 132, 255 },
    { 168, 88, 255 },
    { 255, 96, 180 },
    { 255, 148, 48 },
    { 255, 220, 64 },
    { 160, 255, 64 },
    { 244, 244, 248 },
    { 232, 188, 72 },
    { 176, 24, 48 },
    { 18, 18, 22 }
};

inline int ClampTintIndex( int Index, int Fallback = 3 ) {
    if ( Index < 0 || Index >= EspTintCount )
        return Fallback;
    return Index;
}

inline RgbColor GetEspTint( int Index, int Fallback = 3 ) {
    return EspTintPalette[ ClampTintIndex( Index, Fallback ) ];
}

inline int PickFeatTint( const Coat& Dye, int Feat, bool Seen, int Fallback = 3 ) {
    if ( Feat < 0 || Feat >= FeatCount )
        return Fallback;
    int Pick = Seen ? Dye.vis[ Feat ] : Dye.hid[ Feat ];
    return ClampTintIndex( Pick, Fallback );
}

inline void ComputeHealthBar( float BoxLeft, float BoxTop, float BoxHeight, float Scale, float HealthRatio,
                              float& OutRailLeft, float& OutRailTop, float& OutRailW, float& OutRailH,
                              float& OutFillTop, float& OutFillH, float BarWidth = 3.0f, float Margin = 6.0f ) {
    float W = BarWidth * Scale;
    float Left = BoxLeft - Margin * Scale;
    OutRailLeft = Left;
    OutRailTop = BoxTop;
    OutRailW = W;
    OutRailH = BoxHeight;
    float FillHeight = BoxHeight * HealthRatio;
    OutFillTop = BoxTop + BoxHeight - FillHeight;
    OutFillH = FillHeight;
}

inline void ComputeTopCenteredText( float BoxLeft, float BoxTop, float BoxWidth, float TextWidth, float TextHeight, float Scale, float& OutX, float& OutY, float Gap = 3.0f ) {
    OutX = BoxLeft + ( BoxWidth - TextWidth ) * 0.5f;
    OutY = BoxTop - TextHeight - Gap * Scale;
}

inline void ComputeBottomCenteredText( float BoxLeft, float BoxBottom, float BoxWidth, float TextWidth, float Scale, float& OutX, float& OutY, float Gap = 3.0f ) {
    OutX = BoxLeft + ( BoxWidth - TextWidth ) * 0.5f;
    OutY = BoxBottom + Gap * Scale;
}

inline void ComputeSnaplineTarget( float BoxLeft, float BoxBottom, float BoxWidth, float& OutX, float& OutY ) {
    OutX = BoxLeft + BoxWidth * 0.5f;
    OutY = BoxBottom;
}

template< typename TVec3 >
inline void ComputePartExtents( const TVec3& Pos, const TVec3& Size,
                                TVec3& OutHi, TVec3& OutLo, TVec3& OutRight, TVec3& OutLeft ) {
    OutHi = Pos;
    OutLo = Pos;
    OutHi.y += Size.y * 0.5f + 0.15f;
    OutLo.y -= Size.y * 0.5f;
    OutHi.x += Size.x * 0.5f;
    OutLo.x -= Size.x * 0.5f;
    OutRight = Pos;
    OutRight.x += Size.x * 0.5f;
    OutRight.z += Size.z * 0.5f;
    OutLeft = Pos;
    OutLeft.x -= Size.x * 0.5f;
    OutLeft.z -= Size.z * 0.5f;
}

} // namespace esp
