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

TEST_CASE( "Catalog: ClampToneIndex boundary limits" ) {
    CHECK_EQ( skin::ClampToneIndex( 0 ), 0 );
    CHECK_EQ( skin::ClampToneIndex( 1 ), 1 );
    CHECK_EQ( skin::ClampToneIndex( 2 ), 2 );
    CHECK_EQ( skin::ClampToneIndex( -1 ), 0 );
    CHECK_EQ( skin::ClampToneIndex( 3 ), 0 );
    CHECK_EQ( skin::ClampToneIndex( 999 ), 0 );
}

TEST_CASE( "Catalog: ClampLookIndex boundary limits" ) {
    CHECK_EQ( skin::ClampLookIndex( 0 ), 0 );
    CHECK_EQ( skin::ClampLookIndex( 3 ), 3 );
    CHECK_EQ( skin::ClampLookIndex( 5 ), 5 );
    CHECK_EQ( skin::ClampLookIndex( -1 ), 0 );
    CHECK_EQ( skin::ClampLookIndex( 6 ), 0 );
    CHECK_EQ( skin::ClampLookIndex( 100 ), 0 );
}

TEST_CASE( "Catalog: ClampAtmosphereIndex boundary limits" ) {
    CHECK_EQ( skin::ClampAtmosphereIndex( 1 ), 1 );
    CHECK_EQ( skin::ClampAtmosphereIndex( 3 ), 3 );
    CHECK_EQ( skin::ClampAtmosphereIndex( 5 ), 5 );
    // Index 0 or invalid indices fall back to 1 (Thunder)
    CHECK_EQ( skin::ClampAtmosphereIndex( 0 ), 1 );
    CHECK_EQ( skin::ClampAtmosphereIndex( -5 ), 1 );
    CHECK_EQ( skin::ClampAtmosphereIndex( 6 ), 1 );
    CHECK_EQ( skin::ClampAtmosphereIndex( 50 ), 1 );
}

TEST_CASE( "Catalog: FormatShaderTint buffer formatting and validation" ) {
    const char* Base = "Float3 BaseColor = Float3( 1.0, 1.0, 1.0 );";
    const float Deep[ 3 ] = { 0.018f, 0.024f, 0.038f };
    const float Mid[ 3 ] = { 0.07f, 0.11f, 0.17f };
    const float High[ 3 ] = { 0.26f, 0.36f, 0.48f };

    char Out[ 1024 ] = { };
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, Mid, High, Out, sizeof( Out ) ), true );
    CHECK( strstr( Out, Base ) != nullptr );
    CHECK( strstr( Out, "Float3 Deep = Float3( 0.0180, 0.0240, 0.0380 );" ) != nullptr );
    CHECK( strstr( Out, "Float3 Mid = Float3( 0.0700, 0.1100, 0.1700 );" ) != nullptr );
    CHECK( strstr( Out, "Float3 High = Float3( 0.2600, 0.3600, 0.4800 );" ) != nullptr );
    CHECK( strstr( Out, "Final.rgb = Saturate(" ) != nullptr );

    // Validation failures
    CHECK_EQ( skin::FormatShaderTint( nullptr, Deep, Mid, High, Out, sizeof( Out ) ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, nullptr, Mid, High, Out, sizeof( Out ) ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, nullptr, High, Out, sizeof( Out ) ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, Mid, nullptr, Out, sizeof( Out ) ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, Mid, High, nullptr, sizeof( Out ) ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, Mid, High, Out, 0 ), false );
    CHECK_EQ( skin::FormatShaderTint( Base, Deep, Mid, High, Out, 20 ), false ); // Buffer too small
}

TEST_CASE( "Ice: ComputeIceSeedParams deterministic PRNG kinematics" ) {
    float ShiftX = 0.0f;
    float ShiftY = 0.0f;
    float Twist = 0.0f;
    float Grain = 0.0f;

    // Tick = 0
    ice::ComputeIceSeedParams( 0ULL, ShiftX, ShiftY, Twist, Grain );
    CHECK_CLOSE( ShiftX, 0.0f, 0.001f );
    CHECK_CLOSE( ShiftY, 0.0f, 0.001f );
    CHECK_CLOSE( Twist, 0.0f, 0.001f );
    CHECK_CLOSE( Grain, 1.7f, 0.001f );

    // Tick = 1000:
    // 1000 % 997 = 3 -> 3 * 0.041f = 0.123f
    // (1000 / 997) % 991 = 1 -> 1 * 0.037f = 0.037f
    // 1000 % 628 = 372 -> 372 * 0.01f = 3.72f
    // 1000 % 80 = 40 -> 1.7f + 40 * 0.01f = 2.1f
    ice::ComputeIceSeedParams( 1000ULL, ShiftX, ShiftY, Twist, Grain );
    CHECK_CLOSE( ShiftX, 0.123f, 0.001f );
    CHECK_CLOSE( ShiftY, 0.037f, 0.001f );
    CHECK_CLOSE( Twist, 3.72f, 0.001f );
    CHECK_CLOSE( Grain, 2.1f, 0.001f );
}

TEST_CASE( "Ice: FormatIceMetalShader procedural HLSL formatting" ) {
    const float Deep[ 3 ] = { 0.018f, 0.024f, 0.038f };
    const float Mid[ 3 ] = { 0.07f, 0.11f, 0.17f };
    const float High[ 3 ] = { 0.26f, 0.36f, 0.48f };

    char Out[ 2048 ] = { };
    CHECK_EQ( ice::FormatIceMetalShader( 0.123f, 0.037f, 3.72f, 2.1f, Deep, Mid, High, Out, sizeof( Out ) ), true );
    CHECK( strstr( Out, "Float2 Uv = Screen * Float2( 0.0026, 0.0115 ) + Float2( 0.1230, 0.0370 );" ) != nullptr );
    CHECK( strstr( Out, "Float3 Deep = Float3( 0.0180, 0.0240, 0.0380 );" ) != nullptr );
    CHECK( strstr( Out, "Float3 Mid = Float3( 0.0700, 0.1100, 0.1700 );" ) != nullptr );
    CHECK( strstr( Out, "Float3 High = Float3( 0.2600, 0.3600, 0.4800 );" ) != nullptr );
    CHECK( strstr( Out, "Final.rgb = Saturate( Tint );" ) != nullptr );

    // Validation failures
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, nullptr, Mid, High, Out, sizeof( Out ) ), false );
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, Deep, nullptr, High, Out, sizeof( Out ) ), false );
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, Deep, Mid, nullptr, Out, sizeof( Out ) ), false );
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, Deep, Mid, High, nullptr, sizeof( Out ) ), false );
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, Deep, Mid, High, Out, 0 ), false );
    CHECK_EQ( ice::FormatIceMetalShader( 0.0f, 0.0f, 0.0f, 1.7f, Deep, Mid, High, Out, 50 ), false ); // Too small
}

