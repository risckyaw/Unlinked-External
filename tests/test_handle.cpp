#include "test_framework.hpp"
#include "handle.hpp"

#include <Windows.h>

TEST_CASE( "Handle: Default construction and null state" ) {
    unlinked::UniqueHandle h;
    CHECK( !h.is_valid( ) );
    CHECK( !h );
    CHECK_EQ( h.get( ), nullptr );
}

TEST_CASE( "Handle: INVALID_HANDLE_VALUE conversion" ) {
    unlinked::UniqueHandle h( INVALID_HANDLE_VALUE );
    CHECK( !h.is_valid( ) );
    CHECK( !h );
    CHECK_EQ( h.get( ), nullptr );
}

TEST_CASE( "Handle: Valid event handle lifecycle and release" ) {
    HANDLE ev = CreateEventA( nullptr, TRUE, FALSE, nullptr );
    CHECK( ev != nullptr && ev != INVALID_HANDLE_VALUE );

    {
        unlinked::UniqueHandle h( ev );
        CHECK( h.is_valid( ) );
        CHECK( ( bool )h );
        CHECK_EQ( h.get( ), ev );

        HANDLE raw = h.release( );
        CHECK_EQ( raw, ev );
        CHECK( !h.is_valid( ) );
        CHECK_EQ( h.get( ), nullptr );

        // Close the released handle manually
        CloseHandle( raw );
    }
}

TEST_CASE( "Handle: Move semantics transfer ownership" ) {
    HANDLE ev = CreateEventA( nullptr, TRUE, FALSE, nullptr );
    CHECK( ev != nullptr && ev != INVALID_HANDLE_VALUE );

    unlinked::UniqueHandle h1( ev );
    CHECK_EQ( h1.get( ), ev );

    // Move construct
    unlinked::UniqueHandle h2( std::move( h1 ) );
    CHECK_EQ( h1.get( ), nullptr );
    CHECK( !h1.is_valid( ) );
    CHECK_EQ( h2.get( ), ev );
    CHECK( h2.is_valid( ) );

    // Move assign
    unlinked::UniqueHandle h3;
    h3 = std::move( h2 );
    CHECK_EQ( h2.get( ), nullptr );
    CHECK( !h2.is_valid( ) );
    CHECK_EQ( h3.get( ), ev );
    CHECK( h3.is_valid( ) );
}

TEST_CASE( "Handle: Reset semantics" ) {
    HANDLE ev1 = CreateEventA( nullptr, TRUE, FALSE, nullptr );
    HANDLE ev2 = CreateEventA( nullptr, TRUE, FALSE, nullptr );

    unlinked::UniqueHandle h( ev1 );
    CHECK_EQ( h.get( ), ev1 );

    // Reset with another handle (closes ev1)
    h.reset( ev2 );
    CHECK_EQ( h.get( ), ev2 );

    // Reset to null (closes ev2)
    h.reset( );
    CHECK( !h.is_valid( ) );
    CHECK_EQ( h.get( ), nullptr );
}
