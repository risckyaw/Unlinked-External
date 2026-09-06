#pragma once

/**
 * @file sense.hpp
 * @brief Unlinked External - ESP overlay visualizers, 2D/3D boxes, skeleton joints, snaplines, and player tags.
 */

#include <Windows.h>
#include <ShlObj.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace sense {

enum TeamHow : int {
    TeamAuto = 0,
    TeamNone,
    TeamPtr,
    TeamName,
    TeamColor
};

enum VisHow : int {
    VisAuto = 0,
    VisNone,
    VisRay
};

struct Card {
    uint64_t place = 0;
    int teamHow = TeamAuto;
    int visHow = VisAuto;
    bool teamReady = false;
    bool visReady = false;
    unsigned nextTeam = 0;
    unsigned nextVis = 0;
};

inline Card& Live( ) {
    static Card Store;
    return Store;
}

inline int TeamHow( ) {
    return Live( ).teamReady ? Live( ).teamHow : TeamAuto;
}

inline int VisHow( ) {
    return Live( ).visReady ? Live( ).visHow : VisAuto;
}

inline bool TeamWorks( ) {
    int How = TeamHow( );
    return How != TeamNone;
}

inline bool VisWorks( ) {
    return VisHow( ) != VisNone;
}

inline const char*& CustomRoot( ) {
    static const char* Path = nullptr;
    return Path;
}

inline void SetCustomRoot( const char* Path ) {
    CustomRoot( ) = Path;
}

inline bool Folder( char* Out, int Cap ) {
    if ( !Out || Cap <= 0 )
        return false;
    if ( CustomRoot( ) ) {
        int Res = snprintf( Out, Cap, "%s", CustomRoot( ) );
        if ( Res <= 0 || Res >= Cap )
            return false;
        CreateDirectoryA( Out, nullptr );
        return true;
    }
    char App[ MAX_PATH ] = { };
    if ( FAILED( SHGetFolderPathA( nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, App ) ) )
        return false;
    snprintf( Out, Cap, "%s\\Unlinked", App );
    CreateDirectoryA( Out, nullptr );
    snprintf( Out, Cap, "%s\\Unlinked\\games", App );
    CreateDirectoryA( Out, nullptr );
    return true;
}

[[nodiscard]] inline bool FormatGameFilePath( const char* Dir, uint64_t Place, const char* Kind, char* Out, int Cap ) noexcept {
    if ( !Dir || !Dir[ 0 ] || !Place || !Kind || !Kind[ 0 ] || !Out || Cap <= 0 )
        return false;
    int Res = snprintf( Out, Cap, "%s\\%llu.%s", Dir, ( unsigned long long )Place, Kind );
    return Res > 0 && Res < Cap;
}

inline bool PathOf( uint64_t Place, const char* Kind, char* Out, int Cap ) {
    char Dir[ MAX_PATH ] = { };
    if ( !Folder( Dir, MAX_PATH ) )
        return false;
    return FormatGameFilePath( Dir, Place, Kind, Out, Cap );
}

[[nodiscard]] inline bool ExtractMethodValue( const char* Body, char* Method, int Cap ) noexcept {
    if ( Method && Cap > 0 )
        Method[ 0 ] = 0;
    if ( !Body || !Method || Cap <= 0 )
        return false;
    const char* At = strstr( Body, "method=" );
    if ( !At )
        return false;
    At += 7;
    int Write = 0;
    while ( At[ Write ] && At[ Write ] != '\r' && At[ Write ] != '\n' && Write < Cap - 1 ) {
        Method[ Write ] = At[ Write ];
        Write++;
    }
    Method[ Write ] = 0;
    return Method[ 0 ] != 0;
}

[[nodiscard]] constexpr int DecideTeamMethod( int PtrN, int NameN, int ColorN, int Teamed ) noexcept {
    if ( PtrN >= 2 || ( Teamed > 0 && PtrN >= 1 ) )
        return TeamPtr;
    if ( NameN >= 2 )
        return TeamName;
    if ( ColorN >= 2 )
        return TeamColor;
    return TeamNone;
}

[[nodiscard]] constexpr int DecideVisMethod( int Walls ) noexcept {
    if ( Walls >= 6 )
        return VisRay;
    return VisAuto;
}

[[nodiscard]] constexpr bool IsVisReady( int Walls ) noexcept {
    return Walls >= 6;
}

[[nodiscard]] constexpr bool ShouldThrottleDecision( bool Ready, unsigned Now, unsigned NextTick ) noexcept {
    return Ready && ( Now < NextTick );
}

inline bool FormatExtraTeamInfo( int PtrN, int NameN, int ColorN, int Teamed, char* Out, int Cap ) noexcept {
    if ( !Out || Cap <= 0 )
        return false;
    int Res = snprintf( Out, Cap, "ptrs=%d names=%d colors=%d teamed=%d", PtrN, NameN, ColorN, Teamed );
    return Res > 0 && Res < Cap;
}

inline bool FormatExtraVisInfo( int Walls, char* Out, int Cap ) noexcept {
    if ( !Out || Cap <= 0 )
        return false;
    int Res = snprintf( Out, Cap, "walls=%d", Walls );
    return Res > 0 && Res < Cap;
}

inline const char* TeamWord( int How ) {
    if ( How == TeamPtr )
        return "ptr";
    if ( How == TeamName )
        return "name";
    if ( How == TeamColor )
        return "color";
    if ( How == TeamNone )
        return "none";
    return "auto";
}

inline const char* VisWord( int How ) {
    if ( How == VisRay )
        return "ray";
    if ( How == VisNone )
        return "none";
    return "auto";
}

inline int TeamFrom( const char* Word ) {
    if ( Word && !_stricmp( Word, "ptr" ) )
        return TeamPtr;
    if ( Word && !_stricmp( Word, "name" ) )
        return TeamName;
    if ( Word && !_stricmp( Word, "color" ) )
        return TeamColor;
    if ( Word && !_stricmp( Word, "none" ) )
        return TeamNone;
    return TeamAuto;
}

inline int VisFrom( const char* Word ) {
    if ( Word && !_stricmp( Word, "ray" ) )
        return VisRay;
    if ( Word && !_stricmp( Word, "none" ) )
        return VisNone;
    return VisAuto;
}

inline bool LoadKind( uint64_t Place, const char* Kind, char* Method, int Cap ) {
    if ( Method && Cap > 0 )
        Method[ 0 ] = 0;
    char Path[ MAX_PATH ] = { };
    if ( !PathOf( Place, Kind, Path, MAX_PATH ) )
        return false;
    FILE* File = nullptr;
    if ( fopen_s( &File, Path, "rb" ) != 0 || !File )
        return false;
    char Body[ 256 ] = { };
    size_t Got = fread( Body, 1, sizeof( Body ) - 1, File );
    fclose( File );
    Body[ Got ] = 0;
    return ExtractMethodValue( Body, Method, Cap );
}

inline void SaveKind( uint64_t Place, const char* Kind, const char* Method, const char* Extra ) {
    char Path[ MAX_PATH ] = { };
    if ( !PathOf( Place, Kind, Path, MAX_PATH ) )
        return;
    FILE* File = nullptr;
    if ( fopen_s( &File, Path, "wb" ) != 0 || !File )
        return;
    fprintf( File, "place=%llu\r\nmethod=%s\r\n", ( unsigned long long )Place, Method ? Method : "none" );
    if ( Extra && Extra[ 0 ] )
        fprintf( File, "%s\r\n", Extra );
    fclose( File );
}

inline void BindPlace( uint64_t Place ) {
    Card& S = Live( );
    if ( Place == S.place && ( S.teamReady || S.visReady ) )
        return;
    S.place = Place;
    S.teamHow = TeamAuto;
    S.visHow = VisAuto;
    S.teamReady = false;
    S.visReady = false;
    S.nextTeam = 0;
    S.nextVis = 0;
    if ( !Place )
        return;
    char Method[ 24 ] = { };
    if ( LoadKind( Place, "team", Method, ( int )sizeof( Method ) ) ) {
        S.teamHow = TeamFrom( Method );
        S.teamReady = S.teamHow != TeamAuto;
    }
    if ( LoadKind( Place, "vis", Method, ( int )sizeof( Method ) ) ) {
        int How = VisFrom( Method );
        if ( How == VisNone )
            How = VisAuto;
        S.visHow = How;
        S.visReady = How == VisRay;
    }
}

inline void DecideTeam( uint64_t Place, int PtrN, int NameN, int ColorN, int Teamed ) {
    Card& S = Live( );
    if ( !Place )
        return;
    BindPlace( Place );
    unsigned Now = GetTickCount( );
    if ( ShouldThrottleDecision( S.teamReady, Now, S.nextTeam ) )
        return;
    S.nextTeam = Now + 8000;
    int How = DecideTeamMethod( PtrN, NameN, ColorN, Teamed );
    S.teamHow = How;
    S.teamReady = true;
    char Extra[ 80 ];
    FormatExtraTeamInfo( PtrN, NameN, ColorN, Teamed, Extra, sizeof( Extra ) );
    SaveKind( Place, "team", TeamWord( How ), Extra );
}

inline void DecideVis( uint64_t Place, int Walls ) {
    Card& S = Live( );
    if ( !Place )
        return;
    BindPlace( Place );
    unsigned Now = GetTickCount( );
    if ( ShouldThrottleDecision( S.visReady, Now, S.nextVis ) )
        return;
    S.nextVis = Now + 8000;
    S.visHow = DecideVisMethod( Walls );
    S.visReady = IsVisReady( Walls );
    if ( S.visReady ) {
        char Extra[ 48 ];
        FormatExtraVisInfo( Walls, Extra, sizeof( Extra ) );
        SaveKind( Place, "vis", VisWord( S.visHow ), Extra );
    }
}

}
