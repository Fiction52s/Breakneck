#include "Actor.h"
#include "GameSession.h"
#include "VectorMath.h"
#include <iostream>
#include <assert.h>

using namespace sf;
using namespace std;

#define V2d sf::Vector2<double>

Actor::Actor( GameSession *gs )
	:owner( gs )
	{
		activeEdges = new Edge*[16]; //this can probably be really small I don't think it matters. 
		numActiveEdges = 0;

		assert( Shader::isAvailable() && "help me" );
		if (!sh.loadFromFile("player_shader.frag", sf::Shader::Fragment))
		//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
		{
			cout << "PLAYER SHADER NOT LOADING CORRECTLY" << endl;
			assert( 0 && "player shader not loaded" );
		}

		/*if( !testSound.loadFromFile( "fair.wav" ) )
		{
			assert( 0 && "failed to load test fair noise" );
		}
		fairSound.setBuffer( testSound );*/
		
		slopeLaunchMinSpeed = 15;

		offsetX = 0;
		sprite = new Sprite;
		velocity = Vector2<double>( 0, 0 );
		
		queryMode = "";

		//tileset setup
		{
		actionLength[DAIR] = 10 * 2;
		tileset[DAIR] = owner->GetTileset( "dair.png", 128, 64 );

		actionLength[DASH] = 45;
		tileset[DASH] = owner->GetTileset( "dash.png", 64, 64 );

		actionLength[DOUBLE] = 28 + 10;
		tileset[DOUBLE] = owner->GetTileset( "double.png", 64, 64 );

		actionLength[FAIR] = 10 * 2;
		tileset[FAIR] = owner->GetTileset( "fair.png", 128, 64 );

		actionLength[JUMP] = 2;
		tileset[JUMP] = owner->GetTileset( "jump.png", 64, 64 );

		actionLength[LAND] = 1;
		tileset[LAND] = owner->GetTileset( "land.png", 64, 64 );

		actionLength[LAND2] = 1;
		tileset[LAND2] = owner->GetTileset( "land2.png", 64, 64 );

		actionLength[RUN] = 10 * 4;
		tileset[RUN] = owner->GetTileset( "run.png", 128, 64 );

		actionLength[SLIDE] = 1;
		tileset[SLIDE] = owner->GetTileset( "slide.png", 64, 64 );

		actionLength[SPRINT] = 8 * 3;
		tileset[SPRINT] = owner->GetTileset( "sprint.png", 128, 64 );		

		actionLength[STAND] = 20 * 8;
		tileset[STAND] = owner->GetTileset( "stand.png", 64, 64 );

		actionLength[STANDD] = 6 * 2;
		tileset[STANDD] = owner->GetTileset( "standd.png", 128, 64 );

		actionLength[STANDN] = 5 * 2;
		tileset[STANDN] = owner->GetTileset( "standn.png", 128, 64 );

		actionLength[UAIR] = 9 * 3;
		tileset[UAIR] = owner->GetTileset( "uair.png", 128, 128 );

		actionLength[WALLCLING] = 1;
		tileset[WALLCLING] = owner->GetTileset( "wallcling.png", 64, 64 );

		actionLength[WALLJUMP] = 9 * 2;
		tileset[WALLJUMP] = owner->GetTileset( "walljump.png", 64, 64 );

		actionLength[GRINDBALL] = 1;
		tileset[GRINDBALL] = owner->GetTileset( "grindball.png", 32, 32 );

		actionLength[STEEPSLIDE] = 1;
		tileset[STEEPSLIDE] = owner->GetTileset( "steepslide.png", 64, 32 );

		actionLength[AIRDASH] = 60;
		tileset[AIRDASH] = owner->GetTileset( "airdash.png", 64, 64 );

		}

		action = JUMP;
		frame = 1;

		timeSlowStrength = 5;
		slowMultiple = 1;
		slowCounter = 1;

		reversed = false;

		framesInAir = 0;
		wallJumpFrameCounter = 0;
		wallJumpMovementLimit = 10; //10 frames

		steepThresh = .4; // go between 0 and 1

		gravity = 2;
		maxFallSpeed = 60;

		wallJumpStrength.x = 10;
		wallJumpStrength.y = 25;
		clingSpeed = 3;

		dashSpeed = 17;

		jumpStrength = 27.5;

		hasDoubleJump = true;
		doubleJumpStrength = 26.5;

		ground = NULL;
		groundSpeed = 0;
		maxNormalRun = 100;
		runAccel = 1;
		facingRight = true;
		collision = false;
	
		airAccel = 1.5;
		maxAirXSpeed = 100;
		
		airDashSpeed = 10;

		airSlow = .3;

		groundOffsetX = 0;

		grindEdge = NULL;
		grindQuantity = 0;
		grindSpeed = 0;

		maxRunInit = 8;
		maxAirXControl = maxRunInit;

		maxGroundSpeed = 100;
		runAccelInit = 1;
		runAccel = .01;
		sprintAccel = 1;

		holdDashAccel = .05;

		dashHeight = 10;
		normalHeight = 23;
		doubleJumpHeight = 10;
		sprintHeight = 16;

		//CollisionBox b;
		b.isCircle = false;
		b.offsetAngle = 0;
		b.offset.x = 0;
		b.offset.y = 0;
		b.rw = 10;
		b.rh = normalHeight;

		
		b.type = b.Physics;
	}

void Actor::ActionEnded()
{
	if( frame >= actionLength[action] )
	{
		switch( action )
		{
		case STAND:
			frame = 0;
			break;
		case RUN:
			frame = 0;
		case JUMP:
			frame = 1;
			break;
		case LAND:
			frame = 0;
			break;
		case LAND2:
			frame = 0;
			break;
		case WALLCLING:
			frame = 0;
			break;
		case WALLJUMP:
			action = JUMP;
			frame = 1;
			break;
		case STANDN:
			action = STAND;
			frame = 0;
			break;
		case STANDD:
			action = STAND;
			frame = 0;
			break;
		case FAIR:
			action = JUMP;
			frame = 1;
			break;
		case DAIR:
			action = JUMP;
			frame = 1;
			break;
		case UAIR:
			action = JUMP;
			frame = 1;
			break;
		case DASH:
			action = STAND;
			frame = 0;
			break;
		case DOUBLE:
			action = JUMP;
			frame = 1;
			break;
		case SLIDE:
			frame = 0;
			break;
		case SPRINT:
			frame = 0;
			break;
		case GRINDBALL:
			frame = 0;
			break;
		case AIRDASH:
			action = JUMP;
			frame = 1;
			break;
		}
	}
}

void Actor::UpdatePrePhysics()
{
	if( reversed )
	{
		bool up = currInput.LUp();
		bool down = currInput.LDown();

		if( up ) currInput.leftStickPad -= 1;
		if( down ) currInput.leftStickPad -= 2;

		if( up ) currInput.leftStickPad += 2;
		if( down ) currInput.leftStickPad += 1;
	}


	ActionEnded();
	V2d gNorm;
	if( ground != NULL )
		gNorm = ground->Normal();
	//choose action

	
	bool canStandUp = true;
	if( b.rh < normalHeight )
	{
		canStandUp = CheckStandUp();
		if( canStandUp )
		{
			b.rh = normalHeight;
			b.offset.y = 0;
		}
	}

	switch( action )
	{
	case STAND:
		{
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			else if( currInput.LLeft() || currInput.LRight() )
			{
				if( currInput.LDown() )
				{
					action = SPRINT;
					frame = 0;
				}
				else
				{
					action = RUN;
					frame = 0;
				}
				break;
				
			}
			else if( currInput.X && !prevInput.X )
			{
				if( currInput.LDown() )
				{
					action = STANDD;
					frame = 0;
				}
				else
				{
					action = STANDN;
					frame = 0;
				}
			}
			else if( currInput.B && !prevInput.B )
			{
				action = DASH;
				frame = 0;
			}
			else if( currInput.LDown() )
			{
				action = SLIDE;
				frame = 0;
			}
			
			break;
		}
	case RUN:
		{
			if( currInput.Y && !prevInput.Y )
			{
				action = GRINDBALL;
				grindEdge = ground;
				frame = 0;
				grindSpeed = groundSpeed;
				grindQuantity = edgeQuantity;
				break;
			}


			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh )
				{
					action = STEEPSLIDE;
					frame = 0;
					break;
				}
			}
			else
			{
				if( gNorm.y > -steepThresh )
				{
					action = STEEPSLIDE;
					frame = 0;
					break;
				}
			}

			bool t = (!currInput.LUp() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ));
			if(!( currInput.LLeft() || currInput.LRight() ) )//&& t )
			{
				if( currInput.LDown())
				{
					action = SLIDE;
					frame = 0;
				}
				else if( currInput.LUp() )
				{
					//stay running

					break;
				}
				else
				{
					action = STAND;
					frame = 0;
				}
				break;
				
			}
			else
			{

				if( facingRight && currInput.LLeft() )
				{
					
					if( ( currInput.LDown() && gNorm.x < 0 ) || ( currInput.LUp() && gNorm.x > 0 ) )
					{
						action = SPRINT;
					}
					
					groundSpeed = 0;
					facingRight = false;
					frame = 0;
					break;
				}
				else if( !facingRight && currInput.LRight() )
				{
					if( ( currInput.LDown() && gNorm.x > 0 ) || ( currInput.LUp() && gNorm.x < 0 ) )
					{
						action = SPRINT;
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					break;
				}
				else if( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) )
				{
					
					action = SPRINT;
					frame = frame / 4;

					if( frame < 3 )
					{
						frame = frame + 1;
					}
					else if ( frame == 8)
					{
						frame = 7;
					}

					else if ( frame == 9)
					{
						frame = 0;
					}
					frame = frame * 3;
					break;
				}

			}
			if( currInput.B && !prevInput.B )
			{
					action = DASH;
					frame = 0;
					break;
			}
			break;
		}
	case JUMP:
		{
			if( hasDoubleJump && currInput.A && !prevInput.A )
			{
				action = DOUBLE;
				frame = 0;
				break;
			}
			//cout << CheckWall( true ) << endl;
			
			if( CheckWall( false ) )
			{
				if( currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = true;
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{				
				if( currInput.LLeft() && !prevInput.LLeft() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = false;
					break;
				}
			}

	
			if( currInput.X && !prevInput.X )
			{
				if( !currInput.LLeft() && !currInput.LRight() )
				{
					if( currInput.LUp() )
					{
						action = UAIR;
						frame = 0;
						break;
					}
					else if( currInput.LDown() )
					{
						action = DAIR;
						frame = 0;
						break;
					}
				}

				action = FAIR;
				frame = 0;
			}

			break;
		}
		case DOUBLE:
		{
			
			if( CheckWall( false ) )
			{
				if( currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = true;
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{				
				if( currInput.LLeft() && !prevInput.LLeft() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = false;
					break;
				}
			}

			if( currInput.X && !prevInput.X )
			{
				if( !currInput.LLeft() && !currInput.LRight() )
				{
					if( currInput.LUp() )
					{
						action = UAIR;
						frame = 0;
						break;
					}
					else if( currInput.LDown() )
					{
						action = DAIR;
						frame = 0;
						break;
					}
				}

				action = FAIR;
				frame = 0;
			}

			break;
		}
	case LAND:
	case LAND2:
		{
			if( reversed )
			{
				if( -gNorm.y > -steepThresh )
				{
					action = STEEPSLIDE;
					frame = 0;
				}
				else
				{
					if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else if( currInput.LUp() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			else
			{
			
				if( gNorm.y > -steepThresh )
				{
					action = STEEPSLIDE;
					frame = 0;
				}
				else
				{
					if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
		

			break;
		}
	case WALLCLING:
		{
			if( (facingRight && currInput.LRight()) || (!facingRight && currInput.LLeft() ) )
			{

				action = WALLJUMP;
				frame = 0;
				//facingRight = !facingRight;
			}
			break;
		}
	case WALLJUMP:
		{
			
			if( hasDoubleJump && currInput.A && !prevInput.A )
			{
				action = DOUBLE;
				frame = 0;
				break;
			}

			if( CheckWall( false ) )
			{
				if( currInput.LRight() && !prevInput.LRight() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = true;
					break;
				}
			}
			
			
			if( CheckWall( true ) )
			{				
				if( currInput.LLeft() && !prevInput.LLeft() )
				{
					action = WALLJUMP;
					frame = 0;
					facingRight = false;
					break;
				}
			}

		
			{
				if( currInput.X && !prevInput.X )
				{
					if( !currInput.LLeft() && !currInput.LRight() )
					{
						if( currInput.LUp() )
						{
							action = UAIR;
							frame = 0;
							break;
						}
						else if( currInput.LDown() )
						{
							action = DAIR;
							frame = 0;
							break;
						}
					}

					action = FAIR;
					frame = 0;
				}
			}
		}
		break;
	case FAIR:
		{
			break;
		}
	case DAIR:
		{
			break;
		}
	case UAIR:
		{
			break;
		}
	case DASH:
		{
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			if( canStandUp )
			{
				if( !currInput.B )
				{
					if( currInput.LLeft() || currInput.LRight() )
					{
						action = RUN;
						frame = 0;
					}
					else
					{
						action = STAND;
						frame = 0;
					}
				}
			}
			else
			{
				frame = 10;
			}
			break;
		}
	case SLIDE:
		{
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}
			else if( currInput.B && !prevInput.B )
			{
					action = DASH;
					frame = 0;
					break;
			}
			else if( currInput.X && !prevInput.X )
			{
				action = STANDD;
				frame = 0;
				break;
			}
			else if( !currInput.LLeft() && !currInput.LRight() )
			{
				if( !currInput.LDown() )
				{
					action = STAND;
					frame = 0;
					break;
				}
			}
			else
			{
				if( currInput.LDown() )
				{
					action = SPRINT;
					frame = 0;
					break;
				}
				else
				{
					action = RUN;
					frame = 0;
					break;
				}
			}
		}
	case SPRINT:
		{
			
			if( canStandUp )
			{
			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if(!( currInput.LLeft() || currInput.LRight() ))
			{
				if( currInput.LDown())
				{
					action = SLIDE;
					frame = 0;
				}
				else if( currInput.LUp() && ( (gNorm.x < 0 && facingRight) || (gNorm.x > 0 && !facingRight) ) )
				{
					break;
				}
				else
				{
					action = STAND;
					frame = 0;
				}
				break;
				
			}
			else
			{
				if( facingRight && currInput.LLeft() )
				{
					
					if( ( currInput.LDown() && gNorm.x < 0 ) || ( currInput.LUp() && gNorm.x > 0 ) )
					{
						frame = 0;
					}
					else
					{
						action = RUN;
					}

					groundSpeed = 0;
					facingRight = false;
					frame = 0;
					break;
				}
				else if( !facingRight && currInput.LRight() )
				{
					if( ( currInput.LDown() && gNorm.x > 0 ) || ( currInput.LUp() && gNorm.x < 0 ) )
					{
						frame = 0;	
					}
					else
					{
						action = RUN;
						
					}

					groundSpeed = 0;
					facingRight = true;
					frame = 0;
					break;
				}
				else if( !( (currInput.LDown() && ((gNorm.x > 0 && facingRight) || ( gNorm.x < 0 && !facingRight ) ))
					|| (currInput.LUp() && ((gNorm.x < 0 && facingRight) || ( gNorm.x > 0 && !facingRight ) )) ) )
				{
					action = RUN;
					frame = frame / 3;
					if( frame < 3)
					{
						frame = frame + 1;
					}
					else if ( frame == 3 || frame == 4)
					{
						frame = 7;
					}
					else if ( frame == 5 || frame == 6)
					{
						frame = 8;
					}
					else if ( frame == 7)
					{
						frame = 2;
					}
					frame = frame * 4;
					break;
				}

			}
			}
			
			if( currInput.B && !prevInput.B )
			{
					action = DASH;
					frame = 0;
			}
			break;
		}

	case GRINDBALL:
		{
			if( !currInput.Y && grindEdge->Normal().y < 0 )
			{
				ground = grindEdge;
				edgeQuantity = grindQuantity;
				grindEdge = NULL;
				action = LAND;
				frame = 0;
				


				position.x += offsetX + b.offset.x;

				if( gNorm.y < 0 )
				{
					position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
					//cout << "offset: " << b.offset.y << endl;
				}

				
				if( grindSpeed > 0 )
				{
					facingRight = true;
				}
				else
				{
					facingRight = false;
				}
			}
			break;
		}
	case STEEPSLIDE:
		{
			if( reversed )
			{
				if( -gNorm.y <= -steepThresh )
				{
					action = LAND2;
					frame = 0;
				}
			}
			else
			{
				if( gNorm.y <= -steepThresh )
				{
					action = LAND2;
					frame = 0;
					//not steep
				/*if( currInput.LLeft() || currInput.RRight() )
				{
					if( currInput.LLeft() && currInput.LDown() && gNorm.x < 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LLeft() && currInput.LUp() && gNorm.x > 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LRight() && currInput.LDown() && gNorm.x > 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else if( currInput.LRight() && currInput.LUp() && gNorm.x < 0 )
					{
						action = SPRINT;
						frame = 0;
					}
					else
					{
						action = RUN;
						frame = 0;
					}
				}
				else
				{
					if( currInput.LDown() )
					{
						action = SLIDE;
						frame = 0;
					}
					else
				}*/
				}
			}
			
			break;
		}
	case AIRDASH:
		{

			break;
		}
	}
	
	//cout << "standing up? : " << CheckStandUp() << endl;
	


	
	//react to action
	switch( action )
	{
	case STAND:
		{
		groundSpeed = 0;
		break;
		}
	case RUN:
		{
		if( currInput.LLeft() )
		{
			if( groundSpeed > 0 )
			{
				groundSpeed = 0;
			}
			else
			{
				if( groundSpeed > -maxRunInit )
				{
					groundSpeed -= runAccelInit / slowMultiple;
					if( groundSpeed < -maxRunInit )
						groundSpeed = -maxRunInit;
				}
				else
				{
					groundSpeed -= runAccel / slowMultiple;
				}
				
			}
			
			if( currInput.B )
			{
				groundSpeed -= holdDashAccel / slowMultiple;
			}

			facingRight = false;
		}
		else if( currInput.LRight() )
		{
			if (groundSpeed < 0 )
				groundSpeed = 0;
			else
			{
				if( groundSpeed < maxRunInit )
				{
					groundSpeed += runAccelInit / slowMultiple;
					if( groundSpeed > maxRunInit )
						groundSpeed = maxRunInit;
				}
				else
				{
					groundSpeed += runAccel / slowMultiple;
				}
			}

			if( currInput.B )
			{
				groundSpeed -= holdDashAccel / slowMultiple;
			}

			facingRight = true;
		}

		break;
		}
	case JUMP:
		{
		if( frame == 0 )
		{
			if( ground != NULL ) //this should always be true but we haven't implemented running off an edge yet
			{
				velocity = groundSpeed * normalize(ground->v1 - ground->v0 );

				//velocity.x = groundSpeed;//(groundSpeed * normalize(ground->v1 - ground->v0 )).x;
			//	if( (groundSpeed > 0 && ground->Normal().x > 0) || (groundSpeed < 0 && ground->Normal().x < 0 ) )
					velocity.x = groundSpeed;// * normalize(ground->v1 - ground->v0 );

				if( reversed )
				{
					ground = NULL;
					reversed = false;

				}
				else
				{
					if( velocity.y > 0 )
						velocity.y = 0;
					velocity.y -= jumpStrength;
					ground = NULL;
					holdJump = true;
				}
				
			}

			
		}
		else
		{
			if( holdJump && velocity.y >= -8 )
				holdJump = false;



			if( holdJump && !currInput.A )
			{
				if( velocity.y < -8 )
				{
					velocity.y = -8;
				}
			}


			if( currInput.LLeft() )
			{
				if( velocity.x > -maxAirXControl )
				{
					velocity.x -= airAccel;
					if( velocity.x < -maxAirXControl )
						velocity.x = -maxAirXControl;
				}
				
			}
			else if( currInput.LRight() )
			{
				if( velocity.x < maxAirXControl )
				{
					velocity.x += airAccel;
					if( velocity.x > maxAirXControl )
						velocity.x = maxAirXControl;
				}
				//cout << "setting velocity.x to : "<< maxAirXControl << endl;
				
			}
			else if( !currInput.LUp() && !currInput.LDown() )
			{
				if( velocity.x > 0 )
				{
					velocity.x -= airSlow;
					if( velocity.x < 0 ) velocity.x = 0;
				}
				else if( velocity.x < 0 )
				{
					velocity.x += airSlow;
					if( velocity.x > 0 ) velocity.x = 0;
				}
			}
			//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;
			
		}
		break;
		}
	case WALLCLING:
		{
			
			if( velocity.y > clingSpeed )
			{
				//cout << "running wallcling" << endl;
				velocity.y = clingSpeed;
			}
			if( currInput.LLeft() )
			{
				//if( !( velocity.x > -maxAirXSpeedNormal && velocity.x - airAccel < -maxAirXSpeedNormal ) )
				//{
					velocity.x -= airAccel;
				//}
					
			}
			else if( currInput.LRight() )
			{
				//if( !( velocity.x < maxAirXSpeedNormal && velocity.x + airAccel > maxAirXSpeedNormal ) )
				//{
					velocity.x += airAccel;
				//}
			}

			
			break;
		}
		
	case WALLJUMP:
		{
			if( frame == 0 )
			{
				wallJumpFrameCounter = 0;
			
				if( facingRight )
				{
					velocity.x = wallJumpStrength.x;
				}
				else
				{
					velocity.x = -wallJumpStrength.x;
				}

				velocity.y = -wallJumpStrength.y;
			}
			else if( frame > 10 )
			{
			if( currInput.LLeft() )
			{
				if( velocity.x > -maxAirXControl )
				{
					velocity.x -= airAccel;
					if( velocity.x < -maxAirXControl )
						velocity.x = -maxAirXControl;
				}
				
			}
			else if( currInput.LRight() )
			{
				if( velocity.x < maxAirXControl )
				{
					velocity.x += airAccel;
					if( velocity.x > maxAirXControl )
						velocity.x = maxAirXControl;
				}
				
			}
			else if( !currInput.LUp() && !currInput.LDown() )
			{
				if( velocity.x > 0 )
				{
					velocity.x -= airSlow;
					if( velocity.x < 0 ) velocity.x = 0;
				}
				else if( velocity.x < 0 )
				{
					velocity.x += airSlow;
					if( velocity.x > 0 ) velocity.x = 0;
				}
			}
			}
			break;
		}
	case FAIR:
		{
			
			if( frame == 0 )
			{
				//fairSound.play();
			}
			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{
				if( currInput.LLeft() )
				{
					if( velocity.x > -maxAirXControl )
					{
						velocity.x -= airAccel;
						if( velocity.x < -maxAirXControl )
							velocity.x = -maxAirXControl;
					}
				
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxAirXControl )
					{
						velocity.x += airAccel;
						if( velocity.x > maxAirXControl )
							velocity.x = maxAirXControl;
					}
				
				}
				else if( !currInput.LUp() && !currInput.LDown() )
				{
					if( velocity.x > 0 )
					{
						velocity.x -= airSlow;
						if( velocity.x < 0 ) velocity.x = 0;
					}
					else if( velocity.x < 0 )
					{
						velocity.x += airSlow;
						if( velocity.x > 0 ) velocity.x = 0;
					}
				}
			}

			break;
		}
	case DAIR:
		{
			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{		
				if( currInput.LLeft() )
				{
					if( velocity.x > -maxAirXControl )
					{
						velocity.x -= airAccel;
						if( velocity.x < -maxAirXControl )
							velocity.x = -maxAirXControl;
					}
				
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxAirXControl )
					{
						velocity.x += airAccel;
						if( velocity.x > maxAirXControl )
							velocity.x = maxAirXControl;
					}
				
				}
				else if( !currInput.LUp() && !currInput.LDown() )
				{
					if( velocity.x > 0 )
					{
						velocity.x -= airSlow;
						if( velocity.x < 0 ) velocity.x = 0;
					}
					else if( velocity.x < 0 )
					{
						velocity.x += airSlow;
						if( velocity.x > 0 ) velocity.x = 0;
					}
				}
			}
			break;
		}
	case UAIR:
		{
			if( wallJumpFrameCounter >= wallJumpMovementLimit )
			{	
				if( currInput.LLeft() )
				{
					if( velocity.x > -maxAirXControl )
					{
						velocity.x -= airAccel;
						if( velocity.x < -maxAirXControl )
							velocity.x = -maxAirXControl;
					}
				
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxAirXControl )
					{
						velocity.x += airAccel;
						if( velocity.x > maxAirXControl )
							velocity.x = maxAirXControl;
					}
				
				}
				else if( !currInput.LUp() && !currInput.LDown() )
				{
					if( velocity.x > 0 )
					{
						velocity.x -= airSlow;
						if( velocity.x < 0 ) velocity.x = 0;
					}
					else if( velocity.x < 0 )
					{
						velocity.x += airSlow;
						if( velocity.x > 0 ) velocity.x = 0;
					}
				}
			}
			break;
		}
	case DASH:
		{
			b.rh = dashHeight;
			b.offset.y = (normalHeight - dashHeight);
			if( reversed )
				b.offset.y = -b.offset.y;
			if( currInput.LLeft() && facingRight )
			{
				facingRight = false;
				groundSpeed = -dashSpeed;
				frame = 0;
			}
			else if( currInput.LRight() && !facingRight )
			{
				facingRight = true;
				groundSpeed = dashSpeed;
				frame = 0;
			}
			else if( !facingRight )
			{
				if( groundSpeed > -dashSpeed )
					groundSpeed = -dashSpeed;
			}
			else
			{
				if( groundSpeed < dashSpeed )
					groundSpeed = dashSpeed;
			}

			if( currInput.LDown() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				if( facingRight )
				{
					groundSpeed += sprintAccel * abs( gNorm.x );
				}
				else 
				{
					groundSpeed -= sprintAccel * abs( gNorm.x );
				}
			}
			else if( currInput.LUp() && (( facingRight && gNorm.x > 0 ) || ( !facingRight && gNorm.x < 0 ) ) )
			{
				if( facingRight )
				{
					groundSpeed += sprintAccel/2;
				}
				else 
				{
					groundSpeed -= sprintAccel/2;
				}
			}
			else
			{
				if( facingRight )
				{
					groundSpeed += holdDashAccel;
				}
				else
				{
					groundSpeed -= holdDashAccel;
				}
			
			}
			break;
		}
	case DOUBLE:
		{
			b.rh = doubleJumpHeight;
		//	b.offset.y = -5;
			if( frame == 0 )
			{
				//velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
				if( velocity.y > 0 )
					velocity.y = 0;
				velocity.y = -doubleJumpStrength;
				hasDoubleJump = false;

				if( currInput.LLeft() )
				{
					if( velocity.x > -maxRunInit )
					{
						velocity.x = -maxRunInit;
					}
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxRunInit )
					{
						velocity.x = maxRunInit;
					}
				}
				else
				{
					velocity.x = 0;
				}
			}
			else
			{
				
						
				if( currInput.LLeft() )
				{
					if( velocity.x > -maxAirXControl )
					{
						velocity.x -= airAccel;
						if( velocity.x < -maxAirXControl )
							velocity.x = -maxAirXControl;
					}
				
				}
				else if( currInput.LRight() )
				{
					if( velocity.x < maxAirXControl )
					{
						velocity.x += airAccel;
						if( velocity.x > maxAirXControl )
							velocity.x = maxAirXControl;
					}
				
				}
				else if( !currInput.LUp() && !currInput.LDown() )
				{
					if( velocity.x > 0 )
					{
						velocity.x -= airSlow;
						if( velocity.x < 0 ) velocity.x = 0;
					}
					else if( velocity.x < 0 )
					{
						velocity.x += airSlow;
						if( velocity.x > 0 ) velocity.x = 0;
					}
				}
				//cout << PhantomResolve( owner->edges, owner->numPoints, V2d( 10, 0 ) ) << endl;
			
			}
			break;
		}
	case SLIDE:
		{
			//groundSpeed = 
			break;
		}
	case SPRINT:
		{
			b.rh = sprintHeight;
			
			

			b.offset.y = (normalHeight - sprintHeight);

			if( reversed )
				b.offset.y = -b.offset.y;

			if( currInput.LLeft() )
			{
				if( groundSpeed > 0 )
				{
					groundSpeed = 0;
				}
				else
				{
					if( groundSpeed > -maxRunInit )
					{
						groundSpeed -= runAccelInit * 2 / slowMultiple;
						if( groundSpeed < -maxRunInit )
							groundSpeed = -maxRunInit;
					}
					else
					{
						if( gNorm.x > 0 )
						{
							//up a slope
							groundSpeed -= sprintAccel / 2 / slowMultiple; 
						}
						else
						{
							groundSpeed -= sprintAccel * abs( gNorm.x ) / slowMultiple;
							//down a slope
						}
					}
				
				}
				facingRight = false;
			}
			else if( currInput.LRight() )
			{
				if (groundSpeed < 0 )
					groundSpeed = 0;
				else
				{
					V2d gn = ground->Normal();
					if( groundSpeed < maxRunInit )
					{
						groundSpeed += runAccelInit * 2 / slowMultiple;
						if( groundSpeed > maxRunInit )
							groundSpeed = maxRunInit;
					}
					else
					{
						if( gNorm.x < 0 )
						{
							//up a slope
							groundSpeed += sprintAccel / 2 / slowMultiple;
						}
						else
						{
							groundSpeed += sprintAccel * abs( gNorm.x ) / slowMultiple;
							//down a slope
						}
					}
				}
				facingRight = true;
			}

			break;
		}
	case GRINDBALL:
		{
			
			//grindSpeed =  ;
			break;
		}
	case STEEPSLIDE:
		{

			break;
		}
	case AIRDASH:
		{

			break;
		}
	}


	if( currInput.leftTrigger > 200 )
	{
		if( prevInput.leftTrigger <= 200 )
			slowCounter = 1;

		slowMultiple = timeSlowStrength;
	}
	else
	{
		slowCounter = 1;
		slowMultiple = 1;
	}

	if( ground == NULL )
	{
		if( velocity.x > maxAirXSpeed )
			velocity.x = maxAirXSpeed;
		else if( velocity.x < -maxAirXSpeed )
			velocity.x = -maxAirXSpeed;

		velocity += V2d( 0, gravity / slowMultiple );
		if( velocity.y > maxFallSpeed )
			velocity.y = maxFallSpeed;
	}
	else
	{
		if( groundSpeed > maxGroundSpeed )
			groundSpeed = maxGroundSpeed;
		else if( groundSpeed < -maxGroundSpeed )
		{
			groundSpeed = -maxGroundSpeed;
		}
	}
	//cout << "position: " << position.x << ", " << position.y << endl;
//	cout << "velocity: " << velocity.x << ", " << velocity.y << endl;
	collision = false;
	
	oldVelocity.x = velocity.x;
	oldVelocity.y = velocity.y;

	cout << "pre vel: " << velocity.x << ", " << velocity.y << endl;

	groundSpeed /= slowMultiple;
	velocity /= (double)slowMultiple;
	grindSpeed /= slowMultiple;
}

bool Actor::CheckWall( bool right )
{
	double wallThresh = 5;
	V2d vel;
	if( right )
	{
		vel.x = wallThresh;
	}
	else
	{
		vel.x = -wallThresh;
	}
	V2d newPos = (position) + vel;
	Contact test;
	test.collisionPriority = 10000;
	test.edge = NULL;


	for( int i = 0; i < owner->numPoints; ++i )
	{
		Contact *c = owner->coll.collideEdge( newPos , b, owner->edges[i], vel, owner->window );
		if( c != NULL )
		{
			if( (c->collisionPriority < test.collisionPriority ))//&& c->collisionPriority >= 0 )
			//	|| (test.collisionPriority == 10000 ) )
			{
				test.collisionPriority = c->collisionPriority;
				test.edge = c->edge;
				test.position = c->position;
				test.resolution = c->resolution;
			}
		
		}
	}

	bool wally = false;
	if( test.edge != NULL )
	{
		double quant = test.edge->GetQuantity( test.position );
		bool zero = false;
		bool one = false;
		if( quant <= 0 )
		{
			zero = true;
			quant = 0;
		}
		else if( quant >= length( test.edge->v1 - test.edge->v0 ) )
		{
			one = true;
			quant = length( test.edge->v1 - test.edge->v0 );
		}

		//if( !zero && !one )
		//	return false;

		//cout << "zero: " << zero << ", one: " << one << endl;
		//cout << "haha: "  << quant << ", " << length( test.edge->v1 - test.edge->v0 ) << endl;
		Edge *e = test.edge;
		V2d en = e->Normal();
		Edge *e0 = e->edge0;
		Edge *e1 = e->edge1;

	//	cout << "here: " << test.position.x << ", " << test.position.y << " .. " << e->v0.x << ", " << e->v0.y << endl;	

		/*CircleShape cs;
		cs.setFillColor( Color::Cyan );
		cs.setRadius( 10 );
		cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
		cs.setPosition( e->GetPoint( quant ).x, e->GetPoint( quant ).y );

		owner->window->draw( cs );*/


		if( approxEquals(en.x,1) || approxEquals(en.x,-1) )
		{
			//wallNormal = minContact.edge->Normal();
			return true;
		}

		
		if( en.y > 0 && abs( en.x ) > .8 )
		{
			//cout << "here" << endl;
			return true;
		}

		if( (zero && en.x < 0 && en.y < 0 ) )
		{
			//cout << "?>>>>>" << endl;
			V2d te = e0->v0 - e0->v1;
			if( te.x > 0 )
			{
				return true;
			}
		}
		
		if( (one && en.x < 0 && en.y > 0 ) )
		{
			//cout << "%%%%%" << endl;
			V2d te = e1->v1 - e1->v0;
			if( te.x > 0 )
			{
				return true;
			}
		}

		if( (one && en.x < 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}
		
		if( (zero && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e0->v0 - e0->v1;
			if( te.x > 0 )
			{	
				return true;
			}
		}
	
		if( ( one && en.x > 0 && en.y < 0 ) )
		{
			V2d te = e1->v1 - e1->v0;
			if( te.x < 0 )
			{
				return true;
			}
		}
		if( (zero && en.x > 0 && en.y > 0 ) )
		{
			V2d te = e0->v0 - e0->v1;
			if( te.x < 0 )
			{
				return true;
			}
		}
		

		{
		//	cout << en.x << ", " << en.y << endl;
		//	cout << "misery" << endl;
		}
	}
	return false;

}

bool Actor::CheckStandUp()
{
	if( b.rh >= normalHeight )
	{
		cout << "WEIRD" << endl;
		return true;
	}
	else
	{
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - normalHeight, 2 * b.rw, 2 * normalHeight );
	//	Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight * 2, 2 * b.rw, 2 * normalHeight * 2 );

	//	Rect<double> r( position.x + offsetX + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);

		
		Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		sf::RectangleShape rs;
		rs.setSize( Vector2f(r.width, r.height ));
		rs.setFillColor( Color::Yellow );
		rs.setPosition( r.left, r.top );

		//owner->window->draw( rs );

		//queryMode = "check";
		checkValid = true;
		Query( this, owner->testTree, r );
	//	cout << "col number: " << possibleEdgeCount << endl;
		possibleEdgeCount = 0;
		return checkValid;
	}
	
}


bool Actor::ResolvePhysics( Edge** edges, int numPoints, V2d vel )
{
	possibleEdgeCount = 0;
	position += vel;
	
	Rect<double> r( position.x + b.offset.x - b.rw, position.y + b.offset.y - b.rh, 2 * b.rw, 2 * b.rh );
	minContact.collisionPriority = 1000000;

/*	sf::RectangleShape rs;
	rs.setSize( Vector2f(r.width, r.height ));
	rs.setFillColor( Color::Yellow );
	rs.setPosition( r.left, r.top );

	owner->window->draw( rs );*/
	col = false;
	tempVel = vel;

	queryMode = "resolve";
	Query( this, owner->testTree, r );
	//cout << "resolve: " << vel.x << ", " << vel.y << endl;

	//cout << "possible edge count: " << possibleEdgeCount << ", before: " << owner->numPoints << endl;
	//int collisionNumber = 0;
			
	
	//for( int i = 0; i < numPoints; ++i )
	//{
		
	//}
	return col;
}

void Actor::UpdateReversePhysics( Edge **edges, int numPoints )
{
	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );
	wallNormal.x = 0;
	wallNormal.y = 0;
	if( ground != NULL )
	{
		movement = groundSpeed;
	}
	else
	{
		movementVec = velocity;
	}

	movement = -movement;

	if( grindEdge != NULL )
	{
		Edge *e0 = grindEdge->edge0;
		Edge *e1 = grindEdge->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();
		
		double q = grindQuantity;
		while( !approxEquals(movement, 0 ) )
		{
			double gLen = length( grindEdge->v1 - grindEdge->v0 );
			if( movement > 0 )
			{
				double extra = q + movement - gLen;
				if( extra > 0 )
				{
					movement -= gLen - q;
					grindEdge = e1;
					q = 0;
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
			else if( movement < 0 )
			{
				double extra = q + movement;
				if( extra < 0 )
				{
					movement -= movement - extra;
					grindEdge = e0;
					q = length( e0->v1 - e0->v0 );
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
		}
		grindQuantity = q;
		return;
	}
		
	while( (ground != NULL && movement != 0) || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else 
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}


			double extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();
			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			gNormal = -gNormal;
			e0n = -e0n;
			e1n = -e1n;
			offsetX = -offsetX;

			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			

			bool transferLeft =  q == 0 && movement < 0
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
				|| (offsetX == b.rw && e0n.x >= 0 && e0n.y != 0 ));
			bool transferRight = q == groundLength && movement > 0 
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && e1n.y != 0 ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
				
			if( transferLeft )
			{
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				V2d nextNorm = e0n;
				if( nextNorm.y < 0 && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && groundSpeed < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x ) )
				{
					ground = next;
					q = length( ground->v1 - ground->v0 );	
				}
				else
				{
					reversed = false;
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
					leftGround = true;

					ground = NULL;
				}
			}
			else if( transferRight )
			{
				Edge *next = ground->edge1;
				V2d nextNorm = e1n;
				if( nextNorm.y < 0 && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && groundSpeed > slopeLaunchMinSpeed && nextNorm.x > 0 ) )
				{
					ground = next;
					q = 0;
				}
				else
				{
					velocity = normalize(ground->v1 - ground->v0 ) * -groundSpeed;
						
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
					leftGround = true;
					reversed = false;
					ground = NULL;
					//cout << "leaving ground RIGHT!!!!!!!!" << endl;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "slide: " << q << ", " << offsetX << endl;
				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;
					offsetX += m;
				}

				if(!approxEquals( m, 0 ) )
				{
					bool hit = ResolvePhysics( edges, numPoints, V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{
							if( minContact.position.y >= position.y + b.rh - 5 )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offsetX = b.rw;
									continue;
								}
								

							}
							else
							{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
							}
						}
						else
						{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				

				if( m == 0 )
				{
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					break;
				}

			//	if(m != 0 )//!approxEquals( m, 0 ) )
				{	
					
					bool hit = ResolvePhysics( edges, numPoints, normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && ( minContact.edge != ground->edge0) ) || ( m < 0 && ( minContact.edge != ground->edge1 ) ) ) )
					{
						V2d eNorm = minContact.edge->Normal();
						eNorm = -eNorm;
						if( minContact.position.y < position.y + b.offset.y - b.rh + 5 && eNorm.y >= 0 )
						{
							if( minContact.position == minContact.edge->v0 ) 
							{
								if( minContact.edge->edge0->Normal().y >= 0 )
								{
									minContact.edge = minContact.edge->edge0;
									eNorm = minContact.edge->Normal();
									eNorm = -eNorm;
								}
							}
							else if( minContact.position == minContact.edge->v1 )
							{
								if( minContact.edge->edge1->Normal().y >= 0 )
								{
									minContact.edge = minContact.edge->edge1;
									eNorm = minContact.edge->Normal();
									eNorm = -eNorm;
								}
							}
						}

						if( eNorm.y < 0 )
						{
							if( minContact.position.y <= position.y + minContact.resolution.y - b.rh + b.offset.y + 5 )
							{
								double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									
								if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
								{
									cout << "BROKEN OFFSET: " << test << endl;
								}
								else
								{	
								//	cout << "c" << endl;   
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									V2d eNorm = minContact.edge->Normal();			
									offsetX = position.x + minContact.resolution.x - minContact.position.x;
									offsetX = -offsetX;
								}
							}
							else
							{
								cout << "xx" << endl;
								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								groundSpeed = 0;
								edgeQuantity = q;
								offsetX = -offsetX;
								break;
							}
						}
						else
						{
							cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;
							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
							groundSpeed = 0;
							offsetX = -offsetX;
							edgeQuantity = q;
							break;
						}						
					}
						
				}

				
			/*	else
				{
					edgeQuantity = q;
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
				//	assert( false && "secret!" );
					break;
					//offsetX = -offsetX;
			//		cout << "prev: " << e0n.x << ", " << e0n.y << endl;
					//break;
				}*/
					
			}

			offsetX = -offsetX;

			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
	}
}

void Actor::UpdatePhysics( Edge **edges, int numPoints )
{
	if( reversed )
	{
		UpdateReversePhysics( edges, numPoints );
		return;
	}



	leftGround = false;
	double movement = 0;
	double maxMovement = min( b.rw, b.rh );
	V2d movementVec;
	V2d lastExtra( 100000, 100000 );
	wallNormal.x = 0;
	wallNormal.y = 0;
	if( ground != NULL ) movement = groundSpeed;
	else
	{
		movementVec = velocity;
	}

	if( grindEdge != NULL )
	{
		Edge *e0 = grindEdge->edge0;
		Edge *e1 = grindEdge->edge1;
		V2d e0n = e0->Normal();
		V2d e1n = e1->Normal();
		
		double q = grindQuantity;
		while( !approxEquals(movement, 0 ) )
		{
			double gLen = length( grindEdge->v1 - grindEdge->v0 );
			if( movement > 0 )
			{
				double extra = q + movement - gLen;
				if( extra > 0 )
				{
					movement -= gLen - q;
					grindEdge = e1;
					q = 0;
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
			else if( movement < 0 )
			{
				double extra = q + movement;
				if( extra < 0 )
				{
					movement -= movement - extra;
					grindEdge = e0;
					q = length( e0->v1 - e0->v0 );
				}
				else
				{
					q += movement;
					movement = 0;
				}
			}
		}
		grindQuantity = q;
		return;
	}
		
	while( (ground != NULL && movement != 0) || ( ground == NULL && length( movementVec ) > 0 ) )
	{
		if( ground != NULL )
		{
			double steal = 0;
			if( movement > 0 )
			{
				if( movement > maxMovement )
				{
					steal = movement - maxMovement;
					movement = maxMovement;
				}
			}
			else 
			{
				if( movement < -maxMovement )
				{
					steal = movement + maxMovement;
					movement = -maxMovement;
				}
			}


			double extra = 0;
			bool leaveGround = false;
			double q = edgeQuantity;

			V2d gNormal = ground->Normal();


			double m = movement;
			double groundLength = length( ground->v1 - ground->v0 ); 

			if( approxEquals( q, 0 ) )
				q = 0;
			else if( approxEquals( q, groundLength ) )
				q = groundLength;

			if( approxEquals( offsetX, b.rw ) )
				offsetX = b.rw;
			else if( approxEquals( offsetX, -b.rw ) )
				offsetX = -b.rw;

			Edge *e0 = ground->edge0;
			Edge *e1 = ground->edge1;
			V2d e0n = e0->Normal();
			V2d e1n = e1->Normal();

			bool transferLeft =  q == 0 && movement < 0
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
				|| (offsetX == b.rw && e0n.x >= 0 && e0n.y != 0 ));
			bool transferRight = q == groundLength && movement > 0 
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && e1n.y != 0 ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
				
			if( transferLeft )
			{
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				if( next->Normal().y < 0 && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && groundSpeed < -slopeLaunchMinSpeed && next->Normal().x < gNormal.x ) )
				{
					ground = next;
					q = length( ground->v1 - ground->v0 );	
				}
				else
				{
					velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
					leftGround = true;

					ground = NULL;
				}
			}
			else if( transferRight )
			{
				Edge *next = ground->edge1;
				if( next->Normal().y < 0 && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && groundSpeed > slopeLaunchMinSpeed && next->Normal().x > 0 ) )
				{
					ground = next;
					q = 0;
				}
				else
				{
					velocity = normalize(ground->v1 - ground->v0 ) * groundSpeed;
						
					movementVec = normalize( ground->v1 - ground->v0 ) * extra;
						
					leftGround = true;
					ground = NULL;
					//cout << "leaving ground RIGHT!!!!!!!!" << endl;
				}

			}
			else if( changeOffset || (( gNormal.x == 0 && movement > 0 && offsetX < b.rw ) || ( gNormal.x == 0 && movement < 0 && offsetX > -b.rw ) )  )
			{
				//cout << "slide: " << q << ", " << offsetX << endl;
				if( movement > 0 )
					extra = (offsetX + movement) - b.rw;
				else 
				{
					extra = (offsetX + movement) + b.rw;
				}
				double m = movement;
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					m -= extra;
					movement = extra;

					if( movement > 0 )
					{
						offsetX = b.rw;
					}
					else
					{
						offsetX = -b.rw;
					}
				}
				else
				{
					movement = 0;
					offsetX += m;
				}

				if(!approxEquals( m, 0 ) )
				{
					bool hit = ResolvePhysics( edges, numPoints, V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{
							if( minContact.position.y >= position.y + b.rh - 5 )
							{
								if( m > 0 && eNorm.x < 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offsetX = -b.rw;
									continue;
								}
								else if( m < 0 && eNorm.x > 0 )
								{
									ground = minContact.edge;
									q = ground->GetQuantity( minContact.position );
									edgeQuantity = q;
									offsetX = b.rw;
									continue;
								}
								

							}
							else
							{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
							}
						}
						else
						{
								offsetX += minContact.resolution.x;
								groundSpeed = 0;
								break;
						}
					}
				}
			}
			else
			{
				if( movement > 0 )
				{	
					extra = (q + movement) - groundLength;
				}
				else 
				{
					extra = (q + movement);
				}
					
				if( (movement > 0 && extra > 0) || (movement < 0 && extra < 0) )
				{
					if( movement > 0 )
					{
						q = groundLength;
					}
					else
					{
						q = 0;
					}
					movement = extra;
					m -= extra;
						
				}
				else
				{
					movement = 0;
					q += m;
				}
				

				if( m == 0 )
				{
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					break;
				}

			//	if(m != 0 )//!approxEquals( m, 0 ) )
				{	
					bool down = true;
					bool hit = ResolvePhysics( edges, numPoints, normalize( ground->v1 - ground->v0 ) * m);
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
						if( down)
						{
							V2d eNorm = minContact.edge->Normal();
							if( minContact.position.y > position.y + b.offset.y + b.rh - 5 && eNorm.y >= 0 )
							{
								if( minContact.position == minContact.edge->v0 ) 
								{
									if( minContact.edge->edge0->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge0;
										eNorm = minContact.edge->Normal();
									}
								}
								else if( minContact.position == minContact.edge->v1 )
								{
									if( minContact.edge->edge1->Normal().y <= 0 )
									{
										minContact.edge = minContact.edge->edge1;
										eNorm = minContact.edge->Normal();
									}
								}
							}



							if( eNorm.y < 0 )
							{
								//bool 
								//cout << "min:" << minContact.position.x << ", " << minContact.position.y  << endl;
								//cout << "lel: " << position.y + minContact.resolution.y + b.rh - 5 << endl;
								//cout << "res: " << minContact.resolution.y << endl;

								/*CircleShape cs;
								cs.setFillColor( Color::Cyan );
								cs.setRadius( 20 );
								cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
								cs.setPosition( minContact.resolution.x, minContact.resolution.y );

								owner->window->draw( cs );
								cs.setPosition( position.x, position.y + minContact.resolution.y + b.rh - 5);
									cs.setRadius( 10 );
								cs.setFillColor( Color::Magenta );
								owner->window->draw( cs );*/

								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh + b.offset.y - 5 )
								{
									double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									
									if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										cout << "BROKEN OFFSET: " << test << endl;
									}
									else
									{	
									//	cout << "c" << endl;
										ground = minContact.edge;
										q = ground->GetQuantity( minContact.position );
										V2d eNorm = minContact.edge->Normal();			
										offsetX = position.x + minContact.resolution.x - minContact.position.x;
									}

									/*if( offsetX < -b.rw || offsetX > b.rw )
									{
										cout << "BROKEN OFFSET: " << offsetX << endl;
										assert( false && "T_T" );
									}*/
								}
								else
								{
									cout << "xx" << endl;
									q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
									groundSpeed = 0;
									edgeQuantity = q;
									break;
								}
							}
							else
							{
								cout << "zzz: " << q << ", " << eNorm.x << ", " << eNorm.y << endl;
								q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
								groundSpeed = 0;
								edgeQuantity = q;
								break;
							}
						}
						else
						{
							cout << "Sdfsdfd" << endl;
							q = ground->GetQuantity( ground->GetPoint( q ) + minContact.resolution);
							groundSpeed = 0;
							edgeQuantity = q;
							break;
						}
						
					}
						
				}

				
			/*	else
				{
					edgeQuantity = q;
					cout << "secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
				//	assert( false && "secret!" );
					break;
					//offsetX = -offsetX;
			//		cout << "prev: " << e0n.x << ", " << e0n.y << endl;
					//break;
				}*/
					
			}
			if( movement == extra )
				movement += steal;
			else
				movement = steal;

			edgeQuantity = q;
		}
		else
		{
			V2d stealVec(0,0);
			double moveLength = length( movementVec );
			V2d velDir = normalize( movementVec );
			if( moveLength > maxMovement )
			{
				stealVec = velDir * ( moveLength - maxMovement);
				movementVec = velDir * maxMovement;
			}

			V2d newVel( 0, 0 );
				
			//cout << "moving you: " << movementVec.x << ", " << movementVec.y << endl;
			bool tempCollision = ResolvePhysics( edges, numPoints, movementVec );
			V2d extraVel(0, 0);
			if( tempCollision  )
			{
				collision = true;
			//	if( length( minContact.resolution ) <= length(movementVec) )
					position += minContact.resolution;
				Edge *e = minContact.edge;
				V2d en = e->Normal();
				Edge *e0 = e->edge0;
				Edge *e1 = e->edge1;
				V2d e0n = e0->Normal();
				V2d e1n = e1->Normal();

				if( approxEquals(minContact.edge->Normal().x,1) || approxEquals(minContact.edge->Normal().x,-1) )
				{
					wallNormal = minContact.edge->Normal();
				}


				V2d extraDir =  normalize( minContact.edge->v1 - minContact.edge->v0 );
			//	if( ( minContact.position == e->v0 && en.x <= 0 && (e0n.x >= 0 || e0n.y > 0 ) ) 
			//		|| ( minContact.position == e->v1 && en.x <= 0 && (e1n.x >= 0 || e1n.y > 0 ) ) )
			//	{
			//		extraDir = V2d( -1, 0 );
			//	}
				if( (minContact.position == e->v0 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v1 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x > 0 )
					{
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}

				else if( (minContact.position == e->v1 && en.x < 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x > 0 )
					{	
						extraDir = V2d( 0, -1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v1 && en.x > 0 && en.y < 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = extraDir;
					}
				}
				else if( (minContact.position == e->v0 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.x < 0 )
					{
						extraDir = V2d( 0, 1 );
						wallNormal = extraDir;
						//cout << "this thing" << endl;
					}
				}

			//	if( approxEquals(minContact.position.x, e->v1.x) && approxEquals(minContact.position.y, e->v1.y) && en.x < 0 && en.y < 0 )
			//	{
			//		cout << "________________________________" << endl;
			//	}
			//	else if( approxEquals(minContact.position.x, e->v0.x) && approxEquals(minContact.position.y, e->v0.y) && en.x > 0 && en.y > 0 )
			//	{
			//		cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
			//	}
			//	else
				{
					//cout << "edge normal : " << minContact.edge->Normal().x << ", "<< minContact.edge->Normal().y << endl;

				}


				if( (minContact.position == e->v1 && en.x > 0 && en.y > 0 ) )
				{
					V2d te = e1->v1 - e1->v0;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
						//cout << "here" << endl;
					}
				}
				else if( (minContact.position == e->v0 && en.x < 0 && en.y > 0 ) )
				{
					V2d te = e0->v0 - e0->v1;
					if( te.y < 0 )
					{
						extraDir = V2d( -1, 0 );
					}
				}

					
				//en.y <= 0 && e0n.y >= 0  ) 
				//	|| ( minContact.position == e->v1 && en.y <= 0 && e1n.y >= 0 ) )
				/*{
					if( en.x < 0 )
					{
						extraDir = V2d( 0, -1 );
					}
					else
					{
						extraDir = V2d( 0, 1 );
					}
				}*/
				extraVel = dot( normalize( velocity ), extraDir ) * extraDir * length(minContact.resolution);
				newVel = dot( normalize( velocity ), extraDir ) * extraDir * length( velocity );
				//cout << "extra vel: " << extraVel.x << ", " << extraVel.y << endl;
				if( length( stealVec ) > 0 )
				{
					stealVec = length( stealVec ) * normalize( extraVel );
				}
				if( approxEquals( extraVel.x, lastExtra.x ) && approxEquals( extraVel.y, lastExtra.y ) )
				{
					cout << "glitcffff: " << extraVel.x << ", " << extraVel.y << endl;
					//extraVel.x = 0;k
					//extraVel.y = 0;
					//cout << "glitchffff" << endl;
					break;
					//newVel.x = 0;
					//newVel.y = 0;
						
				}
				if( length( extraVel ) > 0 )
				{
				lastExtra.x = extraVel.x;
				lastExtra.y = extraVel.y;
				}

			//	cout << "extra vel 1: " << extraVel.x << ", " << extraVel.y << endl;
			}
			else if( length( stealVec ) == 0 )
			{
				movementVec.x = 0;
				movementVec.y = 0;
			}

			//cout << framesInAir << endl;
			//cout << "blah: " << minContact.position.y - (position.y + b.rh ) << ", " << tempCollision << endl;
			int maxJumpHeightFrame = 10;
			if( ((action == JUMP && !holdJump) || framesInAir > maxJumpHeightFrame ) && tempCollision && minContact.edge->Normal().y < 0 && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{
				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );

				if( velocity.x < 0 )
				{
					groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
				}
				else if( velocity.x > 0 )
				{
					groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
				}
				//groundSpeed  = max( abs( velocity.x ), ( - ) );
				
				if( velocity.x < 0 )
				{
				//	groundSpeed = -groundSpeed;
				}

				cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << endl;

				movement = 0;
			
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;
				//cout << "groundinggg" << endl;
			}
			else if( tempCollision && currInput.B && minContact.edge->Normal().y > 0 && minContact.position.y <= position.y - b.rh + b.offset.y + 1 )
			{
				reversed = true;
				b.offset.y = -b.offset.y;
				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );

				if( velocity.x < 0 )
				{
					groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
				}
				else if( velocity.x > 0 )
				{
					groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
				}
				//groundSpeed  = max( abs( velocity.x ), ( - ) );
				
				if( velocity.x < 0 )
				{
				//	groundSpeed = -groundSpeed;
				}

				cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << endl;

				movement = 0;
			
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;
			}
			else if( tempCollision )
			{
					velocity = newVel;
			}

			if( length( extraVel ) > 0 )
			{
				movementVec = stealVec + extraVel;
			//	cout << "x1: " << movementVec.x << ", " << movementVec.y << endl;
			}

			else
			{
				movementVec = stealVec;
				//cout << "x2:  " << movementVec.x << ", " << movementVec.y << endl;
			//	cout << "x21: " << stealVec.x << ", " << stealVec.y << endl;
				//cout << "x22: " << movementVec.x << ", " << movementVec.y << endl;
			}


		}
	}
}

void Actor::UpdatePostPhysics()
{
	//if( slowMultiple > 1 )
	//{
		velocity *= (double)slowMultiple;
		groundSpeed *= slowMultiple;
		grindSpeed *= slowMultiple;

		cout << "post vel: " << velocity.x << ", " << velocity.y << endl;
	//}
	//if( collision )
	//	cout << "collision" << endl;
	//else
	//	cout << "no collision" << endl;

	V2d gn;
	if( grindEdge != NULL )
	{
		framesInAir = 0;
		V2d grindPoint = grindEdge->GetPoint( grindQuantity );
		position = grindPoint;
	}
	else if( ground != NULL )
	{
		framesInAir = 0;
		gn = ground->Normal();
		if( collision )
		{
			if( currInput.LLeft() || currInput.LRight() )
			{
				action = LAND2;
				frame = 0;
			}
			else
			{
				action = LAND;
				frame = 0;
			}
			

			hasDoubleJump = true;

			
		}
		Vector2<double> groundPoint = ground->GetPoint( edgeQuantity );
		position = groundPoint;
		
		position.x += offsetX + b.offset.x;

		if( reversed )
		{
			if( gn.y > 0 )
			{
				position.y += normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
				//cout << "offset: " << b.offset.y << endl;
			}
		}
		else
		{
			if( gn.y < 0 )
			{
				position.y += -normalHeight; //could do the math here but this is what i want //-b.rh - b.offset.y;// * 2;		
				//cout << "offset: " << b.offset.y << endl;
			}
		}
	}
	else
	{
		if( slowCounter == slowMultiple )
		{
			if( wallJumpFrameCounter < wallJumpMovementLimit )
				wallJumpFrameCounter++;
			framesInAir++;
		}
		if( collision )
		{
			//cout << "wallcling" << endl;
			if( length( wallNormal ) > 0 && oldVelocity.y > 0 )
			//if( false )
			{
				
				if( wallNormal.x > 0)
				{
					//cout << "facing right: " << endl;
					if( currInput.LLeft() )
					{
						facingRight = true;
						action = WALLCLING;
						frame = 0;
					}
				}
				else
				{
					if( currInput.LRight() )
					{
					//	cout << "facing left: " << endl;
						facingRight = false;
						action = WALLCLING;
						frame = 0;
					}
					
				}
			}
		}
		else
		

		if( action == WALLCLING && length( wallNormal ) == 0 )
		{
			action = JUMP;
			frame = 1;
		}

		if( leftGround )
		{
			action = JUMP;
			frame = 1;
		}
	}

	//display action

	switch( action )
	{
	case STAND:
		{	
			
		sprite->setTexture( *(tileset[STAND]->texture));
			
		//sprite->setTextureRect( tilesetStand->GetSubRect( frame / 4 ) );
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[STAND]->GetSubRect( frame / 8 ) );
		}
		else
		{
			sf::IntRect ir = tileset[STAND]->GetSubRect( frame / 8 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		//sprite->setRotation(  );


		if( ground != NULL )
		{
			double angle = 0;
			//cout << "offsetx: " <<  offsetX << endl;
			//if( edgeQuantity == 0 || edgeQuantity == length( ground->v1 - ground->v0 ) )
			
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			sprite->setRotation( angle / PI * 180 );


			//cout << "angle: " << angle / PI * 180  << endl;
		}

		//sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		//sprite->setPosition( position.x, position.y );
		//cout << "setting to frame: " << frame / 4 << endl;
		break;
		}
	case RUN:
		{	
			
		sprite->setTexture( *(tileset[RUN]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[RUN]->GetSubRect( frame / 4 ) );
		}
		else
		{
			sf::IntRect ir = tileset[RUN]->GetSubRect( frame / 4 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
			

		if( ground != NULL )
		{
			double angle = 0;
			
			
			//if( edgeQuantity == 0 || edgeQuantity == length( ground->v1 - ground->v0 ) )
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			//sprite->setOrigin( b.rw, 2 * b.rh );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );

			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			//sprite->setPosition( position.x, position.y );
			
			//sprite->setPosition( position.x, position.y );
			//cout << "angle: " << angle / PI * 180  << endl;
		}
		break;
		}
	case SPRINT:
		{	
			
		sprite->setTexture( *(tileset[SPRINT]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[SPRINT]->GetSubRect( frame / 3 ) );
		}
		else
		{
			sf::IntRect ir = tileset[SPRINT]->GetSubRect( frame / 3 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
			

		if( ground != NULL )
		{
			double angle = 0;
			//if( edgeQuantity == 0 || edgeQuantity == length( ground->v1 - ground->v0 ) )
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			//sprite->setOrigin( b.rw, 2 * b.rh );
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
		}
		break;
		}
	case JUMP:
		{
		sprite->setTexture( *(tileset[JUMP]->texture));
		{
		sf::IntRect ir;

		if( frame == 0 )
		{
			ir = tileset[JUMP]->GetSubRect( 0 );
		}
		else if( velocity.y < -15)
		{
			ir = tileset[JUMP]->GetSubRect( 1 );
		}
		else if( velocity.y < 7 )
		{
			ir = tileset[JUMP]->GetSubRect( 2 );
		}
		else if( velocity.y < 9 )
		{
			ir = tileset[JUMP]->GetSubRect( 3 );
		}
		else if( velocity.y < 12 )
		{
			ir = tileset[JUMP]->GetSubRect( 4 );
		}
		else if( velocity.y < 35)
		{
			ir = tileset[JUMP]->GetSubRect( 5 );
		}
		else if( velocity.y < 37 )
		{
			ir = tileset[JUMP]->GetSubRect( 6 );
		}
		else if( velocity.y < 40 )
		{
			ir = tileset[JUMP]->GetSubRect( 7 );
		}
		else
		{
			ir = tileset[JUMP]->GetSubRect( 8 );
		}

		if( frame > 0 )
		{
			sprite->setRotation( 0 );
		}

		if( !facingRight )
		{
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		else
		{
			sprite->setTextureRect( ir );
		}
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );

		break;
		}
	case LAND: 
		{
		sprite->setTexture( *(tileset[LAND]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[LAND]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[LAND]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}


		double angle = 0;
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			if( reversed )
					angle = PI;
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}

		

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );

		break;
		}
	case LAND2: 
		{
		sprite->setTexture( *(tileset[LAND2]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[LAND2]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[LAND2]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		
		double angle = 0;
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			if( reversed )
					angle = PI;
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
		sprite->setRotation( angle / PI * 180 );
		V2d pp = ground->GetPoint( edgeQuantity );
		if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
			sprite->setPosition( pp.x + offsetX, pp.y );
		else
			sprite->setPosition( pp.x, pp.y );
		break;
		}
	case WALLCLING:
		{
		sprite->setTexture( *(tileset[WALLCLING]->texture));
		if( facingRight )
		{
			sprite->setTextureRect( tileset[WALLCLING]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[WALLCLING]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}
		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
		sprite->setPosition( position.x, position.y );
		sprite->setRotation( 0 );
		break;
		}
	case WALLJUMP:
		{
			sprite->setTexture( *(tileset[WALLJUMP]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[WALLJUMP]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[WALLJUMP]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case SLIDE:
		{
		sprite->setTexture( *(tileset[SLIDE]->texture));
		if( (facingRight && !reversed ) || (!facingRight && reversed ) )
		{
			sprite->setTextureRect( tileset[SLIDE]->GetSubRect( 0 ) );
		}
		else
		{
			sf::IntRect ir = tileset[SLIDE]->GetSubRect( 0 );
				
			sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
		}

		double angle = 0;
		if( !approxEquals( abs(offsetX), b.rw ) )
		{
			if( reversed )
					angle = PI;
		}
		else
		{
			angle = atan2( gn.x, -gn.y );
		}

		sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
		sprite->setRotation( angle / PI * 180 );
		V2d pp = ground->GetPoint( edgeQuantity );
		if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
		break;
		}
	case STANDN:
		{
			sprite->setTexture( *(tileset[STANDN]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDN]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDN]->GetSubRect( frame / 2 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			
			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case STANDD:
		{
			sprite->setTexture( *(tileset[STANDD]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STANDD]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[STANDD]->GetSubRect( frame / 2 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			
			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case FAIR:
		{
	
			sprite->setTexture( *(tileset[FAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[FAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case DAIR:
		{
	
			sprite->setTexture( *(tileset[DAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[DAIR]->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[DAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case UAIR:
		{
	
			sprite->setTexture( *(tileset[UAIR]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[UAIR]->GetSubRect( frame / 3 ) );
			}
			else
			{
				sf::IntRect ir = tileset[UAIR]->GetSubRect( frame / 3 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case DOUBLE:
		{
	
			int fr = frame;
			if ( frame > 27)
			{
				fr = 27;
			}
			sprite->setTexture( *(tileset[DOUBLE]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[DOUBLE]->GetSubRect( fr / 1 ) );
			}
			else
			{
				sf::IntRect ir = tileset[DOUBLE]->GetSubRect( fr / 1 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case DASH:
		{
	
			sprite->setTexture( *(tileset[DASH]->texture));

			sf::IntRect ir;
			if( frame < 10 )
			{
				ir = tileset[DASH]->GetSubRect( frame / 2 );
			}
			else if( frame < actionLength[DASH] - 2 )
			{
				ir = tileset[DASH]->GetSubRect( 5 );
			}
			else
				ir = tileset[DASH]->GetSubRect( 6 );
			


			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( ir );
			}
			else
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			if( (angle == 0 && !reversed ) || (approxEquals(angle, PI) && reversed ))
				sprite->setPosition( pp.x + offsetX, pp.y );
			else
				sprite->setPosition( pp.x, pp.y );
			break;
		}
	case GRINDBALL:
		{
			assert( grindEdge != NULL );
			sprite->setTexture( *(tileset[GRINDBALL]->texture));

			sf::IntRect ir;
			
			ir = tileset[GRINDBALL]->GetSubRect( 0 );
			


			if( facingRight )
			{
				sprite->setTextureRect( ir );
			}
			else
			{
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}

			double angle = 0;

			if( !approxEquals( abs(offsetX), b.rw ) )
			{

			}
			else
			{
				angle = asin( dot( ground->Normal(), V2d( 1, 0 ) ) ); 
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2);
			sprite->setRotation( angle / PI * 180 );
			V2d pp = grindEdge->GetPoint( grindQuantity );
			sprite->setPosition( pp.x, pp.y );
			break;
		}
	case STEEPSLIDE:
		{
			sprite->setTexture( *(tileset[STEEPSLIDE]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPSLIDE]->GetSubRect( 0 ) );
			//	sprite->setOrigin( sprite->getLocalBounds().width - 10, sprite->getLocalBounds().height);
			}
			else
			{
				sf::IntRect ir = tileset[STEEPSLIDE]->GetSubRect( 0 );
				
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			//	sprite->setOrigin( 10, sprite->getLocalBounds().height);
			}

			double angle = 0;
			if( !approxEquals( abs(offsetX), b.rw ) )
			{
				if( reversed )
					angle = PI;
				//this should never happen
			}
			else
			{
				angle = atan2( gn.x, -gn.y );
			}

			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height );
			sprite->setRotation( angle / PI * 180 );
			V2d pp = ground->GetPoint( edgeQuantity );
			sprite->setPosition( pp.x, pp.y );
			//if( angle == 0 )
			//	sprite->setPosition( pp.x + offsetX, pp.y );
			//else
			//	sprite->setPosition( pp.x, pp.y );
			break;
		}
		
	}

	//cout << "offsetX: " << offsetX << endl;
	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	}
	else
		slowCounter++;
	//cout << "end frame: " << position.x << ", " << position.y << endl;
}

void Actor::HandleEdge( Edge *e )
{
	assert( queryMode != "" );

	if( ground == e )
			return;
		

	if( queryMode == "resolve" )
	{
		Contact *c = owner->coll.collideEdge( position + b.offset , b, e, tempVel, owner->window );
		if( c != NULL )
		{
		//	collisionNumber++;
			if( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -1 )
			{	
				if( c->collisionPriority == minContact.collisionPriority )
				{
					if( length(c->resolution) > length(minContact.resolution) )
					{
						minContact.collisionPriority = c->collisionPriority;
						minContact.edge = e;
						minContact.resolution = c->resolution;
						minContact.position = c->position;
						col = true;
					}
				}
				else
				{

					minContact.collisionPriority = c->collisionPriority;
					minContact.edge = e;
					minContact.resolution = c->resolution;
					minContact.position = c->position;
					col = true;
				}
			}
		}
	}
	else if( queryMode == "check" )
	{
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight );
		//Rect<double> r( position.x + b.offset.x - b.rw * 2, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		//Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if( IsEdgeTouchingBox( e, r ) )
		{
			checkValid = false;

		}
	}
	++possibleEdgeCount;
}