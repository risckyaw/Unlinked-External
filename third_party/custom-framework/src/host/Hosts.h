#pragma once

/**
 * @file Hosts.h
 * @brief Unlinked UI Framework (UR) - host/Hosts.h
 * 
 * Part of the Unlinked immediate-mode graphics and user interface framework.
 */

#include "Engine.h"

class CHost {
public:
    virtual ~CHost( ) = default;

    virtual bool Create( void* Window, int Width, int Height ) = 0;
    virtual void Destroy( ) = 0;

    virtual void Resize( int Width, int Height ) = 0;
    virtual void Begin( CColor Backdrop ) = 0;

    virtual void* Stream( ) = 0;
    virtual CGraphics* Graphics( ) = 0;

    virtual void End( bool VerticalSync ) = 0;
};