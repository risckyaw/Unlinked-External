#pragma once

/**
 * @file Pictures.h
 * @brief Unlinked UI Framework (UR) - engine/Ports/Platform/Pictures/Pictures.h
 * 
 * Part of the Unlinked immediate-mode graphics and user interface framework.
 */

#include <memory>
#include <vector>

class CPictures {
public:
    bool Load( const char* Path, std::vector< unsigned char >& Pixels, int& Width, int& Height, int Longest = 0 );
    bool Decode( const unsigned char* Bytes, size_t Length, std::vector< unsigned char >& Pixels, int& Width, int& Height, int Longest = 0 );

    bool Vector( const char* Path, std::vector< unsigned char >& Pixels, int& Width, int& Height, int Size );
    bool VectorBytes( const unsigned char* Bytes, size_t Length, std::vector< unsigned char >& Pixels, int& Width, int& Height, int Size );
};

inline auto Pictures = std::make_unique< CPictures >( );