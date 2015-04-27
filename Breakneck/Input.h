#ifndef __INPUT_H__
#define __INPUT_H__
#include <Windows.h>
#include <Xinput.h>

//true if down, false if up
//^^make this more efficient if I'm using it for networking
//  later

//
#pragma comment(lib, "XInput.lib")

/** Remarks:
For each button, its bool its true if the it is down 
(pressed) and false if it is up (not pressed).
*/
struct ControllerState
{
	ControllerState();
	void Set( const ControllerState &state );
	float leftStickMagnitude; // 0 - 1.0
	float leftStickRadians;
	float rightStickMagnitude; // 0 - 1.0
	float rightStickRadians;
	BYTE leftTrigger;
	BYTE rightTrigger;
	bool start;
	bool back;
	bool leftShoulder;
	bool rightShoulder;
	bool A;
	bool B;
	bool X;
	bool Y;
	unsigned char pad;
	unsigned char altPad;
	bool Up();
	bool Down();
	bool Left();
	bool Right();//0x1 = up, 0x2 = down, 0x4 = left, 
				 //0x8 = right
	bool AltUp();
	bool AltDown();
	bool AltLeft();
	bool AltRight();
};


/** Remarks:
Wrapper for XINPUT controller. Used to access the actual
controllers and generate state information for use in the 
game.
*/
class GameController
{
public:
	///index 0-3, corresponding to the different physical
	///controllers. 0 = top left light, 1 = top right light,
	///2 = bottom left light, 3 = bottom right light
	GameController( DWORD index );
	///Gets if the controller is plugged in or functioning
	bool IsConnected();
	//Gets the that was passed to the constructor
	DWORD GetIndex();
	//Queries and stores the current state of the physical 
	///controller
	bool UpdateState();
	///Gets the state most recently queried from the
	///controller. Used to update external ControllerStates
	ControllerState & GetState();
private:
	DWORD m_index;
	ControllerState m_state;
	const static DWORD LEFT_STICK_DEADZONE;
	const static DWORD RIGHT_STICK_DEADZONE;
	const static DWORD TRIGGER_THRESHOLD;
};

//wButtons
/*XINPUT_GAMEPAD_DPAD_UP         0x00000001
XINPUT_GAMEPAD_DPAD_DOWN        0x00000002
XINPUT_GAMEPAD_DPAD_LEFT        0x00000004
XINPUT_GAMEPAD_DPAD_RIGHT       0x00000008
XINPUT_GAMEPAD_START            0x00000010
XINPUT_GAMEPAD_BACK             0x00000020
XINPUT_GAMEPAD_LEFT_THUMB       0x00000040
XINPUT_GAMEPAD_RIGHT_THUMB      0x00000080
XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
XINPUT_GAMEPAD_A                0x1000
XINPUT_GAMEPAD_B                0x2000
XINPUT_GAMEPAD_X                0x4000
XINPUT_GAMEPAD_Y                0x8000*/


#endif