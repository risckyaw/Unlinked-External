#pragma once

/**
 * @file key_labels.hpp
 * @brief Unlinked External - Key code naming and bitmask option formatting.
 */

#include <Windows.h>
#include <cstdio>
#include <cstring>

namespace ui {

inline const char* KeyLabel( int Code ) {
    switch ( Code ) {
    case VK_LBUTTON: return "Mouse 1";
    case VK_RBUTTON: return "Mouse 2";
    case VK_MBUTTON: return "Mouse 3";
    case VK_XBUTTON1: return "Mouse 4";
    case VK_XBUTTON2: return "Mouse 5";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "Page Up";
    case VK_NEXT: return "Page Down";
    case VK_SPACE: return "Space";
    case VK_TAB: return "Tab";
    case VK_RETURN: return "Enter";
    case VK_BACK: return "Backspace";
    case VK_PAUSE: return "Pause";
    case VK_SNAPSHOT: return "Print Screen";
    case VK_LEFT: return "Left";
    case VK_RIGHT: return "Right";
    case VK_UP: return "Up";
    case VK_DOWN: return "Down";
    case VK_SHIFT:
    case VK_LSHIFT: return "Shift";
    case VK_RSHIFT: return "Right Shift";
    case VK_CONTROL:
    case VK_LCONTROL: return "Ctrl";
    case VK_RCONTROL: return "Right Ctrl";
    case VK_MENU:
    case VK_LMENU: return "Alt";
    case VK_RMENU: return "Right Alt";
    case VK_CAPITAL: return "Caps Lock";
    case VK_NUMLOCK: return "Num Lock";
    case VK_SCROLL: return "Scroll Lock";
    case VK_OEM_1: return ";";
    case VK_OEM_PLUS: return "=";
    case VK_OEM_COMMA: return ",";
    case VK_OEM_MINUS: return "-";
    case VK_OEM_PERIOD: return ".";
    case VK_OEM_2: return "/";
    case VK_OEM_3: return "";
    case VK_OEM_4: return "[";
    case VK_OEM_5: return "\\";
    case VK_OEM_6: return "]";
    case VK_OEM_7: return "'";
    case VK_MULTIPLY: return "Num *";
    case VK_ADD: return "Num +";
    case VK_SUBTRACT: return "Num -";
    case VK_DECIMAL: return "Num .";
    case VK_DIVIDE: return "Num /";
    default:
        break;
    }

    if ( Code >= VK_NUMPAD0 && Code <= VK_NUMPAD9 ) {
        static char Num[ 8 ];
        snprintf( Num, sizeof( Num ), "Num %d", Code - VK_NUMPAD0 );
        return Num;
    }

    if ( Code >= VK_F1 && Code <= VK_F24 ) {
        static char Fn[ 8 ];
        snprintf( Fn, sizeof( Fn ), "F%d", Code - VK_F1 + 1 );
        return Fn;
    }

    if ( Code >= '0' && Code <= '9' ) {
        static char Digit[ 2 ];
        Digit[ 0 ] = ( char )Code;
        Digit[ 1 ] = 0;
        return Digit;
    }

    if ( Code >= 'A' && Code <= 'Z' ) {
        static char Letter[ 2 ];
        Letter[ 0 ] = ( char )Code;
        Letter[ 1 ] = 0;
        return Letter;
    }

    static char Fallback[ 16 ];
    snprintf( Fallback, sizeof( Fallback ), "Key %d", Code );
    return Fallback;
}

inline const char* BitLabel( const char* const* Options, int Count, int Bits ) {
    static char Line[ 96 ];
    Line[ 0 ] = 0;
    int Used = 0;
    for ( int Index = 0; Index < Count; Index++ ) {
        if ( ( Bits & ( 1 << Index ) ) == 0 )
            continue;
        char Piece[ 96 ];
        snprintf( Piece, sizeof( Piece ), "%s%s", Used ? ", " : "", Options[ Index ] );
        size_t Have = strlen( Line );
        snprintf( Line + Have, sizeof( Line ) - Have, "%s", Piece );
        Used += 1;
    }
    if ( !Used )
        snprintf( Line, sizeof( Line ), "None" );
    return Line;
}

inline bool DetectRisingEdge( bool Current, bool& Prior ) {
    bool Hit = Current && !Prior;
    Prior = Current;
    return Hit;
}

inline bool DetectFallingEdge( bool Current, bool& Prior ) {
    bool Hit = !Current && Prior;
    Prior = Current;
    return Hit;
}

template <typename KeyStateFn>
inline int PollKeyBind( bool AllowMouse1, KeyStateFn&& IsKeyDown, bool* KeyHistory, int MaxCode = 256 ) {
    int Hit = 0;
    for ( int Code = 1; Code < MaxCode; Code++ ) {
        if ( Code == VK_ESCAPE )
            continue;
        if ( Code == VK_LBUTTON && !AllowMouse1 )
            continue;
        bool Now = IsKeyDown( Code );
        if ( Now && !KeyHistory[ Code ] )
            Hit = Code;
        KeyHistory[ Code ] = Now;
    }
    return Hit;
}

template <typename KeyStateFn>
inline void SyncKeyHistory( KeyStateFn&& IsKeyDown, bool* KeyHistory, int MaxCode = 256 ) {
    for ( int Code = 1; Code < MaxCode; Code++ )
        KeyHistory[ Code ] = IsKeyDown( Code );
}

}
