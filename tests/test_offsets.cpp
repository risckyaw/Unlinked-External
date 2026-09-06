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
