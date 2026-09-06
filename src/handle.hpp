#pragma once

/**
 * @file handle.hpp
 * @brief Zero-overhead RAII wrapper for Win32 HANDLE ensuring leak-free resource lifecycle.
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <utility>

namespace unlinked {

/**
 * @brief Scoped RAII wrapper for Win32 HANDLE.
 *
 * Automatically calls CloseHandle() upon destruction. Correctly handles both
 * nullptr and INVALID_HANDLE_VALUE as empty/invalid states.
 */
class UniqueHandle {
public:
    UniqueHandle( ) noexcept : handle_( nullptr ) { }

    explicit UniqueHandle( HANDLE h ) noexcept
        : handle_( ( h == INVALID_HANDLE_VALUE ) ? nullptr : h ) { }

    ~UniqueHandle( ) noexcept {
        reset( );
    }

    UniqueHandle( const UniqueHandle& ) = delete;
    UniqueHandle& operator=( const UniqueHandle& ) = delete;

    UniqueHandle( UniqueHandle&& other ) noexcept
        : handle_( other.release( ) ) { }

    UniqueHandle& operator=( UniqueHandle&& other ) noexcept {
        if ( this != &other ) {
            reset( other.release( ) );
        }
        return *this;
    }

    [[nodiscard]] HANDLE get( ) const noexcept {
        return handle_;
    }

    [[nodiscard]] bool is_valid( ) const noexcept {
        return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
    }

    explicit operator bool( ) const noexcept {
        return is_valid( );
    }

    HANDLE release( ) noexcept {
        HANDLE temp = handle_;
        handle_ = nullptr;
        return temp;
    }

    void reset( HANDLE h = nullptr ) noexcept {
        if ( handle_ && handle_ != INVALID_HANDLE_VALUE ) {
            CloseHandle( handle_ );
        }
        handle_ = ( h == INVALID_HANDLE_VALUE ) ? nullptr : h;
    }

    HANDLE* addressof( ) noexcept {
        reset( );
        return &handle_;
    }

private:
    HANDLE handle_ = nullptr;
};

} // namespace unlinked
