/**
 * @file Format.cpp
 * @brief Unlinked UI Framework (UR) - engine/Text/Format/Format.cpp
 * 
 * Part of the Unlinked immediate-mode graphics and user interface framework.
 */

#include <cstdarg>
#include <cstdio>

#include "Arena.h"
#include "Format.h"

char* CFormat::Print( const char* Pattern, ... ) {
    va_list Details;
    va_start( Details, Pattern );

    va_list Probe;
    va_copy( Probe, Details );

    int Needed = vsnprintf( nullptr, 0, Pattern, Probe );
    va_end( Probe );

    if ( Needed < 0 ) {
        va_end( Details );

        char* Empty = ( char* )Arena->Allocate( 1 );
        Empty[ 0 ] = 0;

        return Empty;
    }

    char* Output = ( char* )Arena->Allocate( ( size_t )Needed + 1 );

    vsnprintf( Output, ( size_t )Needed + 1, Pattern, Details );
    va_end( Details );

    return Output;
}