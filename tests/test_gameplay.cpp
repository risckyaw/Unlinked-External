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
