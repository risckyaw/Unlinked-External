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
