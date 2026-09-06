#include "test_framework.hpp"
#include "ui/key_labels.hpp"

TEST_CASE( "KeyLabels: Mouse buttons mapping" ) {
    CHECK_EQ( std::string( ui::KeyLabel( VK_LBUTTON ) ), "Mouse 1" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RBUTTON ) ), "Mouse 2" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_MBUTTON ) ), "Mouse 3" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_XBUTTON1 ) ), "Mouse 4" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_XBUTTON2 ) ), "Mouse 5" );
}

TEST_CASE( "KeyLabels: Navigation and control keys" ) {
    CHECK_EQ( std::string( ui::KeyLabel( VK_INSERT ) ), "Insert" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_DELETE ) ), "Delete" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_HOME ) ), "Home" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_END ) ), "End" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_PRIOR ) ), "Page Up" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_NEXT ) ), "Page Down" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_SPACE ) ), "Space" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_TAB ) ), "Tab" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RETURN ) ), "Enter" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_BACK ) ), "Backspace" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_LEFT ) ), "Left" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RIGHT ) ), "Right" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_UP ) ), "Up" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_DOWN ) ), "Down" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_SHIFT ) ), "Shift" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_LSHIFT ) ), "Shift" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RSHIFT ) ), "Right Shift" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_CONTROL ) ), "Ctrl" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_LCONTROL ) ), "Ctrl" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RCONTROL ) ), "Right Ctrl" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_MENU ) ), "Alt" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_RMENU ) ), "Right Alt" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_CAPITAL ) ), "Caps Lock" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_NUMLOCK ) ), "Num Lock" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_SCROLL ) ), "Scroll Lock" );
}

TEST_CASE( "KeyLabels: Numpad numbers and operations" ) {
    CHECK_EQ( std::string( ui::KeyLabel( VK_NUMPAD0 ) ), "Num 0" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_NUMPAD5 ) ), "Num 5" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_NUMPAD9 ) ), "Num 9" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_MULTIPLY ) ), "Num *" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_ADD ) ), "Num +" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_SUBTRACT ) ), "Num -" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_DECIMAL ) ), "Num ." );
    CHECK_EQ( std::string( ui::KeyLabel( VK_DIVIDE ) ), "Num /" );
}

TEST_CASE( "KeyLabels: Function keys F1 to F24" ) {
    CHECK_EQ( std::string( ui::KeyLabel( VK_F1 ) ), "F1" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_F5 ) ), "F5" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_F12 ) ), "F12" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_F24 ) ), "F24" );
}

TEST_CASE( "KeyLabels: Alphanumeric characters and OEM symbols" ) {
    CHECK_EQ( std::string( ui::KeyLabel( 'A' ) ), "A" );
    CHECK_EQ( std::string( ui::KeyLabel( 'Z' ) ), "Z" );
    CHECK_EQ( std::string( ui::KeyLabel( '0' ) ), "0" );
    CHECK_EQ( std::string( ui::KeyLabel( '9' ) ), "9" );

    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_1 ) ), ";" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_PLUS ) ), "=" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_COMMA ) ), "," );
    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_MINUS ) ), "-" );
    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_PERIOD ) ), "." );
    CHECK_EQ( std::string( ui::KeyLabel( VK_OEM_2 ) ), "/" );
}

TEST_CASE( "KeyLabels: Fallback for unregistered key codes" ) {
    CHECK_EQ( std::string( ui::KeyLabel( 999 ) ), "Key 999" );
}

TEST_CASE( "BitLabels: Bitmask formatting with options" ) {
    static const char* Bones[ 4 ] = { "Head", "Neck", "Chest", "Stomach" };

    // Zero bits returns "None"
    CHECK_EQ( std::string( ui::BitLabel( Bones, 4, 0 ) ), "None" );

    // Single bit
    CHECK_EQ( std::string( ui::BitLabel( Bones, 4, 1 ) ), "Head" );
    CHECK_EQ( std::string( ui::BitLabel( Bones, 4, 2 ) ), "Neck" );

    // Multiple bits (1 | 4) -> Head, Chest
    CHECK_EQ( std::string( ui::BitLabel( Bones, 4, 5 ) ), "Head, Chest" );

    // All bits (1 | 2 | 4 | 8)
    CHECK_EQ( std::string( ui::BitLabel( Bones, 4, 15 ) ), "Head, Neck, Chest, Stomach" );
}
