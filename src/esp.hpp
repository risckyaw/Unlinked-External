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

} // namespace esp
