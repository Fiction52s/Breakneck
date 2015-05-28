#ifndef __MACINPUT_H__
#define __MACINPUT_H__

struct MacControllerState
{
	MacControllerState();
	void Set( const MacControllerState &state );
	double leftStickMagnitude; // 0 - 1.0
	double leftStickRadians;
	double rightStickMagnitude; // 0 - 1.0
	double rightStickRadians;
	char leftTrigger;
	char rightTrigger;
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

class MacGameController
{
public:
	///index 0-3, corresponding to the different physical
	///controllers. 0 = top left light, 1 = top right light,
	///2 = bottom left light, 3 = bottom right light
	MacGameController( int index );
	///Gets if the controller is plugged in or functioning
	bool IsConnected();
	
	//Gets the that was passed to the constructor
	int GetIndex();
	//Queries and stores the current state of the physical 
	///controller
	bool UpdateState();
	///Gets the state most recently queried from the
	///controller. Used to update external ControllerStates
	MacControllerState & GetState();
private:
	MacControllerState m_state;
	int m_index;
};

#endif