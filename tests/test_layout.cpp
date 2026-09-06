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

TEST_CASE( "Layout: ComputeDockOffset right placement and overflow flip" ) {
    // Fits on right: MenuWidth 500, Gap 16 -> Dock = 516
    float RightDock = ui::ComputeDockOffset( 50.0f, 500.0f, 300.0f, 16.0f, 1920.0f, 8.0f );
    CHECK_CLOSE( RightDock, 516.0f, 0.001f );

    // Overflows right: 1400 + 516 + 300 = 2216 > 1912 -> flips to left: -300 - 16 = -316
    float LeftDock = ui::ComputeDockOffset( 1400.0f, 500.0f, 300.0f, 16.0f, 1920.0f, 8.0f );
    CHECK_CLOSE( LeftDock, -316.0f, 0.001f );
}

TEST_CASE( "Layout: ComputePanelChrome header and pane dimensions" ) {
    ui::RectBounds Header, Pane;
    ui::ComputePanelChrome( 100.0f, 100.0f, 400.0f, 500.0f, 1.0f, 52.0f, 10.0f, Header, Pane );
    CHECK_CLOSE( Header.left, 100.0f, 0.001f );
    CHECK_CLOSE( Header.top, 100.0f, 0.001f );
    CHECK_CLOSE( Header.width, 400.0f, 0.001f );
    CHECK_CLOSE( Header.height, 52.0f, 0.001f );

    CHECK_CLOSE( Pane.left, 110.0f, 0.001f );
    CHECK_CLOSE( Pane.top, 162.0f, 0.001f );
    CHECK_CLOSE( Pane.width, 380.0f, 0.001f );
    CHECK_CLOSE( Pane.height, 428.0f, 0.001f );

    // Scaled test: Scale = 1.5
    ui::ComputePanelChrome( 100.0f, 100.0f, 400.0f, 500.0f, 1.5f, 52.0f, 10.0f, Header, Pane );
    CHECK_CLOSE( Header.height, 78.0f, 0.001f );
    CHECK_CLOSE( Pane.left, 115.0f, 0.001f );
    CHECK_CLOSE( Pane.top, 193.0f, 0.001f );
    CHECK_CLOSE( Pane.width, 370.0f, 0.001f );
    CHECK_CLOSE( Pane.height, 392.0f, 0.001f );
}

TEST_CASE( "Layout: UpdateDragState lifecycle transitions" ) {
    float OriginX = 100.0f;
    float OriginY = 100.0f;
    float GrabX = 0.0f;
    float GrabY = 0.0f;
    bool Held = false;

    // 1. Initial click inside bounds: start dragging
    ui::UpdateDragState( true, true, 120.0f, 130.0f, OriginX, OriginY, GrabX, GrabY, Held );
    CHECK( Held );
    CHECK_CLOSE( GrabX, 20.0f, 0.001f );
    CHECK_CLOSE( GrabY, 30.0f, 0.001f );
    CHECK_CLOSE( OriginX, 100.0f, 0.001f );
    CHECK_CLOSE( OriginY, 100.0f, 0.001f );

    // 2. Drag motion: update origin
    ui::UpdateDragState( true, false, 220.0f, 250.0f, OriginX, OriginY, GrabX, GrabY, Held );
    CHECK( Held );
    CHECK_CLOSE( OriginX, 200.0f, 0.001f );
    CHECK_CLOSE( OriginY, 220.0f, 0.001f );

    // 3. Mouse released: stop dragging
    ui::UpdateDragState( false, false, 220.0f, 250.0f, OriginX, OriginY, GrabX, GrabY, Held );
    CHECK( !Held );
    CHECK_CLOSE( OriginX, 200.0f, 0.001f );
    CHECK_CLOSE( OriginY, 220.0f, 0.001f );

    // 4. Click outside: should not start dragging
    ui::UpdateDragState( true, false, 300.0f, 300.0f, OriginX, OriginY, GrabX, GrabY, Held );
    CHECK( !Held );
    CHECK_CLOSE( OriginX, 200.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeBadgeSize padding and scale" ) {
    float Wide = 0.0f;
    float Tall = 0.0f;

    // Scale 1.0f with default PadX 12, PadY 7
    ui::ComputeBadgeSize( 100.0f, 20.0f, 1.0f, Wide, Tall );
    CHECK_CLOSE( Wide, 124.0f, 0.001f );
    CHECK_CLOSE( Tall, 34.0f, 0.001f );

    // Scale 1.5f: PadX becomes 18, PadY becomes 10.5 -> 2*PadX=36, 2*PadY=21
    ui::ComputeBadgeSize( 100.0f, 20.0f, 1.5f, Wide, Tall );
    CHECK_CLOSE( Wide, 136.0f, 0.001f );
    CHECK_CLOSE( Tall, 41.0f, 0.001f );
}


