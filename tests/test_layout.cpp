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

TEST_CASE( "Layout: Slider clamping, ratio, and value from point" ) {
    // Clamping
    CHECK_CLOSE( ui::ClampSliderValue( -10.0f, 0.0f, 100.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( ui::ClampSliderValue( 115.0f, 0.0f, 100.0f ), 100.0f, 0.001f );
    CHECK_CLOSE( ui::ClampSliderValue( 49.6f, 0.0f, 100.0f ), 50.0f, 0.001f );

    // Ratio
    CHECK_CLOSE( ui::ComputeSliderRatio( 50.0f, 0.0f, 100.0f ), 0.5f, 0.001f );
    CHECK_CLOSE( ui::ComputeSliderRatio( -10.0f, 0.0f, 100.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( ui::ComputeSliderRatio( 150.0f, 0.0f, 100.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( ui::ComputeSliderRatio( 50.0f, 50.0f, 50.0f ), 0.0f, 0.001f );

    // Value from Point
    float ValMid = ui::ComputeSliderValueFromPoint( 150.0f, 100.0f, 100.0f, 0.0f, 100.0f );
    CHECK_CLOSE( ValMid, 50.0f, 0.001f );

    float ValLeft = ui::ComputeSliderValueFromPoint( 50.0f, 100.0f, 100.0f, 0.0f, 100.0f );
    CHECK_CLOSE( ValLeft, 0.0f, 0.001f );

    float ValRight = ui::ComputeSliderValueFromPoint( 250.0f, 100.0f, 100.0f, 0.0f, 100.0f );
    CHECK_CLOSE( ValRight, 100.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeGrooveWidth calculation" ) {
    float W = ui::ComputeGrooveWidth( 300.0f, 50.0f, 20.0f, 14.0f, 1.0f );
    // 300 - 50 - 20 - 14 - 18 = 198
    CHECK_CLOSE( W, 198.0f, 0.001f );

    // Tiny width clamps to 48 * Scale
    float SmallW = ui::ComputeGrooveWidth( 50.0f, 50.0f, 20.0f, 14.0f, 1.0f );
    CHECK_CLOSE( SmallW, 48.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeTabBounds and ComputeTabPlate" ) {
    ui::RectBounds Tab = ui::ComputeTabBounds( 0.0f, 50.0f, 80.0f, 70.0f, 0.0f, 1.0f, 2 );
    CHECK_CLOSE( Tab.left, 0.0f, 0.001f );
    CHECK_CLOSE( Tab.top, 190.0f, 0.001f );
    CHECK_CLOSE( Tab.width, 80.0f, 0.001f );
    CHECK_CLOSE( Tab.height, 70.0f, 0.001f );

    // TabPlate Top only
    ui::RectBounds PlateTop = ui::ComputeTabPlate( Tab, true, false, 12.0f );
    CHECK_CLOSE( PlateTop.top, 190.0f, 0.001f );
    CHECK_CLOSE( PlateTop.height, 82.0f, 0.001f );

    // TabPlate Bot only
    ui::RectBounds PlateBot = ui::ComputeTabPlate( Tab, false, true, 12.0f );
    CHECK_CLOSE( PlateBot.top, 178.0f, 0.001f );
    CHECK_CLOSE( PlateBot.height, 82.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCloseBounds positioning and scale" ) {
    ui::RectBounds Close = ui::ComputeCloseBounds( 700.0f, 0.0f, 52.0f, 1.0f, 32.0f, 8.0f );
    CHECK_CLOSE( Close.left, 660.0f, 0.001f );
    CHECK_CLOSE( Close.top, 10.0f, 0.001f );
    CHECK_CLOSE( Close.width, 32.0f, 0.001f );
    CHECK_CLOSE( Close.height, 32.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCaretTips downward and rightward" ) {
    float TipsX[ 3 ] = { };
    float TipsY[ 3 ] = { };

    // Downward caret
    ui::ComputeCaretTips( 100.0f, 100.0f, true, 1.0f, TipsX, TipsY );
    CHECK_CLOSE( TipsX[ 0 ], 100.0f - 3.6f, 0.001f );
    CHECK_CLOSE( TipsX[ 1 ], 100.0f + 3.6f, 0.001f );
    CHECK_CLOSE( TipsX[ 2 ], 100.0f, 0.001f );
    CHECK_CLOSE( TipsY[ 2 ], 100.0f + 3.6f * 0.75f, 0.001f );

    // Rightward caret
    ui::ComputeCaretTips( 100.0f, 100.0f, false, 1.0f, TipsX, TipsY );
    CHECK_CLOSE( TipsX[ 1 ], 100.0f + 3.6f * 0.8f, 0.001f );
    CHECK_CLOSE( TipsY[ 1 ], 100.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeSwitchTrack and ComputeSwitchKnob" ) {
    ui::RectBounds Track;
    ui::ComputeSwitchTrack( 200.0f, 10.0f, 30.0f, 1.0f, Track );
    CHECK_CLOSE( Track.left, 156.0f, 0.001f );
    CHECK_CLOSE( Track.top, 14.0f, 0.001f );
    CHECK_CLOSE( Track.width, 44.0f, 0.001f );
    CHECK_CLOSE( Track.height, 22.0f, 0.001f );

    // Switch Knob: OFF (0.0f)
    ui::RectBounds KnobOff;
    ui::ComputeSwitchKnob( Track, 1.0f, 0.0f, KnobOff );
    CHECK_CLOSE( KnobOff.left, 158.0f, 0.001f );
    CHECK_CLOSE( KnobOff.top, 16.0f, 0.001f );
    CHECK_CLOSE( KnobOff.width, 18.0f, 0.001f );

    // Switch Knob: ON (1.0f)
    ui::RectBounds KnobOn;
    ui::ComputeSwitchKnob( Track, 1.0f, 1.0f, KnobOn );
    CHECK_CLOSE( KnobOn.left, 180.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeDropListBox downward and upward flip" ) {
    ui::RectBounds Downward;
    bool OkDown = ui::ComputeDropListBox( 10.0f, 50.0f, 78.0f, 200.0f, 3, 800.0f, 1.0f, Downward );
    CHECK( OkDown );
    CHECK_CLOSE( Downward.left, 10.0f, 0.001f );
    CHECK_CLOSE( Downward.top, 82.0f, 0.001f );
    CHECK_CLOSE( Downward.height, 84.0f, 0.001f );

    // Near screen bottom: should flip upwards
    ui::RectBounds Upward;
    bool OkUp = ui::ComputeDropListBox( 10.0f, 750.0f, 778.0f, 200.0f, 3, 800.0f, 1.0f, Upward );
    CHECK( OkUp );
    // 750 - 4 - 84 = 662
    CHECK_CLOSE( Upward.top, 662.0f, 0.001f );

    // Zero items returns false
    ui::RectBounds Empty;
    CHECK( !ui::ComputeDropListBox( 0.0f, 0.0f, 0.0f, 0.0f, 0, 800.0f, 1.0f, Empty ) );
}

TEST_CASE( "Layout: ValidatePickIndex boundary clamp" ) {
    CHECK_EQ( ui::ValidatePickIndex( -1, 5 ), 0 );
    CHECK_EQ( ui::ValidatePickIndex( 5, 5 ), 0 );
    CHECK_EQ( ui::ValidatePickIndex( 2, 5 ), 2 );
}

TEST_CASE( "Layout: ComputeStackedRow vertical stepping" ) {
    ui::RectBounds Row0 = ui::ComputeStackedRow( 10.0f, 20.0f, 300.0f, 28.0f, 2.0f, 0 );
    CHECK_CLOSE( Row0.left, 10.0f, 0.001f );
    CHECK_CLOSE( Row0.top, 20.0f, 0.001f );
    CHECK_CLOSE( Row0.width, 300.0f, 0.001f );
    CHECK_CLOSE( Row0.height, 28.0f, 0.001f );

    ui::RectBounds Row1 = ui::ComputeStackedRow( 10.0f, 20.0f, 300.0f, 28.0f, 2.0f, 1 );
    CHECK_CLOSE( Row1.top, 50.0f, 0.001f );

    ui::RectBounds Row5 = ui::ComputeStackedRow( 10.0f, 20.0f, 300.0f, 28.0f, 2.0f, 5 );
    CHECK_CLOSE( Row5.top, 170.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeSplitPair two-column split" ) {
    ui::RectBounds Left;
    ui::RectBounds Right;
    ui::ComputeSplitPair( 15.0f, 50.0f, 208.0f, 8.0f, 32.0f, Left, Right );

    // Available width 208, Gap 8 -> (208 - 8) / 2 = 100 each
    CHECK_CLOSE( Left.left, 15.0f, 0.001f );
    CHECK_CLOSE( Left.top, 50.0f, 0.001f );
    CHECK_CLOSE( Left.width, 100.0f, 0.001f );
    CHECK_CLOSE( Left.height, 32.0f, 0.001f );

    CHECK_CLOSE( Right.left, 123.0f, 0.001f );
    CHECK_CLOSE( Right.top, 50.0f, 0.001f );
    CHECK_CLOSE( Right.width, 100.0f, 0.001f );
    CHECK_CLOSE( Right.height, 32.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCardContainer header and body partitioning" ) {
    ui::RectBounds Card;
    ui::RectBounds Bar;
    ui::RectBounds Body;
    ui::ComputeCardContainer( 20.0f, 30.0f, 400.0f, 42.0f, 200.0f, Card, Bar, Body );

    CHECK_CLOSE( Card.left, 20.0f, 0.001f );
    CHECK_CLOSE( Card.top, 30.0f, 0.001f );
    CHECK_CLOSE( Card.width, 400.0f, 0.001f );
    CHECK_CLOSE( Card.height, 242.0f, 0.001f );

    CHECK_CLOSE( Bar.left, 20.0f, 0.001f );
    CHECK_CLOSE( Bar.top, 30.0f, 0.001f );
    CHECK_CLOSE( Bar.width, 400.0f, 0.001f );
    CHECK_CLOSE( Bar.height, 42.0f, 0.001f );

    CHECK_CLOSE( Body.left, 20.0f, 0.001f );
    CHECK_CLOSE( Body.top, 72.0f, 0.001f );
    CHECK_CLOSE( Body.width, 400.0f, 0.001f );
    CHECK_CLOSE( Body.height, 200.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeFoldCard expanding accordion geometry" ) {
    ui::RectBounds Card;
    ui::RectBounds Bar;
    ui::RectBounds Body;

    // Closed (Open = 0.0f)
    ui::ComputeFoldCard( 10.0f, 15.0f, 350.0f, 38.0f, 120.0f, 0.0f, Card, Bar, Body );
    CHECK_CLOSE( Card.height, 38.0f, 0.001f );
    CHECK_CLOSE( Bar.height, 38.0f, 0.001f );
    CHECK_CLOSE( Body.top, 53.0f, 0.001f );
    CHECK_CLOSE( Body.height, 120.0f, 0.001f );

    // Half-open (Open = 0.5f)
    ui::ComputeFoldCard( 10.0f, 15.0f, 350.0f, 38.0f, 120.0f, 0.5f, Card, Bar, Body );
    CHECK_CLOSE( Card.height, 98.0f, 0.001f );

    // Fully open (Open = 1.0f)
    ui::ComputeFoldCard( 10.0f, 15.0f, 350.0f, 38.0f, 120.0f, 1.0f, Card, Bar, Body );
    CHECK_CLOSE( Card.height, 158.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeFoldArrow indicator positioning" ) {
    ui::RectBounds Bar{ 10.0f, 20.0f, 300.0f, 40.0f };
    ui::RectBounds Arrow = ui::ComputeFoldArrow( Bar, 1.0f, 15.0f, 14.0f );

    // Right = 310, Mark = 15, Margin = 14 -> Left = 310 - 15 - 14 = 281
    CHECK_CLOSE( Arrow.left, 281.0f, 0.001f );
    // Top = 20 + (40 - 15) * 0.5 = 32.5
    CHECK_CLOSE( Arrow.top, 32.5f, 0.001f );
    CHECK_CLOSE( Arrow.width, 15.0f, 0.001f );
    CHECK_CLOSE( Arrow.height, 15.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCenteredBounds dialog centering" ) {
    ui::RectBounds Center = ui::ComputeCenteredBounds( 1920.0f, 1080.0f, 448.0f, 300.0f );
    CHECK_CLOSE( Center.left, 736.0f, 0.001f );
    CHECK_CLOSE( Center.top, 390.0f, 0.001f );
    CHECK_CLOSE( Center.width, 448.0f, 0.001f );
    CHECK_CLOSE( Center.height, 300.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeModalTall calculation" ) {
    // 7 steps, scale 1.0f, Line 16.0f, StepGap 6.0f, AfterSteps 18.0f, ActH 32.0f, Pad 18.0f, HeadH 42.0f
    float Tall = ui::ComputeModalTall( 42.0f, 16.0f, 6.0f, 7, 18.0f, 32.0f, 18.0f, 1.0f );
    // Base: 42 + 14 + 16 + 6 + 16 + 12 + 16 + 10 = 132
    // 7 steps * (16 + 6) = 154
    // Bottom: 18 + 32 + 18 = 68
    // Total: 132 + 154 + 68 = 354
    CHECK_CLOSE( Tall, 354.0f, 0.001f );
}

TEST_CASE( "Layout: EaseOutQuint and ComputePageSlide" ) {
    CHECK_CLOSE( ui::EaseOutQuint( 0.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( ui::EaseOutQuint( 1.0f ), 1.0f, 0.001f );
    CHECK_CLOSE( ui::EaseOutQuint( -0.5f ), 0.0f, 0.001f );
    CHECK_CLOSE( ui::EaseOutQuint( 1.5f ), 1.0f, 0.001f );

    // At halfway (0.5), Ease = 1 - (0.5)^5 = 1 - 0.03125 = 0.96875
    CHECK_CLOSE( ui::EaseOutQuint( 0.5f ), 0.96875f, 0.001f );

    // Slide at PageIn = 0.0f: full slide (1 - 0) * 36 * 1.0 * 1 = 36.0f
    float Slide0 = ui::ComputePageSlide( 0.0f, 1.0f, 1.0f, 36.0f );
    CHECK_CLOSE( Slide0, 36.0f, 0.001f );

    // Slide at PageIn = 1.0f: 0.0f
    float Slide1 = ui::ComputePageSlide( 1.0f, 1.0f, 1.0f, 36.0f );
    CHECK_CLOSE( Slide1, 0.0f, 0.001f );

    // Slide in reverse direction (-1.0f)
    float SlideRev = ui::ComputePageSlide( 0.0f, 1.0f, -1.0f, 36.0f );
    CHECK_CLOSE( SlideRev, -36.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCenteredIcon generic box centering" ) {
    ui::RectBounds Container{ 100.0f, 200.0f, 40.0f, 30.0f };
    ui::RectBounds Icon = ui::ComputeCenteredIcon( Container, 14.0f );

    // Center X: 100 + (40 - 14) * 0.5 = 100 + 13 = 113
    CHECK_CLOSE( Icon.left, 113.0f, 0.001f );
    // Center Y: 200 + (30 - 14) * 0.5 = 200 + 8 = 208
    CHECK_CLOSE( Icon.top, 208.0f, 0.001f );
    CHECK_CLOSE( Icon.width, 14.0f, 0.001f );
    CHECK_CLOSE( Icon.height, 14.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeTitleLayout with and without logo" ) {
    ui::RectBounds LogoB;
    float TextX = 0.0f, TextY = 0.0f;

    // With Logo: TextW = 80, TextH = 20, Header = (0, 0, 300, 50), Scale = 1.0f
    // LogoSize = 25, Gap = 8 -> Total = 80 + 25 + 8 = 113
    // Left = 0 + (300 - 113) * 0.5 = 93.5
    // Top = 0 + (50 - 20) * 0.5 = 15.0
    ui::ComputeTitleLayout( 0.0f, 0.0f, 300.0f, 50.0f, 80.0f, 20.0f, true, 1.0f, LogoB, TextX, TextY );
    CHECK_CLOSE( LogoB.left, 93.5f, 0.001f );
    CHECK_CLOSE( LogoB.top, 12.5f, 0.001f ); // (50 - 25) * 0.5 = 12.5
    CHECK_CLOSE( LogoB.width, 25.0f, 0.001f );
    CHECK_CLOSE( TextX, 93.5f + 25.0f + 8.0f, 0.001f ); // 126.5
    CHECK_CLOSE( TextY, 15.0f, 0.001f );

    // Without Logo: Total = 80
    // Left = (300 - 80) * 0.5 = 110.0
    ui::ComputeTitleLayout( 0.0f, 0.0f, 300.0f, 50.0f, 80.0f, 20.0f, false, 1.0f, LogoB, TextX, TextY );
    CHECK_CLOSE( TextX, 110.0f, 0.001f );
    CHECK_CLOSE( TextY, 15.0f, 0.001f );
}

TEST_CASE( "Layout: UpdateTabSlide interpolation and clamping" ) {
    // Normal step: Current = 0.0f, Target = 2, Dt = 0.016f, Speed = 20 -> Step = 0.32
    // NewAt = 0.0f + (2 - 0) * 0.32 = 0.64
    float StepNorm = ui::UpdateTabSlide( 0.0f, 2, 0.016f, 20.0f );
    CHECK_CLOSE( StepNorm, 0.64f, 0.001f );

    // Huge Dt: Step clamped to 1.0f -> reaches target immediately without overshoot
    float StepHuge = ui::UpdateTabSlide( 0.0f, 3, 2.0f, 20.0f );
    CHECK_CLOSE( StepHuge, 3.0f, 0.001f );

    // Already at target -> remains unchanged
    float StepSame = ui::UpdateTabSlide( 4.0f, 4, 0.016f, 20.0f );
    CHECK_CLOSE( StepSame, 4.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeTabActiveWeight distance falloff" ) {
    // Exact match
    CHECK_CLOSE( ui::ComputeTabActiveWeight( 2.0f, 2 ), 1.0f, 0.001f );

    // Distance 0.3 -> weight 0.7
    CHECK_CLOSE( ui::ComputeTabActiveWeight( 2.3f, 2 ), 0.7f, 0.001f );

    // Neighbor at index 3 -> distance 0.7 -> weight 0.3
    CHECK_CLOSE( ui::ComputeTabActiveWeight( 2.3f, 3 ), 0.3f, 0.001f );

    // Dist >= 1.0 -> weight 0.0
    CHECK_CLOSE( ui::ComputeTabActiveWeight( 2.3f, 0 ), 0.0f, 0.001f );
    CHECK_CLOSE( ui::ComputeTabActiveWeight( 2.3f, 4 ), 0.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeTabSwipeGeometry rail and boundary caps" ) {
    // Rail: Left=10, Top=20, Width=80, TabHeight=70, TabGap=0, Scale=1.0, 5 tabs
    // Top tab (TabAt = 0.0f)
    ui::TabSwipeGeometry TopGeo = ui::ComputeTabSwipeGeometry( 10.0f, 20.0f, 80.0f, 70.0f, 0.0f, 1.0f, 5, 0.0f );
    CHECK_CLOSE( TopGeo.stack.height, 350.0f, 0.001f );
    CHECK_CLOSE( TopGeo.fill.top, 20.0f, 0.001f );
    CHECK_CLOSE( TopGeo.fill.height, 70.0f, 0.001f );
    CHECK( TopGeo.capTop );
    CHECK( !TopGeo.capBot );
    CHECK_CLOSE( TopGeo.round, 12.0f, 0.001f );

    // Middle tab (TabAt = 2.0f)
    ui::TabSwipeGeometry MidGeo = ui::ComputeTabSwipeGeometry( 10.0f, 20.0f, 80.0f, 70.0f, 0.0f, 1.0f, 5, 2.0f );
    CHECK_CLOSE( MidGeo.fill.top, 160.0f, 0.001f );
    CHECK( !MidGeo.capTop );
    CHECK( !MidGeo.capBot );
    CHECK_CLOSE( MidGeo.round, 0.0f, 0.001f );

    // Bottom tab (TabAt = 4.0f)
    ui::TabSwipeGeometry BotGeo = ui::ComputeTabSwipeGeometry( 10.0f, 20.0f, 80.0f, 70.0f, 0.0f, 1.0f, 5, 4.0f );
    CHECK_CLOSE( BotGeo.fill.top, 300.0f, 0.001f );
    CHECK( !BotGeo.capTop );
    CHECK( BotGeo.capBot );
    CHECK_CLOSE( BotGeo.round, 12.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeTabItemGeometry glyph and label placement" ) {
    ui::RectBounds Glyph;
    float LabelX = 0.0f, LabelY = 0.0f;

    // Tab: Left = 10, Top = 50, Width = 80, TextWidth = 40, Scale = 1.0f
    // Mark = 24.0
    // Glyph.left = 10 + (80 - 24) * 0.5 = 10 + 28 = 38
    // Glyph.top = 50 + 13 = 63
    // LabelX = 10 + (80 - 40) * 0.5 = 30
    // LabelY = (63 + 24) + 7 = 94
    ui::ComputeTabItemGeometry( 10.0f, 50.0f, 80.0f, 40.0f, 1.0f, Glyph, LabelX, LabelY );
    CHECK_CLOSE( Glyph.left, 38.0f, 0.001f );
    CHECK_CLOSE( Glyph.top, 63.0f, 0.001f );
    CHECK_CLOSE( Glyph.width, 24.0f, 0.001f );
    CHECK_CLOSE( Glyph.height, 24.0f, 0.001f );
    CHECK_CLOSE( LabelX, 30.0f, 0.001f );
    CHECK_CLOSE( LabelY, 94.0f, 0.001f );
}

TEST_CASE( "Layout: ValidateBitmask empty fallback" ) {
    // Valid non-zero bits within mask
    CHECK_EQ( ui::ValidateBitmask( 5, 4, 1 ), 5 );
    CHECK_EQ( ui::ValidateBitmask( 2, 4, 1 ), 2 );

    // Zero bits falls back to default
    CHECK_EQ( ui::ValidateBitmask( 0, 4, 1 ), 1 );
    CHECK_EQ( ui::ValidateBitmask( 0, 4, 4 ), 4 );

    // Out of range bits (e.g. 16 for 4 items: mask is 15 -> (16 & 15) == 0)
    CHECK_EQ( ui::ValidateBitmask( 16, 4, 1 ), 1 );
}

TEST_CASE( "Layout: ToggleBitmaskOption toggle and non-empty enforcement" ) {
    int Bits = 1; // Item 0 selected

    // Toggling item 1 ON (1 -> 1 | 2 = 3)
    Bits = ui::ToggleBitmaskOption( Bits, 1, 4 );
    CHECK_EQ( Bits, 3 );

    // Toggling item 0 OFF (3 -> 2)
    Bits = ui::ToggleBitmaskOption( Bits, 0, 4 );
    CHECK_EQ( Bits, 2 );

    // Toggling the only selected item (item 1) OFF does NOT produce 0:
    // It enforces non-empty by keeping the toggled item (1 << 1 = 2)
    Bits = ui::ToggleBitmaskOption( Bits, 1, 4 );
    CHECK_EQ( Bits, 2 );
}

TEST_CASE( "Layout: ComputeGridItemBounds column and row layout" ) {
    // 3 columns, item size 20x20, gap 5x5, starting at (10, 10)
    // Index 0: col 0, row 0 -> (10, 10)
    ui::RectBounds Item0 = ui::ComputeGridItemBounds( 10.0f, 10.0f, 20.0f, 20.0f, 5.0f, 5.0f, 3, 0 );
    CHECK_CLOSE( Item0.left, 10.0f, 0.001f );
    CHECK_CLOSE( Item0.top, 10.0f, 0.001f );

    // Index 1: col 1, row 0 -> (10 + 25 = 35, 10)
    ui::RectBounds Item1 = ui::ComputeGridItemBounds( 10.0f, 10.0f, 20.0f, 20.0f, 5.0f, 5.0f, 3, 1 );
    CHECK_CLOSE( Item1.left, 35.0f, 0.001f );
    CHECK_CLOSE( Item1.top, 10.0f, 0.001f );

    // Index 4: col 1, row 1 -> (35, 10 + 25 = 35)
    ui::RectBounds Item4 = ui::ComputeGridItemBounds( 10.0f, 10.0f, 20.0f, 20.0f, 5.0f, 5.0f, 3, 4 );
    CHECK_CLOSE( Item4.left, 35.0f, 0.001f );
    CHECK_CLOSE( Item4.top, 35.0f, 0.001f );

    // Guard against Columns < 1
    ui::RectBounds ItemZeroCol = ui::ComputeGridItemBounds( 10.0f, 10.0f, 20.0f, 20.0f, 5.0f, 5.0f, 0, 2 );
    CHECK_CLOSE( ItemZeroCol.top, 10.0f + 25.0f * 2.0f, 0.001f );
}

TEST_CASE( "Layout: ComputePageFit dynamic height adjustment with options" ) {
    // Base fit at scale 1.0f with DrawFov = false, LimitFps = false
    ui::PageFit BaseFit = ui::ComputePageFit( 1.0f, false, false );
    CHECK_CLOSE( BaseFit.target, 276.0f, 0.001f );
    CHECK_CLOSE( BaseFit.misc, 138.0f, 0.001f );
    CHECK_CLOSE( BaseFit.inset, 10.0f, 0.001f );
    CHECK_CLOSE( BaseFit.general, 200.0f, 0.001f );

    // DrawFov = true adds 32.0f to target height
    ui::PageFit FovFit = ui::ComputePageFit( 1.0f, true, false );
    CHECK_CLOSE( FovFit.target, 308.0f, 0.001f );

    // LimitFps = true adds 28.0f to misc height
    ui::PageFit LimitFit = ui::ComputePageFit( 1.0f, false, true );
    CHECK_CLOSE( LimitFit.misc, 166.0f, 0.001f );

    // Scale = 1.5f scales all fit dimensions proportionally
    ui::PageFit ScaledFit = ui::ComputePageFit( 1.5f, true, true );
    CHECK_CLOSE( ScaledFit.target, 308.0f * 1.5f, 0.001f );
    CHECK_CLOSE( ScaledFit.misc, 166.0f * 1.5f, 0.001f );
    CHECK_CLOSE( ScaledFit.inset, 15.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeClampedScroll clamping and wheel delta" ) {
    // 30 items * 20px = 600px, Viewport = 400px -> Most = 200px
    // Scroll down (WheelDelta = -1, Step = 42)
    float S1 = ui::ComputeClampedScroll( 0.0f, -1.0f, 42.0f, 30, 20.0f, 400.0f );
    CHECK_CLOSE( S1, 42.0f, 0.001f );

    // Clamp to max bounds (Most = 200)
    float S2 = ui::ComputeClampedScroll( 190.0f, -1.0f, 42.0f, 30, 20.0f, 400.0f );
    CHECK_CLOSE( S2, 200.0f, 0.001f );

    // Scroll up (WheelDelta = 1, Step = 42) clamped to 0
    float S3 = ui::ComputeClampedScroll( 20.0f, 1.0f, 42.0f, 30, 20.0f, 400.0f );
    CHECK_CLOSE( S3, 0.0f, 0.001f );

    // Content fits in viewport (10 items * 20px = 200px < 400px -> Most = 0)
    float S4 = ui::ComputeClampedScroll( 0.0f, -5.0f, 42.0f, 10, 20.0f, 400.0f );
    CHECK_CLOSE( S4, 0.0f, 0.001f );
}

TEST_CASE( "Layout: IsRowVisible viewport culling" ) {
    float VTop = 100.0f;
    float VBottom = 500.0f;
    float RowH = 24.0f;

    // Above viewport
    CHECK( !ui::IsRowVisible( 50.0f, RowH, VTop, VBottom ) );

    // Partially visible at top edge
    CHECK( ui::IsRowVisible( 90.0f, RowH, VTop, VBottom ) );

    // Fully inside viewport
    CHECK( ui::IsRowVisible( 250.0f, RowH, VTop, VBottom ) );

    // Partially visible at bottom edge
    CHECK( ui::IsRowVisible( 490.0f, RowH, VTop, VBottom ) );

    // Below viewport
    CHECK( !ui::IsRowVisible( 510.0f, RowH, VTop, VBottom ) );
}

TEST_CASE( "Layout: ComputeTreeRowBounds and ComputeTreeItemElements indentation" ) {
    ui::RectBounds RowB = ui::ComputeTreeRowBounds( 10.0f, 50.0f, 300.0f, 2, 24.0f, 10.0f );
    CHECK_CLOSE( RowB.left, 10.0f, 0.001f );
    CHECK_CLOSE( RowB.top, 88.0f, 0.001f ); // 50 + 2 * 24 - 10 = 88
    CHECK_CLOSE( RowB.width, 300.0f, 0.001f );
    CHECK_CLOSE( RowB.height, 24.0f, 0.001f );

    // Depth = 0 (Root or top-level item)
    ui::TreeItemElements E0 = ui::ComputeTreeItemElements( 10.0f, 300.0f, 88.0f, 24.0f, 0, 1.0f, 15.0f, 16.0f );
    CHECK_CLOSE( E0.arm.left, 14.0f, 0.001f ); // 10 + 4
    CHECK_CLOSE( E0.arm.top, 88.0f, 0.001f );
    CHECK_CLOSE( E0.arm.width, 14.0f, 0.001f );
    CHECK_CLOSE( E0.caretCenterX, 21.0f, 0.001f ); // 14 + 7
    CHECK_CLOSE( E0.caretCenterY, 100.0f, 0.001f );  // 88 + 12
    CHECK_CLOSE( E0.mark.left, 30.0f, 0.001f ); // 14 + 14 + 2
    CHECK_CLOSE( E0.mark.top, 92.5f, 0.001f );  // 88 + (24 - 15) * 0.5
    CHECK_CLOSE( E0.textX, 51.0f, 0.001f );     // 30 + 15 + 6
    CHECK_CLOSE( E0.textY, 92.0f, 0.001f );     // 88 + (24 - 16) * 0.5
    CHECK_CLOSE( E0.highlight.left, 12.0f, 0.001f );
    CHECK_CLOSE( E0.highlight.width, 296.0f, 0.001f );

    // Depth = 2 (Nested item)
    ui::TreeItemElements E2 = ui::ComputeTreeItemElements( 10.0f, 300.0f, 88.0f, 24.0f, 2, 1.0f, 15.0f, 16.0f );
    CHECK_CLOSE( E2.arm.left, 38.0f, 0.001f ); // 10 + 4 + 12 * 2 = 38
}

TEST_CASE( "Layout: FormatTreeCaption formatting and extra count" ) {
    char Out[ 96 ] = { };

    // Standard item
    CHECK( ui::FormatTreeCaption( Out, sizeof( Out ), "Workspace", "Folder", 0, false ) );
    CHECK_EQ( std::string( Out ), "Workspace [Folder]" );

    // Closed item with extra count does not append +N
    CHECK( ui::FormatTreeCaption( Out, sizeof( Out ), "Players", "Folder", 5, false ) );
    CHECK_EQ( std::string( Out ), "Players [Folder]" );

    // Open item with extra count appends +N
    CHECK( ui::FormatTreeCaption( Out, sizeof( Out ), "Players", "Folder", 5, true ) );
    CHECK_EQ( std::string( Out ), "Players [Folder] +5" );

    // Nullptr safety
    CHECK( ui::FormatTreeCaption( Out, sizeof( Out ), nullptr, nullptr, 0, false ) );
    CHECK_EQ( std::string( Out ), " []" );

    // Buffer validation
    CHECK( !ui::FormatTreeCaption( nullptr, sizeof( Out ), "Item", "Class", 0, false ) );
    CHECK( !ui::FormatTreeCaption( Out, 0, "Item", "Class", 0, false ) );
}

TEST_CASE( "Layout: ComputeExplorerLayout panel partitioning" ) {
    ui::ExplorerLayout L = ui::ComputeExplorerLayout( 50.0f, 60.0f, 600.0f, 400.0f, 1.0f );

    // Search bar: Left 50, Top 60, Width 600, Height 28
    CHECK_CLOSE( L.search.left, 50.0f, 0.001f );
    CHECK_CLOSE( L.search.top, 60.0f, 0.001f );
    CHECK_CLOSE( L.search.width, 600.0f, 0.001f );
    CHECK_CLOSE( L.search.height, 28.0f, 0.001f );

    // Tree width: 600 * 0.56 = 336
    CHECK_CLOSE( L.treePane.left, 50.0f, 0.001f );
    CHECK_CLOSE( L.treePane.top, 94.0f, 0.001f ); // 60 + 28 + 6 = 94
    CHECK_CLOSE( L.treePane.width, 336.0f, 0.001f );
    CHECK_CLOSE( L.treePane.height, 366.0f, 0.001f ); // (60 + 400) - 94 = 366

    // Side width: 600 - 336 - 8 = 256
    CHECK_CLOSE( L.sidePane.left, 394.0f, 0.001f ); // 50 + 336 + 8 = 394
    CHECK_CLOSE( L.sidePane.top, 94.0f, 0.001f );
    CHECK_CLOSE( L.sidePane.width, 256.0f, 0.001f );
    CHECK_CLOSE( L.sidePane.height, 366.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCircleBounds centered bounding box" ) {
    ui::RectBounds C = ui::ComputeCircleBounds( 500.0f, 400.0f, 50.0f );
    CHECK_CLOSE( C.left, 450.0f, 0.001f );
    CHECK_CLOSE( C.top, 350.0f, 0.001f );
    CHECK_CLOSE( C.width, 100.0f, 0.001f );
    CHECK_CLOSE( C.height, 100.0f, 0.001f );

    // Zero radius
    ui::RectBounds Zero = ui::ComputeCircleBounds( 100.0f, 200.0f, 0.0f );
    CHECK_CLOSE( Zero.left, 100.0f, 0.001f );
    CHECK_CLOSE( Zero.top, 200.0f, 0.001f );
    CHECK_CLOSE( Zero.width, 0.0f, 0.001f );
    CHECK_CLOSE( Zero.height, 0.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeCenteredTextPos centering inside box" ) {
    float OutX = 0.0f, OutY = 0.0f;

    // 200x50 box at (100, 200), text is 80x20
    ui::ComputeCenteredTextPos( 100.0f, 200.0f, 200.0f, 50.0f, 80.0f, 20.0f, OutX, OutY );
    // OutX = 100 + (200 - 80) * 0.5 = 160
    // OutY = 200 + (50 - 20) * 0.5 = 215
    CHECK_CLOSE( OutX, 160.0f, 0.001f );
    CHECK_CLOSE( OutY, 215.0f, 0.001f );

    // Text identical size to box
    ui::ComputeCenteredTextPos( 50.0f, 60.0f, 100.0f, 40.0f, 100.0f, 40.0f, OutX, OutY );
    CHECK_CLOSE( OutX, 50.0f, 0.001f );
    CHECK_CLOSE( OutY, 60.0f, 0.001f );
}

TEST_CASE( "Layout: ComputeClickThrough and IsWidgetHovered state gates" ) {
    // Click through only when NOT hovered, NOT moving, NOT sliding
    CHECK( ui::ComputeClickThrough( false, false, false ) );
    CHECK( !ui::ComputeClickThrough( true, false, false ) );
    CHECK( !ui::ComputeClickThrough( false, true, false ) );
    CHECK( !ui::ComputeClickThrough( false, false, true ) );
    CHECK( !ui::ComputeClickThrough( true, true, true ) );

    // Widget hovered only when contains point, NOT moving, NOT sliding
    CHECK( ui::IsWidgetHovered( true, false, false ) );
    CHECK( !ui::IsWidgetHovered( false, false, false ) );
    CHECK( !ui::IsWidgetHovered( true, true, false ) );
    CHECK( !ui::IsWidgetHovered( true, false, true ) );
    CHECK( !ui::IsWidgetHovered( true, true, true ) );
}

TEST_CASE( "Layout: ComputeDefaultBadgeOrigin bottom-left positioning" ) {
    float X = 0.0f, Y = 0.0f;

    // 1080p screen, badge height 32, scale 1.0f, default margin 14.0f
    ui::ComputeDefaultBadgeOrigin( 1080.0f, 32.0f, 1.0f, 14.0f, X, Y );
    CHECK_CLOSE( X, 14.0f, 0.001f );
    CHECK_CLOSE( Y, 1034.0f, 0.001f ); // 1080 - 32 - 14 = 1034

    // 1440p screen, badge height 48, scale 1.5f, default margin 14.0f
    ui::ComputeDefaultBadgeOrigin( 1440.0f, 48.0f, 1.5f, 14.0f, X, Y );
    CHECK_CLOSE( X, 21.0f, 0.001f ); // 14 * 1.5 = 21
    CHECK_CLOSE( Y, 1371.0f, 0.001f ); // 1440 - 48 - 21 = 1371
}

TEST_CASE( "Layout: ComputeTwoColumnPartition symmetric column geometry" ) {
    float Left = 0.0f, Right = 0.0f, ColW = 0.0f;

    // Container at 100, width 400, pad 14, gap 16
    // Inner = 400 - 28 = 372
    // ColW = (372 - 16) * 0.5 = 178
    // Left = 100 + 14 = 114
    // Right = 114 + 178 + 16 = 308
    ui::ComputeTwoColumnPartition( 100.0f, 400.0f, 14.0f, 16.0f, Left, Right, ColW );
    CHECK_CLOSE( Left, 114.0f, 0.001f );
    CHECK_CLOSE( Right, 308.0f, 0.001f );
    CHECK_CLOSE( ColW, 178.0f, 0.001f );

    // Verify symmetry: Right + ColW + Pad == ContainerLeft + ContainerWidth
    CHECK_CLOSE( Right + ColW + 14.0f, 100.0f + 400.0f, 0.001f );
}





