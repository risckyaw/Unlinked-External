#include "test_framework.hpp"
#include "silent.hpp"

TEST_CASE( "Silent: RayState layout and alignment" ) {
    CHECK_EQ( sizeof( silent::RayState ), ( size_t )0x20 );
    CHECK_EQ( offsetof( silent::RayState, active ), ( size_t )0x00 );
    CHECK_EQ( offsetof( silent::RayState, reserved ), ( size_t )0x04 );
    CHECK_EQ( offsetof( silent::RayState, x ), ( size_t )0x08 );
    CHECK_EQ( offsetof( silent::RayState, y ), ( size_t )0x0C );
    CHECK_EQ( offsetof( silent::RayState, z ), ( size_t )0x10 );
    CHECK_EQ( offsetof( silent::RayState, scale ), ( size_t )0x14 );
    CHECK_EQ( offsetof( silent::RayState, calls ), ( size_t )0x18 );
}

TEST_CASE( "Silent: AppendU64 little-endian binary serialization" ) {
    std::vector< uint8_t > Code;
    silent::AppendU64( Code, 0x1122334455667788ULL );

    CHECK_EQ( Code.size( ), ( size_t )8 );
    CHECK_EQ( Code[ 0 ], ( uint8_t )0x88 );
    CHECK_EQ( Code[ 1 ], ( uint8_t )0x77 );
    CHECK_EQ( Code[ 2 ], ( uint8_t )0x66 );
    CHECK_EQ( Code[ 3 ], ( uint8_t )0x55 );
    CHECK_EQ( Code[ 4 ], ( uint8_t )0x44 );
    CHECK_EQ( Code[ 5 ], ( uint8_t )0x33 );
    CHECK_EQ( Code[ 6 ], ( uint8_t )0x22 );
    CHECK_EQ( Code[ 7 ], ( uint8_t )0x11 );
}

TEST_CASE( "Silent: PatchRel32 branch displacement calculation" ) {
    std::vector< uint8_t > Buffer( 32, 0 );

    // Forward jump: from At=4 to Till=20. Expected: 20 - (4 + 4) = 12
    silent::PatchRel32( Buffer, 4, 20 );
    int32_t RelForward = 0;
    memcpy( &RelForward, Buffer.data( ) + 4, 4 );
    CHECK_EQ( RelForward, 12 );

    // Zero displacement: Till == At + 4
    silent::PatchRel32( Buffer, 8, 12 );
    int32_t RelZero = -1;
    memcpy( &RelZero, Buffer.data( ) + 8, 4 );
    CHECK_EQ( RelZero, 0 );

    // Backward jump: from At=24 to Till=10. Expected: 10 - (24 + 4) = -18
    silent::PatchRel32( Buffer, 24, 10 );
    int32_t RelBack = 0;
    memcpy( &RelBack, Buffer.data( ) + 24, 4 );
    CHECK_EQ( RelBack, -18 );
}

TEST_CASE( "Silent: MakeThunk machine code generation and structure" ) {
    uintptr_t MockState = 0x7FFE10002000ULL;
    uintptr_t MockOrig = 0x7FFE50006000ULL;

    std::vector< uint8_t > Thunk = silent::MakeThunk( MockState, MockOrig );
    CHECK( Thunk.size( ) >= 64 );

    // Prologue: sub rsp, 0x68
    CHECK_EQ( Thunk[ 0 ], ( uint8_t )0x48 );
    CHECK_EQ( Thunk[ 1 ], ( uint8_t )0x83 );
    CHECK_EQ( Thunk[ 2 ], ( uint8_t )0xEC );
    CHECK_EQ( Thunk[ 3 ], ( uint8_t )0x68 );

    // mov r10, MockState (0x49 0xBA <8 bytes>)
    CHECK_EQ( Thunk[ 4 ], ( uint8_t )0x49 );
    CHECK_EQ( Thunk[ 5 ], ( uint8_t )0xBA );
    uint64_t StateInThunk = 0;
    memcpy( &StateInThunk, Thunk.data( ) + 6, 8 );
    CHECK_EQ( StateInThunk, ( uint64_t )MockState );

    // Epilogue: add rsp, 0x68; ret
    size_t N = Thunk.size( );
    CHECK_EQ( Thunk[ N - 1 ], ( uint8_t )0xC3 ); // ret
    CHECK_EQ( Thunk[ N - 5 ], ( uint8_t )0x48 ); // add rsp, 0x68
    CHECK_EQ( Thunk[ N - 4 ], ( uint8_t )0x83 );
    CHECK_EQ( Thunk[ N - 3 ], ( uint8_t )0xC4 );
    CHECK_EQ( Thunk[ N - 2 ], ( uint8_t )0x68 );
}
