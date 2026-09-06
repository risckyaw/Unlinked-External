#include "test_framework.hpp"
#include "ui/layout.hpp"
#include "aim.hpp"
#include <cmath>

TEST_CASE( "Layout: ClampBox normal placement within screen bounds" ) {
    float X = 100.0f;
    float Y = 150.0f;
    ui::ClampBox( X, Y, 800.0f, 600.0f, 200.0f, 100.0f, 8.0f );
    CHECK_CLOSE( X, 100.0f, 0.001f );
    CHECK_CLOSE( Y, 150.0f, 0.001f );
}

TEST_CASE( "Layout: ClampBox clamps to left and top margin" ) {
    float X = -20.0f;
    float Y = 4.0f;
    ui::ClampBox( X, Y, 800.0f, 600.0f, 200.0f, 100.0f, 8.0f );
    CHECK_CLOSE( X, 8.0f, 0.001f );
    CHECK_CLOSE( Y, 8.0f, 0.001f );
}

TEST_CASE( "Layout: ClampBox clamps to right and bottom bounds" ) {
    float X = 700.0f;
    float Y = 550.0f;
    // Across = 800, Wide = 200, Margin = 8 -> MaxX = 800 - 200 - 8 = 592
    // Vertical = 600, Tall = 100, Margin = 8 -> MaxY = 600 - 100 - 8 = 492
    ui::ClampBox( X, Y, 800.0f, 600.0f, 200.0f, 100.0f, 8.0f );
    CHECK_CLOSE( X, 592.0f, 0.001f );
    CHECK_CLOSE( Y, 492.0f, 0.001f );
}

TEST_CASE( "Layout: ClampBox handles window smaller than box" ) {
    float X = 50.0f;
    float Y = 50.0f;
    // Across = 100, Wide = 200 -> MaxX guarded to Margin (8.0f)
    ui::ClampBox( X, Y, 100.0f, 50.0f, 200.0f, 100.0f, 8.0f );
    CHECK_CLOSE( X, 8.0f, 0.001f );
    CHECK_CLOSE( Y, 8.0f, 0.001f );
}

TEST_CASE( "Layout: ClampBox supports custom margin" ) {
    float X = 0.0f;
    float Y = 0.0f;
    ui::ClampBox( X, Y, 500.0f, 500.0f, 100.0f, 100.0f, 20.0f );
    CHECK_CLOSE( X, 20.0f, 0.001f );
    CHECK_CLOSE( Y, 20.0f, 0.001f );
}

TEST_CASE( "Layout: Swatch size and gap scaling" ) {
    CHECK_CLOSE( ui::SwatchSize( 1.0f ), 16.0f, 0.001f );
    CHECK_CLOSE( ui::SwatchSize( 1.5f ), 24.0f, 0.001f );
    CHECK_CLOSE( ui::SwatchGap( 1.0f ), 3.0f, 0.001f );
    CHECK_CLOSE( ui::SwatchGap( 2.0f ), 6.0f, 0.001f );
}

TEST_CASE( "Layout: SwatchColumns calculation" ) {
    // When width is abundant: 5 items with size 16, gap 3 -> need 5*16 + 4*3 = 92
    // Wide = 200 -> Columns should be 5
    CHECK_EQ( ui::SwatchColumns( 200.0f, 5, 1.0f ), 5 );

    // When width is constrained: Wide = 50
    // (50 + 3) / (16 + 3) = 53 / 19 = 2 columns
    CHECK_EQ( ui::SwatchColumns( 50.0f, 5, 1.0f ), 2 );

    // When width is tiny: should clamp to minimum 1 column
    CHECK_EQ( ui::SwatchColumns( 5.0f, 5, 1.0f ), 1 );
}

TEST_CASE( "Layout: SwatchTall calculation" ) {
    // 5 items fitting in 1 row: Size = 16
    CHECK_CLOSE( ui::SwatchTall( 200.0f, 5, 1.0f ), 16.0f, 0.001f );

    // 5 items split into 2 columns -> (5 + 2 - 1) / 2 = 3 rows
    // Height = 3 * 16 + 2 * 3 = 48 + 6 = 54
    CHECK_CLOSE( ui::SwatchTall( 50.0f, 5, 1.0f ), 54.0f, 0.001f );
}

TEST_CASE( "AimRadius: ComputeAimRadius standard field of view" ) {
    float Wide = 1920.0f;
    float Tall = 1080.0f;
    float Half = std::sqrt( Wide * Wide + Tall * Tall ) * 0.5f;

    // FOV 72 deg = 72 / 360 = 0.2
    float Radius = aim::ComputeAimRadius( Wide, Tall, 1.0f, 72.0f );
    CHECK_CLOSE( Radius, Half * 0.2f, 0.01f );
}

TEST_CASE( "AimRadius: ComputeAimRadius full 360 FOV clamp" ) {
    float Wide = 800.0f;
    float Tall = 600.0f;
    float Half = std::sqrt( Wide * Wide + Tall * Tall ) * 0.5f;

    float Radius359 = aim::ComputeAimRadius( Wide, Tall, 1.0f, 359.0f );
    CHECK_CLOSE( Radius359, Half, 0.01f );

    float Radius360 = aim::ComputeAimRadius( Wide, Tall, 1.0f, 360.0f );
    CHECK_CLOSE( Radius360, Half, 0.01f );
}

TEST_CASE( "AimRadius: ComputeAimRadius scale and edge cases" ) {
    float Wide = 1000.0f;
    float Tall = 1000.0f;
    float Half = std::sqrt( Wide * Wide + Tall * Tall ) * 0.5f;

    // Scale 0 or negative defaults to 1.0f
    float RadiusZeroScale = aim::ComputeAimRadius( Wide, Tall, 0.0f, 180.0f );
    CHECK_CLOSE( RadiusZeroScale, Half * 0.5f, 0.01f );

    // Negative FOV clamps to 0.0f
    float RadiusNegFov = aim::ComputeAimRadius( Wide, Tall, 1.0f, -10.0f );
    CHECK_CLOSE( RadiusNegFov, 0.0f, 0.001f );

    // Custom scale 1.5f
    float RadiusScale15 = aim::ComputeAimRadius( Wide, Tall, 1.5f, 360.0f );
    CHECK_CLOSE( RadiusScale15, Half * 1.5f, 0.01f );
}
