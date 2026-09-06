#include "test_framework.hpp"
#include "world.hpp"

TEST_CASE( "Geometry: Printable character validation" ) {
    const char* Valid = "ValidPlayerName123_!";
    CHECK_EQ( world::Printable( Valid, ( int )strlen( Valid ) ), true );

    const char* WithTab = "Name\tInvalid";
    CHECK_EQ( world::Printable( WithTab, ( int )strlen( WithTab ) ), false );

    const char* WithNewline = "Line1\nLine2";
    CHECK_EQ( world::Printable( WithNewline, ( int )strlen( WithNewline ) ), false );

    const char* HighAscii = "Name\x80\xFF";
    CHECK_EQ( world::Printable( HighAscii, ( int )strlen( HighAscii ) ), false );

    // Length boundary tests
    CHECK_EQ( world::Printable( "", 0 ), false );
    CHECK_EQ( world::Printable( Valid, -1 ), false );

    char TooLong[ 70 ];
    memset( TooLong, 'A', sizeof( TooLong ) );
    CHECK_EQ( world::Printable( TooLong, 65 ), false );
}

TEST_CASE( "Geometry: DigitOnly string verification" ) {
    CHECK_EQ( world::DigitOnly( "" ), true );
    CHECK_EQ( world::DigitOnly( nullptr ), true );
    CHECK_EQ( world::DigitOnly( "1234567890" ), true );
    CHECK_EQ( world::DigitOnly( "0" ), true );
    CHECK_EQ( world::DigitOnly( "123a456" ), false );
    CHECK_EQ( world::DigitOnly( "hello" ), false );
    CHECK_EQ( world::DigitOnly( "-123" ), false );
}

TEST_CASE( "Geometry: NameOk username validation" ) {
    CHECK_EQ( world::NameOk( nullptr ), false );
    CHECK_EQ( world::NameOk( "" ), false );
    CHECK_EQ( world::NameOk( "PlayerOne" ), true );
    CHECK_EQ( world::NameOk( "Robloxian_99" ), true );

    // Pure digits are rejected as valid player names
    CHECK_EQ( world::NameOk( "123456789" ), false );

    // Non-printable chars rejected
    CHECK_EQ( world::NameOk( "Player\x07Name" ), false );
}

TEST_CASE( "Geometry: Ident identifier validation" ) {
    CHECK_EQ( world::Ident( nullptr ), false );
    CHECK_EQ( world::Ident( "" ), false );
    CHECK_EQ( world::Ident( "1Player" ), false ); // Must start with letter
    CHECK_EQ( world::Ident( "A" ), false );        // Min length 2
    CHECK_EQ( world::Ident( "A1" ), false );       // Needs at least 2 letters
    CHECK_EQ( world::Ident( "AB" ), true );
    CHECK_EQ( world::Ident( "HumanoidRootPart" ), true );
    CHECK_EQ( world::Ident( "Left Arm" ), true );  // Space allowed
    CHECK_EQ( world::Ident( "Upper_Torso_1" ), true );
    CHECK_EQ( world::Ident( "Bad@Symbol" ), false );
}

TEST_CASE( "Geometry: NormPing latency normalization" ) {
    // Negative or NaN fallback to 0
    CHECK_CLOSE( world::NormPing( -10.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( world::NormPing( 2500.0f ), 0.0f, 0.001f );

    // Values <= 1.0f are treated as already in seconds
    CHECK_CLOSE( world::NormPing( 0.035f ), 0.035f, 0.001f );
    CHECK_CLOSE( world::NormPing( 0.500f ), 0.500f, 0.001f );

    // Values > 1.0f are in milliseconds, converted to seconds (/ 1000)
    CHECK_CLOSE( world::NormPing( 65.0f ), 0.065f, 0.001f );
    CHECK_CLOSE( world::NormPing( 120.0f ), 0.120f, 0.001f );
    CHECK_CLOSE( world::NormPing( 1000.0f ), 1.000f, 0.001f );
}

TEST_CASE( "Geometry: SlabHit AABB ray intersection" ) {
    world::Vec3 Center{ 0.0f, 0.0f, 0.0f };
    world::Vec3 Half{ 1.0f, 1.0f, 1.0f };

    // Direct hit from (0, 0, -10) aiming along +Z
    world::Vec3 Origin{ 0.0f, 0.0f, -10.0f };
    world::Vec3 Dir{ 0.0f, 0.0f, 1.0f };
    CHECK_EQ( world::SlabHit( Origin, Dir, 50.0f, Center, Half, nullptr ), true );

    // Ray stopped short (MaxT = 5.0f when box boundary is at 9.0f)
    CHECK_EQ( world::SlabHit( Origin, Dir, 5.0f, Center, Half, nullptr ), false );

    // Ray pointing in opposite direction (-Z)
    world::Vec3 DirBack{ 0.0f, 0.0f, -1.0f };
    CHECK_EQ( world::SlabHit( Origin, DirBack, 50.0f, Center, Half, nullptr ), false );

    // Ray offset and missing the box
    world::Vec3 OriginMiss{ 5.0f, 0.0f, -10.0f };
    CHECK_EQ( world::SlabHit( OriginMiss, Dir, 50.0f, Center, Half, nullptr ), false );
}

TEST_CASE( "Geometry: SlabHit OBB rotated ray intersection" ) {
    world::Vec3 Center{ 0.0f, 0.0f, 0.0f };
    world::Vec3 Half{ 0.5f, 2.0f, 0.5f }; // Tall thin box

    // Identity rotation matrix (row-major)
    float RotIdent[ 9 ] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    world::Vec3 Origin{ 0.0f, 0.0f, -5.0f };
    world::Vec3 Dir{ 0.0f, 0.0f, 1.0f };
    CHECK_EQ( world::SlabHit( Origin, Dir, 20.0f, Center, Half, RotIdent ), true );

    // Rotate 90 degrees around Z axis (swapping X and Y axes)
    // X' = -Y, Y' = X
    float RotZ90[ 9 ] = {
        0.0f, 1.0f, 0.0f,
       -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    // Now width along X is 2.0, along Y is 0.5.
    // Ray at X=1.5, Y=0.0 aiming along +Z should hit the rotated box, but miss unrotated
    world::Vec3 OriginWide{ 1.2f, 0.0f, -5.0f };
    CHECK_EQ( world::SlabHit( OriginWide, Dir, 20.0f, Center, Half, RotIdent ), false );
    CHECK_EQ( world::SlabHit( OriginWide, Dir, 20.0f, Center, Half, RotZ90 ), true );
}
