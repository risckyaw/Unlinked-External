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

