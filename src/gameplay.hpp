#pragma once

/**
 * @file gameplay.hpp
 * @brief Unlinked External - Player physics modifiers, jump power adjustments, and movement hooks.
 */

#include "world.hpp"

#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace play {

inline bool UpdateFramerateCap( std::string& Body, bool Uncap ) {
    const char* Key = "<int name=\"FramerateCap\">";
    size_t At = Body.find( Key );
    if ( At == std::string::npos )
        return false;
    size_t Start = At + strlen( Key );
    size_t End = Body.find( "</", Start );
    if ( End == std::string::npos )
        return false;
    Body.replace( Start, End - Start, Uncap ? "10000" : "240" );
    return true;
}

struct FpsFilter {
    float smooth = 0.0f;
    float shown = 0.0f;
    float wait = 0.0f;

    void Reset( ) {
        smooth = 0.0f;
        shown = 0.0f;
        wait = 0.0f;
    }

    float Update( float DeltaTime, float FallbackFps ) {
        float Instant = DeltaTime > 0.00005f ? 1.0f / DeltaTime : FallbackFps;
        if ( Instant < 1.0f )
            Instant = FallbackFps;
        if ( smooth < 1.0f )
            smooth = Instant;
        else {
            float Rate = DeltaTime * 1.4f;
            if ( Rate > 0.08f )
                Rate = 0.08f;
            smooth += ( Instant - smooth ) * Rate;
        }
        wait += DeltaTime;
        if ( wait >= 0.4f || shown < 1.0f ) {
            shown = smooth;
            wait = 0.0f;
        }
        return shown;
    }
};

inline float ClampFpsLimit( float Fps ) {
    if ( Fps < 60.0f )
        return 60.0f;
    if ( Fps > 1000.0f )
        return 1000.0f;
    return Fps;
}

inline bool FormatWatermark( bool Watermark, bool ShowFps, float Fps, char* Out, size_t Cap ) {
    if ( !Out || Cap == 0 )
        return false;
    if ( Fps < 0.0f )
        Fps = 0.0f;
    if ( Watermark && ShowFps )
        snprintf( Out, Cap, "Unlinked   %.0f fps", ( double )Fps );
    else if ( Watermark )
        snprintf( Out, Cap, "Unlinked" );
    else if ( ShowFps )
        snprintf( Out, Cap, "%.0f fps", ( double )Fps );
    else {
        Out[ 0 ] = 0;
        return false;
    }
    return true;
}

inline void PatchXml( bool Uncap ) {
    char Path[ MAX_PATH ] = { };
    char Root[ MAX_PATH ] = { };
    if ( GetEnvironmentVariableA( "LOCALAPPDATA", Root, MAX_PATH ) == 0 || !Root[ 0 ] )
        return;
    snprintf( Path, sizeof( Path ), "%s\\Roblox\\GlobalBasicSettings_13.xml", Root );
    unlinked::UniqueHandle File( CreateFileA( Path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) );
    if ( !File )
        return;
    DWORD Size = GetFileSize( File.get( ), nullptr );
    if ( Size == 0 || Size > 1u << 20 ) {
        return;
    }
    std::string Body;
    Body.resize( Size );
    DWORD Got = 0;
    if ( !ReadFile( File.get( ), Body.data( ), Size, &Got, nullptr ) ) {
        return;
    }
    File.reset( );

    if ( !UpdateFramerateCap( Body, Uncap ) )
        return;

    File.reset( CreateFileA( Path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr ) );
    if ( !File )
        return;
    DWORD Put = 0;
    WriteFile( File.get( ), Body.data( ), ( DWORD )Body.size( ), &Put, nullptr );
}

inline void TickAfk( bool On, double Dt ) {
    static double Wait = 0.0;
    if ( !On ) {
        Wait = 0.0;
        return;
    }
    Wait += Dt;
    if ( Wait < 18.0 )
        return;
    Wait = 0.0;

    HWND Window = world::GameWindow( );
    if ( !Window )
        return;

    RECT Box = { };
    GetClientRect( Window, &Box );
    int X = ( Box.right - Box.left ) / 2;
    int Y = ( Box.bottom - Box.top ) / 2;
    LPARAM Spot = MAKELPARAM( X, Y );
    PostMessageW( Window, WM_MOUSEMOVE, 0, Spot );
    PostMessageW( Window, WM_RBUTTONDOWN, MK_RBUTTON, Spot );
    PostMessageW( Window, WM_RBUTTONUP, 0, Spot );
}

inline void TickUncap( bool On ) {
    static int Applied = 0;
    static unsigned Next = 0;
    if ( !On ) {
        if ( Applied && world::Attach( ) ) {
            world::SetFps( 240.0 );
            PatchXml( false );
        }
        Applied = 0;
        Next = 0;
        return;
    }
    unsigned Now = GetTickCount( );
    if ( Applied && Now < Next )
        return;
    if ( !world::Attach( ) )
        return;
    if ( !world::SetFps( 10000.0 ) )
        return;
    if ( !Applied )
        PatchXml( true );
    Applied = 1;
    Next = Now + 2500;
}

}