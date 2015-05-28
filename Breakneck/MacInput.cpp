#include "MacInput.h"
#include <math.h>


#define PI 3.14159265359

MacControllerState::MacControllerState()
	:leftStickMagnitude( 0 ), leftStickRadians( 0 ), rightStickMagnitude( 0 ), 
	rightStickRadians( 0 ), leftTrigger( 0 ), rightTrigger( 0 ), start( false ), 
	back( false ), leftShoulder( false ), rightShoulder( false ), A( false ), B( false ), 
	X( false ), Y( false ), pad( 0 ), altPad( 0 )
{}

void MacControllerState::Set( const MacControllerState &state )
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
	altPad = state.altPad;
}

bool MacControllerState::Up()
{
	return pad & 1;
}

bool MacControllerState::Down()
{
	return pad & 2;
}

bool MacControllerState::Left()
{
	return pad & 4;
}

bool MacControllerState::Right()
{
	return pad & 8;
}

bool MacControllerState::AltUp()
{
	return altPad & 1;
}

bool MacControllerState::AltDown()
{
	return altPad & 2;
}

bool MacControllerState::AltLeft()
{
	return altPad & 4;
}

bool MacControllerState::AltRight()
{
	return altPad & 8;
}

bool MacGameController::UpdateState()
{
	return false;
}

MacGameController::MacGameController( int index )
	:m_index( index )
{
}

MacControllerState & MacGameController::GetState()
{
	return m_state;
}

bool MacGameController::IsConnected()
{
	return false;
}

int MacGameController::GetIndex()
{
	return m_index;
}