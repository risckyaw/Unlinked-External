#include "test_framework.hpp"
#include "sense.hpp"

TEST_CASE( "Sense: Team enum to keyword serialization" ) {
    CHECK_EQ( std::string( sense::TeamWord( sense::TeamPtr ) ), "ptr" );
    CHECK_EQ( std::string( sense::TeamWord( sense::TeamName ) ), "name" );
    CHECK_EQ( std::string( sense::TeamWord( sense::TeamColor ) ), "color" );
    CHECK_EQ( std::string( sense::TeamWord( sense::TeamNone ) ), "none" );
    CHECK_EQ( std::string( sense::TeamWord( sense::TeamAuto ) ), "auto" );
    CHECK_EQ( std::string( sense::TeamWord( 999 ) ), "auto" );
}

TEST_CASE( "Sense: Visibility enum to keyword serialization" ) {
    CHECK_EQ( std::string( sense::VisWord( sense::VisRay ) ), "ray" );
    CHECK_EQ( std::string( sense::VisWord( sense::VisNone ) ), "none" );
    CHECK_EQ( std::string( sense::VisWord( sense::VisAuto ) ), "auto" );
    CHECK_EQ( std::string( sense::VisWord( -42 ) ), "auto" );
}

TEST_CASE( "Sense: Keyword parsing to Team enum (case-insensitive & fallbacks)" ) {
    CHECK_EQ( sense::TeamFrom( "ptr" ), sense::TeamPtr );
    CHECK_EQ( sense::TeamFrom( "PTR" ), sense::TeamPtr );
    CHECK_EQ( sense::TeamFrom( "Ptr" ), sense::TeamPtr );
    CHECK_EQ( sense::TeamFrom( "name" ), sense::TeamName );
    CHECK_EQ( sense::TeamFrom( "NAME" ), sense::TeamName );
    CHECK_EQ( sense::TeamFrom( "color" ), sense::TeamColor );
    CHECK_EQ( sense::TeamFrom( "COLOR" ), sense::TeamColor );
    CHECK_EQ( sense::TeamFrom( "none" ), sense::TeamNone );
    CHECK_EQ( sense::TeamFrom( "NONE" ), sense::TeamNone );
    CHECK_EQ( sense::TeamFrom( "unknown_val" ), sense::TeamAuto );
    CHECK_EQ( sense::TeamFrom( nullptr ), sense::TeamAuto );
}

TEST_CASE( "Sense: Keyword parsing to Vis enum (case-insensitive & fallbacks)" ) {
    CHECK_EQ( sense::VisFrom( "ray" ), sense::VisRay );
    CHECK_EQ( sense::VisFrom( "RAY" ), sense::VisRay );
    CHECK_EQ( sense::VisFrom( "none" ), sense::VisNone );
    CHECK_EQ( sense::VisFrom( "NONE" ), sense::VisNone );
    CHECK_EQ( sense::VisFrom( "invalid" ), sense::VisAuto );
    CHECK_EQ( sense::VisFrom( nullptr ), sense::VisAuto );
}

TEST_CASE( "Sense: DecideTeamMethod heuristics" ) {
    // Ptr priority: 2 or more pointers, or teamed with 1 pointer
    CHECK_EQ( sense::DecideTeamMethod( 2, 0, 0, 0 ), sense::TeamPtr );
    CHECK_EQ( sense::DecideTeamMethod( 3, 5, 5, 0 ), sense::TeamPtr );
    CHECK_EQ( sense::DecideTeamMethod( 1, 0, 0, 1 ), sense::TeamPtr );
    CHECK_EQ( sense::DecideTeamMethod( 1, 0, 0, 2 ), sense::TeamPtr );

    // Single pointer without team is not sufficient
    CHECK_EQ( sense::DecideTeamMethod( 1, 0, 0, 0 ), sense::TeamNone );

    // Name priority: 2 or more names when ptr conditions are not met
    CHECK_EQ( sense::DecideTeamMethod( 0, 2, 0, 0 ), sense::TeamName );
    CHECK_EQ( sense::DecideTeamMethod( 0, 4, 3, 0 ), sense::TeamName );
    CHECK_EQ( sense::DecideTeamMethod( 1, 2, 0, 0 ), sense::TeamName );
    CHECK_EQ( sense::DecideTeamMethod( 0, 1, 0, 0 ), sense::TeamNone );

    // Color priority: 2 or more colors when ptr and name conditions are not met
    CHECK_EQ( sense::DecideTeamMethod( 0, 0, 2, 0 ), sense::TeamColor );
    CHECK_EQ( sense::DecideTeamMethod( 0, 1, 3, 0 ), sense::TeamColor );
    CHECK_EQ( sense::DecideTeamMethod( 0, 0, 1, 0 ), sense::TeamNone );

    // Empty / insufficient signals
    CHECK_EQ( sense::DecideTeamMethod( 0, 0, 0, 0 ), sense::TeamNone );
    CHECK_EQ( sense::DecideTeamMethod( 1, 1, 1, 0 ), sense::TeamNone );
}

TEST_CASE( "Sense: DecideVisMethod and IsVisReady threshold" ) {
    // Under 6 walls -> VisAuto, not ready
    CHECK_EQ( sense::DecideVisMethod( 0 ), sense::VisAuto );
    CHECK_EQ( sense::DecideVisMethod( 3 ), sense::VisAuto );
    CHECK_EQ( sense::DecideVisMethod( 5 ), sense::VisAuto );
    CHECK_EQ( sense::IsVisReady( 0 ), false );
    CHECK_EQ( sense::IsVisReady( 5 ), false );

    // 6 or more walls -> VisRay, ready
    CHECK_EQ( sense::DecideVisMethod( 6 ), sense::VisRay );
    CHECK_EQ( sense::DecideVisMethod( 12 ), sense::VisRay );
    CHECK_EQ( sense::DecideVisMethod( 100 ), sense::VisRay );
    CHECK_EQ( sense::IsVisReady( 6 ), true );
    CHECK_EQ( sense::IsVisReady( 12 ), true );
}

TEST_CASE( "Sense: ShouldThrottleDecision timing predicate" ) {
    // If not ready, never throttle regardless of timestamps
    CHECK_EQ( sense::ShouldThrottleDecision( false, 500, 1000 ), false );
    CHECK_EQ( sense::ShouldThrottleDecision( false, 1500, 1000 ), false );

    // If ready, throttle when current time is strictly less than next deadline
    CHECK_EQ( sense::ShouldThrottleDecision( true, 500, 1000 ), true );
    CHECK_EQ( sense::ShouldThrottleDecision( true, 999, 1000 ), true );
    CHECK_EQ( sense::ShouldThrottleDecision( true, 1000, 1000 ), false );
    CHECK_EQ( sense::ShouldThrottleDecision( true, 1001, 1000 ), false );
}

TEST_CASE( "Sense: ExtractMethodValue token parsing" ) {
    char Out[ 32 ] = { };

    // Valid with \r\n line ending
    const char* Body1 = "place=1829283\r\nmethod=ptr\r\nextra=info\r\n";
    CHECK_EQ( sense::ExtractMethodValue( Body1, Out, sizeof( Out ) ), true );
    CHECK_EQ( std::string( Out ), "ptr" );

    // Valid with \n line ending
    const char* Body2 = "place=482910\nmethod=ray\nwalls=8\n";
    CHECK_EQ( sense::ExtractMethodValue( Body2, Out, sizeof( Out ) ), true );
    CHECK_EQ( std::string( Out ), "ray" );

    // Valid at end of string without newline
    const char* Body3 = "method=color";
    CHECK_EQ( sense::ExtractMethodValue( Body3, Out, sizeof( Out ) ), true );
    CHECK_EQ( std::string( Out ), "color" );

    // Missing key
    const char* Body4 = "place=1829283\r\nother=ptr\r\n";
    CHECK_EQ( sense::ExtractMethodValue( Body4, Out, sizeof( Out ) ), false );

    // Empty value
    const char* Body5 = "place=1829283\r\nmethod=\r\n";
    CHECK_EQ( sense::ExtractMethodValue( Body5, Out, sizeof( Out ) ), false );

    // Invalid parameters
    CHECK_EQ( sense::ExtractMethodValue( nullptr, Out, sizeof( Out ) ), false );
    CHECK_EQ( sense::ExtractMethodValue( Body1, nullptr, sizeof( Out ) ), false );
    CHECK_EQ( sense::ExtractMethodValue( Body1, Out, 0 ), false );
}

TEST_CASE( "Sense: FormatGameFilePath and extra telemetry strings" ) {
    char Path[ 128 ] = { };
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 12345678ULL, "team", Path, sizeof( Path ) ), true );
    CHECK_EQ( std::string( Path ), "C:\\Games\\12345678.team" );

    CHECK_EQ( sense::FormatGameFilePath( "D:\\Root\\Dir", 9999ULL, "vis", Path, sizeof( Path ) ), true );
    CHECK_EQ( std::string( Path ), "D:\\Root\\Dir\\9999.vis" );

    // Validation failures
    CHECK_EQ( sense::FormatGameFilePath( nullptr, 123ULL, "team", Path, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "", 123ULL, "team", Path, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 0ULL, "team", Path, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 123ULL, nullptr, Path, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 123ULL, "", Path, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 123ULL, "team", nullptr, sizeof( Path ) ), false );
    CHECK_EQ( sense::FormatGameFilePath( "C:\\Games", 123ULL, "team", Path, 0 ), false );

    // Telemetry formatting
    char Extra[ 80 ] = { };
    CHECK_EQ( sense::FormatExtraTeamInfo( 2, 3, 1, 4, Extra, sizeof( Extra ) ), true );
    CHECK_EQ( std::string( Extra ), "ptrs=2 names=3 colors=1 teamed=4" );

    char VisExtra[ 48 ] = { };
    CHECK_EQ( sense::FormatExtraVisInfo( 8, VisExtra, sizeof( VisExtra ) ), true );
    CHECK_EQ( std::string( VisExtra ), "walls=8" );
}

TEST_CASE( "Sense: Filesystem lifecycle and CustomRoot integration" ) {
    char TempPath[ MAX_PATH ] = { };
    GetTempPathA( MAX_PATH, TempPath );
    char TestDir[ MAX_PATH ] = { };
    snprintf( TestDir, sizeof( TestDir ), "%sunlinked_sense_test_%lu", TempPath, GetCurrentProcessId( ) );
    CreateDirectoryA( TestDir, nullptr );

    sense::SetCustomRoot( TestDir );

    const uint64_t TestPlace = 884729103ULL;

    // 1. Save and Load team method
    sense::SaveKind( TestPlace, "team", "ptr", "ptrs=2 names=0 colors=0 teamed=0" );
    char LoadedMethod[ 24 ] = { };
    CHECK_EQ( sense::LoadKind( TestPlace, "team", LoadedMethod, sizeof( LoadedMethod ) ), true );
    CHECK_EQ( std::string( LoadedMethod ), "ptr" );

    // 2. Save and Load vis method
    sense::SaveKind( TestPlace, "vis", "ray", "walls=7" );
    char LoadedVis[ 24 ] = { };
    CHECK_EQ( sense::LoadKind( TestPlace, "vis", LoadedVis, sizeof( LoadedVis ) ), true );
    CHECK_EQ( std::string( LoadedVis ), "ray" );

    // 3. BindPlace integration
    sense::BindPlace( TestPlace );
    sense::Card& C = sense::Live( );
    CHECK_EQ( C.place, TestPlace );
    CHECK_EQ( C.teamHow, sense::TeamPtr );
    CHECK_EQ( C.teamReady, true );
    CHECK_EQ( C.visHow, sense::VisRay );
    CHECK_EQ( C.visReady, true );

    CHECK_EQ( sense::TeamWorks( ), true );
    CHECK_EQ( sense::VisWorks( ), true );
    CHECK_EQ( sense::TeamHow( ), sense::TeamPtr );
    CHECK_EQ( sense::VisHow( ), sense::VisRay );

    // 4. Reset custom root
    sense::SetCustomRoot( nullptr );
}

