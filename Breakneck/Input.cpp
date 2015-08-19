#include "Input.h"
#include <math.h>


#define PI 3.14159265359
const DWORD GameController::LEFT_STICK_DEADZONE = 7849;
const DWORD GameController::RIGHT_STICK_DEADZONE = 8689;
const DWORD GameController::TRIGGER_THRESHOLD = 30;

ControllerState::ControllerState()
	:leftStickMagnitude( 0 ), leftStickRadians( 0 ), rightStickMagnitude( 0 ), 
	rightStickRadians( 0 ), leftTrigger( 0 ), rightTrigger( 0 ), start( false ), 
	back( false ), leftShoulder( false ), rightShoulder( false ), A( false ), B( false ), 
	X( false ), Y( false ), pad( 0 ), leftStickPad( 0 ), rightStickPad( 0 ), 
	leftPress( false ), rightPress( false )
{

	
}

void ControllerState::Set( const ControllerState &state )
{
	leftStickMagnitude = state.leftStickMagnitude;
	leftStickRadians = state.leftStickRadians;
	rightStickMagnitude = state.rightStickMagnitude;
	rightStickRadians = state.rightStickRadians;
	leftTrigger = state.leftTrigger;
	rightTrigger = state.rightTrigger;
	start = state.start;
	back = state.back;
	leftShoulder = state.leftShoulder;
	rightShoulder = state.rightShoulder;
	A = state.A;
	B = state.B;
	X = state.X;
	Y = state.Y;
	pad = state.pad;
	leftStickPad = state.leftStickPad;
	rightStickPad = state.rightStickPad;
	leftPress = state.leftPress;
	rightPress = state.rightPress;
//	altPad = state.altPad;
}

bool ControllerState::PUp()
{
	return pad & 1;
}

bool ControllerState::PDown()
{
	return pad & 2;
}

bool ControllerState::PLeft()
{
	return pad & 4;
}

bool ControllerState::PRight()
{
	return pad & 8;
}

bool ControllerState::LUp()
{
	return leftStickPad & 1;
}

bool ControllerState::LDown()
{
	return leftStickPad & 2;
}

bool ControllerState::LLeft()
{
	return leftStickPad & 4;
}

bool ControllerState::LRight()
{
	return leftStickPad & 8;
}

bool ControllerState::LPress()
{
	return leftPress;
}

bool ControllerState::RUp()
{
	return rightStickPad & 1;
}

bool ControllerState::RDown()
{
	return rightStickPad & 2;
}

bool ControllerState::RLeft()
{
	return rightStickPad & 4;
}

bool ControllerState::RRight()
{
	return rightStickPad & 8;
}

bool ControllerState::RPress()
{
	return rightPress;
}

bool GameController::UpdateState()
{
	XINPUT_STATE state;
	ZeroMemory( &state, sizeof( XINPUT_STATE ) );
	DWORD result = XInputGetState( m_index, &state );

	if( result == ERROR_SUCCESS )
	{
		double LX = state.Gamepad.sThumbLX;
		double LY = state.Gamepad.sThumbLY;
		//i dont think i need a magnitude, magnitude should be calculated for differences
		//between controller states by some other function
		double magnitude = sqrt( LX * LX + LY * LY );

		double normalizedLX = LX / magnitude;
		double normalizedLY = LY / magnitude;

		double normalizedMagnitude = 0;

		if( magnitude > LEFT_STICK_DEADZONE )
		{
			if( magnitude > 32767 )
				magnitude = 32767;

			magnitude -= LEFT_STICK_DEADZONE;
			normalizedMagnitude = magnitude / (32767 - LEFT_STICK_DEADZONE);
		}
		else 
		{
			magnitude = 0.0f;
			normalizedMagnitude = 0.0;
		}

		m_state.leftStickMagnitude = normalizedMagnitude;
		m_state.leftStickRadians = atan( normalizedLY / normalizedLX );
		if( normalizedLX < 0.0f )
			m_state.leftStickRadians += PI;

		double RX = state.Gamepad.sThumbRX;
		double RY = state.Gamepad.sThumbRY;

		magnitude = sqrt( RX * RX + RY * RY );

		double normalizedRX = RX / magnitude;
		double normalizedRY = RY / magnitude;

		if( magnitude > RIGHT_STICK_DEADZONE )
		{
			if( magnitude > 32767 )
				magnitude = 32767;

			magnitude -= RIGHT_STICK_DEADZONE;
			normalizedMagnitude = magnitude / ( 32767 - RIGHT_STICK_DEADZONE );
		}
		else 
		{
			magnitude = 0.0f;
			normalizedMagnitude = 0.0f;
		}

		m_state.rightStickMagnitude = normalizedMagnitude;
		m_state.rightStickRadians = atan( normalizedRY / normalizedRX );
		if( normalizedRX < 0.0f )
			m_state.rightStickRadians += PI;

		m_state.leftTrigger = state.Gamepad.bLeftTrigger;
		m_state.rightTrigger = state.Gamepad.bRightTrigger;

		WORD b = state.Gamepad.wButtons;
		m_state.start = (b & 0x10) > 0;
		m_state.back = (b & 0x20) > 0;
		m_state.leftShoulder = (b & 0x100) > 0;
		m_state.rightShoulder = (b & 0x200) > 0;
		m_state.A = (b & 0x1000) > 0;
		m_state.B = (b & 0x2000) > 0;
		m_state.X = (b & 0x4000) > 0;
		m_state.Y = (b & 0x8000) > 0;
		m_state.leftPress = b & XINPUT_GAMEPAD_LEFT_THUMB;
		m_state.rightPress = b & XINPUT_GAMEPAD_RIGHT_THUMB;
		m_state.pad = ( b & 1 ) | ( b & 2 ) | ( b & 4 ) | ( b & 8 ); 

		m_state.leftStickPad = 0;
		m_state.rightStickPad = 0;
		if( m_state.leftStickMagnitude > stickThresh )
		{
			//cout << "left stick radians: " << currInput.leftStickRadians << endl;
			float x = cos( m_state.leftStickRadians );
			float y = sin( m_state.leftStickRadians );

			if( x > stickThresh )
				m_state.leftStickPad += 1 << 3;
			if( x < -stickThresh )
				m_state.leftStickPad += 1 << 2;
			if( y > stickThresh )
				m_state.leftStickPad += 1;
			if( y < -stickThresh )
				m_state.leftStickPad += 1 << 1;
		}

		if( m_state.rightStickMagnitude > stickThresh )
		{
			//cout << "left stick radians: " << m_state.leftStickRadians << endl;
			float x = cos( m_state.rightStickRadians );
			float y = sin( m_state.rightStickRadians );

			if( x > stickThresh )
				m_state.rightStickPad += 1 << 3;
			if( x < -stickThresh )
				m_state.rightStickPad += 1 << 2;
			if( y > stickThresh )
				m_state.rightStickPad += 1;
			if( y < -stickThresh )
				m_state.rightStickPad += 1 << 1;
		}
	}


	

	return ( result == ERROR_SUCCESS );
}

GameController::GameController( DWORD index )
	:m_index( index )
{
	stickThresh = .4;
}

ControllerState & GameController::GetState()
{
	return m_state;
}

bool GameController::IsConnected()
{
	XINPUT_STATE state;
	ZeroMemory( &state, sizeof( XINPUT_STATE ) );
	DWORD result = XInputGetState( m_index, &state );

	return ( result == ERROR_SUCCESS );
}

DWORD GameController::GetIndex()
{
	return m_index;
}