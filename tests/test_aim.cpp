#include "test_framework.hpp"
#include "aim.hpp"

TEST_CASE( "Aim: PredictLead stationary and velocity threshold" ) {
    world::Vec3 Pos{ 100.0f, 50.0f, 200.0f };
    world::Vec3 SlowVel{ 0.5f, 0.0f, 0.5f }; // Speed ~0.707f < 1.5f

    world::Vec3 Result = aim::PredictLead( Pos, SlowVel, 50.0f, 0.03f, 0.03f );
    CHECK_EQ( Result.x, Pos.x );
    CHECK_EQ( Result.y, Pos.y );
    CHECK_EQ( Result.z, Pos.z );
}

TEST_CASE( "Aim: PredictLead moving target and velocity clamping" ) {
    world::Vec3 Pos{ 0.0f, 10.0f, 0.0f };
    world::Vec3 FastVel{ 200.0f, 0.0f, 0.0f }; // Speed 200.0f > 90.0f, should be clamped to 90.0f

    world::Vec3 Result = aim::PredictLead( Pos, FastVel, 100.0f, 0.05f, 0.05f, 800.0f );
    // Speed clamped to 90.0f. ClampedVel.x = 90.0f
    // Ping = 0.05f
    // Time = ( 0.05f * 0.5f + 100.0f / 800.0f ) * 0.80f = ( 0.025f + 0.125f ) * 0.80f = 0.15f * 0.80f = 0.12f
    // Out.x = 0.0f + 90.0f * 0.12f = 10.8f
    CHECK( Result.x > 10.7f && Result.x < 10.9f );
    CHECK_EQ( Result.y, 10.0f );
    CHECK_EQ( Result.z, 0.0f );
}

TEST_CASE( "Aim: PredictLead time cap and vertical damping" ) {
    world::Vec3 Pos{ 0.0f, 0.0f, 0.0f };
    world::Vec3 Vel{ 0.0f, 40.0f, 0.0f }; // Only vertical velocity

    // Far distance and high ping to exceed 0.45s cap
    world::Vec3 Result = aim::PredictLead( Pos, Vel, 5000.0f, 1.0f, 1.0f, 800.0f );
    // Clamped ping = 0.25f. Time capped at 0.45f.
    // Vertical velocity is damped by 0.25f: Out.y = 0.0f + 40.0f * 0.45f * 0.25f = 4.5f
    CHECK( Result.y > 4.4f && Result.y < 4.6f );
    CHECK_EQ( Result.x, 0.0f );
    CHECK_EQ( Result.z, 0.0f );
}

TEST_CASE( "Aim: PredictLeadSilent threshold and time cap" ) {
    world::Vec3 Pos{ 10.0f, 20.0f, 30.0f };
    world::Vec3 Vel{ 0.0f, 0.0f, 50.0f };

    // Small distance and ping
    world::Vec3 Res1 = aim::PredictLeadSilent( Pos, Vel, 10.0f, 0.04f, 0.04f );
    // Ping = 0.04f. Time = 0.04f * 0.25f + 10.0f / 1200.0f = 0.01f + 0.008333f = 0.018333f
    // Out.z = 30.0f + 50.0f * 0.018333f = 30.9166f
    CHECK( Res1.z > 30.8f && Res1.z < 31.0f );

    // Huge distance and ping capping time at 0.18f
    world::Vec3 Res2 = aim::PredictLeadSilent( Pos, Vel, 5000.0f, 0.5f, 0.5f );
    // Clamped ping = 0.25f, Time capped at 0.18f. Out.z = 30.0f + 50.0f * 0.18f = 39.0f
    CHECK( Res2.z > 38.9f && Res2.z < 39.1f );
}

TEST_CASE( "Aim: SelectBone head and torso priority" ) {
    world::Actor Actor{};
    Actor.head = world::Vec3{ 10.0f, 100.0f, 20.0f };
    Actor.high = world::Vec3{ 10.0f, 110.0f, 20.0f }; // high.y > head.y by 10.0f

    // Bones & 1 (Head selected) with high.y adjustment
    world::Vec3 HeadPt = aim::SelectBone( Actor, 1 );
    // Expected y = 100.0f + 10.0f * 0.35f = 103.5f
    CHECK_EQ( HeadPt.x, 10.0f );
    CHECK( HeadPt.y > 103.4f && HeadPt.y < 103.6f );
    CHECK_EQ( HeadPt.z, 20.0f );

    // Torso selection (Bones = 2): Slot[1] = BoneUpper
    Actor.boneOk[ world::BoneUpper ] = true;
    Actor.world[ world::BoneUpper ] = world::Vec3{ 10.0f, 80.0f, 20.0f };
    Actor.boneOk[ world::BoneHead ] = true;

    world::Vec3 TorsoPt = aim::SelectBone( Actor, 2 );
    // Index 1 with BoneHead OK averages Head and BoneUpper: (100 + 80) * 0.5 = 90.0f
    CHECK_EQ( TorsoPt.x, 10.0f );
    CHECK_EQ( TorsoPt.y, 90.0f );
    CHECK_EQ( TorsoPt.z, 20.0f );
}

TEST_CASE( "Aim: SilentBone selection logic" ) {
    world::Actor Actor{};
    Actor.head = world::Vec3{ 5.0f, 50.0f, 5.0f };
    Actor.high = world::Vec3{ 5.0f, 60.0f, 5.0f };
    Actor.boneOk[ world::BoneHead ] = true;
    Actor.world[ world::BoneHead ] = world::Vec3{ 5.0f, 50.0f, 5.0f };

    // Bones & 1: 0.28f high.y adjustment
    world::Vec3 HeadPt = aim::SilentBone( Actor, 1 );
    CHECK( HeadPt.y > 52.7f && HeadPt.y < 52.9f ); // 50 + 10 * 0.28 = 52.8f

    // Bones = 4: Slot[2] = BoneUpper
    Actor.boneOk[ world::BoneUpper ] = true;
    Actor.world[ world::BoneUpper ] = world::Vec3{ 5.0f, 40.0f, 5.0f };
    world::Vec3 TorsoPt = aim::SilentBone( Actor, 4 );
    CHECK_EQ( TorsoPt.y, 40.0f );
}

TEST_CASE( "Aim: ScoreTarget heuristics" ) {
    float FovLimit = 150.0f;
    float FarDist = 300.0f;

    // Target outside FOV limit returns 1.0e9f
    CHECK_EQ( aim::ScoreTarget( 160.0f, FovLimit, 50.0f, FarDist, 0 ), 1.0e9f );
    CHECK_EQ( aim::ScoreTarget( 160.0f, FovLimit, 50.0f, FarDist, 1 ), 1.0e9f );
    CHECK_EQ( aim::ScoreTarget( 160.0f, FovLimit, 50.0f, FarDist, 2 ), 1.0e9f );

    // Target inside FOV:
    // Sort <= 0: returns ScreenDist
    CHECK_EQ( aim::ScoreTarget( 45.0f, FovLimit, 100.0f, FarDist, 0 ), 45.0f );

    // Sort == 1: returns WorldDist
    CHECK_EQ( aim::ScoreTarget( 45.0f, FovLimit, 100.0f, FarDist, 1 ), 100.0f );

    // Sort == 2 (Balanced): Sn * 0.5f + Dn * 0.5f
    // Sn = 45 / 150 = 0.30f
    // Dn = 100 / 300 = 0.333333f
    // Score = 0.15f + 0.166667f = 0.316667f
    float Balanced = aim::ScoreTarget( 45.0f, FovLimit, 100.0f, FarDist, 2 );
    CHECK( Balanced > 0.31f && Balanced < 0.32f );
}
