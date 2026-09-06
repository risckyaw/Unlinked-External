#pragma once

/**
 * @file tab_configs.hpp
 * @brief Unlinked External - Configuration profile management, serialization, and preset library tab.
 */

static void PackPut( char* Out, int Cap, const char* Key, int Value ) {
    char Line[ 64 ];
    snprintf( Line, sizeof( Line ), "%s %d\n", Key, Value );
    size_t Have = strlen( Out );
    snprintf( Out + Have, ( size_t )Cap - Have, "%s", Line );
}

static void PackState( char* Out, int Cap ) {
    Out[ 0 ] = 0;
    PackPut( Out, Cap, "menuKey", Menu.menuKey );
    PackPut( Out, Cap, "limit", Menu.limit ? 1 : 0 );
    PackPut( Out, Cap, "fps", ( int )Menu.fps );
    PackPut( Out, Cap, "afk", Menu.afk ? 1 : 0 );
    PackPut( Out, Cap, "uncap", Menu.uncap ? 1 : 0 );
    PackPut( Out, Cap, "vsync", Menu.vsync ? 1 : 0 );
    PackPut( Out, Cap, "watermark", Menu.watermark ? 1 : 0 );
    PackPut( Out, Cap, "showFps", Menu.showFps ? 1 : 0 );
    PackPut( Out, Cap, "stream", Menu.stream ? 1 : 0 );
    PackPut( Out, Cap, "fade", ( int )Menu.fade );
    PackPut( Out, Cap, "tone", skin::tone( ) );
    PackPut( Out, Cap, "look", skin::look( ) );
    PackPut( Out, Cap, "weather", weather::mode( ) );
    PackPut( Out, Cap, "aim.on", Aim.on ? 1 : 0 );
    PackPut( Out, Cap, "aim.team", Aim.team ? 1 : 0 );
    PackPut( Out, Cap, "aim.vis", Aim.vis ? 1 : 0 );
    PackPut( Out, Cap, "aim.sticky", Aim.sticky ? 1 : 0 );
    PackPut( Out, Cap, "aim.pred", Aim.pred ? 1 : 0 );
    PackPut( Out, Cap, "aim.drawFov", Aim.drawFov ? 1 : 0 );
    PackPut( Out, Cap, "aim.fov", ( int )Aim.fov );
    PackPut( Out, Cap, "aim.smooth", ( int )Aim.smooth );
    PackPut( Out, Cap, "aim.key", Aim.key );
    PackPut( Out, Cap, "aim.bones", Aim.bones );
    PackPut( Out, Cap, "aim.sort", Aim.sort );
    PackPut( Out, Cap, "silent.on", Mute.on ? 1 : 0 );
    PackPut( Out, Cap, "silent.team", Mute.team ? 1 : 0 );
    PackPut( Out, Cap, "silent.vis", Mute.vis ? 1 : 0 );
    PackPut( Out, Cap, "silent.pred", Mute.pred ? 1 : 0 );
    PackPut( Out, Cap, "silent.key", Mute.key );
    PackPut( Out, Cap, "silent.bones", Mute.bones );
    PackPut( Out, Cap, "silent.sort", Mute.sort );
    {
        move::Cfg& Move = move::Live( );
        PackPut( Out, Cap, "move.jump", Move.jump ? 1 : 0 );
        PackPut( Out, Cap, "move.jumpPower", ( int )Move.jumpPower );
        PackPut( Out, Cap, "move.infJump", Move.infJump ? 1 : 0 );
        PackPut( Out, Cap, "move.noclip", Move.noclip ? 1 : 0 );
    }
    PackPut( Out, Cap, "esp.on", Esp.on ? 1 : 0 );
    PackPut( Out, Cap, "esp.box", Esp.box ? 1 : 0 );
    PackPut( Out, Cap, "esp.name", Esp.name ? 1 : 0 );
    PackPut( Out, Cap, "esp.health", Esp.health ? 1 : 0 );
    PackPut( Out, Cap, "esp.dist", Esp.dist ? 1 : 0 );
    PackPut( Out, Cap, "esp.skel", Esp.skeleton ? 1 : 0 );
    PackPut( Out, Cap, "esp.snap", Esp.snap ? 1 : 0 );
    PackPut( Out, Cap, "esp.team", Esp.team ? 1 : 0 );
    PackPut( Out, Cap, "esp.range", ( int )Esp.range );
    PackPut( Out, Cap, "esp.feat", Dye.feat );
    PackPut( Out, Cap, "esp.globVis", Dye.globVis );
    PackPut( Out, Cap, "esp.globHid", Dye.globHid );
    for ( int Index = 0; Index < FeatCount; Index++ ) {
        char Key[ 24 ];
        snprintf( Key, sizeof( Key ), "esp.vis.%d", Index );
        PackPut( Out, Cap, Key, Dye.vis[ Index ] );
        snprintf( Key, sizeof( Key ), "esp.hid.%d", Index );
        PackPut( Out, Cap, Key, Dye.hid[ Index ] );
    }
    PackPut( Out, Cap, "mark.x", ( int )( Badge.origin.Horizontal + 0.5f ) );
    PackPut( Out, Cap, "mark.y", ( int )( Badge.origin.Vertical + 0.5f ) );
}

static void ApplyState( const char* Body ) {
    if ( !Body )
        return;
    store::TakeB( Body, "limit", Menu.limit );
    store::TakeF( Body, "fps", Menu.fps );
    store::TakeB( Body, "afk", Menu.afk );
    store::TakeB( Body, "uncap", Menu.uncap );
    store::TakeB( Body, "vsync", Menu.vsync );
    store::TakeB( Body, "watermark", Menu.watermark );
    store::TakeB( Body, "showFps", Menu.showFps );
    store::TakeB( Body, "stream", Menu.stream );
    store::TakeF( Body, "fade", Menu.fade );
    store::Take( Body, "tone", skin::tone( ) );
    store::Take( Body, "look", skin::look( ) );
    store::Take( Body, "weather", weather::mode( ) );
    store::Take( Body, "menuKey", Menu.menuKey );
    store::TakeB( Body, "aim.on", Aim.on );
    store::TakeB( Body, "aim.team", Aim.team );
    store::TakeB( Body, "aim.vis", Aim.vis );
    store::TakeB( Body, "aim.sticky", Aim.sticky );
    store::TakeB( Body, "aim.pred", Aim.pred );
    store::TakeB( Body, "aim.drawFov", Aim.drawFov );
    store::TakeF( Body, "aim.fov", Aim.fov );
    store::TakeF( Body, "aim.smooth", Aim.smooth );
    store::Take( Body, "aim.key", Aim.key );
    store::Take( Body, "aim.bones", Aim.bones );
    store::Take( Body, "aim.sort", Aim.sort );
    store::TakeB( Body, "aim.silent", Mute.on );
    store::TakeB( Body, "silent.on", Mute.on );
    store::TakeB( Body, "silent.team", Mute.team );
    store::TakeB( Body, "silent.vis", Mute.vis );
    store::TakeB( Body, "silent.pred", Mute.pred );
    store::Take( Body, "silent.key", Mute.key );
    store::Take( Body, "silent.bones", Mute.bones );
    store::Take( Body, "silent.sort", Mute.sort );
    {
        move::Cfg& Move = move::Live( );
        store::TakeB( Body, "move.jump", Move.jump );
        store::TakeF( Body, "move.jumpPower", Move.jumpPower );
        store::TakeB( Body, "move.infJump", Move.infJump );
        store::TakeB( Body, "move.noclip", Move.noclip );
        move::Clamp( );
    }
    store::TakeB( Body, "esp.on", Esp.on );
    store::TakeB( Body, "esp.box", Esp.box );
    store::TakeB( Body, "esp.name", Esp.name );
    store::TakeB( Body, "esp.health", Esp.health );
    store::TakeB( Body, "esp.dist", Esp.dist );
    store::TakeB( Body, "esp.skel", Esp.skeleton );
    store::TakeB( Body, "esp.snap", Esp.snap );
    store::TakeB( Body, "esp.team", Esp.team );
    store::TakeF( Body, "esp.range", Esp.range );
    store::Take( Body, "esp.feat", Dye.feat );
    store::Take( Body, "esp.globVis", Dye.globVis );
    store::Take( Body, "esp.globHid", Dye.globHid );
    for ( int Index = 0; Index < FeatCount; Index++ ) {
        char Key[ 24 ];
        snprintf( Key, sizeof( Key ), "esp.vis.%d", Index );
        store::Take( Body, Key, Dye.vis[ Index ] );
        snprintf( Key, sizeof( Key ), "esp.hid.%d", Index );
        store::Take( Body, Key, Dye.hid[ Index ] );
    }
    int MarkX = ( int )Badge.origin.Horizontal;
    int MarkY = ( int )Badge.origin.Vertical;
    if ( store::Take( Body, "mark.x", MarkX ) && store::Take( Body, "mark.y", MarkY ) ) {
        Badge.origin = CVector( ( float )MarkX, ( float )MarkY );
        Badge.ready = true;
    }
    if ( skin::look( ) < 0 || skin::look( ) >= skin::LookCount )
        skin::look( ) = 0;
    if ( skin::tone( ) < 0 || skin::tone( ) >= skin::ToneCount )
        skin::tone( ) = 0;
    if ( weather::mode( ) < 0 || weather::mode( ) >= weather::ModeCount )
        weather::mode( ) = weather::Snow;
    if ( Menu.fps < 60.0f )
        Menu.fps = 60.0f;
    if ( Menu.fade < 40.0f )
        Menu.fade = 40.0f;
    if ( Esp.range < 25.0f )
        Esp.range = 25.0f;
    if ( Esp.range > 2000.0f )
        Esp.range = 2000.0f;
    if ( Aim.smooth < 0.0f )
        Aim.smooth = 0.0f;
    if ( Aim.smooth > 100.0f )
        Aim.smooth = 100.0f;
    if ( Aim.fov < 10.0f )
        Aim.fov = 10.0f;
    if ( Aim.fov > 360.0f )
        Aim.fov = 360.0f;
    if ( Aim.bones == 0 )
        Aim.bones = 1;
    if ( Aim.sort < 0 || Aim.sort > 2 )
        Aim.sort = 0;
    if ( Mute.bones == 0 )
        Mute.bones = 1;
    if ( Mute.sort < 0 || Mute.sort > 2 )
        Mute.sort = 0;
    if ( !Mute.key )
        Mute.key = 'M';
    move::Clamp( );
}

static void PackNote( const char* Text ) {
    lstrcpynA( Packs.note, Text ? Text : "", ( int )sizeof( Packs.note ) );
    Packs.noteAge = 2.4f;
}

static void PackRefresh( ) {
    Packs.count = store::List( Packs.names );
    if ( Packs.pick >= Packs.count )
        Packs.pick = Packs.count > 0 ? Packs.count - 1 : 0;
    for ( int Index = 0; Index < Packs.count; Index++ ) {
        if ( _stricmp( Packs.names[ Index ], Packs.live ) == 0 )
            Packs.pick = Index;
    }
}

static bool PackSave( const char* Name ) {
    char Body[ store::BodyCap ];
    PackState( Body, store::BodyCap );
    if ( !store::Write( Name, Body ) )
        return false;
    lstrcpynA( Packs.live, Name, store::NameCap );
    store::SetCurrent( Name );
    PackRefresh( );
    return true;
}

static bool PackLoad( const char* Name ) {
    char Body[ store::BodyCap ];
    if ( !store::Read( Name, Body, store::BodyCap ) )
        return false;
    ApplyState( Body );
    lstrcpynA( Packs.live, Name, store::NameCap );
    store::SetCurrent( Name );
    PackRefresh( );
    Tokens( );
    return true;
}

static void PackBoot( ) {
    if ( Packs.ready )
        return;
    PackRefresh( );
    char Last[ store::NameCap ] = { };
    if ( store::Current( Last, store::NameCap ) && PackLoad( Last ) ) {
        Packs.ready = true;
        if ( Menu.visible )
            OpenLiveFolds( );
        return;
    }
    if ( Packs.count > 0 && PackLoad( Packs.names[ 0 ] ) ) {
        Packs.ready = true;
        if ( Menu.visible )
            OpenLiveFolds( );
        return;
    }
    lstrcpynA( Packs.live, "Default", store::NameCap );
    PackSave( "Default" );
    Packs.ready = true;
    if ( Menu.visible )
        OpenLiveFolds( );
}

static void PackDraft( ) {
    if ( !Packs.type )
        return;
    if ( Edge( VK_BACK, KeyWas[ VK_BACK ] ) ) {
        size_t Len = strlen( Packs.draft );
        if ( Len )
            Packs.draft[ Len - 1 ] = 0;
        return;
    }
    if ( Edge( VK_RETURN, KeyWas[ VK_RETURN ] ) ) {
        store::Sanitize( Packs.draft );
        if ( store::Valid( Packs.draft ) && PackSave( Packs.draft ) )
            PackNote( "Created" );
        Packs.type = false;
        return;
    }
    bool Shift = Held( VK_SHIFT );
    for ( int Code = 'A'; Code <= 'Z'; Code++ ) {
        if ( !Edge( Code, KeyWas[ Code ] ) )
            continue;
        size_t Len = strlen( Packs.draft );
        if ( Len >= store::NameCap - 1 )
            return;
        Packs.draft[ Len ] = ( char )( Shift ? Code : Code + 32 );
        Packs.draft[ Len + 1 ] = 0;
        return;
    }
    for ( int Code = '0'; Code <= '9'; Code++ ) {
        if ( !Edge( Code, KeyWas[ Code ] ) )
            continue;
        size_t Len = strlen( Packs.draft );
        if ( Len >= store::NameCap - 1 )
            return;
        Packs.draft[ Len ] = ( char )Code;
        Packs.draft[ Len + 1 ] = 0;
        return;
    }
    char Extra = 0;
    if ( Edge( VK_SPACE, KeyWas[ VK_SPACE ] ) )
        Extra = ' ';
    else if ( Edge( VK_OEM_MINUS, KeyWas[ VK_OEM_MINUS ] ) )
        Extra = '-';
    if ( Extra ) {
        size_t Len = strlen( Packs.draft );
        if ( Len >= store::NameCap - 1 )
            return;
        Packs.draft[ Len ] = Extra;
        Packs.draft[ Len + 1 ] = 0;
    }
}

static bool DrawConfigs( const CRectangle& Content, const CVector& Point, bool Click, bool Press, float Scale, float Ease ) {
    ( void )Press;
    float Keep = Canvas->Opacity;
    Canvas->Opacity = Keep * Ease;

    if ( Packs.noteAge > 0.0f )
        Packs.noteAge -= Context->DeltaTime;

    float Inset = 10.0f * Scale;
    float Gap = 8.0f * Scale;
    float Head = 40.0f * Scale;
    float Round = 8.0f * Scale;
    float Left = Content.Left + Inset;
    float Top = Content.Top + Inset;
    float Full = Content.Width - Inset * 2.0f;
    float ListW = Full * 0.58f;
    float SideW = Full - ListW - Gap;
    float RowH = 34.0f * Scale;
    float LibPad = 24.0f * Scale;
    int Rows = Packs.count > 0 ? Packs.count : 1;
    float Room = Content.Height - Inset * 2.0f - Head;
    float LibBodyH = LibPad + RowH * ( float )Rows;
    if ( LibBodyH > Room )
        LibBodyH = Room;
    float Line = Font ? Font->LineSpan : 16.0f * Scale;
    float FieldH = 30.0f * Scale;
    float ActH = 32.0f * Scale;
    float ManNeed = 12.0f * Scale + Line + 2.0f * Scale + Line + 8.0f * Scale + Line + 4.0f * Scale + FieldH + 8.0f * Scale + ActH + 6.0f * Scale + ActH + 8.0f * Scale + Line + 14.0f * Scale;
    float ManBodyH = ManNeed;
    if ( ManBodyH > Room )
        ManBodyH = Room;
    if ( ManBodyH < 168.0f * Scale && Room > 168.0f * Scale )
        ManBodyH = 168.0f * Scale;

    ui::RectBounds LibB, LibBarB, LibBodyB;
    ui::ComputeCardContainer( Left, Top, ListW, Head, LibBodyH, LibB, LibBarB, LibBodyB );
    CRectangle Lib( LibB.left, LibB.top, LibB.width, LibB.height );
    CRectangle LibBar( LibBarB.left, LibBarB.top, LibBarB.width, LibBarB.height );
    CRectangle LibBody( LibBodyB.left, LibBodyB.top, LibBodyB.width, LibBodyB.height );
    Canvas->Rectangle( Lib, Dress.card, Round );
    DrawIce( LibBar, CRectangle( Left, Top, ListW, Head + Round ), Round, 1.0f );
    Canvas->Border( Lib, Dress.foldLine, Round, 1.0f );
    Canvas->Write( Heading.get( ), CVector( Left + 14.0f * Scale, LibBar.Top + ( Head - Heading->LineSpan ) * 0.5f ), Dress.inkHot, "Library" );
    char Count[ 16 ];
    snprintf( Count, sizeof( Count ), "%d", Packs.count );
    CVector CountSize = Font->Measure( Count );
    Canvas->Text( CVector( LibBar.Right( ) - 14.0f * Scale - CountSize.Horizontal, LibBar.Top + ( Head - Font->LineSpan ) * 0.5f ), Style->Faint, Count );

    float Pad = 10.0f * Scale;
    CRectangle Pane( LibBody.Left + Pad, LibBody.Top + 8.0f * Scale, LibBody.Width - Pad * 2.0f, LibBody.Height - 16.0f * Scale );
    if ( Pane.Contains( Point ) && Input->WheelDelta != 0.0f ) {
        Packs.scroll -= Input->WheelDelta * 36.0f * Scale;
        Input->WheelDelta = 0.0f;
    }
    float Need = ( float )Packs.count * RowH;
    float Most = Need - Pane.Height;
    if ( Most < 0.0f )
        Most = 0.0f;
    if ( Packs.scroll > Most )
        Packs.scroll = Most;
    if ( Packs.scroll < 0.0f )
        Packs.scroll = 0.0f;

    bool Busy = false;
    Canvas->PushClip( Pane );
    if ( Packs.count == 0 ) {
        Canvas->Text( CVector( Pane.Left + 6.0f * Scale, Pane.Top + 8.0f * Scale ), Style->Faint, "No configs yet." );
    }
    for ( int Index = 0; Index < Packs.count; Index++ ) {
        CRectangle Row( Pane.Left, Pane.Top + ( float )Index * RowH - Packs.scroll, Pane.Width, RowH - 4.0f * Scale );
        if ( Row.Bottom( ) < Pane.Top || Row.Top > Pane.Bottom( ) )
            continue;
        bool Over = Row.Contains( Point ) && Pane.Contains( Point ) && !Moving( );
        bool On = Packs.pick == Index;
        bool Live = _stricmp( Packs.names[ Index ], Packs.live ) == 0;
        float Tone = ur::motion::toward( Packs.names[ Index ], On ? 1.0f : ( Over ? 0.45f : 0.0f ), 26.0f );
        if ( On )
            DrawIce( Row, Row, 6.0f * Scale, 0.55f + Tone * 0.25f );
        else if ( Over )
            Canvas->Rectangle( Row, CColor( 255, 255, 255, 12 ), 6.0f * Scale );
        Canvas->Text( CVector( Row.Left + 12.0f * Scale, Row.Top + ( Row.Height - Font->LineSpan ) * 0.5f ), On ? Dress.inkHot : Style->Text, Packs.names[ Index ] );
        if ( Live ) {
            CVector Tag = Font->Measure( "loaded" );
            Canvas->Text( CVector( Row.Right( ) - 12.0f * Scale - Tag.Horizontal, Row.Top + ( Row.Height - Font->LineSpan ) * 0.5f ), Style->AccentSoft, "loaded" );
        }
        if ( Over && Click ) {
            if ( Packs.pick == Index )
                PackLoad( Packs.names[ Index ] );
            Packs.pick = Index;
            Packs.confirm = false;
            lstrcpynA( Packs.draft, Packs.names[ Index ], store::NameCap );
            Busy = true;
        }
        Busy = Busy || Over;
    }
    Canvas->PopClip( );

    float Side = Left + ListW + Gap;
    ui::RectBounds BoxB, BarB, InnerB;
    ui::ComputeCardContainer( Side, Top, SideW, Head, ManBodyH, BoxB, BarB, InnerB );
    CRectangle Box( BoxB.left, BoxB.top, BoxB.width, BoxB.height );
    CRectangle Bar( BarB.left, BarB.top, BarB.width, BarB.height );
    CRectangle Inner( InnerB.left, InnerB.top, InnerB.width, InnerB.height );
    Canvas->Rectangle( Box, Dress.card, Round );
    DrawIce( Bar, CRectangle( Side, Top, SideW, Head + Round ), Round, 1.0f );
    Canvas->Border( Box, Dress.foldLine, Round, 1.0f );
    Canvas->Write( Heading.get( ), CVector( Side + 14.0f * Scale, Bar.Top + ( Head - Heading->LineSpan ) * 0.5f ), Dress.inkHot, "Manage" );

    float PadX = 14.0f * Scale;
    float CursorY = Inner.Top + 10.0f * Scale;
    float InnerW = Inner.Width - PadX * 2.0f;
    Canvas->PushClip( Inner );
    Canvas->Text( CVector( Side + PadX, CursorY ), Style->Faint, "Loaded" );
    CursorY += Line + 2.0f * Scale;
    Canvas->Text( CVector( Side + PadX, CursorY ), Dress.inkHot, Packs.live[ 0 ] ? Packs.live : "None" );
    CursorY += Line + 8.0f * Scale;

    Canvas->Text( CVector( Side + PadX, CursorY ), Style->Faint, "Name" );
    CursorY += Line + 4.0f * Scale;
    float MakeW = 78.0f * Scale;
    CRectangle Field( Side + PadX, CursorY, InnerW - MakeW - 8.0f * Scale, FieldH );
    bool OverField = Field.Contains( Point ) && !Moving( );
    if ( OverField && Click ) {
        Packs.type = true;
        SyncBindKeys( );
    }
    float Wait = ur::motion::toward( "cfg.type", Packs.type ? 1.0f : ( OverField ? 0.4f : 0.0f ), 26.0f );
    DrawIce( Field, Field, 6.0f * Scale, 0.55f + Wait * 0.45f );
    Canvas->Border( Field, Mix( Dress.foldLine, Style->AccentSoft, Wait ), 6.0f * Scale, 1.0f );
    const char* Shown = Packs.draft[ 0 ] ? Packs.draft : ( Packs.type ? "" : "new config" );
    Canvas->Text( CVector( Field.Left + 10.0f * Scale, Field.Top + ( Field.Height - Font->LineSpan ) * 0.5f ), Packs.draft[ 0 ] ? Style->Text : Style->Faint, Shown );
    if ( Packs.type && ( ( int )( Context->Elapsed * 2.0 ) & 1 ) ) {
        CVector Caret = Font->Measure( Packs.draft );
        Canvas->Rectangle( CRectangle( Field.Left + 10.0f * Scale + Caret.Horizontal + 1.0f * Scale, Field.Top + 7.0f * Scale, 1.0f * Scale, Field.Height - 14.0f * Scale ), Dress.inkHot, 0.0f );
    }
    CRectangle Make( Field.Right( ) + 8.0f * Scale, CursorY, MakeW, FieldH );
    if ( DrawAction( Make, "Create", Point, Click, Scale, false ) ) {
        store::Sanitize( Packs.draft );
        if ( store::Valid( Packs.draft ) && PackSave( Packs.draft ) )
            PackNote( "Created" );
        else
            PackNote( "Need a name" );
        Packs.type = false;
        Packs.confirm = false;
        Busy = true;
    }
    CursorY += FieldH + 8.0f * Scale;

    ui::RectBounds LoadB, SaveB;
    ui::ComputeSplitPair( Side + PadX, CursorY, InnerW, 8.0f * Scale, ActH, LoadB, SaveB );
    CRectangle Load( LoadB.left, LoadB.top, LoadB.width, LoadB.height );
    CRectangle Save( SaveB.left, SaveB.top, SaveB.width, SaveB.height );
    if ( DrawAction( Load, "Load", Point, Click, Scale, false ) ) {
        if ( Packs.count > 0 && PackLoad( Packs.names[ Packs.pick ] ) )
            PackNote( "Loaded" );
        else
            PackNote( "Nothing to load" );
        Packs.confirm = false;
        Busy = true;
    }
    if ( DrawAction( Save, "Save", Point, Click, Scale, false ) ) {
        const char* Target = Packs.count > 0 ? Packs.names[ Packs.pick ] : Packs.live;
        if ( store::Valid( Target ) && PackSave( Target ) )
            PackNote( "Saved" );
        else
            PackNote( "Save failed" );
        Packs.confirm = false;
        Busy = true;
    }
    CursorY += ActH + 6.0f * Scale;

    ui::RectBounds KillB, FolderB;
    ui::ComputeSplitPair( Side + PadX, CursorY, InnerW, 8.0f * Scale, ActH, KillB, FolderB );
    CRectangle Kill( KillB.left, KillB.top, KillB.width, KillB.height );
    CRectangle Folder( FolderB.left, FolderB.top, FolderB.width, FolderB.height );
    if ( DrawAction( Kill, Packs.confirm ? "Sure?" : "Delete", Point, Click, Scale, true ) ) {
        if ( Packs.count > 0 ) {
            if ( !Packs.confirm ) {
                Packs.confirm = true;
                PackNote( "Click again to delete" );
            } else if ( store::Remove( Packs.names[ Packs.pick ] ) ) {
                PackNote( "Deleted" );
                Packs.confirm = false;
                if ( _stricmp( Packs.live, Packs.names[ Packs.pick ] ) == 0 )
                    Packs.live[ 0 ] = 0;
                PackRefresh( );
            }
        }
        Busy = true;
    }
    if ( DrawAction( Folder, "Folder", Point, Click, Scale, false ) ) {
        store::OpenFolder( );
        Busy = true;
    }
    CursorY += ActH + 8.0f * Scale;
    if ( Packs.noteAge > 0.0f && Packs.note[ 0 ] ) {
        float Fade = Packs.noteAge > 1.0f ? 1.0f : Packs.noteAge;
        float Hold = Canvas->Opacity;
        Canvas->Opacity = Hold * Fade;
        Canvas->Text( CVector( Side + PadX, CursorY ), Style->AccentSoft, Packs.note );
        Canvas->Opacity = Hold;
    }
    Canvas->PopClip( );

    if ( Click && !OverField )
        Packs.type = false;

    Canvas->Opacity = Keep;
    return Busy || OverField;
}
