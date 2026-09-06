#pragma once

/**
 * @file aim.hpp
 * @brief Unlinked External - Target lead prediction, bone targeting, and aimbot sorting heuristics.
 */

#include "world.hpp"
#include <cmath>

namespace aim {

inline world::Vec3 PredictLead( const world::Vec3& Pos, const world::Vec3& Vel, float Dist, float LocalPing, float TargetPing, float BulletSpeed = 800.0f ) {
    float Speed = sqrtf( Vel.x * Vel.x + Vel.y * Vel.y + Vel.z * Vel.z );
    if ( Speed < 1.5f )
        return Pos;

    world::Vec3 ClampedVel = Vel;
    if ( Speed > 90.0f ) {
        ClampedVel.x *= 90.0f / Speed;
        ClampedVel.y *= 90.0f / Speed;
        ClampedVel.z *= 90.0f / Speed;
    }

    float Theirs = TargetPing > 0.0f ? TargetPing : LocalPing;
    float Ping = ( LocalPing + Theirs ) * 0.5f;
    if ( Ping > 0.25f )
        Ping = 0.25f;

    if ( Dist < 1.0f )
        Dist = 1.0f;
    if ( BulletSpeed < 100.0f )
        BulletSpeed = 800.0f;

    float Time = ( Ping * 0.5f + Dist / BulletSpeed ) * 0.80f;
    if ( Time > 0.45f )
        Time = 0.45f;

    world::Vec3 Out = Pos;
    Out.x += ClampedVel.x * Time;
    Out.y += ClampedVel.y * Time * 0.25f;
    Out.z += ClampedVel.z * Time;
    return Out;
}

inline world::Vec3 PredictLeadSilent( const world::Vec3& Pos, const world::Vec3& Vel, float Dist, float LocalPing, float TargetPing ) {
    float Speed = sqrtf( Vel.x * Vel.x + Vel.y * Vel.y + Vel.z * Vel.z );
    if ( Speed < 1.5f )
        return Pos;

    world::Vec3 ClampedVel = Vel;
    if ( Speed > 90.0f ) {
        ClampedVel.x *= 90.0f / Speed;
        ClampedVel.y *= 90.0f / Speed;
        ClampedVel.z *= 90.0f / Speed;
    }

    float Ping = LocalPing;
    if ( TargetPing > 0.0f )
        Ping = ( Ping + TargetPing ) * 0.5f;
    if ( Ping > 0.25f )
        Ping = 0.25f;

    if ( Dist < 1.0f )
        Dist = 1.0f;

    float Time = Ping * 0.25f + Dist / 1200.0f;
    if ( Time > 0.18f )
        Time = 0.18f;

    world::Vec3 Out = Pos;
    Out.x += ClampedVel.x * Time;
    Out.y += ClampedVel.y * Time * 0.25f;
    Out.z += ClampedVel.z * Time;
    return Out;
}

inline world::Vec3 SelectBone( const world::Actor& Item, int Bones ) {
    static const int Slot[ 6 ] = {
        world::BoneHead, world::BoneUpper, world::BoneUpper,
        world::BoneLower, world::BoneRoot, world::BoneLLegU
    };
    world::Vec3 Pos = Item.head;
    if ( Bones & 1 ) {
        Pos = Item.head;
        if ( Item.high.y > Item.head.y )
            Pos.y += ( Item.high.y - Item.head.y ) * 0.35f;
    } else {
        for ( int Index = 0; Index < 6; Index++ ) {
            if ( ( Bones & ( 1 << Index ) ) == 0 )
                continue;
            int Bone = Slot[ Index ];
            if ( Item.boneOk[ Bone ] ) {
                Pos = Item.world[ Bone ];
                if ( Index == 1 && Item.boneOk[ world::BoneHead ] ) {
                    Pos.x = ( Item.head.x + Item.world[ Bone ].x ) * 0.5f;
                    Pos.y = ( Item.head.y + Item.world[ Bone ].y ) * 0.5f;
                    Pos.z = ( Item.head.z + Item.world[ Bone ].z ) * 0.5f;
                }
                break;
            }
        }
    }
    return Pos;
}

inline world::Vec3 AimPoint( const world::Actor& Item, bool UsePred, int Bones, bool AllowPred, float LocalPing ) {
    world::Vec3 Pos = SelectBone( Item, Bones );
    if ( !UsePred || !AllowPred )
        return Pos;
    return PredictLead( Pos, Item.vel, Item.dist, LocalPing, Item.ping, 800.0f );
}

inline world::Vec3 SilentBone( const world::Actor& Item, int Bones ) {
    static const int Slot[ 6 ] = {
        world::BoneHead, world::BoneUpper, world::BoneUpper,
        world::BoneLower, world::BoneRoot, world::BoneLLegU
    };
    if ( Bones & 1 ) {
        world::Vec3 Pos = Item.boneOk[ world::BoneHead ] ? Item.world[ world::BoneHead ] : Item.head;
        if ( Item.high.y > Pos.y )
            Pos.y += ( Item.high.y - Pos.y ) * 0.28f;
        return Pos;
    }
    for ( int Index = 1; Index < 6; Index++ ) {
        if ( ( Bones & ( 1 << Index ) ) == 0 )
            continue;
        int Bone = Slot[ Index ];
        if ( Item.boneOk[ Bone ] )
            return Item.world[ Bone ];
    }
    if ( Item.boneOk[ world::BoneHead ] )
        return Item.world[ world::BoneHead ];
    return Item.head;
}

inline float ScoreTarget( float ScreenDist, float FovLimit, float WorldDist, float MaxWorldDist, int SortMode ) {
    if ( ScreenDist > FovLimit )
        return 1.0e9f;
    if ( SortMode <= 0 )
        return ScreenDist;
    if ( SortMode == 1 )
        return WorldDist;
    float Sn = FovLimit > 1.0f ? ScreenDist / FovLimit : ScreenDist;
    float Dn = MaxWorldDist > 1.0f ? WorldDist / MaxWorldDist : WorldDist;
    return Sn * 0.5f + Dn * 0.5f;
}

inline float ComputeAimRadius( float Wide, float Tall, float Scale, float Fov ) {
    float Half = sqrtf( Wide * Wide + Tall * Tall ) * 0.5f;
    if ( Scale <= 0.0f )
        Scale = 1.0f;
    if ( Fov >= 359.0f )
        return Half * Scale;
    if ( Fov < 0.0f )
        Fov = 0.0f;
    return Half * ( Fov / 360.0f ) * Scale;
}

}
