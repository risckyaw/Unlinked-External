#include "test_framework.hpp"
#include "world.hpp"

TEST_CASE( "Math: Vec3 distance calculation" ) {
    world::Vec3 A{ 0.0f, 0.0f, 0.0f };
    world::Vec3 B{ 3.0f, 4.0f, 0.0f };
    CHECK_CLOSE( world::Dist( A, B ), 5.0f, 0.001f );

    world::Vec3 C{ 1.0f, 2.0f, 3.0f };
    world::Vec3 D{ 1.0f, 2.0f, 3.0f };
    CHECK_CLOSE( world::Dist( C, D ), 0.0f, 0.001f );

    world::Vec3 E{ 2.0f, -3.0f, 6.0f };
    CHECK_CLOSE( world::Dist( A, E ), 7.0f, 0.001f );
}

TEST_CASE( "Math: World-to-Screen Project behind camera (W < 0.05)" ) {
    world::Vec3 World{ 0.0f, 0.0f, -10.0f };
    // Identity view matrix with W = -10
    float M[ 16 ] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };
    world::Dot Out;
    bool Projected = world::Project( World, M, 1920, 1080, Out );
    CHECK_EQ( Projected, false );
}

TEST_CASE( "Math: World-to-Screen Project center screen" ) {
    world::Vec3 World{ 0.0f, 0.0f, 10.0f };
    // Perspective matrix projecting (0, 0, 10) to center screen (960, 540)
    float M[ 16 ] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.1f, 0.0f
    };
    world::Dot Out;
    bool Projected = world::Project( World, M, 1920, 1080, Out );
    CHECK_EQ( Projected, true );
    CHECK_CLOSE( Out.x, 960.0f, 0.1f );
    CHECK_CLOSE( Out.y, 540.0f, 0.1f );
}
