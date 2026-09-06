#include "test_framework.hpp"
#include "gameplay.hpp"

TEST_CASE( "Gameplay: UpdateFramerateCap uncap to 10000" ) {
    std::string MockXml =
        "<roblox version=\"1\">\n"
        "  <Settings>\n"
        "    <int name=\"FramerateCap\">60</int>\n"
        "    <bool name=\"Fullscreen\">true</bool>\n"
        "  </Settings>\n"
        "</roblox>";

    bool Ok = play::UpdateFramerateCap( MockXml, true );
    CHECK_EQ( Ok, true );
    CHECK( MockXml.find( "<int name=\"FramerateCap\">10000</int>" ) != std::string::npos );
    CHECK( MockXml.find( "<bool name=\"Fullscreen\">true</bool>" ) != std::string::npos );
}

TEST_CASE( "Gameplay: UpdateFramerateCap cap to 240" ) {
    std::string MockXml =
        "<roblox version=\"1\">\n"
        "  <Settings>\n"
        "    <int name=\"FramerateCap\">10000</int>\n"
        "  </Settings>\n"
        "</roblox>";

    bool Ok = play::UpdateFramerateCap( MockXml, false );
    CHECK_EQ( Ok, true );
    CHECK( MockXml.find( "<int name=\"FramerateCap\">240</int>" ) != std::string::npos );
}

TEST_CASE( "Gameplay: UpdateFramerateCap missing key handles gracefully" ) {
    std::string MockXml =
        "<roblox version=\"1\">\n"
        "  <Settings>\n"
        "    <bool name=\"Fullscreen\">true</bool>\n"
        "  </Settings>\n"
        "</roblox>";

    std::string Original = MockXml;
    bool Ok = play::UpdateFramerateCap( MockXml, true );
    CHECK_EQ( Ok, false );
    CHECK_EQ( MockXml, Original );
}

TEST_CASE( "Gameplay: UpdateFramerateCap malformed XML tag handles gracefully" ) {
    std::string BadXml = "<int name=\"FramerateCap\">999999";
    std::string Original = BadXml;

    bool Ok = play::UpdateFramerateCap( BadXml, true );
    CHECK_EQ( Ok, false );
    CHECK_EQ( BadXml, Original );
}

TEST_CASE( "Gameplay: FpsFilter steady state and damping" ) {
    play::FpsFilter Filter;
    // Initial update sets both smooth and shown
    float First = Filter.Update( 1.0f / 60.0f, 60.0f );
    CHECK_CLOSE( First, 60.0f, 1.0f );

    // Consecutive steady 60fps frames maintain ~60fps
    for ( int i = 0; i < 30; i++ ) {
        Filter.Update( 1.0f / 60.0f, 60.0f );
    }
    float Steady = Filter.Update( 1.0f / 60.0f, 60.0f );
    CHECK_CLOSE( Steady, 60.0f, 1.0f );

    // Reset clears state
    Filter.Reset( );
    CHECK_CLOSE( Filter.smooth, 0.0f, 0.001f );
    CHECK_CLOSE( Filter.shown, 0.0f, 0.001f );
}

TEST_CASE( "Gameplay: FpsFilter tiny delta time fallback" ) {
    play::FpsFilter Filter;
    // When DeltaTime is <= 0.00005f, fallback framerate is used
    float Out = Filter.Update( 0.00001f, 144.0f );
    CHECK_CLOSE( Out, 144.0f, 0.5f );
}

TEST_CASE( "Gameplay: ClampFpsLimit boundary limits" ) {
    CHECK_CLOSE( play::ClampFpsLimit( 30.0f ), 60.0f, 0.001f );
    CHECK_CLOSE( play::ClampFpsLimit( 60.0f ), 60.0f, 0.001f );
    CHECK_CLOSE( play::ClampFpsLimit( 240.0f ), 240.0f, 0.001f );
    CHECK_CLOSE( play::ClampFpsLimit( 1000.0f ), 1000.0f, 0.001f );
    CHECK_CLOSE( play::ClampFpsLimit( 1500.0f ), 1000.0f, 0.001f );
}

TEST_CASE( "Gameplay: FormatWatermark combination modes" ) {
    char Buffer[ 64 ] = { };

    // Watermark and Fps
    CHECK( play::FormatWatermark( true, true, 60.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "Unlinked   60 fps" );

    // Watermark only
    CHECK( play::FormatWatermark( true, false, 60.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "Unlinked" );

    // Fps only
    CHECK( play::FormatWatermark( false, true, 144.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "144 fps" );

    // Neither enabled
    CHECK( !play::FormatWatermark( false, false, 60.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "" );

    // Negative FPS clamps to 0
    CHECK( play::FormatWatermark( false, true, -10.0f, Buffer, sizeof( Buffer ) ) );
    CHECK_EQ( std::string( Buffer ), "0 fps" );
}

TEST_CASE( "Gameplay: ComputeFrameGoalTime clamping and inversion" ) {
    // 60 FPS -> 1.0 / 60.0 ~ 0.016666...
    CHECK_CLOSE( ( float )play::ComputeFrameGoalTime( 60.0f ), 0.0166667f, 0.0001f );

    // 144 FPS -> 1.0 / 144.0 ~ 0.006944...
    CHECK_CLOSE( ( float )play::ComputeFrameGoalTime( 144.0f ), 0.0069444f, 0.0001f );

    // Below 60 FPS clamps to 60 FPS
    CHECK_CLOSE( ( float )play::ComputeFrameGoalTime( 20.0f ), 0.0166667f, 0.0001f );

    // Above 1000 FPS clamps to 1000 FPS -> 0.001
    CHECK_CLOSE( ( float )play::ComputeFrameGoalTime( 2000.0f ), 0.0010000f, 0.0001f );
}

TEST_CASE( "Gameplay: ComputeFramePacingAction thresholds" ) {
    double Goal = 0.0166667; // ~60 FPS

    // Action 0: Target time elapsed or exceeded -> proceed immediately
    CHECK_EQ( play::ComputeFramePacingAction( Goal, 0.0166667 ), 0 );
    CHECK_EQ( play::ComputeFramePacingAction( Goal, 0.0200000 ), 0 );

    // Action 1: More than 2ms remaining -> sleep
    // Spent = 0.010 -> Remaining = ~0.00667 > 0.002 -> Action 1
    CHECK_EQ( play::ComputeFramePacingAction( Goal, 0.0100000 ), 1 );

    // Action 2: Less than 2ms remaining -> spin / busy wait
    // Spent = 0.015 -> Remaining = ~0.00167 <= 0.002 -> Action 2
    CHECK_EQ( play::ComputeFramePacingAction( Goal, 0.0150000 ), 2 );
}

TEST_CASE( "Gameplay: ShouldTriggerAfkPulse accumulator and thresholding" ) {
    double Wait = 10.0;
    // Disabled state resets wait timer
    CHECK_EQ( play::ShouldTriggerAfkPulse( false, Wait, 1.0 ), false );
    CHECK_CLOSE( ( float )Wait, 0.0f, 0.0001f );

    // Accumulates time while active
    CHECK_EQ( play::ShouldTriggerAfkPulse( true, Wait, 5.0 ), false );
    CHECK_CLOSE( ( float )Wait, 5.0f, 0.0001f );

    CHECK_EQ( play::ShouldTriggerAfkPulse( true, Wait, 10.0 ), false );
    CHECK_CLOSE( ( float )Wait, 15.0f, 0.0001f );

    CHECK_EQ( play::ShouldTriggerAfkPulse( true, Wait, 2.9 ), false );
    CHECK_CLOSE( ( float )Wait, 17.9f, 0.0001f );

    // Reaching 18.0 seconds triggers pulse and resets wait timer
    CHECK_EQ( play::ShouldTriggerAfkPulse( true, Wait, 0.2 ), true );
    CHECK_CLOSE( ( float )Wait, 0.0f, 0.0001f );

    // Custom threshold support
    CHECK_EQ( play::ShouldTriggerAfkPulse( true, Wait, 3.5, 3.0 ), true );
    CHECK_CLOSE( ( float )Wait, 0.0f, 0.0001f );
}

TEST_CASE( "Gameplay: ComputeWindowCenterPoint client rect centering" ) {
    RECT Rect1 = { 0, 0, 1920, 1080 };
    POINT Pt1 = play::ComputeWindowCenterPoint( Rect1 );
    CHECK_EQ( Pt1.x, 960 );
    CHECK_EQ( Pt1.y, 540 );

    RECT Rect2 = { 100, 200, 900, 800 };
    POINT Pt2 = play::ComputeWindowCenterPoint( Rect2 );
    CHECK_EQ( Pt2.x, 400 );
    CHECK_EQ( Pt2.y, 300 );

    RECT Rect3 = { 0, 0, 0, 0 };
    POINT Pt3 = play::ComputeWindowCenterPoint( Rect3 );
    CHECK_EQ( Pt3.x, 0 );
    CHECK_EQ( Pt3.y, 0 );
}

TEST_CASE( "Gameplay: ShouldThrottleUncap interval deadline checking" ) {
    // When not applied yet, never throttle (allow first apply)
    CHECK_EQ( play::ShouldThrottleUncap( false, 0, 1000 ), false );
    CHECK_EQ( play::ShouldThrottleUncap( false, 500, 1000 ), false );

    // When applied, throttle if current time is before deadline
    CHECK_EQ( play::ShouldThrottleUncap( true, 500, 1000 ), true );
    CHECK_EQ( play::ShouldThrottleUncap( true, 999, 1000 ), true );
    CHECK_EQ( play::ShouldThrottleUncap( true, 1000, 1000 ), false );
    CHECK_EQ( play::ShouldThrottleUncap( true, 1500, 1000 ), false );
}

TEST_CASE( "Gameplay: FormatRobloxSettingsPath path formatting and validation" ) {
    char Path[ MAX_PATH ] = { };
    CHECK_EQ( play::FormatRobloxSettingsPath( "C:\\AppData\\Local", Path, sizeof( Path ) ), true );
    CHECK_EQ( std::string( Path ), "C:\\AppData\\Local\\Roblox\\GlobalBasicSettings_13.xml" );

    // Validation failures
    CHECK_EQ( play::FormatRobloxSettingsPath( nullptr, Path, sizeof( Path ) ), false );
    CHECK_EQ( play::FormatRobloxSettingsPath( "", Path, sizeof( Path ) ), false );
    CHECK_EQ( play::FormatRobloxSettingsPath( "C:\\AppData\\Local", nullptr, sizeof( Path ) ), false );
    CHECK_EQ( play::FormatRobloxSettingsPath( "C:\\AppData\\Local", Path, 0 ), false );
    CHECK_EQ( play::FormatRobloxSettingsPath( "C:\\AppData\\Local", Path, 10 ), false ); // Truncation
}

TEST_CASE( "Gameplay: IsValidXmlFileSize boundary checking" ) {
    CHECK_EQ( play::IsValidXmlFileSize( 0 ), false );
    CHECK_EQ( play::IsValidXmlFileSize( 1 ), true );
    CHECK_EQ( play::IsValidXmlFileSize( 4096 ), true );
    CHECK_EQ( play::IsValidXmlFileSize( 1u << 20 ), true );
    CHECK_EQ( play::IsValidXmlFileSize( ( 1u << 20 ) + 1 ), false );
    CHECK_EQ( play::IsValidXmlFileSize( 10000000 ), false );

    // Custom max size
    CHECK_EQ( play::IsValidXmlFileSize( 50, 40 ), false );
    CHECK_EQ( play::IsValidXmlFileSize( 40, 40 ), true );
}



