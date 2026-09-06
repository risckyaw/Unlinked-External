#include "test_framework.hpp"
#include "catalog.hpp"

TEST_CASE( "Catalog: Tone names and boundary clamping" ) {
    CHECK_EQ( std::string( skin::toneName( 0 ) ), "Dark Knight" );
    CHECK_EQ( std::string( skin::toneName( 1 ) ), "Coffee" );
    CHECK_EQ( std::string( skin::toneName( 2 ) ), "Matcha" );

    // Out of bounds falls back to 0 ("Dark Knight")
    CHECK_EQ( std::string( skin::toneName( -1 ) ), "Dark Knight" );
    CHECK_EQ( std::string( skin::toneName( 3 ) ), "Dark Knight" );
    CHECK_EQ( std::string( skin::toneName( 999 ) ), "Dark Knight" );
}

TEST_CASE( "Catalog: Look names and boundary clamping" ) {
    CHECK_EQ( std::string( skin::lookName( 0 ) ), "Ice" );
    CHECK_EQ( std::string( skin::lookName( 1 ) ), "Thunder" );
    CHECK_EQ( std::string( skin::lookName( 2 ) ), "Ether" );
    CHECK_EQ( std::string( skin::lookName( 3 ) ), "Snow" );
    CHECK_EQ( std::string( skin::lookName( 4 ) ), "Bends" );
    CHECK_EQ( std::string( skin::lookName( 5 ) ), "Clouds" );

    // Out of bounds falls back to 0 ("Ice")
    CHECK_EQ( std::string( skin::lookName( -1 ) ), "Ice" );
    CHECK_EQ( std::string( skin::lookName( 6 ) ), "Ice" );
    CHECK_EQ( std::string( skin::lookName( 50 ) ), "Ice" );
}

TEST_CASE( "Catalog: Color palette lookup values and clamping" ) {
    const float* Deep0 = skin::deep( 0 );
    CHECK_CLOSE( Deep0[ 0 ], 0.018f, 0.001f );
    CHECK_CLOSE( Deep0[ 1 ], 0.024f, 0.001f );
    CHECK_CLOSE( Deep0[ 2 ], 0.038f, 0.001f );

    const float* Mid1 = skin::mid( 1 );
    CHECK_CLOSE( Mid1[ 0 ], 0.22f, 0.001f );
    CHECK_CLOSE( Mid1[ 1 ], 0.12f, 0.001f );
    CHECK_CLOSE( Mid1[ 2 ], 0.06f, 0.001f );

    const float* High2 = skin::high( 2 );
    CHECK_CLOSE( High2[ 0 ], 0.28f, 0.001f );
    CHECK_CLOSE( High2[ 1 ], 0.48f, 0.001f );
    CHECK_CLOSE( High2[ 2 ], 0.26f, 0.001f );

    // Clamping on invalid tone index
    const float* DeepOob = skin::deep( -10 );
    CHECK_CLOSE( DeepOob[ 0 ], Deep0[ 0 ], 0.001f );
    const float* HighOob = skin::high( 100 );
    CHECK_CLOSE( HighOob[ 0 ], skin::high( 0 )[ 0 ], 0.001f );
}

TEST_CASE( "Catalog: Atmosphere shader template generation" ) {
    // Index 0 falls back to index 1 (Thunder) because Ice uses IceMetal directly
    CHECK_EQ( skin::Atmosphere( 0 ), skin::Atmosphere( 1 ) );

    // Indices 1..5 return valid HLSL shader snippets
    for ( int i = 1; i < skin::LookCount; ++i ) {
        const char* Body = skin::Atmosphere( i );
        CHECK( Body != nullptr );
        CHECK( strstr( Body, "Final.rgb =" ) != nullptr );
    }

    // Out of bounds fallback to index 1 (Thunder)
    const char* Fallback = skin::Atmosphere( 99 );
    CHECK( Fallback != nullptr );
    CHECK_EQ( Fallback, skin::Atmosphere( 1 ) );
}

TEST_CASE( "Catalog: Tinted shader code synthesis" ) {
    const char* Base = "Float3 BaseColor = Float3( 1.0, 1.0, 1.0 );";
    const char* Tinted = skin::Tinted( Base, 0 );
    CHECK( Tinted != nullptr );
    CHECK( strstr( Tinted, Base ) != nullptr );
    CHECK( strstr( Tinted, "Float3 Deep = Float3(" ) != nullptr );
    CHECK( strstr( Tinted, "Final.rgb = Saturate(" ) != nullptr );
}

TEST_CASE( "Catalog: IceMetal procedural shader generator" ) {
    const char* Shader = IceMetal(
        0.018f, 0.024f, 0.038f,
        0.070f, 0.110f, 0.170f,
        0.260f, 0.360f, 0.480f
    );
    CHECK( Shader != nullptr );
    CHECK( strstr( Shader, "Float2 Uv = Screen" ) != nullptr );
    CHECK( strstr( Shader, "Final.rgb = Saturate( Tint );" ) != nullptr );
}
