#include "test_framework.hpp"
#include "store.hpp"

TEST_CASE( "Store: Sanitize valid and invalid characters" ) {
    char Name1[ 32 ] = "My Config 123";
    store::Sanitize( Name1 );
    CHECK_EQ( std::string( Name1 ), "My Config 123" );

    char Name2[ 32 ] = "Illegal!@#$%^&*()";
    store::Sanitize( Name2 );
    CHECK_EQ( std::string( Name2 ), "Illegal" );

    char Name3[ 32 ] = "Trailing spaces   ";
    store::Sanitize( Name3 );
    CHECK_EQ( std::string( Name3 ), "Trailing spaces" );

    char Name4[ 32 ] = "valid-name_with_chars";
    store::Sanitize( Name4 );
    CHECK_EQ( std::string( Name4 ), "valid-name_with_chars" );
}

TEST_CASE( "Store: Valid name checker" ) {
    CHECK_EQ( store::Valid( "LegitName123" ), true );
    CHECK_EQ( store::Valid( "Name With Spaces" ), true );
    CHECK_EQ( store::Valid( "Name-With-Dashes" ), true );
    CHECK_EQ( store::Valid( "" ), false );
    CHECK_EQ( store::Valid( nullptr ), false );
    CHECK_EQ( store::Valid( "Bad/Path\\Name" ), false );
}

TEST_CASE( "Store: Take integer parsing and key boundary isolation" ) {
    const char* Config = 
        "aim 1\n"
        "aim.fov 75\n"
        "aim.smooth 20\n"
        "esp.range 500\n";

    int Aim = 0;
    CHECK_EQ( store::Take( Config, "aim", Aim ), true );
    CHECK_EQ( Aim, 1 );

    int Fov = 0;
    CHECK_EQ( store::Take( Config, "aim.fov", Fov ), true );
    CHECK_EQ( Fov, 75 );

    int Smooth = 0;
    CHECK_EQ( store::Take( Config, "aim.smooth", Smooth ), true );
    CHECK_EQ( Smooth, 20 );

    int Missing = -1;
    CHECK_EQ( store::Take( Config, "nonexistent", Missing ), false );
    CHECK_EQ( Missing, -1 );
}

TEST_CASE( "Store: TakeF decimal floating point parsing" ) {
    const char* Config = 
        "aim.fov 72.50\n"
        "aim.smooth 40.25\n"
        "fade 0.75\n"
        "negative -12.5\n";

    float Fov = 0.0f;
    CHECK_EQ( store::TakeF( Config, "aim.fov", Fov ), true );
    CHECK_CLOSE( Fov, 72.50f, 0.001f );

    float Smooth = 0.0f;
    CHECK_EQ( store::TakeF( Config, "aim.smooth", Smooth ), true );
    CHECK_CLOSE( Smooth, 40.25f, 0.001f );

    float Fade = 0.0f;
    CHECK_EQ( store::TakeF( Config, "fade", Fade ), true );
    CHECK_CLOSE( Fade, 0.75f, 0.001f );

    float Neg = 0.0f;
    CHECK_EQ( store::TakeF( Config, "negative", Neg ), true );
    CHECK_CLOSE( Neg, -12.5f, 0.001f );
}

TEST_CASE( "Store: TakeB boolean parsing" ) {
    const char* Config = 
        "aim.on 1\n"
        "aim.vis 0\n";

    bool On = false;
    CHECK_EQ( store::TakeB( Config, "aim.on", On ), true );
    CHECK_EQ( On, true );

    bool Vis = true;
    CHECK_EQ( store::TakeB( Config, "aim.vis", Vis ), true );
    CHECK_EQ( Vis, false );
}
