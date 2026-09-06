#include "test_framework.hpp"
#include "move.hpp"

TEST_CASE( "Move: Want evaluation across toggle states" ) {
    move::Cfg& C = move::Live( );
    C.jump = false;
    C.infJump = false;
    C.noclip = false;
    CHECK_EQ( move::Want( ), false );

    C.jump = true;
    CHECK_EQ( move::Want( ), true );
    C.jump = false;

    C.infJump = true;
    CHECK_EQ( move::Want( ), true );
    C.infJump = false;

    C.noclip = true;
    CHECK_EQ( move::Want( ), true );
    C.noclip = false;
}

TEST_CASE( "Move: Jump power bounds clamping" ) {
    move::Cfg& C = move::Live( );

    C.jumpPower = -50.0f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 1.0f, 0.001f );

    C.jumpPower = 0.0f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 1.0f, 0.001f );

    C.jumpPower = 0.99f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 1.0f, 0.001f );

    C.jumpPower = 150.0f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 150.0f, 0.001f );

    C.jumpPower = 500.0f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 500.0f, 0.001f );

    C.jumpPower = 500.1f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 500.0f, 0.001f );

    C.jumpPower = 10000.0f;
    move::Clamp( );
    CHECK_CLOSE( C.jumpPower, 500.0f, 0.001f );
}

TEST_CASE( "Move: Runtime state defaults" ) {
    move::State& S = move::Run( );
    CHECK_EQ( S.jumpOn, false );
    CHECK_EQ( S.clipOn, false );
    CHECK_EQ( S.clipN, 0 );
    CHECK_CLOSE( S.savedJump, 50.0f, 0.001f );
    CHECK_CLOSE( S.savedJumpH, 7.2f, 0.001f );
}

TEST_CASE( "Move: ComputeJumpHeight calculates Roblox Humanoid height scale" ) {
    CHECK_CLOSE( move::ComputeJumpHeight( 50.0f ), 7.2f, 0.001f );
    CHECK_CLOSE( move::ComputeJumpHeight( 90.0f ), 12.96f, 0.001f );
    CHECK_CLOSE( move::ComputeJumpHeight( 0.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( move::ComputeJumpHeight( -10.0f ), -1.44f, 0.001f );
    CHECK_CLOSE( move::ComputeJumpHeight( 100.0f, 0.2f ), 20.0f, 0.001f );
}

TEST_CASE( "Move: ShouldExecuteInfJump evaluates continuous repeat and edge triggers" ) {
    // InfJump continuous repeat: triggers whenever space is down, regardless of edge
    CHECK_EQ( move::ShouldExecuteInfJump( true, false, true, true ), true );
    CHECK_EQ( move::ShouldExecuteInfJump( true, false, true, false ), true );
    CHECK_EQ( move::ShouldExecuteInfJump( true, true, true, false ), true );
    CHECK_EQ( move::ShouldExecuteInfJump( true, false, false, false ), false );
    CHECK_EQ( move::ShouldExecuteInfJump( true, true, false, false ), false );

    // Regular Jump (Once): triggers only on rising edge when InfJump is disabled
    CHECK_EQ( move::ShouldExecuteInfJump( false, true, true, true ), true );
    CHECK_EQ( move::ShouldExecuteInfJump( false, true, true, false ), false );
    CHECK_EQ( move::ShouldExecuteInfJump( false, true, false, false ), false );

    // Neither enabled: never executes
    CHECK_EQ( move::ShouldExecuteInfJump( false, false, true, true ), false );
    CHECK_EQ( move::ShouldExecuteInfJump( false, false, true, false ), false );
    CHECK_EQ( move::ShouldExecuteInfJump( false, false, false, false ), false );
}

TEST_CASE( "Move: ComputeEffectiveJumpPower handles custom jump vs default power" ) {
    CHECK_CLOSE( move::ComputeEffectiveJumpPower( true, 90.0f ), 90.0f, 0.001f );
    CHECK_CLOSE( move::ComputeEffectiveJumpPower( true, 120.0f ), 120.0f, 0.001f );
    CHECK_CLOSE( move::ComputeEffectiveJumpPower( false, 90.0f ), 50.0f, 0.001f );
    CHECK_CLOSE( move::ComputeEffectiveJumpPower( false, 120.0f, 45.0f ), 45.0f, 0.001f );
}

TEST_CASE( "Move: ComputeBoostedJumpVelocity boosts upward velocity without capping higher velocity" ) {
    // Boost from falling / negative velocity
    CHECK_CLOSE( move::ComputeBoostedJumpVelocity( -25.0f, 50.0f ), 50.0f, 0.001f );
    // Boost from low upward velocity
    CHECK_CLOSE( move::ComputeBoostedJumpVelocity( 15.0f, 50.0f ), 50.0f, 0.001f );
    // At exact power
    CHECK_CLOSE( move::ComputeBoostedJumpVelocity( 50.0f, 50.0f ), 50.0f, 0.001f );
    // Preserves existing higher upward velocity
    CHECK_CLOSE( move::ComputeBoostedJumpVelocity( 80.0f, 50.0f ), 80.0f, 0.001f );
}

TEST_CASE( "Move: ShouldRescanClip handles initial scan and throttle intervals" ) {
    // Must scan when clipping is not yet active
    CHECK_EQ( move::ShouldRescanClip( false, 0 ), true );
    CHECK_EQ( move::ShouldRescanClip( false, 500 ), true );

    // When clipping is already active, throttle at interval
    CHECK_EQ( move::ShouldRescanClip( true, 0 ), false );
    CHECK_EQ( move::ShouldRescanClip( true, 200 ), false );
    CHECK_EQ( move::ShouldRescanClip( true, 350 ), false );
    CHECK_EQ( move::ShouldRescanClip( true, 351 ), true );
    CHECK_EQ( move::ShouldRescanClip( true, 500 ), true );

    // Custom rescan interval
    CHECK_EQ( move::ShouldRescanClip( true, 400, 500 ), false );
    CHECK_EQ( move::ShouldRescanClip( true, 501, 500 ), true );
}

TEST_CASE( "Move: ClampJumpPower clamps within min and max bounds" ) {
    CHECK_CLOSE( move::ClampJumpPower( -100.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 0.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 0.99f ), 1.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 1.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 75.0f ), 75.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 500.0f ), 500.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 500.1f ), 500.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 2000.0f ), 500.0f, 0.001f );

    // Custom bounds
    CHECK_CLOSE( move::ClampJumpPower( 5.0f, 10.0f, 100.0f ), 10.0f, 0.001f );
    CHECK_CLOSE( move::ClampJumpPower( 150.0f, 10.0f, 100.0f ), 100.0f, 0.001f );
}

