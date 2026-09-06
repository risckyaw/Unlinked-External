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
