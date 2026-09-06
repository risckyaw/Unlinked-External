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

TEST_CASE( "Aim: ClampAimDt boundary enforcement" ) {
    CHECK_EQ( aim::ClampAimDt( 0.00010f ), 0.00025f );
    CHECK_EQ( aim::ClampAimDt( 0.00025f ), 0.00025f );
    CHECK_EQ( aim::ClampAimDt( 0.01666f ), 0.01666f );
    CHECK_EQ( aim::ClampAimDt( 0.05000f ), 0.05000f );
    CHECK_EQ( aim::ClampAimDt( 0.10000f ), 0.05000f );
}

TEST_CASE( "Aim: Deadzone and target validity checks" ) {
    // Within deadzone (default threshold squared = 4.0f)
    CHECK( aim::IsWithinDeadzone( 1.0f, 1.0f ) );       // 1 + 1 = 2 < 4
    CHECK( aim::IsWithinDeadzone( 0.0f, 1.9f ) );       // 3.61 < 4
    CHECK( !aim::IsWithinDeadzone( 2.0f, 0.0f ) );      // 4 >= 4
    CHECK( !aim::IsWithinDeadzone( 5.0f, 5.0f ) );

    // IsTargetValid checks
    // Teammate filtering
    CHECK( !aim::IsTargetValid( true, true, true, false ) );   // mate + filterTeam -> invalid
    CHECK( aim::IsTargetValid( true, true, false, false ) );    // mate without filterTeam -> valid
    CHECK( aim::IsTargetValid( false, true, true, false ) );   // enemy + filterTeam -> valid

    // Visibility filtering
    CHECK( !aim::IsTargetValid( false, false, false, true ) );  // invisible + filterVis -> invalid
    CHECK( aim::IsTargetValid( false, false, false, false ) );  // invisible without filterVis -> valid
    CHECK( aim::IsTargetValid( false, true, false, true ) );   // visible + filterVis -> valid
}

TEST_CASE( "Aim: ComputeSmoothParams piecewise tiers" ) {
    // T = 0% (Smooth = 0.0f)
    aim::SmoothParams P0 = aim::ComputeSmoothParams( 0.0f );
    CHECK_EQ( P0.tau, 0.012f );
    CHECK_EQ( P0.capPx, 18000.0f );

    // T = 5% (Smooth = 5.0f)
    aim::SmoothParams P5 = aim::ComputeSmoothParams( 5.0f );
    CHECK( fabsf( P5.tau - 0.040f ) < 0.001f );
    CHECK( fabsf( P5.capPx - 14000.0f ) < 1.0f );

    // T = 50% (Smooth = 50.0f)
    aim::SmoothParams P50 = aim::ComputeSmoothParams( 50.0f );
    CHECK( fabsf( P50.tau - 0.40f ) < 0.001f );
    CHECK( fabsf( P50.capPx - 1400.0f ) < 1.0f );

    // T = 100% (Smooth = 100.0f)
    aim::SmoothParams P100 = aim::ComputeSmoothParams( 100.0f );
    CHECK( fabsf( P100.tau - 2.40f ) < 0.001f );
    CHECK( fabsf( P100.capPx - 80.0f ) < 1.0f );

    // Clamping on negative and overflow
    aim::SmoothParams PNeg = aim::ComputeSmoothParams( -25.0f );
    CHECK_EQ( PNeg.tau, P0.tau );
    CHECK_EQ( PNeg.capPx, P0.capPx );

    aim::SmoothParams POver = aim::ComputeSmoothParams( 200.0f );
    CHECK_EQ( POver.tau, P100.tau );
    CHECK_EQ( POver.capPx, P100.capPx );
}

TEST_CASE( "Aim: ComputeSmoothMouseStep deadzone behavior" ) {
    float RestX = 0.0f;
    float RestY = 0.0f;
    aim::MouseStep Step = aim::ComputeSmoothMouseStep( 1.0f, 1.0f, 20.0f, 0.016f, RestX, RestY );
    CHECK( !Step.moved );
    CHECK_EQ( Step.moveX, 0 );
    CHECK_EQ( Step.moveY, 0 );
    CHECK_EQ( RestX, 0.0f );
    CHECK_EQ( RestY, 0.0f );
}

TEST_CASE( "Aim: ComputeSmoothMouseStep instant snap mode" ) {
    // Smooth < 0.5 (T < 0.005) is instant snap with 22px cap
    float RestX = 0.0f;
    float RestY = 0.0f;
    // Step exceeding 22.0f cap
    aim::MouseStep Step = aim::ComputeSmoothMouseStep( 100.0f, 0.0f, 0.0f, 0.016f, RestX, RestY );
    CHECK( Step.moved );
    CHECK_EQ( Step.moveX, 22 );
    CHECK_EQ( Step.moveY, 0 );
    CHECK( fabsf( RestX ) < 0.01f );

    // Step within 22.0f cap
    RestX = 0.0f;
    RestY = 0.0f;
    aim::MouseStep StepSmall = aim::ComputeSmoothMouseStep( 15.0f, 0.0f, 0.0f, 0.016f, RestX, RestY );
    CHECK( StepSmall.moved );
    CHECK_EQ( StepSmall.moveX, 15 );
    CHECK_EQ( StepSmall.moveY, 0 );
}

TEST_CASE( "Aim: ComputeSmoothMouseStep subpixel integration" ) {
    float RestX = 0.0f;
    float RestY = 0.0f;

    // Small step where fractional movement accumulates into RestX/RestY
    aim::MouseStep S1 = aim::ComputeSmoothMouseStep( 10.0f, 0.0f, 50.0f, 0.001f, RestX, RestY );
    // At T=0.50, Tau=0.40, Dt=0.001, Alpha ~ 1 - exp(-0.001/0.40) ~ 0.002497
    // Dx * Alpha ~ 0.025. Rounded integer MoveX is 0, so RestX retains ~0.025
    if ( !S1.moved ) {
        CHECK_EQ( S1.moveX, 0 );
        CHECK( RestX > 0.0f );
    }

    // Now test a standard delta with realistic frame time (60 FPS, Dt = 0.0166f, Smooth = 20.0f)
    RestX = 0.0f;
    RestY = 0.0f;
    aim::MouseStep S2 = aim::ComputeSmoothMouseStep( 80.0f, -60.0f, 20.0f, 0.0166f, RestX, RestY );
    CHECK( S2.moved );
    CHECK( S2.moveX > 0 );
    CHECK( S2.moveY < 0 );
    // Remainder should be strictly within (-0.5f, 0.5f)
    CHECK( RestX > -0.5f && RestX < 0.5f );
    CHECK( RestY > -0.5f && RestY < 0.5f );
}

TEST_CASE( "Aim: ComputeFovPulse bounds and oscillation" ) {
    for ( double Time = 0.0; Time < 10.0; Time += 0.25 ) {
        float Pulse = aim::ComputeFovPulse( Time );
        // Pulse formula: 0.7f + 0.3f * [0, 1] => range is [0.70f, 1.00f]
        CHECK( Pulse >= 0.699f && Pulse <= 1.001f );
    }
}

TEST_CASE( "Aim: ComputeFarDistance snapshot scanning and fallback" ) {
    struct MockActor { float dist; };

    // Empty list -> returns MinFar default (1.0f)
    CHECK_CLOSE( aim::ComputeFarDistance<MockActor>( nullptr, 0 ), 1.0f, 0.001f );
    CHECK_CLOSE( aim::ComputeFarDistance<MockActor>( nullptr, 0, 5.0f ), 5.0f, 0.001f );

    // Single actor smaller than floor
    MockActor SingleSmall[] = { { 0.5f } };
    CHECK_CLOSE( aim::ComputeFarDistance( SingleSmall, 1 ), 1.0f, 0.001f );

    // Single actor larger than floor
    MockActor SingleLarge[] = { { 150.0f } };
    CHECK_CLOSE( aim::ComputeFarDistance( SingleLarge, 1 ), 150.0f, 0.001f );

    // Multiple actors
    MockActor List[] = { { 45.0f }, { 210.5f }, { 88.0f }, { 12.0f } };
    CHECK_CLOSE( aim::ComputeFarDistance( List, 4 ), 210.5f, 0.001f );
}

TEST_CASE( "Aim: ComputeScreenDistance 2D Euclidean distance" ) {
    // Center exactly on aim point
    CHECK_CLOSE( aim::ComputeScreenDistance( 960.0f, 540.0f, 960.0f, 540.0f ), 0.0f, 0.001f );

    // Horizontal offset
    CHECK_CLOSE( aim::ComputeScreenDistance( 1060.0f, 540.0f, 960.0f, 540.0f ), 100.0f, 0.001f );
    CHECK_CLOSE( aim::ComputeScreenDistance( 860.0f, 540.0f, 960.0f, 540.0f ), 100.0f, 0.001f );

    // Vertical offset
    CHECK_CLOSE( aim::ComputeScreenDistance( 960.0f, 600.0f, 960.0f, 540.0f ), 60.0f, 0.001f );

    // Diagonal 3-4-5 triangle
    CHECK_CLOSE( aim::ComputeScreenDistance( 963.0f, 544.0f, 960.0f, 540.0f ), 5.0f, 0.001f );
}

TEST_CASE( "Aim: IsBetterTarget scoring and threshold rejection" ) {
    // Normal case: lower score is better
    CHECK( aim::IsBetterTarget( 50.0f, 100.0f ) );
    CHECK( !aim::IsBetterTarget( 150.0f, 100.0f ) );
    CHECK( !aim::IsBetterTarget( 100.0f, 100.0f ) ); // Equal is not better

    // Scores >= threshold (1.0e9f) are rejected
    CHECK( !aim::IsBetterTarget( 1.0e9f, 2.0e9f ) );
    CHECK( !aim::IsBetterTarget( 1.5e9f, 2.0e9f ) );

    // Custom threshold
    CHECK( aim::IsBetterTarget( 40.0f, 80.0f, 50.0f ) );
    CHECK( !aim::IsBetterTarget( 60.0f, 80.0f, 50.0f ) );
}

TEST_CASE( "Aim: IsPointInViewBounds viewport bounds and margin checking" ) {
    float W = 1920.0f, H = 1080.0f;

    // Inside view
    CHECK( aim::IsPointInViewBounds( 960.0f, 540.0f, W, H ) );
    CHECK( aim::IsPointInViewBounds( 0.0f, 0.0f, W, H ) );
    CHECK( aim::IsPointInViewBounds( W, H, W, H ) );

    // Within 48px margin
    CHECK( aim::IsPointInViewBounds( -30.0f, 540.0f, W, H, 48.0f ) );
    CHECK( aim::IsPointInViewBounds( W + 40.0f, 540.0f, W, H, 48.0f ) );
    CHECK( aim::IsPointInViewBounds( 960.0f, -40.0f, W, H, 48.0f ) );
    CHECK( aim::IsPointInViewBounds( 960.0f, H + 40.0f, W, H, 48.0f ) );

    // Outside margin
    CHECK( !aim::IsPointInViewBounds( -50.0f, 540.0f, W, H, 48.0f ) );
    CHECK( !aim::IsPointInViewBounds( W + 50.0f, 540.0f, W, H, 48.0f ) );
    CHECK( !aim::IsPointInViewBounds( 960.0f, -50.0f, W, H, 48.0f ) );
    CHECK( !aim::IsPointInViewBounds( 960.0f, H + 50.0f, W, H, 48.0f ) );

    // Undersized viewport (< 8px)
    CHECK( !aim::IsPointInViewBounds( 2.0f, 2.0f, 6.0f, 1080.0f ) );
    CHECK( !aim::IsPointInViewBounds( 2.0f, 2.0f, 1920.0f, 4.0f ) );
}


