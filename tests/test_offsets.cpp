#include "test_framework.hpp"
#include "offsets.hpp"

TEST_CASE( "Offsets: Parser primitive string and number parsing" ) {
    const char* Json = "\"Hello World\" 123456";
    offsets::Parser P;
    P.At = Json;
    P.End = Json + strlen( Json );

    std::string Str = P.String( );
    CHECK_EQ( Str, "Hello World" );
    CHECK_EQ( P.Fail, false );

    uintptr_t Num = P.Number( );
    CHECK_EQ( Num, ( uintptr_t )123456 );
    CHECK_EQ( P.Fail, false );
}

TEST_CASE( "Offsets: Parse structured offsets JSON payload" ) {
    std::string MockJson = 
        "{\n"
        "  \"Roblox Version\": \"version-123456789\",\n"
        "  \"Dumped At\": \"2026-09-06T12:00:00Z\",\n"
        "  \"Total Offsets\": 3,\n"
        "  \"Offsets\": {\n"
        "    \"Humanoid\": {\n"
        "      \"Health\": 368,\n"
        "      \"MaxHealth\": 392\n"
        "    },\n"
        "    \"Player\": {\n"
        "      \"Character\": 248\n"
        "    }\n"
        "  }\n"
        "}";

    bool Ok = offsets::Parse( MockJson );
    CHECK_EQ( Ok, true );
    CHECK_EQ( offsets::Data( ).version, "version-123456789" );
    CHECK_EQ( offsets::Data( ).total, 3 );
    CHECK_EQ( offsets::Data( ).map[ "Humanoid" ][ "Health" ], ( uintptr_t )368 );
    CHECK_EQ( offsets::Data( ).map[ "Humanoid" ][ "MaxHealth" ], ( uintptr_t )392 );
    CHECK_EQ( offsets::Data( ).map[ "Player" ][ "Character" ], ( uintptr_t )248 );
}

TEST_CASE( "Offsets: Parse handles malformed JSON gracefully" ) {
    std::string BadJson = "Not valid JSON at all";
    bool Ok = offsets::Parse( BadJson );
    CHECK_EQ( Ok, false );
}

TEST_CASE( "Offsets: ExtractVersion from process executable paths" ) {
    char Out[ 64 ] = { };

    // Windows backslash path
    const char* WinPath = "C:\\Users\\User\\AppData\\Local\\Roblox\\Versions\\version-a1b2c3d4e5f6\\RobloxPlayerBeta.exe";
    CHECK( offsets::ExtractVersion( WinPath, Out, ( int )sizeof( Out ) ) );
    CHECK_EQ( std::string( Out ), "version-a1b2c3d4e5f6" );

    // Forward slash path
    const char* UnixPath = "C:/Roblox/Versions/version-deadbeef999/RobloxPlayer.exe";
    CHECK( offsets::ExtractVersion( UnixPath, Out, ( int )sizeof( Out ) ) );
    CHECK_EQ( std::string( Out ), "version-deadbeef999" );

    // Trailing directory without trailing slash
    const char* DirectPath = "D:\\Games\\Roblox\\version-isolated123";
    CHECK( offsets::ExtractVersion( DirectPath, Out, ( int )sizeof( Out ) ) );
    CHECK_EQ( std::string( Out ), "version-isolated123" );

    // Path without version- tag
    const char* NoVer = "C:\\Windows\\System32\\cmd.exe";
    CHECK( !offsets::ExtractVersion( NoVer, Out, ( int )sizeof( Out ) ) );
    CHECK_EQ( Out[ 0 ], 0 );

    // Null or invalid arguments
    CHECK( !offsets::ExtractVersion( nullptr, Out, 64 ) );
    CHECK( !offsets::ExtractVersion( WinPath, nullptr, 64 ) );
    CHECK( !offsets::ExtractVersion( WinPath, Out, 4 ) ); // Cap < 8
}

TEST_CASE( "Offsets: IsVersionMatch comparison" ) {
    // Identical versions
    CHECK( offsets::IsVersionMatch( "version-123456", "version-123456" ) );

    // Case-insensitive match
    CHECK( offsets::IsVersionMatch( "version-AbCdEf", "version-aBcDeF" ) );

    // Mismatched versions
    CHECK( !offsets::IsVersionMatch( "version-111111", "version-222222" ) );

    // Null or empty handling
    CHECK( !offsets::IsVersionMatch( nullptr, "version-123" ) );
    CHECK( !offsets::IsVersionMatch( "version-123", nullptr ) );
    CHECK( !offsets::IsVersionMatch( "", "version-123" ) );
    CHECK( !offsets::IsVersionMatch( "version-123", "" ) );
}

TEST_CASE( "Offsets: Query helpers (Get, CopyVersion, Total, Ready)" ) {
    // Set up test table
    {
        std::lock_guard< std::recursive_mutex > Hold( offsets::Gate( ) );
        offsets::Data( ).ready = true;
        offsets::Data( ).version = "version-test42";
        offsets::Data( ).total = 2;
        offsets::Data( ).map[ "Workspace" ][ "Camera" ] = 0x1A0;
        offsets::Data( ).map[ "Workspace" ][ "Terrain" ] = 0x1B8;
    }

    CHECK_EQ( offsets::Get( "Workspace", "Camera" ), ( uintptr_t )0x1A0 );
    CHECK_EQ( offsets::Get( "Workspace", "Terrain" ), ( uintptr_t )0x1B8 );
    CHECK_EQ( offsets::Get( "Workspace", "MissingField" ), ( uintptr_t )0 );
    CHECK_EQ( offsets::Get( "MissingClass", "Camera" ), ( uintptr_t )0 );
    CHECK_EQ( offsets::Get( nullptr, "Camera" ), ( uintptr_t )0 );

    char VerCopy[ 32 ] = { };
    offsets::CopyVersion( VerCopy, sizeof( VerCopy ) );
    CHECK_EQ( std::string( VerCopy ), "version-test42" );

    CHECK_EQ( offsets::Total( ), 2 );
    CHECK( offsets::Ready( ) );
}

TEST_CASE( "Offsets: StageText descriptions" ) {
    offsets::Data( ).stage.store( ( int )offsets::Stage::Idle );
    CHECK_EQ( std::string( offsets::StageText( ) ), "" );

    offsets::Data( ).stage.store( ( int )offsets::Stage::Check );
    CHECK_EQ( std::string( offsets::StageText( ) ), "checking" );

    offsets::Data( ).stage.store( ( int )offsets::Stage::Fetch );
    CHECK_EQ( std::string( offsets::StageText( ) ), "downloading" );

    offsets::Data( ).stage.store( ( int )offsets::Stage::Apply );
    CHECK_EQ( std::string( offsets::StageText( ) ), "applying" );

    // Reset back to Idle
    offsets::Data( ).stage.store( ( int )offsets::Stage::Idle );
}
