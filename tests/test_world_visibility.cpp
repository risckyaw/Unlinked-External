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
