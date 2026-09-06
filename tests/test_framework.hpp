#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

namespace test {

struct TestCase {
    const char* name;
    void ( *fn )( );
};

inline std::vector< TestCase >& Registry( ) {
    static std::vector< TestCase > list;
    return list;
}

struct AutoReg {
    AutoReg( const char* Name, void ( *Fn )( ) ) {
        Registry( ).push_back( { Name, Fn } );
    }
};

inline int g_AssertPass = 0;
inline int g_AssertFail = 0;

inline void RecordAssert( bool Ok, const char* Expr, const char* File, int Line, const std::string& Extra = "" ) {
    if ( Ok ) {
        g_AssertPass++;
    } else {
        g_AssertFail++;
        printf( "  [FAIL] %s:%d: Assertion '%s' failed", File, Line, Expr );
        if ( !Extra.empty( ) )
            printf( " (%s)", Extra.c_str( ) );
        printf( "\n" );
    }
}

inline bool FloatClose( float A, float B, float Eps = 0.001f ) {
    return std::fabs( A - B ) <= Eps;
}

inline int RunAll( ) {
    printf( "\n==================================================\n" );
    printf( " Running Unlinked Unit Test Suite\n" );
    printf( "==================================================\n\n" );

    int CasesRun = 0;
    int CasesPassed = 0;

    for ( const auto& Case : Registry( ) ) {
        int PrevFails = g_AssertFail;
        printf( "[RUN ] %s ... ", Case.name );
        Case.fn( );
        CasesRun++;
        if ( g_AssertFail == PrevFails ) {
            CasesPassed++;
            printf( "PASSED\n" );
        } else {
            printf( "FAILED\n" );
        }
    }

    printf( "\n--------------------------------------------------\n" );
    printf( " Test Results: %d/%d test cases passed\n", CasesPassed, CasesRun );
    printf( " Assertions  : %d passed, %d failed\n", g_AssertPass, g_AssertFail );
    printf( "==================================================\n\n" );

    return g_AssertFail > 0 ? 1 : 0;
}

} // namespace test

#define TEST_CONCAT_INNER( a, b ) a##b
#define TEST_CONCAT( a, b ) TEST_CONCAT_INNER( a, b )

#define TEST_CASE( name ) \
    static void TEST_CONCAT( TestCase_, __LINE__ )( ); \
    static test::AutoReg TEST_CONCAT( AutoReg_, __LINE__ )( name, TEST_CONCAT( TestCase_, __LINE__ ) ); \
    static void TEST_CONCAT( TestCase_, __LINE__ )( )

#define CHECK( expr ) \
    test::RecordAssert( ( expr ), #expr, __FILE__, __LINE__ )

#define CHECK_EQ( a, b ) \
    test::RecordAssert( ( ( a ) == ( b ) ), #a " == " #b, __FILE__, __LINE__ )

#define CHECK_NE( a, b ) \
    test::RecordAssert( ( ( a ) != ( b ) ), #a " != " #b, __FILE__, __LINE__ )

#define CHECK_CLOSE( a, b, eps ) \
    test::RecordAssert( test::FloatClose( ( a ), ( b ), ( eps ) ), #a " =~ " #b, __FILE__, __LINE__ )
