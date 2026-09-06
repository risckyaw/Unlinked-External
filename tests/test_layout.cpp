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
