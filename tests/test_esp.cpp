#include "test_framework.hpp"
#include "esp.hpp"
#include <cstring>

TEST_CASE( "ESP: Skeleton topology link counts and endpoints" ) {
    int CountR15 = 0;
    const esp::BoneLink* LinksR15 = esp::GetSkeletonLinks( true, CountR15 );
    CHECK_EQ( CountR15, 14 );
    CHECK( LinksR15 != nullptr );
    // Head -> UpperTorso
    CHECK_EQ( LinksR15[ 0 ].from, 0 );
    CHECK_EQ( LinksR15[ 0 ].to, 2 );
    // UpperTorso -> LowerTorso
    CHECK_EQ( LinksR15[ 1 ].from, 2 );
    CHECK_EQ( LinksR15[ 1 ].to, 3 );
    // RightLeg terminal: RightLowerLeg -> RightFoot
    CHECK_EQ( LinksR15[ 13 ].from, 14 );
    CHECK_EQ( LinksR15[ 13 ].to, 15 );

    int CountR6 = 0;
    const esp::BoneLink* LinksR6 = esp::GetSkeletonLinks( false, CountR6 );
    CHECK_EQ( CountR6, 5 );
    CHECK( LinksR6 != nullptr );
    // Head -> Torso
    CHECK_EQ( LinksR6[ 0 ].from, 0 );
    CHECK_EQ( LinksR6[ 0 ].to, 2 );
    // Torso -> Right Leg
    CHECK_EQ( LinksR6[ 4 ].from, 2 );
    CHECK_EQ( LinksR6[ 4 ].to, 13 );
}

TEST_CASE( "ESP: BBox2D accumulation and boundary detection" ) {
    esp::BBox2D Box;
    CHECK( !Box.IsValid( ) );
    CHECK_EQ( Box.hits, 0 );

    // Single point is insufficient for a bounding box
    Box.Push( 50.0f, 60.0f );
    CHECK_EQ( Box.hits, 1 );
    CHECK( !Box.IsValid( ) );

    // Second point expands box
    Box.Push( 150.0f, 260.0f );
    CHECK_EQ( Box.hits, 2 );
    CHECK( Box.IsValid( ) );
    CHECK_CLOSE( Box.minX, 50.0f, 0.001f );
    CHECK_CLOSE( Box.maxX, 150.0f, 0.001f );
    CHECK_CLOSE( Box.minY, 60.0f, 0.001f );
    CHECK_CLOSE( Box.maxY, 260.0f, 0.001f );
    CHECK_CLOSE( Box.Width( ), 100.0f, 0.001f );
    CHECK_CLOSE( Box.Height( ), 200.0f, 0.001f );

    // Degenerate points (same location) should not satisfy min size
    esp::BBox2D Flat;
    Flat.Push( 10.0f, 10.0f );
    Flat.Push( 10.5f, 10.5f );
    CHECK( !Flat.IsValid( 2.0f, 2 ) );

    // Reset restores clean slate
    Box.Reset( );
    CHECK_EQ( Box.hits, 0 );
    CHECK( !Box.IsValid( ) );
}

TEST_CASE( "ESP: ComputeHealthRatio boundary clamping and division safety" ) {
    // Normal ratios
    CHECK_CLOSE( esp::ComputeHealthRatio( 100.0f, 100.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( esp::ComputeHealthRatio( 50.0f, 100.0f ), 0.5f, 0.001f );
    CHECK_CLOSE( esp::ComputeHealthRatio( 0.0f, 100.0f ), 0.0f, 0.001f );

    // Overheal clamp
    CHECK_CLOSE( esp::ComputeHealthRatio( 150.0f, 100.0f ), 1.0f, 0.001f );

    // Underflow clamp
    CHECK_CLOSE( esp::ComputeHealthRatio( -25.0f, 100.0f ), 0.0f, 0.001f );

    // Division by zero or negative maxHealth guard
    CHECK_CLOSE( esp::ComputeHealthRatio( 50.0f, 0.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( esp::ComputeHealthRatio( 50.0f, -100.0f ), 0.0f, 0.001f );
}

TEST_CASE( "ESP: FormatDistance buffer formatting and safeguards" ) {
    char Buffer[ 32 ] = { };
    CHECK( esp::FormatDistance( 45.2f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "45m" );

    CHECK( esp::FormatDistance( 120.9f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "121m" );

    CHECK( esp::FormatDistance( 0.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "0m" );

    // Negative distance clamps to 0m
    CHECK( esp::FormatDistance( -10.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "0m" );

    // Null or empty buffer guard
    CHECK( !esp::FormatDistance( 10.0f, nullptr, 32 ) );
    CHECK( !esp::FormatDistance( 10.0f, Buffer, 0 ) );
}
