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

TEST_CASE( "Silent: ExecProtect page permission decoding" ) {
    CHECK_EQ( silent::ExecProtect( PAGE_NOACCESS ), false );
    CHECK_EQ( silent::ExecProtect( PAGE_READONLY ), false );
    CHECK_EQ( silent::ExecProtect( PAGE_READWRITE ), false );
    CHECK_EQ( silent::ExecProtect( PAGE_WRITECOPY ), false );

    CHECK_EQ( silent::ExecProtect( PAGE_EXECUTE ), true );
    CHECK_EQ( silent::ExecProtect( PAGE_EXECUTE_READ ), true );
    CHECK_EQ( silent::ExecProtect( PAGE_EXECUTE_READWRITE ), true );
    CHECK_EQ( silent::ExecProtect( PAGE_EXECUTE_WRITECOPY ), true );

    // Mask with modifier bits (e.g. PAGE_GUARD or PAGE_NOCACHE)
    CHECK_EQ( silent::ExecProtect( PAGE_EXECUTE_READ | PAGE_GUARD ), true );
    CHECK_EQ( silent::ExecProtect( PAGE_READWRITE | PAGE_GUARD ), false );
}

TEST_CASE( "Silent: IsBufferPadding memory cave padding verification" ) {
    uint8_t PadCc[ 16 ];
    memset( PadCc, 0xCC, sizeof( PadCc ) );
    CHECK_EQ( silent::IsBufferPadding( PadCc, sizeof( PadCc ) ), true );

    uint8_t PadZero[ 16 ];
    memset( PadZero, 0x00, sizeof( PadZero ) );
    CHECK_EQ( silent::IsBufferPadding( PadZero, sizeof( PadZero ) ), true );

    // Mixed CC and 00 is not uniform padding
    uint8_t Mixed[ 16 ];
    memset( Mixed, 0xCC, sizeof( Mixed ) );
    Mixed[ 8 ] = 0x00;
    CHECK_EQ( silent::IsBufferPadding( Mixed, sizeof( Mixed ) ), false );

    // Arbitrary code/data
    uint8_t CodeData[ 4 ] = { 0x48, 0x89, 0x5C, 0x24 };
    CHECK_EQ( silent::IsBufferPadding( CodeData, sizeof( CodeData ) ), false );

    // Edge cases: null or zero size
    CHECK_EQ( silent::IsBufferPadding( nullptr, 16 ), false );
    CHECK_EQ( silent::IsBufferPadding( PadCc, 0 ), false );
}

TEST_CASE( "Silent: IsThunkPrologue x64 shellcode prologue pattern matching" ) {
    const uint8_t ValidPrologue[ 6 ] = { 0x48, 0x83, 0xEC, 0x68, 0x49, 0xBA };
    CHECK_EQ( silent::IsThunkPrologue( ValidPrologue, 6 ), true );
    CHECK_EQ( silent::IsThunkPrologue( ValidPrologue, 16 ), true );

    // Truncated buffer
    CHECK_EQ( silent::IsThunkPrologue( ValidPrologue, 5 ), false );
    CHECK_EQ( silent::IsThunkPrologue( nullptr, 6 ), false );

    // Byte mismatches
    uint8_t BadPrologue[ 6 ] = { 0x48, 0x83, 0xEC, 0x68, 0x49, 0xBB };
    CHECK_EQ( silent::IsThunkPrologue( BadPrologue, 6 ), false );

    BadPrologue[ 0 ] = 0x49;
    CHECK_EQ( silent::IsThunkPrologue( BadPrologue, 6 ), false );
}

TEST_CASE( "Silent: MousePosValid cursor coordinate sanity validation" ) {
    // Normal in-bounds cursor coordinates
    CHECK_EQ( silent::MousePosValid( 0.0f, 0.0f ), true );
    CHECK_EQ( silent::MousePosValid( 960.0f, 540.0f ), true );
    CHECK_EQ( silent::MousePosValid( 1920.0f, 1080.0f ), true );

    // Near boundary tests
    CHECK_EQ( silent::MousePosValid( -199.9f, 0.0f ), true );
    CHECK_EQ( silent::MousePosValid( 0.0f, 9999.9f ), true );

    // Out of bounds (< -200 or > 10000)
    CHECK_EQ( silent::MousePosValid( -200.0f, 500.0f ), false );
    CHECK_EQ( silent::MousePosValid( -250.0f, 500.0f ), false );
    CHECK_EQ( silent::MousePosValid( 500.0f, 10000.0f ), false );
    CHECK_EQ( silent::MousePosValid( 500.0f, 10050.0f ), false );
}

TEST_CASE( "Silent: MouseInWarpBounds aim intercept boundary checks" ) {
    float Sw = 1920.0f;
    float Sh = 1080.0f;

    // Inside screen bounds
    CHECK_EQ( silent::MouseInWarpBounds( 100.0f, 100.0f, Sw, Sh ), true );
    CHECK_EQ( silent::MouseInWarpBounds( 960.0f, 540.0f, Sw, Sh ), true );

    // Margin expansion (+200px beyond Sw/Sh)
    CHECK_EQ( silent::MouseInWarpBounds( 2000.0f, 1100.0f, Sw, Sh ), true );
    CHECK_EQ( silent::MouseInWarpBounds( 2119.0f, 1279.0f, Sw, Sh ), true );

    // Exceeding margin
    CHECK_EQ( silent::MouseInWarpBounds( 2121.0f, 540.0f, Sw, Sh ), false );
    CHECK_EQ( silent::MouseInWarpBounds( 960.0f, 1281.0f, Sw, Sh ), false );

    // Lower bound rejection (<= 1.0f)
    CHECK_EQ( silent::MouseInWarpBounds( 1.0f, 500.0f, Sw, Sh ), false );
    CHECK_EQ( silent::MouseInWarpBounds( 500.0f, 1.0f, Sw, Sh ), false );
    CHECK_EQ( silent::MouseInWarpBounds( 0.5f, 0.5f, Sw, Sh ), false );
}

TEST_CASE( "Silent: ScaleScreenCoords high-DPI and aspect ratio scaling" ) {
    // 1:1 identical viewport and client dimensions
    silent::ScreenScale Scale1 = silent::ScaleScreenCoords( 500.0f, 300.0f, 1920, 1080, 1920, 1080 );
    CHECK_CLOSE( Scale1.tx, 500.0f, 0.01f );
    CHECK_CLOSE( Scale1.ty, 300.0f, 0.01f );
    CHECK_CLOSE( Scale1.sw, 1920.0f, 0.01f );
    CHECK_CLOSE( Scale1.sh, 1080.0f, 0.01f );

    // 2x Retina / high-DPI scaling (View 960x540 -> Client 1920x1080)
    silent::ScreenScale Scale2 = silent::ScaleScreenCoords( 200.0f, 150.0f, 960, 540, 1920, 1080 );
    CHECK_CLOSE( Scale2.tx, 400.0f, 0.01f );
    CHECK_CLOSE( Scale2.ty, 300.0f, 0.01f );
    CHECK_CLOSE( Scale2.sw, 1920.0f, 0.01f );
    CHECK_CLOSE( Scale2.sh, 1080.0f, 0.01f );

    // Fallback when viewport dimension is small/invalid (<= 8)
    silent::ScreenScale Scale3 = silent::ScaleScreenCoords( 100.0f, 100.0f, 0, 0, 800, 600 );
    CHECK_CLOSE( Scale3.tx, 100.0f, 0.01f );
    CHECK_CLOSE( Scale3.ty, 100.0f, 0.01f );
    CHECK_CLOSE( Scale3.sw, 800.0f, 0.01f );
    CHECK_CLOSE( Scale3.sh, 600.0f, 0.01f );
}

TEST_CASE( "Silent: ComputeViewWarp viewport projection and ratio calculation" ) {
    float Sw = 1920.0f;
    float Sh = 1080.0f;
    float Cx = Sw * 0.5f; // 960.0f
    float Cy = Sh * 0.5f; // 540.0f

    // Center screen target: Ratio = Cy / Cy = 1.0; Vy = Sh * 1.0 = 1080; Vx = 2*Cx - (2*Cx - Sw) = Sw = 1920
    silent::ViewWarpResult Center = silent::ComputeViewWarp( Cx, Cy, Cx, Cy, Sw, Sh );
    CHECK_EQ( Center.valid, true );
    CHECK_EQ( Center.vx, ( int16_t )1920 );
    CHECK_EQ( Center.vy, ( int16_t )1080 );

    // Target below center (Ty > Cy): Ratio < 1 -> Vy < Sh
    silent::ViewWarpResult Lower = silent::ComputeViewWarp( Cx, 810.0f, Cx, Cy, Sw, Sh );
    CHECK_EQ( Lower.valid, true );
    // Ratio = 540 / 810 = 2/3. Vy = 1080 * 2/3 = 720. Vx = 1920.
    CHECK_EQ( Lower.vx, ( int16_t )1920 );
    CHECK_EQ( Lower.vy, ( int16_t )720 );

    // Invalid screen bounds (too small)
    silent::ViewWarpResult Invalid = silent::ComputeViewWarp( 100.0f, 100.0f, 50.0f, 50.0f, 4.0f, 4.0f );
    CHECK_EQ( Invalid.valid, false );
    CHECK_EQ( Invalid.vx, ( int16_t )0 );
    CHECK_EQ( Invalid.vy, ( int16_t )0 );

    // Clamp tests: Ty clamped to 1.0f and Sh - 1.0f
    silent::ViewWarpResult Clamped = silent::ComputeViewWarp( Cx, 0.0f, Cx, Cy, Sw, Sh );
    CHECK_EQ( Clamped.valid, true );
    // Ty clamped to 1.0f. Ratio = 540 / 1.0 = 540. Vy = 1080 * 540 = 583200 -> clamped to 32767.
    CHECK_EQ( Clamped.vy, ( int16_t )32767 );
}

