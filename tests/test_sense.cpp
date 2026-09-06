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
