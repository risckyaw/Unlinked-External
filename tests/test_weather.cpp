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

TEST_CASE( "Weather: NextRand pure PRNG determinism and sequence" ) {
    uint32_t SeedA = 12345678;
    uint32_t SeedB = 12345678;

    for ( int i = 0; i < 50; ++i ) {
        float ValA = weather::NextRand( SeedA );
        float ValB = weather::NextRand( SeedB );
        CHECK_EQ( ValA, ValB );
        CHECK( ValA >= 0.0f && ValA <= 1.0f );
    }
}

TEST_CASE( "Weather: TargetDropCount by mode" ) {
    CHECK_EQ( weather::TargetDropCount( weather::Snow ), 80 );
    CHECK_EQ( weather::TargetDropCount( weather::Rain ), 70 );
    CHECK_EQ( weather::TargetDropCount( weather::Off ), 0 );
    CHECK_EQ( weather::TargetDropCount( weather::Storm ), 0 );
    CHECK_EQ( weather::TargetDropCount( -1 ), 0 );
    CHECK_EQ( weather::TargetDropCount( 99 ), 0 );
}

TEST_CASE( "Weather: ComputeSnowDrift bounds and periodicity" ) {
    for ( float Y = 0.0f; Y < 1000.0f; Y += 25.0f ) {
        float Drift = weather::ComputeSnowDrift( Y, 2.0f );
        // sin range [-1, 1] multiplied by 18.0f => [-18.0f, 18.0f]
        CHECK( Drift >= -18.001f && Drift <= 18.001f );
    }
}

TEST_CASE( "Weather: IsDropOutOfBounds boundary logic" ) {
    float Wide = 1920.0f;
    float Tall = 1080.0f;

    // Inside screen
    CHECK( !weather::IsDropOutOfBounds( 500.0f, 500.0f, Wide, Tall ) );
    CHECK( !weather::IsDropOutOfBounds( 0.0f, 0.0f, Wide, Tall ) );
    CHECK( !weather::IsDropOutOfBounds( -20.0f, 500.0f, Wide, Tall ) ); // Margin is 30, so -20 is within margin
    CHECK( !weather::IsDropOutOfBounds( 1940.0f, 500.0f, Wide, Tall ) ); // Margin 30, so 1940 is within margin
    CHECK( !weather::IsDropOutOfBounds( 500.0f, -25.0f, Wide, Tall ) ); // Negative Y is above top edge, not out of bounds

    // Outside screen
    CHECK( weather::IsDropOutOfBounds( -35.0f, 500.0f, Wide, Tall ) );   // Beyond left margin (-30)
    CHECK( weather::IsDropOutOfBounds( 1955.0f, 500.0f, Wide, Tall ) );  // Beyond right margin (1920 + 30 = 1950)
    CHECK( weather::IsDropOutOfBounds( 500.0f, 1105.0f, Wide, Tall ) );  // Beyond bottom margin (1080 + 20 = 1100)
}

TEST_CASE( "Weather: UpdateDrop particle kinematic integration" ) {
    // Rain mode: linear velocity only
    weather::Drop RainDrop;
    RainDrop.x = 100.0f;
    RainDrop.y = 200.0f;
    RainDrop.vx = -40.0f;
    RainDrop.vy = 1000.0f;
    RainDrop.size = 10.0f;

    weather::UpdateDrop( RainDrop, weather::Rain, 0.016f );
    // x should be 100.0f + (-40.0f * 0.016f) = 100.0f - 0.64f = 99.36f
    CHECK( fabsf( RainDrop.x - 99.36f ) < 0.001f );
    // y should be 200.0f + (1000.0f * 0.016f) = 216.0f
    CHECK( fabsf( RainDrop.y - 216.0f ) < 0.001f );

    // Snow mode: incorporates horizontal sway drift
    weather::Drop SnowDrop;
    SnowDrop.x = 100.0f;
    SnowDrop.y = 200.0f;
    SnowDrop.vx = 0.0f;
    SnowDrop.vy = 50.0f;
    SnowDrop.size = 2.0f;

    weather::UpdateDrop( SnowDrop, weather::Snow, 0.016f );
    float ExpectedDrift = weather::ComputeSnowDrift( 200.8f, 2.0f );
    float ExpectedX = 100.0f + ExpectedDrift * 0.016f;
    CHECK( fabsf( SnowDrop.x - ExpectedX ) < 0.001f );
    CHECK( fabsf( SnowDrop.y - 200.8f ) < 0.001f );
}

TEST_CASE( "Weather: ComputeRainStreakEnd geometry calculation" ) {
    weather::Point2D End = weather::ComputeRainStreakEnd( 100.0f, 200.0f, -50.0f, 15.0f );
    // Expected x = 100.0f + (-50.0f * 0.018f) = 100.0f - 0.90f = 99.10f
    CHECK( fabsf( End.x - 99.10f ) < 0.001f );
    // Expected y = 200.0f + 15.0f = 215.0f
    CHECK_EQ( End.y, 215.0f );
}

