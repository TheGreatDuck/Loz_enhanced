/*
   Copyright 2016 Aldo J. Nunez

   Licensed under the Apache License, Version 2.0.
   See the LICENSE text file for details.
*/

#include "gameMakerLibrary.hpp"
#include "Common.h"
#include "Input.h"

enum InputAxis
{
    InputAxis_None,
    InputAxis_Horizontal,
    InputAxis_Vertical
};

struct ButtonMapping
{
    uint8_t     SrcCode;
    uint8_t     DstCode;
    const char* SettingName;
};

struct AxisMapping
{
    uint8_t     Stick;
    uint8_t     SrcAxis;
    uint8_t     DstAxis;
    const char* StickSettingName;
    const char* AxisSettingName;
};

static ButtonMapping keyboardMappings[8];/* =
{
    { ALLEGRO_KEY_RIGHT,    InputButtons::Right,    "key.button.right" },
    { ALLEGRO_KEY_LEFT,     InputButtons::Left,     "key.button.left" },
    { ALLEGRO_KEY_DOWN,     InputButtons::Down,     "key.button.down" },
    { ALLEGRO_KEY_UP,       InputButtons::Up,       "key.button.up" },
    { ALLEGRO_KEY_ENTER,    InputButtons::Start,    "key.button.start" },
    { ALLEGRO_KEY_S,        InputButtons::Select,   "key.button.select" },
    { ALLEGRO_KEY_D,        InputButtons::B,        "key.button.b" },
    { ALLEGRO_KEY_F,        InputButtons::A,        "key.button.a" },
};*/

static const char InputSection[] = "input";

static InputButtons oldInputState( 0 );
static InputButtons inputState( 0 );

//static ALLEGRO_KEYBOARD_STATE oldKeyboardState;
//static ALLEGRO_KEYBOARD_STATE keyboardState;


InputButtons::InputButtons( uint value )
    :   Buttons( value )
{
}

bool InputButtons::Has( uint value ) const
{
    return (Buttons & value) != 0;
}

void InputButtons::Mask( uint value )
{
    Buttons &= value;
}

void InputButtons::Clear( uint value )
{
    Buttons = Buttons ^ value;
}

InputButtons Input::GetButtons()
{
    uint buttons = 0;

    buttons = (oldInputState.Buttons ^ inputState.Buttons)
        & inputState.Buttons
        | (inputState.Buttons & 0xF);

    return (InputButtons) buttons;
}

bool Input::IsKeyDown(int keyCode)
{
    return keyboard_check(keyCode);
}

bool Input::IsKeyPressing(int keyCode)
{
    return keyboard_check_pressed(keyCode);
}

ButtonState Input::GetButton(int buttonCode)
{
    int isDown = (inputState.Buttons & buttonCode) ? 1 : 0;
    int wasDown = (oldInputState.Buttons & buttonCode) ? 1 : 0;

    return (ButtonState) ((wasDown << 1) | isDown);
}

static bool ParseInt( const char* str, int& value )
{
    if ( str == nullptr )
        return false;
    char* endPtr = nullptr;
    value = strtol( str, &endPtr, 10 );
    return endPtr != str && *endPtr == '\0';
}

static void LoadButtonMappings( ButtonMapping* mappings, int count )
{
    /*ALLEGRO_CONFIG* config = GetConfig();

    for ( int i = 0; i < count; i++ )
    {
        ButtonMapping& mapping = mappings[i];
        int value;
        const char* strValue = al_get_config_value( config, InputSection, mapping.SettingName );
        if ( ParseInt( strValue, value ) )
            mapping.SrcCode = value;
    }*/
}

static void LoadAxisMappings( AxisMapping* mappings, int count )
{
    /*ALLEGRO_CONFIG* config = GetConfig();

    for ( int i = 0; i < count; i++ )
    {
        AxisMapping& mapping = mappings[i];
        int stick = 0;
        int axis = 0;
        const char* strValue = al_get_config_value( config, InputSection, mapping.StickSettingName );
        if ( !ParseInt( strValue, stick ) )
            continue;
        strValue = al_get_config_value( config, InputSection, mapping.AxisSettingName );
        if ( !ParseInt( strValue, axis ) )
            continue;
        mapping.Stick = stick;
        mapping.SrcAxis = axis;
    }*/
}

static void LoadMappings()
{
    /*ALLEGRO_CONFIG* config = GetConfig();
    if ( config == nullptr )
        return;

    LoadButtonMappings( keyboardMappings, _countof( keyboardMappings ) );
    LoadButtonMappings( joystickButtonMappings, _countof( joystickButtonMappings ) );
    LoadAxisMappings( joystickAxisMappings, _countof( joystickAxisMappings ) );*/
}

void Input::Init()
{
    LoadMappings();
}
