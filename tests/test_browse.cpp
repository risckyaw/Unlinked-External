#include "test_framework.hpp"
#include "browse.hpp"
#include "explorer.hpp"

TEST_CASE( "Browse: SkipClass blacklist filtering" ) {
    CHECK_EQ( browse::SkipClass( "CoreGui" ), true );
    CHECK_EQ( browse::SkipClass( "CorePackages" ), true );
    CHECK_EQ( browse::SkipClass( "CoreReplicator" ), true );
    CHECK_EQ( browse::SkipClass( "HttpRbxApiService" ), true );

    CHECK_EQ( browse::SkipClass( "Workspace" ), false );
    CHECK_EQ( browse::SkipClass( "Players" ), false );
    CHECK_EQ( browse::SkipClass( "Part" ), false );
    CHECK_EQ( browse::SkipClass( "" ), false );
    CHECK_EQ( browse::SkipClass( nullptr ), false );
}

TEST_CASE( "Browse: IsKind case-insensitive equivalence" ) {
    CHECK_EQ( browse::IsKind( "Part", "Part" ), true );
    CHECK_EQ( browse::IsKind( "part", "PART" ), true );
    CHECK_EQ( browse::IsKind( "MeshPart", "Part" ), false );
    CHECK_EQ( browse::IsKind( nullptr, "Part" ), false );
    CHECK_EQ( browse::IsKind( "Part", nullptr ), false );
}

TEST_CASE( "Browse: IsPart classification" ) {
    CHECK_EQ( browse::IsPart( "Part" ), true );
    CHECK_EQ( browse::IsPart( "MeshPart" ), true );
    CHECK_EQ( browse::IsPart( "SpawnLocation" ), true );
    CHECK_EQ( browse::IsPart( "WedgePart" ), true );
    CHECK_EQ( browse::IsPart( "CornerWedgePart" ), true );
    CHECK_EQ( browse::IsPart( "TrussPart" ), true );
    CHECK_EQ( browse::IsPart( "UnionOperation" ), true );
    CHECK_EQ( browse::IsPart( "CustomPartInstance" ), true );

    CHECK_EQ( browse::IsPart( "Script" ), false );
    CHECK_EQ( browse::IsPart( "Folder" ), false );
    CHECK_EQ( browse::IsPart( "" ), false );
    CHECK_EQ( browse::IsPart( nullptr ), false );
}

TEST_CASE( "Browse: IsValue classification" ) {
    CHECK_EQ( browse::IsValue( "StringValue" ), true );
    CHECK_EQ( browse::IsValue( "IntValue" ), true );
    CHECK_EQ( browse::IsValue( "NumberValue" ), true );
    CHECK_EQ( browse::IsValue( "BoolValue" ), true );
    CHECK_EQ( browse::IsValue( "CFrameValue" ), true );

    CHECK_EQ( browse::IsValue( "Humanoid" ), false );
    CHECK_EQ( browse::IsValue( "" ), false );
    CHECK_EQ( browse::IsValue( nullptr ), false );
}

TEST_CASE( "Browse: IContains substring search and bounds protection" ) {
    CHECK_EQ( browse::IContains( "RobloxPlayer", "player" ), true );
    CHECK_EQ( browse::IContains( "DataModel", "data" ), true );
    CHECK_EQ( browse::IContains( "HumanoidRootPart", "root" ), true );
    CHECK_EQ( browse::IContains( "HumanoidRootPart", "xyz" ), false );

    // Needle longer than haystack
    CHECK_EQ( browse::IContains( "Cat", "Caterpillar" ), false );

    // Empty needle matches anything
    CHECK_EQ( browse::IContains( "AnyString", "" ), true );
    CHECK_EQ( browse::IContains( "AnyString", nullptr ), true );

    // Null or empty haystack
    CHECK_EQ( browse::IContains( "", "Needle" ), false );
    CHECK_EQ( browse::IContains( nullptr, "Needle" ), false );
}

TEST_CASE( "Browse: Node query matching with Hits" ) {
    browse::Node Item;
    strcpy_s( Item.name, sizeof( Item.name ), "HeadPart" );
    strcpy_s( Item.klass, sizeof( Item.klass ), "Part" );

    // Match by name
    CHECK_EQ( browse::Hits( Item, "head" ), true );
    CHECK_EQ( browse::Hits( Item, "HEAD" ), true );

    // Match by class
    CHECK_EQ( browse::Hits( Item, "part" ), true );

    // Empty query matches all
    CHECK_EQ( browse::Hits( Item, "" ), true );
    CHECK_EQ( browse::Hits( Item, nullptr ), true );

    // Mismatch
    CHECK_EQ( browse::Hits( Item, "Torso" ), false );
}

TEST_CASE( "Explorer: IconFor class to TreeIcon mapping" ) {
    CHECK_EQ( IconFor( "Workspace" ), TreeIcon::Workspace );
    CHECK_EQ( IconFor( "Players" ), TreeIcon::Players );
    CHECK_EQ( IconFor( "Player" ), TreeIcon::Player );
    CHECK_EQ( IconFor( "Humanoid" ), TreeIcon::Humanoid );
    CHECK_EQ( IconFor( "Camera" ), TreeIcon::Camera );
    CHECK_EQ( IconFor( "Folder" ), TreeIcon::Folder );
    CHECK_EQ( IconFor( "Model" ), TreeIcon::Model );
    CHECK_EQ( IconFor( "Part" ), TreeIcon::Part );
    CHECK_EQ( IconFor( "MeshPart" ), TreeIcon::Part );
    CHECK_EQ( IconFor( "LocalScript" ), TreeIcon::LocalScript );
    CHECK_EQ( IconFor( "Script" ), TreeIcon::Script );
    CHECK_EQ( IconFor( "Sound" ), TreeIcon::Sound );
    CHECK_EQ( IconFor( "UnknownClass" ), TreeIcon::Folder );
    CHECK_EQ( IconFor( "" ), TreeIcon::Folder );
    CHECK_EQ( IconFor( nullptr ), TreeIcon::Folder );
}

TEST_CASE( "Explorer: TreeHasKids hierarchy check" ) {
    // Ugc (root, index 0) has children
    CHECK_EQ( TreeHasKids( 0 ), true );

    // Workspace (index 1) has children
    CHECK_EQ( TreeHasKids( 1 ), true );

    // Baseplate (index 2) is a leaf part without children
    CHECK_EQ( TreeHasKids( 2 ), false );
}

TEST_CASE( "Browse: FormatBreadcrumbPath path joining and truncation" ) {
    char Path[ 64 ];

    // Normal multi-segment path
    const char* Segs[ ] = { "game", "Workspace", "Dummy", "Humanoid" };
    browse::FormatBreadcrumbPath( Segs, 4, Path, sizeof( Path ) );
    CHECK_EQ( std::string( Path ), "game.Workspace.Dummy.Humanoid" );

    // Single segment path
    const char* Single[ ] = { "game" };
    browse::FormatBreadcrumbPath( Single, 1, Path, sizeof( Path ) );
    CHECK_EQ( std::string( Path ), "game" );

    // Empty segments or null element fallback to "Inst"
    const char* Fallback[ ] = { "game", nullptr, "" };
    browse::FormatBreadcrumbPath( Fallback, 3, Path, sizeof( Path ) );
    CHECK_EQ( std::string( Path ), "game.Inst.Inst" );

    // Zero count
    Path[ 0 ] = 'X';
    browse::FormatBreadcrumbPath( Segs, 0, Path, sizeof( Path ) );
    CHECK_EQ( Path[ 0 ], '\0' );

    // Buffer truncation protection
    char Tiny[ 8 ];
    browse::FormatBreadcrumbPath( Segs, 4, Tiny, sizeof( Tiny ) );
    CHECK( strlen( Tiny ) < sizeof( Tiny ) );
}

TEST_CASE( "Browse: FormatVec3 3D vector coordinate formatting" ) {
    char Line[ 64 ];

    world::Vec3 Pos{ 12.345f, -67.891f, 0.0f };
    browse::FormatVec3( Pos, Line, sizeof( Line ) );
    CHECK_EQ( std::string( Line ), "12.35, -67.89, 0.00" );

    world::Vec3 Zero{ 0.0f, 0.0f, 0.0f };
    browse::FormatVec3( Zero, Line, sizeof( Line ) );
    CHECK_EQ( std::string( Line ), "0.00, 0.00, 0.00" );
}

TEST_CASE( "Browse: FormatHexAddr 64-bit pointer hex string formatting" ) {
    char Hex[ 32 ];

    browse::FormatHexAddr( 0x7FFE12345678ULL, Hex, sizeof( Hex ) );
    CHECK_EQ( std::string( Hex ), "0x7ffe12345678" );

    browse::FormatHexAddr( 0, Hex, sizeof( Hex ) );
    CHECK_EQ( std::string( Hex ), "0x0" );
}

TEST_CASE( "Browse: ComputeNudgeValue delta addition and zero clamping" ) {
    // Increment
    CHECK_CLOSE( browse::ComputeNudgeValue( 16.0f, 4.0f ), 20.0f, 0.001f );

    // Decrement
    CHECK_CLOSE( browse::ComputeNudgeValue( 50.0f, -10.0f ), 40.0f, 0.001f );

    // Clamp to 0.0f when delta pushes below 0
    CHECK_CLOSE( browse::ComputeNudgeValue( 5.0f, -10.0f ), 0.0f, 0.001f );
    CHECK_CLOSE( browse::ComputeNudgeValue( 0.0f, -1.0f ), 0.0f, 0.001f );
}

TEST_CASE( "Browse: NudgeFieldOffset humanoid offset selection" ) {
    world::Off O{ };
    O.humanoidHealth = 0x100;
    O.humanoidWalk = 0x108;
    O.humanoidJump = 0x110;
    O.humanoidHip = 0x118;

    CHECK_EQ( browse::NudgeFieldOffset( 1, O ), ( uintptr_t )0x100 ); // Health
    CHECK_EQ( browse::NudgeFieldOffset( 2, O ), ( uintptr_t )0x108 ); // WalkSpeed
    CHECK_EQ( browse::NudgeFieldOffset( 3, O ), ( uintptr_t )0x110 ); // JumpPower
    CHECK_EQ( browse::NudgeFieldOffset( 4, O ), ( uintptr_t )0x118 ); // HipHeight

    // Invalid Which codes return 0
    CHECK_EQ( browse::NudgeFieldOffset( 0, O ), ( uintptr_t )0 );
    CHECK_EQ( browse::NudgeFieldOffset( 5, O ), ( uintptr_t )0 );
    CHECK_EQ( browse::NudgeFieldOffset( -1, O ), ( uintptr_t )0 );
}

TEST_CASE( "Explorer: TreeChildCount parent hierarchy counts" ) {
    // Ugc (root, index 0) has multiple primary top-level services
    int UgcKids = TreeChildCount( 0 );
    CHECK( UgcKids >= 5 );

    // Workspace (index 1) has Baseplate, SpawnLocation, Camera, etc.
    int WsKids = TreeChildCount( 1 );
    CHECK( WsKids >= 3 );

    // Baseplate (index 2) is a leaf part without children
    CHECK_EQ( TreeChildCount( 2 ), 0 );

    // Dummy (index 7) has Humanoid, CoolHat, Face
    CHECK_EQ( TreeChildCount( 7 ), 3 );
}

TEST_CASE( "Explorer: TreeDepth hierarchical distance from root" ) {
    // Ugc (index 0) is root (parent -1), depth 0
    CHECK_EQ( TreeDepth( 0 ), 0 );

    // Workspace (index 1) child of Ugc, depth 1
    CHECK_EQ( TreeDepth( 1 ), 1 );

    // Dummy (index 7) child of Workspace, depth 2
    CHECK_EQ( TreeDepth( 7 ), 2 );

    // Humanoid (index 8) child of Dummy, depth 3
    CHECK_EQ( TreeDepth( 8 ), 3 );

    // Invalid index
    CHECK_EQ( TreeDepth( -1 ), -1 );
    CHECK_EQ( TreeDepth( 9999 ), -1 );
}

TEST_CASE( "Explorer: FindTreeNode case-insensitive lookup" ) {
    CHECK_EQ( FindTreeNode( "Ugc" ), 0 );
    CHECK_EQ( FindTreeNode( "ugc" ), 0 );
    CHECK_EQ( FindTreeNode( "Workspace" ), 1 );
    CHECK_EQ( FindTreeNode( "WORKSPACE" ), 1 );
    CHECK_EQ( FindTreeNode( "Humanoid" ), 8 );

    // Non-existent nodes
    CHECK_EQ( FindTreeNode( "NonExistentService" ), -1 );
    CHECK_EQ( FindTreeNode( "" ), -1 );
    CHECK_EQ( FindTreeNode( nullptr ), -1 );
}

