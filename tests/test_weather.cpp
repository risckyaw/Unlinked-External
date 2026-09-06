#include "test_framework.hpp"
#include "weather.hpp"

TEST_CASE( "Weather: Mode names and boundary clamping" ) {
    CHECK_EQ( std::string( weather::Name( weather::Off ) ), "Off" );
    CHECK_EQ( std::string( weather::Name( weather::Snow ) ), "Snow" );
    CHECK_EQ( std::string( weather::Name( weather::Rain ) ), "Rain" );
    CHECK_EQ( std::string( weather::Name( weather::Storm ) ), "Thunder" );

    // Out of bounds clamping should fallback to "Off" (index 0)
    CHECK_EQ( std::string( weather::Name( -1 ) ), "Off" );
    CHECK_EQ( std::string( weather::Name( weather::ModeCount ) ), "Off" );
    CHECK_EQ( std::string( weather::Name( 100 ) ), "Off" );
}

TEST_CASE( "Weather: PRNG range bounds and determinism" ) {
    weather::Live( ).seed = 42;

    bool AllInRange = true;
    for ( int i = 0; i < 500; ++i ) {
        float Val = weather::Rand( );
        if ( Val < 0.0f || Val > 1.0f ) {
            AllInRange = false;
            break;
        }
    }
    CHECK( AllInRange );

    // Determinism test with fixed seed
    weather::Live( ).seed = 1337;
    float Seq1[ 5 ];
    for ( int i = 0; i < 5; ++i )
        Seq1[ i ] = weather::Rand( );

    weather::Live( ).seed = 1337;
    for ( int i = 0; i < 5; ++i ) {
        float Val = weather::Rand( );
        CHECK_EQ( Val, Seq1[ i ] );
    }
}

TEST_CASE( "Weather: Drop spawning characteristics" ) {
    weather::Drop DropFresh;
    weather::Spawn( DropFresh, weather::Rain, 1920.0f, 1080.0f, true );
    CHECK( DropFresh.x >= 0.0f && DropFresh.x <= 1920.0f );
    CHECK( DropFresh.y >= 0.0f && DropFresh.y <= 1080.0f );
    CHECK( DropFresh.vy >= 980.0f );
    CHECK_EQ( DropFresh.life, 1.0f );

    weather::Drop DropRespawn;
    weather::Spawn( DropRespawn, weather::Rain, 1920.0f, 1080.0f, false );
    CHECK( DropRespawn.y < 0.0f ); // Spawned above top edge
}

TEST_CASE( "Weather: Tick particle simulation counts" ) {
    weather::mode( ) = weather::Off;
    weather::Tick( 0.016f, 1920.0f, 1080.0f );
    CHECK_EQ( weather::Live( ).used, 0 );

    weather::mode( ) = weather::Snow;
    weather::Tick( 0.016f, 1920.0f, 1080.0f );
    CHECK_EQ( weather::Live( ).used, 80 );

    weather::mode( ) = weather::Rain;
    weather::Tick( 0.016f, 1920.0f, 1080.0f );
    CHECK_EQ( weather::Live( ).used, 70 );
}
