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

inline double ComputeFrameGoalTime( float FpsLimit ) {
    return 1.0 / ( double )ClampFpsLimit( FpsLimit );
}

inline int ComputeFramePacingAction( double GoalTime, double SpentTime ) {
    if ( SpentTime >= GoalTime )
        return 0;
    if ( GoalTime - SpentTime > 0.002 )
        return 1;
    return 2;
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

inline constexpr double DefaultFpsCap = 240.0;
inline constexpr double UncappedFps = 10000.0;
inline constexpr unsigned int UncapThrottleMs = 2500;
inline constexpr double AfkPulseIntervalSec = 18.0;

[[nodiscard]] inline bool ShouldTriggerAfkPulse( bool On, double& Wait, double Dt, double ThresholdSec = 18.0 ) noexcept {
    if ( !On ) {
        Wait = 0.0;
        return false;
    }
    Wait += Dt;
    if ( Wait < ThresholdSec )
        return false;
    Wait = 0.0;
    return true;
}

[[nodiscard]] inline POINT ComputeWindowCenterPoint( const RECT& Box ) noexcept {
    POINT Pt;
    Pt.x = ( Box.right - Box.left ) / 2;
    Pt.y = ( Box.bottom - Box.top ) / 2;
    return Pt;
}

[[nodiscard]] inline bool ShouldThrottleUncap( bool Applied, unsigned Now, unsigned NextDeadline ) noexcept {
    return Applied && ( Now < NextDeadline );
}

[[nodiscard]] inline bool FormatRobloxSettingsPath( const char* Root, char* Out, size_t Cap ) noexcept {
    if ( !Root || !Root[ 0 ] || !Out || Cap == 0 )
        return false;
    int Res = snprintf( Out, Cap, "%s\\Roblox\\GlobalBasicSettings_13.xml", Root );
    return Res > 0 && ( size_t )Res < Cap;
}

[[nodiscard]] inline bool IsValidXmlFileSize( DWORD Size, DWORD MaxSize = 1u << 20 ) noexcept {
    return Size > 0 && Size <= MaxSize;
}

inline void PatchXml( bool Uncap ) {
    char Path[ MAX_PATH ] = { };
    char Root[ MAX_PATH ] = { };
    if ( GetEnvironmentVariableA( "LOCALAPPDATA", Root, MAX_PATH ) == 0 || !Root[ 0 ] )
        return;
    if ( !FormatRobloxSettingsPath( Root, Path, sizeof( Path ) ) )
        return;
    unlinked::UniqueHandle File( CreateFileA( Path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr ) );
    if ( !File )
        return;
    DWORD Size = GetFileSize( File.get( ), nullptr );
    if ( !IsValidXmlFileSize( Size ) ) {
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
    if ( !ShouldTriggerAfkPulse( On, Wait, Dt, AfkPulseIntervalSec ) )
        return;

    HWND Window = world::GameWindow( );
    if ( !Window )
        return;

    RECT Box = { };
    GetClientRect( Window, &Box );
    POINT Center = ComputeWindowCenterPoint( Box );
    LPARAM Spot = MAKELPARAM( Center.x, Center.y );
    PostMessageW( Window, WM_MOUSEMOVE, 0, Spot );
    PostMessageW( Window, WM_RBUTTONDOWN, MK_RBUTTON, Spot );
    PostMessageW( Window, WM_RBUTTONUP, 0, Spot );
}

inline void TickUncap( bool On ) {
    static int Applied = 0;
    static unsigned Next = 0;
    if ( !On ) {
        if ( Applied && world::Attach( ) ) {
            world::SetFps( DefaultFpsCap );
            PatchXml( false );
        }
        Applied = 0;
        Next = 0;
        return;
    }
    unsigned Now = GetTickCount( );
    if ( ShouldThrottleUncap( Applied != 0, Now, Next ) )
        return;
    if ( !world::Attach( ) )
        return;
    if ( !world::SetFps( UncappedFps ) )
        return;
    if ( !Applied )
        PatchXml( true );
    Applied = 1;
    Next = Now + UncapThrottleMs;
}

}