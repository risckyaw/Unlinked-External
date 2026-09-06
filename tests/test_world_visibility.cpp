#include "test_framework.hpp"
#include "world.hpp"
#include "sense.hpp"

TEST_CASE( "World: Bone names and mappings (R15 vs R6)" ) {
    CHECK_EQ( std::string( world::BoneName[ world::BoneHead ] ), "Head" );
    CHECK_EQ( std::string( world::BoneName[ world::BoneRoot ] ), "HumanoidRootPart" );
    CHECK_EQ( std::string( world::BoneName[ world::BoneUpper ] ), "UpperTorso" );
    CHECK_EQ( std::string( world::BoneName[ world::BoneLower ] ), "LowerTorso" );
    CHECK_EQ( std::string( world::BoneName[ world::BoneLArmU ] ), "LeftUpperArm" );
    CHECK_EQ( std::string( world::BoneName[ world::BoneRArmU ] ), "RightUpperArm" );

    // R6 mapping maps multipart limbs to traditional simple body parts
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneHead ] ), "Head" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneRoot ] ), "HumanoidRootPart" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneUpper ] ), "Torso" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneLower ] ), "Torso" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneLArmU ] ), "Left Arm" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneRArmU ] ), "Right Arm" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneLLegU ] ), "Left Leg" );
    CHECK_EQ( std::string( world::BoneNameR6[ world::BoneRLegU ] ), "Right Leg" );

    for ( int i = 0; i < world::BoneMax; ++i ) {
        CHECK( world::BoneName[ i ] != nullptr && strlen( world::BoneName[ i ] ) > 0 );
        CHECK( world::BoneNameR6[ i ] != nullptr && strlen( world::BoneNameR6[ i ] ) > 0 );
    }
}

TEST_CASE( "World: Class classification predicates (IsSolid, IsHolder)" ) {
    // IsSolid checks
    CHECK( world::IsSolid( "Part" ) );
    CHECK( world::IsSolid( "MeshPart" ) );
    CHECK( world::IsSolid( "WedgePart" ) );
    CHECK( world::IsSolid( "CornerWedgePart" ) );
    CHECK( world::IsSolid( "TrussPart" ) );
    CHECK( world::IsSolid( "UnionOperation" ) );
    CHECK( world::IsSolid( "Ball" ) );
    CHECK( world::IsSolid( "Cylinder" ) );
    CHECK( world::IsSolid( "TriangleMeshPart" ) );
    CHECK( world::IsSolid( "PartOperation" ) );
    CHECK( world::IsSolid( "SmoothVoxelPart" ) );

    // Case-insensitivity
    CHECK( world::IsSolid( "part" ) );
    CHECK( world::IsSolid( "mEsHpArT" ) );

    // Non-solid classes
    CHECK( !world::IsSolid( "Folder" ) );
    CHECK( !world::IsSolid( "Model" ) );
    CHECK( !world::IsSolid( "Humanoid" ) );
    CHECK( !world::IsSolid( "Player" ) );
    CHECK( !world::IsSolid( "" ) );
    CHECK( !world::IsSolid( nullptr ) );

    // IsHolder checks
    CHECK( world::IsHolder( "Folder" ) );
    CHECK( world::IsHolder( "Model" ) );
    CHECK( world::IsHolder( "Actor" ) );
    CHECK( world::IsHolder( "WorldModel" ) );
    CHECK( world::IsHolder( "WorldRoot" ) );
    CHECK( world::IsHolder( "folder" ) );
    CHECK( world::IsHolder( "mOdEl" ) );

    CHECK( !world::IsHolder( "Part" ) );
    CHECK( !world::IsHolder( "MeshPart" ) );
    CHECK( !world::IsHolder( "Humanoid" ) );
    CHECK( !world::IsHolder( "" ) );
    CHECK( !world::IsHolder( nullptr ) );
}

TEST_CASE( "World: IsMate teammate classification" ) {
    world::Engine& E = world::Core( );
    uintptr_t SavedLocalTeam = E.localTeam;
    int SavedLocalTeamColor = E.localTeamColor;
    char SavedLocalTeamName[ 32 ];
    memcpy( SavedLocalTeamName, E.localTeamName, sizeof( SavedLocalTeamName ) );
    bool SavedColorUseful = E.colorUseful;
    bool SavedTeamReady = sense::Live( ).teamReady;
    int SavedTeamHow = sense::Live( ).teamHow;

    world::Actor Item{};
    Item.self = false;
    Item.team = 0x1000;
    Item.teamColor = 21;
    strcpy_s( Item.teamName, "Red" );

    E.localTeam = 0x1000;
    E.localTeamColor = 21;
    strcpy_s( E.localTeamName, "Red" );
    E.colorUseful = true;
    sense::Live( ).teamReady = true;

    // Self check always returns false
    Item.self = true;
    CHECK( !world::IsMate( Item ) );
    Item.self = false;

    // TeamNone always returns false
    sense::Live( ).teamHow = sense::TeamNone;
    CHECK( !world::IsMate( Item ) );

    // TeamPtr check
    sense::Live( ).teamHow = sense::TeamPtr;
    CHECK( world::IsMate( Item ) );
    Item.team = 0x2000;
    CHECK( !world::IsMate( Item ) );
    Item.team = 0x1000;

    // TeamName check (case-insensitive)
    sense::Live( ).teamHow = sense::TeamName;
    strcpy_s( Item.teamName, "red" );
    CHECK( world::IsMate( Item ) );
    strcpy_s( Item.teamName, "Blue" );
    CHECK( !world::IsMate( Item ) );
    strcpy_s( Item.teamName, "Red" );

    // TeamColor check
    sense::Live( ).teamHow = sense::TeamColor;
    CHECK( world::IsMate( Item ) );
    E.colorUseful = false;
    CHECK( !world::IsMate( Item ) ); // Not useful
    E.colorUseful = true;
    Item.teamColor = 23;
    CHECK( !world::IsMate( Item ) );
    Item.teamColor = 21;

    // TeamAuto fallback checks
    sense::Live( ).teamHow = sense::TeamAuto;
    Item.team = 0x9999; // Ptr mismatch
    Item.teamColor = 99; // Color mismatch
    CHECK( world::IsMate( Item ) ); // Name still matches!
    strcpy_s( Item.teamName, "Green" );
    CHECK( !world::IsMate( Item ) ); // Nothing matches

    // Restore engine state
    E.localTeam = SavedLocalTeam;
    E.localTeamColor = SavedLocalTeamColor;
    memcpy( E.localTeamName, SavedLocalTeamName, sizeof( SavedLocalTeamName ) );
    E.colorUseful = SavedColorUseful;
    sense::Live( ).teamReady = SavedTeamReady;
    sense::Live( ).teamHow = SavedTeamHow;
}

TEST_CASE( "World: RayBlocked raycast occlusion against walls" ) {
    world::Engine& E = world::Core( );
    int SavedWallN = E.wallN;

    world::Vec3 Eye{ 0.0f, 0.0f, 0.0f };
    world::Vec3 Target{ 0.0f, 0.0f, 100.0f };

    // With 0 walls, ray is unobstructed
    E.wallN = 0;
    CHECK( !world::RayBlocked( Eye, Target ) );

    // With reach < 0.35f, ray is never blocked
    CHECK( !world::RayBlocked( Eye, world::Vec3{ 0.0f, 0.0f, 0.2f } ) );

    // Place a blocking wall directly between Eye and Target at (0, 0, 50)
    E.wallN = 1;
    E.wallC[ 0 ] = world::Vec3{ 0.0f, 0.0f, 50.0f };
    E.wallH[ 0 ] = world::Vec3{ 5.0f, 5.0f, 1.0f }; // Thickness 2.0 along Z
    E.wallR[ 0 ][ 0 ] = 1.0f; E.wallR[ 0 ][ 1 ] = 0.0f; E.wallR[ 0 ][ 2 ] = 0.0f;
    E.wallR[ 0 ][ 3 ] = 0.0f; E.wallR[ 0 ][ 4 ] = 1.0f; E.wallR[ 0 ][ 5 ] = 0.0f;
    E.wallR[ 0 ][ 6 ] = 0.0f; E.wallR[ 0 ][ 7 ] = 0.0f; E.wallR[ 0 ][ 8 ] = 1.0f;

    // Ray to (0, 0, 100) hits the wall at Z=50
    CHECK( world::RayBlocked( Eye, Target ) );

    // Ray aimed wide at (25, 0, 100) misses the wall (wall X-half is 5.0)
    CHECK( !world::RayBlocked( Eye, world::Vec3{ 25.0f, 0.0f, 100.0f } ) );

    // Target located before the wall at (0, 0, 30) is unobstructed
    CHECK( !world::RayBlocked( Eye, world::Vec3{ 0.0f, 0.0f, 30.0f } ) );

    // Restore engine state
    E.wallN = SavedWallN;
}

TEST_CASE( "World: ActorClear multi-point line-of-sight checks" ) {
    world::Engine& E = world::Core( );
    int SavedWallN = E.wallN;

    world::Vec3 Eye{ 0.0f, 0.0f, 0.0f };
    world::Actor Item{};
    Item.head = world::Vec3{ 0.0f, 6.0f, 50.0f };
    Item.root = world::Vec3{ 0.0f, 0.0f, 50.0f };

    // With no walls, Actor is clear
    E.wallN = 0;
    CHECK( world::ActorClear( Item, Eye, world::Vec3{ 1.0f, 0.0f, 0.0f } ) );

    // Place a small wall that only obstructs the Head at (0, 6, 25)
    E.wallN = 1;
    E.wallC[ 0 ] = world::Vec3{ 0.0f, 3.0f, 25.0f }; // Wall at Y=3 covers ray to head
    E.wallH[ 0 ] = world::Vec3{ 2.0f, 1.0f, 0.5f };
    E.wallR[ 0 ][ 0 ] = 1.0f; E.wallR[ 0 ][ 1 ] = 0.0f; E.wallR[ 0 ][ 2 ] = 0.0f;
    E.wallR[ 0 ][ 3 ] = 0.0f; E.wallR[ 0 ][ 4 ] = 1.0f; E.wallR[ 0 ][ 5 ] = 0.0f;
    E.wallR[ 0 ][ 6 ] = 0.0f; E.wallR[ 0 ][ 7 ] = 0.0f; E.wallR[ 0 ][ 8 ] = 1.0f;

    // Head ray from (0,0,0) to (0,6,50) passes through (0,3,25), so head is blocked.
    // But Root at (0,0,50) ray passes through (0,0,25) which is outside wall (Y=3 +- 1).
    // ActorClear checks Head, Mid, Root: since Root is clear, ActorClear returns true!
    CHECK( world::ActorClear( Item, Eye, world::Vec3{ 1.0f, 0.0f, 0.0f } ) );

    // Place a massive wall at (0, 0, 25) that blocks Head, Mid, and Root
    E.wallC[ 0 ] = world::Vec3{ 0.0f, 0.0f, 25.0f };
    E.wallH[ 0 ] = world::Vec3{ 20.0f, 20.0f, 1.0f };
    CHECK( !world::ActorClear( Item, Eye, world::Vec3{ 1.0f, 0.0f, 0.0f } ) );

    // Restore engine state
    E.wallN = SavedWallN;
}

TEST_CASE( "World: Heap user-mode pointer address validation" ) {
    // Null and low memory addresses (< 0x10000) are invalid
    CHECK( !world::Heap( 0x0ull ) );
    CHECK( !world::Heap( 0xFFFull ) );
    CHECK( !world::Heap( 0xFFFFull ) );

    // Boundary check at exactly 0x10000
    CHECK( world::Heap( 0x10000ull ) );

    // Typical valid 64-bit user-mode heap addresses
    CHECK( world::Heap( 0x1A2B3C4D000ull ) );
    CHECK( world::Heap( 0x7FF6ABCD0000ull ) );
    CHECK( world::Heap( 0x00007FFFFFFFFFFEull ) );

    // High kernel addresses (>= 0x00007FFFFFFFFFFFull) are invalid
    CHECK( !world::Heap( 0x00007FFFFFFFFFFFull ) );
    CHECK( !world::Heap( 0xFFFF800000000000ull ) );
    CHECK( !world::Heap( 0xFFFFFFFFFFFFFFFFull ) );
}

TEST_CASE( "World: Collision flag masks (HasBlockFlags, FlagCanCollide, FlagCanTouch)" ) {
    CHECK_EQ( ( int )world::FlagCanCollide, 0x08 );
    CHECK_EQ( ( int )world::FlagCanTouch, 0x20 );
    CHECK_EQ( ( int )world::FlagBlockMask, 0x28 );

    // Zero flags: does not block
    CHECK( !world::HasBlockFlags( 0 ) );

    // CanCollide only: blocks
    CHECK( world::HasBlockFlags( world::FlagCanCollide ) );

    // CanTouch only: blocks
    CHECK( world::HasBlockFlags( world::FlagCanTouch ) );

    // Both flags combined: blocks
    CHECK( world::HasBlockFlags( world::FlagBlockMask ) );

    // Unrelated flags (e.g. 0x01, 0x02, 0x04, 0x10, 0x40, 0x80) do not block
    CHECK( !world::HasBlockFlags( 0x01 ) );
    CHECK( !world::HasBlockFlags( 0x02 ) );
    CHECK( !world::HasBlockFlags( 0x04 ) );
    CHECK( !world::HasBlockFlags( 0x10 ) );
    CHECK( !world::HasBlockFlags( 0x40 ) );
    CHECK( !world::HasBlockFlags( 0x100 ) );

    // Mixed unrelated flags with CanCollide: blocks
    CHECK( world::HasBlockFlags( 0x0100 | world::FlagCanCollide ) );
}

TEST_CASE( "World: ComputeHalfSize vector halving" ) {
    world::Vec3 Size{ 4.0f, 6.0f, 10.0f };
    world::Vec3 Half = world::ComputeHalfSize( Size );
    CHECK_CLOSE( Half.x, 2.0f, 0.001f );
    CHECK_CLOSE( Half.y, 3.0f, 0.001f );
    CHECK_CLOSE( Half.z, 5.0f, 0.001f );

    world::Vec3 Zero{ 0.0f, 0.0f, 0.0f };
    world::Vec3 ZeroHalf = world::ComputeHalfSize( Zero );
    CHECK_CLOSE( ZeroHalf.x, 0.0f, 0.001f );
    CHECK_CLOSE( ZeroHalf.y, 0.0f, 0.001f );
    CHECK_CLOSE( ZeroHalf.z, 0.0f, 0.001f );
}

TEST_CASE( "World: Euclidean distance calculation (Dist)" ) {
    world::Vec3 P1{ 0.0f, 0.0f, 0.0f };
    world::Vec3 P2{ 0.0f, 0.0f, 0.0f };
    CHECK_CLOSE( world::Dist( P1, P2 ), 0.0f, 0.001f );

    world::Vec3 P3{ 3.0f, 4.0f, 0.0f };
    CHECK_CLOSE( world::Dist( P1, P3 ), 5.0f, 0.001f );

    world::Vec3 P4{ 3.0f, 4.0f, 12.0f };
    CHECK_CLOSE( world::Dist( P1, P4 ), 13.0f, 0.001f );
}

TEST_CASE( "World: Matrix projection (Project) boundary and clipping" ) {
    // Create a 4x4 matrix: identity with perspective projection along Z
    // Matrix row-major:
    // [ 1  0  0  0 ]
    // [ 0  1  0  0 ]
    // [ 0  0  1  0 ]
    // [ 0  0  1  0 ] -> W = Z
    float Matrix[ 16 ] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    world::Dot Screen;

    // Behind camera: Z = 0.01 (W < 0.05f) returns false
    CHECK( !world::Project( world::Vec3{ 0.0f, 0.0f, 0.01f }, Matrix, 1920, 1080, Screen ) );

    // Center screen: (0, 0, 10) -> X/W = 0, Y/W = 0 -> (960, 540)
    CHECK( world::Project( world::Vec3{ 0.0f, 0.0f, 10.0f }, Matrix, 1920, 1080, Screen ) );
    CHECK( Screen.ok );
    CHECK_CLOSE( Screen.x, 960.0f, 0.001f );
    CHECK_CLOSE( Screen.y, 540.0f, 0.001f );

    // Extremely off-screen point (> 200px outside screen)
    // At X = 50, Z = 10 -> X/W = 5 -> Screen.x = 960 + 5 * 960 = 5760 (out of bounds)
    world::Project( world::Vec3{ 50.0f, 0.0f, 10.0f }, Matrix, 1920, 1080, Screen );
    CHECK( !Screen.ok );
}
