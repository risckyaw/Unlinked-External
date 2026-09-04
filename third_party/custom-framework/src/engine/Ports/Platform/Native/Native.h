#pragma once

/**
 * @file Native.h
 * @brief Unlinked UI Framework (UR) - engine/Ports/Platform/Native/Native.h
 * 
 * Part of the Unlinked immediate-mode graphics and user interface framework.
 */

#include <memory>
#include <string>

class CNative {
public:
    bool Create( void* Window );
    void Destroy( );

    bool Translate( void* Window, unsigned int Message, unsigned long long Primary, long long Secondary );
    float Scale( ) const;

    std::string ClipboardGet( ) const;
    void ClipboardSet( const char* Text ) const;

private:
    void* Handle = nullptr;
    bool Tracking = false;
};

inline auto Native = std::make_unique< CNative >( );