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

inline float ClampAimDt( float Dt ) {
    if ( Dt < 0.00025f )
        return 0.00025f;
    if ( Dt > 0.05f )
        return 0.05f;
    return Dt;
}

inline bool IsWithinDeadzone( float Dx, float Dy, float DeadzoneSq = 4.0f ) {
    return ( Dx * Dx + Dy * Dy ) < DeadzoneSq;
}

inline bool IsTargetValid( bool IsMate, bool IsVis, bool FilterTeam, bool FilterVis ) {
    if ( FilterTeam && IsMate )
        return false;
    if ( FilterVis && !IsVis )
        return false;
    return true;
}

template< typename TActor >
inline float ComputeFarDistance( const TActor* Actors, int Count, float MinFar = 1.0f ) {
    float Far = MinFar;
    if ( !Actors || Count <= 0 )
        return Far;
    for ( int Index = 0; Index < Count; Index++ ) {
        if ( Actors[ Index ].dist > Far )
            Far = Actors[ Index ].dist;
    }
    return Far;
}

inline float ComputeScreenDistance( float PointX, float PointY, float MidX, float MidY ) {
    float Dx = PointX - MidX;
    float Dy = PointY - MidY;
    return sqrtf( Dx * Dx + Dy * Dy );
}

inline bool IsBetterTarget( float CandidateScore, float BestScore, float MaxThreshold = 1.0e9f ) {
    return CandidateScore < MaxThreshold && CandidateScore < BestScore;
}

inline bool IsPointInViewBounds( float X, float Y, float ViewWidth, float ViewHeight, float Margin = 48.0f, float MinDimension = 8.0f ) {
    if ( ViewWidth < MinDimension || ViewHeight < MinDimension )
        return false;
    if ( X < -Margin || Y < -Margin || X > ViewWidth + Margin || Y > ViewHeight + Margin )
        return false;
    return true;
}

struct SmoothParams {
    float tau = 0.035f;
    float capPx = 14000.0f;
};

inline SmoothParams ComputeSmoothParams( float SmoothPercent ) {
    float T = SmoothPercent / 100.0f;
    if ( T < 0.0f )
        T = 0.0f;
    if ( T > 1.0f )
        T = 1.0f;

    float Tau = 0.035f;
    float CapPx = 14000.0f;
    if ( T <= 0.05f ) {
        Tau = 0.012f + ( T / 0.05f ) * 0.028f;
        CapPx = 18000.0f - ( T / 0.05f ) * 4000.0f;
    } else if ( T <= 0.50f ) {
        float U = ( T - 0.05f ) / 0.45f;
        Tau = 0.040f + U * 0.36f;
        CapPx = 14000.0f - U * 12600.0f;
    } else {
        float U = ( T - 0.50f ) / 0.50f;
        Tau = 0.40f + U * 2.00f;
        CapPx = 1400.0f - U * 1320.0f;
    }
    if ( Tau < 0.008f )
        Tau = 0.008f;

    return SmoothParams{ Tau, CapPx };
}

struct MouseStep {
    int moveX = 0;
    int moveY = 0;
    bool moved = false;
};

inline MouseStep ComputeSmoothMouseStep( float Dx, float Dy, float SmoothPercent, float Dt, float& RestX, float& RestY ) {
    if ( IsWithinDeadzone( Dx, Dy ) )
        return MouseStep{ 0, 0, false };

    SmoothParams P = ComputeSmoothParams( SmoothPercent );
    float T = SmoothPercent / 100.0f;
    if ( T < 0.0f )
        T = 0.0f;
    if ( T > 1.0f )
        T = 1.0f;

    float Alpha = 1.0f - expf( -Dt / P.tau );
    if ( T < 0.005f ) {
        Alpha = 1.0f;
        float Cap = 22.0f;
        float Step = sqrtf( Dx * Dx + Dy * Dy );
        if ( Step > Cap ) {
            Dx *= Cap / Step;
            Dy *= Cap / Step;
        }
    } else {
        float Step = sqrtf( Dx * Dx + Dy * Dy ) * Alpha;
        float MaxStep = P.capPx * Dt;
        if ( MaxStep < 0.35f )
            MaxStep = 0.35f;
        if ( Step > MaxStep && Step > 0.001f ) {
            float ScaleStep = MaxStep / Step;
            Alpha *= ScaleStep;
        }
    }

    if ( Alpha < 0.0f )
        Alpha = 0.0f;
    if ( Alpha > 1.0f )
        Alpha = 1.0f;

    RestX += Dx * Alpha;
    RestY += Dy * Alpha;
    int MoveX = ( int )( RestX >= 0.0f ? RestX + 0.5f : RestX - 0.5f );
    int MoveY = ( int )( RestY >= 0.0f ? RestY + 0.5f : RestY - 0.5f );
    if ( MoveX == 0 && MoveY == 0 )
        return MouseStep{ 0, 0, false };

    RestX -= ( float )MoveX;
    RestY -= ( float )MoveY;
    return MouseStep{ MoveX, MoveY, true };
}

inline float ComputeFovPulse( double ElapsedTime ) {
    return 0.7f + 0.3f * ( 0.5f + 0.5f * sinf( ( float )ElapsedTime * 1.8f ) );
}

}
