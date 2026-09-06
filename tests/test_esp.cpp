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

TEST_CASE( "ESP: Palette tints and index clamping (ClampTintIndex, GetEspTint)" ) {
    CHECK_EQ( esp::EspTintCount, 13 );

    // Normal indices
    CHECK_EQ( esp::ClampTintIndex( 0 ), 0 );
    CHECK_EQ( esp::ClampTintIndex( 12 ), 12 );

    // Out of bounds indices fallback to 3
    CHECK_EQ( esp::ClampTintIndex( -1 ), 3 );
    CHECK_EQ( esp::ClampTintIndex( 13 ), 3 );
    CHECK_EQ( esp::ClampTintIndex( 99, 5 ), 5 );

    // RGB values of key palette entries
    esp::RgbColor Green = esp::GetEspTint( 0 );
    CHECK_EQ( ( int )Green.r, 72 );
    CHECK_EQ( ( int )Green.g, 220 );
    CHECK_EQ( ( int )Green.b, 118 );

    esp::RgbColor Blue = esp::GetEspTint( 3 );
    CHECK_EQ( ( int )Blue.r, 64 );
    CHECK_EQ( ( int )Blue.g, 132 );
    CHECK_EQ( ( int )Blue.b, 255 );

    esp::RgbColor Dark = esp::GetEspTint( 12 );
    CHECK_EQ( ( int )Dark.r, 18 );
    CHECK_EQ( ( int )Dark.g, 18 );
    CHECK_EQ( ( int )Dark.b, 22 );
}

TEST_CASE( "ESP: Feature tint resolution with Coat defaults (PickFeatTint)" ) {
    esp::Coat Dye;
    // Default visible: FeatBox -> 3, FeatName -> 9, FeatHealth -> 0
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatBox, true ), 3 );
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatName, true ), 9 );
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatHealth, true ), 0 );

    // Default hidden: FeatBox -> 12, FeatHealth -> 11
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatBox, false ), 12 );
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatHealth, false ), 11 );

    // Out-of-bounds feat returns fallback
    CHECK_EQ( esp::PickFeatTint( Dye, -1, true, 4 ), 4 );
    CHECK_EQ( esp::PickFeatTint( Dye, esp::FeatCount, true, 7 ), 7 );
}

TEST_CASE( "ESP: ComputeHealthBar rail and fill layout" ) {
    float RailLeft = 0.0f, RailTop = 0.0f, RailW = 0.0f, RailH = 0.0f;
    float FillTop = 0.0f, FillH = 0.0f;

    // Full health (Ratio = 1.0f)
    esp::ComputeHealthBar( 100.0f, 50.0f, 200.0f, 1.0f, 1.0f,
                          RailLeft, RailTop, RailW, RailH, FillTop, FillH, 3.0f, 6.0f );
    CHECK_CLOSE( RailLeft, 94.0f, 0.001f ); // 100 - 6
    CHECK_CLOSE( RailTop, 50.0f, 0.001f );
    CHECK_CLOSE( RailW, 3.0f, 0.001f );
    CHECK_CLOSE( RailH, 200.0f, 0.001f );
    CHECK_CLOSE( FillTop, 50.0f, 0.001f );
    CHECK_CLOSE( FillH, 200.0f, 0.001f );

    // Half health (Ratio = 0.5f) -> fill starts at 50 + 200 - 100 = 150
    esp::ComputeHealthBar( 100.0f, 50.0f, 200.0f, 1.0f, 0.5f,
                          RailLeft, RailTop, RailW, RailH, FillTop, FillH, 3.0f, 6.0f );
    CHECK_CLOSE( FillTop, 150.0f, 0.001f );
    CHECK_CLOSE( FillH, 100.0f, 0.001f );

    // Zero health (Ratio = 0.0f) -> fill starts at bottom with 0 height
    esp::ComputeHealthBar( 100.0f, 50.0f, 200.0f, 1.0f, 0.0f,
                          RailLeft, RailTop, RailW, RailH, FillTop, FillH, 3.0f, 6.0f );
    CHECK_CLOSE( FillTop, 250.0f, 0.001f );
    CHECK_CLOSE( FillH, 0.0f, 0.001f );
}

TEST_CASE( "ESP: Text label centering (Top and Bottom)" ) {
    float X = 0.0f, Y = 0.0f;

    // Top text (Name): BoxLeft 100, BoxTop 50, BoxWidth 60, TextW 40, TextH 14, Scale 1, Gap 3
    esp::ComputeTopCenteredText( 100.0f, 50.0f, 60.0f, 40.0f, 14.0f, 1.0f, X, Y, 3.0f );
    CHECK_CLOSE( X, 110.0f, 0.001f ); // 100 + (60 - 40) / 2
    CHECK_CLOSE( Y, 33.0f, 0.001f );  // 50 - 14 - 3

    // Bottom text (Dist): BoxLeft 100, BoxBottom 250, BoxWidth 60, TextW 30, Scale 1, Gap 3
    esp::ComputeBottomCenteredText( 100.0f, 250.0f, 60.0f, 30.0f, 1.0f, X, Y, 3.0f );
    CHECK_CLOSE( X, 115.0f, 0.001f ); // 100 + (60 - 30) / 2
    CHECK_CLOSE( Y, 253.0f, 0.001f ); // 250 + 3
}

TEST_CASE( "ESP: Snapline target calculation" ) {
    float TargetX = 0.0f, TargetY = 0.0f;
    esp::ComputeSnaplineTarget( 100.0f, 300.0f, 80.0f, TargetX, TargetY );
    CHECK_CLOSE( TargetX, 140.0f, 0.001f ); // 100 + 40
    CHECK_CLOSE( TargetY, 300.0f, 0.001f );
}

TEST_CASE( "ESP: ComputePartExtents 3D bounding point expansion" ) {
    struct SimpleVec { float x; float y; float z; };
    SimpleVec Pos{ 10.0f, 20.0f, 30.0f };
    SimpleVec Size{ 2.0f, 4.0f, 2.0f };
    SimpleVec Hi{ }, Lo{ }, Right{ }, Left{ };

    esp::ComputePartExtents( Pos, Size, Hi, Lo, Right, Left );

    // Hi: y = 20 + 2 + 0.15 = 22.15, x = 10 + 1 = 11
    CHECK_CLOSE( Hi.y, 22.15f, 0.001f );
    CHECK_CLOSE( Hi.x, 11.0f, 0.001f );

    // Lo: y = 20 - 2 = 18, x = 10 - 1 = 9
    CHECK_CLOSE( Lo.y, 18.0f, 0.001f );
    CHECK_CLOSE( Lo.x, 9.0f, 0.001f );

    // Right: x = 11, z = 31
    CHECK_CLOSE( Right.x, 11.0f, 0.001f );
    CHECK_CLOSE( Right.z, 31.0f, 0.001f );

    // Left: x = 9, z = 29
    CHECK_CLOSE( Left.x, 9.0f, 0.001f );
    CHECK_CLOSE( Left.z, 29.0f, 0.001f );
}
