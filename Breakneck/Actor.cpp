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
		wallThresh = .999;
		//tileset setup
		{
		actionLength[DAIR] = 10 * 2;
		tileset[DAIR] = owner->GetTileset( "dair.png", 128, 64 );

		actionLength[DASH] = 45;
		tileset[DASH] = owner->GetTileset( "dash.png", 64, 64 );

		actionLength[DOUBLE] = 28 + 10;
		tileset[DOUBLE] = owner->GetTileset( "double.png", 64, 64 );

		actionLength[FAIR] = 10 * 2;
		fairHitboxes[4] = new list<CollisionBox>;

		CollisionBox cb;
		cb.type = CollisionBox::Hit;
		cb.isCircle = true;
		cb.offset.x = 0;
		cb.offset.y = 0;
		//cb.offsetAngle = 0;
		cb.rw = 64;
		cb.rh = 64;
		fairHitboxes[4]->push_back( cb );

		//map<int, list<CollisionBox>> &fairHit = *fairHitboxes;
		//fairHit[4].push_back( CollisionBox() );
		//CollisionBox &cb = fairHit[4].back();
		
			
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

		actionLength[AIRDASH] = 27;
		tileset[AIRDASH] = owner->GetTileset( "airdash.png", 64, 64 );

		actionLength[STEEPCLIMB] = 8 * 4;
		tileset[STEEPCLIMB] = owner->GetTileset( "steepclimb.png", 128, 64 );

		actionLength[AIRHITSTUN] = 1;
		tileset[AIRHITSTUN] = owner->GetTileset( "steepclimb.png", 128, 64 );

		actionLength[GROUNDHITSTUN] = 1;
		tileset[GROUNDHITSTUN] = owner->GetTileset( "steepslide.png", 64, 32 );

		}
		tsgsdodeca = owner->GetTileset( "dodeca.png", 64, 64 ); 	
		tsgstriblue = owner->GetTileset( "triblue.png", 64, 64 ); 	
		tsgstricym = owner->GetTileset( "tricym.png", 128, 128 ); 	
		tsgstrigreen = owner->GetTileset( "trigreen.png", 64, 64 ); 	
		tsgstrioran = owner->GetTileset( "trioran.png", 128, 128 ); 	
		tsgstripurp = owner->GetTileset( "tripurp.png", 128, 128 ); 	
		tsgstrirgb = owner->GetTileset( "trirgb.png", 128, 128 ); 	

		gsdodeca.setTexture( *tsgsdodeca->texture);
		gstriblue.setTexture( *tsgstriblue->texture);
		gstricym.setTexture( *tsgstricym->texture);
		gstrigreen.setTexture( *tsgstrigreen->texture);
		gstrioran.setTexture( *tsgstrioran->texture);
		gstripurp.setTexture( *tsgstripurp->texture);
		gstrirgb.setTexture( *tsgstrirgb->texture);

		ts_fairSword1 = owner->GetTileset( "fairsword1.png", 192, 128 );
		fairSword1.setTexture( *ts_fairSword1->texture );

		grindActionLength = 32;

		action = JUMP;
		frame = 1;
		
		timeSlowStrength = 5;
		slowMultiple = 1;
		slowCounter = 1;

		reversed = false;

		grindActionCurrent = 0;

		framesInAir = 0;
		wallJumpFrameCounter = 0;
		wallJumpMovementLimit = 10; //10 frames

		steepThresh = .4; // go between 0 and 1

		gravity = 1.9;
		maxFallSpeed = 60;

		wallJumpStrength.x = 10;
		wallJumpStrength.y = 25;
		clingSpeed = 3;

		dashSpeed = 17;
		steepClimbSpeedThresh = 15;

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
		
		airDashSpeed = 14;

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

		hasAirDash = true;
		hasGravReverse = true;

		//CollisionBox b;
		b.isCircle = false;
		//b.offsetAngle = 0;
		b.offset.x = 0;
		b.offset.y = 0;
		b.rw = 10;
		b.rh = normalHeight;
		b.type = CollisionBox::BoxType::Physics;

		
		b.type = b.Physics;

		hurtBody.isCircle = false;
		//hurtBody.offsetAngle = 0;
		hurtBody.offset.x = 0;
		hurtBody.offset.y = 0;
		hurtBody.rw = 10;
		hurtBody.rh = normalHeight;
		hurtBody.type = CollisionBox::BoxType::Hurt;

		currHitboxes = NULL;
		currHitboxInfo = NULL;
		receivedHit = NULL;
		hitlagFrames = 0;
		hitstunFrames = 0;
		invincibleFrames = 0;
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
		case STEEPCLIMB:
			frame = 0;
			break;
		case AIRHITSTUN:
			frame = 0;
			break;
		case GROUNDHITSTUN:
			frame = 0;
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

	if( receivedHit != NULL )
	{
		hitlagFrames = receivedHit->hitlagFrames;
		hitstunFrames = receivedHit->hitstunFrames;
		invincibleFrames = receivedHit->damage;
		if( ground == NULL )
		{
			action = AIRHITSTUN;
			frame = 0;
		}
		else
		{
			action = GROUNDHITSTUN;
			frame = 0;
		}
		receivedHit = NULL;
	}

	cout << "hitstunFrames: " << hitstunFrames << endl;
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
			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed < 0 )
							facingRight = true;
						else 
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed < 0 )
							facingRight = true;
						else 
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}


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
			else if( currInput.LDown() )
			{
				action = SLIDE;
				frame = 0;
			}
			
			break;
		}
	case RUN:
		{
			if( currInput.Y && !prevInput.Y && abs( groundSpeed ) > 5)
			{
				action = GRINDBALL;
				grindEdge = ground;
				frame = 0;
				grindSpeed = groundSpeed;
				grindQuantity = edgeQuantity;
				

				if( reversed )
				{
					grindSpeed = -grindSpeed;
				}
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
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
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
			if( hasAirDash && !prevInput.B && currInput.B )
			{
				action = AIRDASH;
				frame = 0;
				break;
			}

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
			if( hasAirDash && !prevInput.B && currInput.B )
			{
				action = AIRDASH;
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
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
				
					if( groundSpeed < 0 && gNorm.x > 0 || groundSpeed > 0 && gNorm.x < 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
							
						action = STEEPCLIMB;

						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
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
			
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						if( groundSpeed > 0 )
							facingRight = true;
						else
							facingRight = false;
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
					
				}
				else
				{
					if( currInput.B )
					{
						action = DASH;
						frame = 0;

						if( currInput.LLeft() )
							facingRight = false;
						else if( currInput.LRight() )
							facingRight = true;
					}
					else if( currInput.LLeft() || currInput.LRight() )
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
			if( hasAirDash && !prevInput.B && currInput.B )
			{
				action = AIRDASH;
				frame = 0;
				break;
			}


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

			if( currInput.Y && !prevInput.Y && abs( groundSpeed ) > 5)
			{
				action = GRINDBALL;
				grindEdge = ground;
				frame = 0;
				grindSpeed = groundSpeed;
				grindQuantity = edgeQuantity;
				//reversed = false;

				if( reversed )
				{
					grindSpeed = -grindSpeed;
				}
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}

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
			if( currInput.Y && !prevInput.Y && abs( groundSpeed ) > 5)
			{
				action = GRINDBALL;
				grindEdge = ground;
				frame = 0;
				grindSpeed = groundSpeed;
				grindQuantity = edgeQuantity;

				if( reversed )
				{
					grindSpeed = -grindSpeed;
				}
				break;
			}


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

			if( currInput.Y && !prevInput.Y && abs( groundSpeed ) > 5)
			{
				action = GRINDBALL;
				grindEdge = ground;
				frame = 0;
				grindSpeed = groundSpeed;
				grindQuantity = edgeQuantity;

				if( reversed )
				{
					grindSpeed = -grindSpeed;
				}
				break;
			}

			if( reversed )
			{
				if( -gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
			else
			{
				if( gNorm.y > -steepThresh && approxEquals( abs( offsetX ), b.rw ) )
				{
					if( groundSpeed > 0 && gNorm.x < 0 || groundSpeed < 0 && gNorm.x > 0 )
					{
						action = STEEPCLIMB;
						frame = 0;
						break;
					}
					else
					{
						action = STEEPSLIDE;
						frame = 0;
						break;
					}
				}
			}
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
		
			if( !currInput.Y )//&& grindEdge->Normal().y < 0 )
			{
				V2d op = position;

				V2d grindNorm = grindEdge->Normal();

				

				if( grindNorm.y < 0 )
				{
					
					double extra = 0;
					if( grindNorm.x > 0 )
					{
						offsetX = b.rw;
						extra = .1;
					}
					else if( grindNorm.x < 0 )
					{
						offsetX = -b.rw;
						extra = -.1;
					}
					else
					{
						offsetX = 0;
					}
				
					position.x += offsetX + extra;

					position.y -= normalHeight + .1;

					if( !CheckStandUp() )
					{
						position = op;
					}
					else
					{
						hasAirDash = true;
						hasGravReverse = true;
						hasDoubleJump = true;
						ground = grindEdge;
						edgeQuantity = grindQuantity;
						action = LAND;
						frame = 0;
						groundSpeed = grindSpeed;
						grindEdge = NULL;
						reversed = false;
					}

				}
				else
				{
					
					
						if( grindNorm.x > 0 )
						{
							position.x += b.rw + .1;
						}
						else if( grindNorm.x < 0 )
						{
							position.x += -b.rw - .1;
						}

						if( grindNorm.y > 0 )
							position.y += normalHeight + .1;

						if( !CheckStandUp() )
						{
							position = op;
						}
						else
						{
							//abs( e0n.x ) < wallThresh )
							if( abs( grindNorm.x ) >= wallThresh || !hasGravReverse )
							{
								velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
								action = JUMP;
								frame = 0;
								ground = NULL;
								grindEdge = NULL;
								reversed = false;
							}
							else
							{
							//	velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
								if( grindNorm.x > 0 )
								{
									offsetX = b.rw;
								}
								else if( grindNorm.x < 0 )
								{
									offsetX = -b.rw;
								}
								else
								{
									offsetX = 0;
								}

								hasAirDash = true;
								hasGravReverse = true;
								hasDoubleJump = true;


								ground = grindEdge;
								groundSpeed = -grindSpeed;
								edgeQuantity = grindQuantity;
								grindEdge = NULL;
								reversed = true;
								hasGravReverse = false;

								if( groundSpeed > 0 )
									facingRight = false;
								else if( groundSpeed < 0 )
								{
									facingRight = true;
								}

								action = LAND2;
								frame = 0;
							}
						}
					}


					
					//velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
				}
			
			break;
		}
	case STEEPSLIDE:
		{

			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}

			if( reversed )
			{
				if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
				}
			}
			else
			{
				if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
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
			if( !currInput.B)
			{
				action = JUMP;
				frame = 1;
				
				velocity = V2d( 0, 0 );
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
	case STEEPCLIMB:
		{

			/*if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				break;
			}*/

			if( currInput.A && !prevInput.A )
			{
				action = JUMP;
				frame = 0;
				groundSpeed *= .5;
				//ground = NULL;
				break;
			}

			if( reversed )
			{
				if( -gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
				}

				if( gNorm.x > 0 && groundSpeed >= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = true;
				}
				else if( gNorm.x < 0 && groundSpeed <= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = false;	
				}
			}
			else
			{
				if( gNorm.y <= -steepThresh || !( approxEquals( offsetX, b.rw ) || approxEquals( offsetX, -b.rw ) ) )
				{
					action = LAND2;
					frame = 0;
					//not steep
				}

				if( gNorm.x > 0 && groundSpeed >= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = true;
				}
				else if( gNorm.x < 0 && groundSpeed <= 0 )
				{
					action = STEEPSLIDE;
					frame = 0;
					facingRight = false;	
				}
			}

			
			break;
		}
	case AIRHITSTUN:
		{
			if( hitstunFrames == 0 )
			{
				action = JUMP;
				frame = 1;
			}
			break;
		}
	case GROUNDHITSTUN:
		{
			if( hitstunFrames == 0 )
			{
				action = LAND;
				frame = 0;
			}
			break;
		}
	}
	
	currHitboxes = NULL;

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
				groundSpeed += holdDashAccel / slowMultiple;
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
				

				//velocity.x = groundSpeed;//(groundSpeed * normalize(ground->v1 - ground->v0 )).x;
			//	if( (groundSpeed > 0 && ground->Normal().x > 0) || (groundSpeed < 0 && ground->Normal().x < 0 ) )
				//	velocity.x = groundSpeed;// * normalize(ground->v1 - ground->v0 );

				if( reversed )
				{
					velocity = -groundSpeed * normalize(ground->v1 - ground->v0 );
					ground = NULL;
					reversed = false;
					

				}
				else
				{
					velocity = groundSpeed * normalize(ground->v1 - ground->v0 );
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
			//currHitboxes = fairHitboxes;
			if( fairHitboxes.count( frame ) > 0 )
			{
				currHitboxes = fairHitboxes[frame];
			}

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
							double upMax = .3;
							double factor = abs( gNorm.x );
							if( factor > upMax  )
								factor = upMax;
							groundSpeed -= sprintAccel * factor / slowMultiple; 
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
			if( reversed )
			{
				//facingRight = !facingRight;
				//reversed = false;
				
			}
			velocity = normalize( grindEdge->v1 - grindEdge->v0 ) * grindSpeed;
			cout << "grindspeedin update: " << grindSpeed << endl;
			//else
			//grindSpeed =  ;
			break;
		}
	case STEEPSLIDE:
		{
			//if( groundSpeed > 0 )
			double fac = gravity * 2.0 / 3;
			if( reversed )
			{
				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, fac), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			break;
		}
	case AIRDASH:
		{
			if( frame == 0 )
			{
				hasAirDash = false;
				startAirDashVel = V2d( velocity.x, 0 );//velocity;//
			}
			velocity = V2d( 0, 0 );//startAirDashVel;
			//velocity = V2d( 0, 0 ) velocity.x, -gravity / slowMultiple );

			if( currInput.LUp() )
			{
				if( startAirDashVel.y > 0 )
				{
					startAirDashVel.y = 0;
					velocity.y = 0;
					velocity.y = -airDashSpeed;
				}
				else
				{
					velocity.y = -airDashSpeed;
				}
				
			}
			else if( currInput.LDown() )
			{
				if( startAirDashVel.y < 0 )
				{
					startAirDashVel.y = 0;
					velocity.y = 0;
					velocity.y = airDashSpeed;
				}
				else
				{
					velocity.y = airDashSpeed;
				}
				
			}


			if( currInput.LLeft() )
			{
				if( startAirDashVel.x > 0 )
				{
					startAirDashVel.x = 0;
					velocity.x = 0;
					velocity.x = -airDashSpeed;
				}
				else
				{
					velocity.x = min( startAirDashVel.x, -airDashSpeed );
				}
				facingRight = false;
				//velocity.y -= gravity / slowMultiple;
			}
			else if( currInput.LRight() )
			{
				if( startAirDashVel.x < 0 )
				{
					startAirDashVel.x = 0;
					velocity.x = 0;
					velocity.x = airDashSpeed;
				}
				else
				{
					velocity.x = max( startAirDashVel.x, airDashSpeed );
				}
				facingRight = true;
				//velocity.y -= gravity / slowMultiple;
			}
			
			if( velocity.x == 0 && velocity.y == 0 )
			{
				startAirDashVel = V2d( 0, 0 );
			}
			velocity.y -= gravity / slowMultiple;


			//velocity = V2d( 10, 0 );
			break;
		}
	case STEEPCLIMB:
		{
			//if( groundSpeed > 0 )
			if( reversed )
			{
				groundSpeed += dot( V2d( 0, gravity), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			else
			{
				groundSpeed += dot( V2d( 0, gravity), normalize( ground->v1 - ground->v0 )) / slowMultiple;
			}
			
			break;
		}
	case AIRHITSTUN:
		{
			hitstunFrames--;
			break;
		}
	case GROUNDHITSTUN:
		{
			hitstunFrames--;
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

	//cout << "pre vel: " << velocity.x << ", " << velocity.y << endl;

	groundSpeed /= slowMultiple;
	velocity /= (double)slowMultiple;
	grindSpeed /= slowMultiple;
}

bool Actor::CheckWall( bool right )
{
	double wThresh = 5;
	V2d vel;
	if( right )
	{
		vel.x = wThresh;
	}
	else
	{
		vel.x = -wThresh;
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
	if( b.rh > normalHeight )
	{
		cout << "WEIRD" << endl;
		return false;
	//	return true;
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

		queryMode = "check";
		checkValid = true;
		Query( this, owner->testTree, r );
	//	cout << "col number: " << possibleEdgeCount << endl;
		possibleEdgeCount = 0;
		return checkValid;
	}
	
}


bool Actor:: ResolvePhysics( V2d vel )
{
	//if( reversed )
	//	vel.x = -vel.x;
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

	//if( reversed )
	//	tempVel = -vel;
	//else
		tempVel = vel;
	minContact.edge = NULL;
	//minContact.resolution( 0, 0 );

	queryMode = "resolve";
	Query( this, owner->testTree, r );

	if( minContact.edge != NULL )
		cout << "blah: " <<  minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << endl;
	//cout << "resolve: " << vel.x << ", " << vel.y << endl;

	//cout << "possible edge count: " << possibleEdgeCount << ", before: " << owner->numPoints << endl;
	//int collisionNumber = 0;
			
	
	//for( int i = 0; i < numPoints; ++i )
	//{
		
	//}
	return col;
}

void Actor::UpdateReversePhysics()
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

			

			bool transferLeft =  q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0) ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ));
			bool transferRight = q == groundLength && movement > 0 //&& (groundSpeed > steepClimbSpeedThresh || e1n.y <= -steepThresh )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ) );
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
				
			if( transferLeft )
			{
				//cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				V2d nextNorm = e0n;
				if( nextNorm.y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && groundSpeed < -slopeLaunchMinSpeed && nextNorm.x < gNormal.x ) )
				{
					if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
					{
						groundSpeed = 0;
						offsetX = -offsetX;
						break;
					}
					else
					{
						ground = next;
						q = length( ground->v1 - ground->v0 );	
					}


					
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
				if( nextNorm.y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && groundSpeed > slopeLaunchMinSpeed && nextNorm.x > 0 ) )
				{

					if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
					{
						groundSpeed = 0;
						offsetX = -offsetX;
						break;
					}
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
					bool hit = ResolvePhysics( V2d( -m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();
						if( eNorm.y > 0 )
						{
							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh);
							if( minContact.position.y <= position.y - b.rh + 5 && !speedTransfer )
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
								offsetX -= minContact.resolution.x;
								groundSpeed = 0;
								offsetX = -offsetX;
								break;
							}
						}
						else
						{
								offsetX -= minContact.resolution.x;
								groundSpeed = 0;
								offsetX = -offsetX;
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
					cout << "reverse secret: " << gNormal.x << ", " << gNormal.y << ", " << q << ", " << offsetX <<  endl;
					groundSpeed = 0;
					offsetX = -offsetX;
					break;
				}

			//	if(m != 0 )//!approxEquals( m, 0 ) )
				{	
					
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
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
							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed >= -steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed <= steepClimbSpeedThresh);
							if( minContact.position.y <= position.y + minContact.resolution.y - b.rh + b.offset.y + 5 && !speedTransfer)
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

void Actor::UpdatePhysics()
{
	if( reversed )
	{
		UpdateReversePhysics();
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
		//cout << "grindSpeed: " << grindSpeed << endl;
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

			bool transferLeft =  q == 0 && movement < 0 //&& (groundSpeed < -steepClimbSpeedThresh || e0n.y <= -steepThresh || e0n.x <= 0 )
				&& ((gNormal.x == 0 && e0n.x == 0 )
				|| ( offsetX == -b.rw && (e0n.x <= 0 || e0n.y > 0)  ) 
				|| (offsetX == b.rw && e0n.x >= 0 && abs( e0n.x ) < wallThresh ) );
			bool transferRight = q == groundLength && movement > 0 //(groundSpeed < -steepClimbSpeedThresh || e1n.y <= -steepThresh || e1n.x >= 0 )
				&& ((gNormal.x == 0 && e1n.x == 0 )
				|| ( offsetX == b.rw && ( e1n.x >= 0 || e1n.y > 0 ))
				|| (offsetX == -b.rw && e1n.x <= 0 && abs( e1n.x ) < wallThresh ));
		//	cout << "transferRight: " << transferRight << ": offset: " << offsetX << endl;
			bool offsetLeft = movement < 0 && offsetX > -b.rw && ( (q == 0 && e0n.x < 0) || (q == groundLength && gNormal.x < 0) );
				
			bool offsetRight = movement > 0 && offsetX < b.rw && ( ( q == groundLength && e1n.x > 0 ) || (q == 0 && gNormal.x > 0) );
			bool changeOffset = offsetLeft || offsetRight;
				
			if( transferLeft )
			{
			//	cout << "transfer left "<< endl;
				Edge *next = ground->edge0;
				if( next->Normal().y < 0 && abs( e0n.x ) < wallThresh && !(currInput.LUp() && !currInput.LLeft() && gNormal.x > 0 && groundSpeed < -slopeLaunchMinSpeed && next->Normal().x < gNormal.x ) )
				{
					if( e0n.x > 0 && e0n.y > -steepThresh && groundSpeed >= -steepClimbSpeedThresh )
					{
					//	cout << "success?: " << e0n.y << ", gs: " << groundSpeed << "st: " << steepThresh <<
					//		", scst: " << steepClimbSpeedThresh  << endl;
						groundSpeed = 0;
						break;
					}
					else
					{
					//	cout << "e0ny: " << e0n.y << ", gs: " << groundSpeed << "st: " << steepThresh <<
					//		", scst: " << steepClimbSpeedThresh  << endl;
						ground = next;
						q = length( ground->v1 - ground->v0 );	
					}
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
				if( next->Normal().y < 0 && abs( e1n.x ) < wallThresh && !(currInput.LUp() && !currInput.LRight() && gNormal.x < 0 && groundSpeed > slopeLaunchMinSpeed && next->Normal().x > 0 ) )
				{

					if( e1n.x < 0 && e1n.y > -steepThresh && groundSpeed <= steepClimbSpeedThresh )
					{
						groundSpeed = 0;
						break;
					}
					else
					{
						ground = next;
						q = 0;
					}
					
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
			//	cout << "slide: " << q << ", " << offsetX << endl;
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
					bool hit = ResolvePhysics( V2d( m, 0 ));
					if( hit && (( m > 0 && minContact.edge != ground->edge0 ) || ( m < 0 && minContact.edge != ground->edge1 ) ) )
					{
					
						V2d eNorm = minContact.edge->Normal();
						if( eNorm.y < 0 )
						{

							bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
							if( minContact.position.y >= position.y + b.rh - 5 && !speedTransfer)
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
					groundSpeed = 0;
					break;
				}

			//	if(m != 0 )//!approxEquals( m, 0 ) )
				{	
					bool down = true;
					bool hit = ResolvePhysics( normalize( ground->v1 - ground->v0 ) * m);
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
								bool speedTransfer = (eNorm.x < 0 && eNorm.y > -steepThresh && groundSpeed > 0 && groundSpeed <= steepClimbSpeedThresh)
									|| (eNorm.x >0  && eNorm.y > -steepThresh && groundSpeed < 0 && groundSpeed >= -steepClimbSpeedThresh);
								if( minContact.position.y >= position.y + minContact.resolution.y + b.rh + b.offset.y - 5  && !speedTransfer)
								{
									double test = position.x + b.offset.x + minContact.resolution.x - minContact.position.x;
									
									if( (test < -b.rw && !approxEquals(test,-b.rw))|| (test > b.rw && !approxEquals(test,b.rw)) )
									{
										cout << "BROKEN OFFSET: " << test << endl;
									}
									else
									{	
										cout << "c" << endl;
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
			bool tempCollision = ResolvePhysics( movementVec );
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

				if( abs(minContact.edge->Normal().x) > wallThresh )//approxEquals(minContact.edge->Normal().x,1) || approxEquals(minContact.edge->Normal().x,-1) )
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
			if( ((action == JUMP && !holdJump) || framesInAir > maxJumpHeightFrame ) && tempCollision && minContact.edge->Normal().y < 0 && abs( minContact.edge->Normal().x ) < wallThresh  && minContact.position.y >= position.y + b.rh + b.offset.y - 1  )
			{


				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				V2d gNorm = ground->Normal();
				

				//if( gNorm.y <= -steepThresh )
				{
					hasGravReverse = true;
					hasAirDash = true;
					hasDoubleJump = true;
				}

				if( velocity.x < 0 && gNorm.y <= -steepThresh )
				{
					groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7);
				}
				else if( velocity.x > 0 && gNorm.y <= -steepThresh )
				{
					groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ) * .7 );
				}
				//groundSpeed  = max( abs( velocity.x ), ( - ) );
				
				if( velocity.x < 0 )
				{
				//	groundSpeed = -groundSpeed;
				}

				cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << ", offset: " << offsetX << endl;

				movement = 0;
			
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;
				cout << "offset now!: " << offsetX << endl;
				//V2d gn = ground->Normal();
				
				if( ground->Normal().x > 0 && offsetX < b.rw && !approxEquals( offsetX, b.rw ) )					
				{
					//cout << "super secret fix offsetx1: " << offsetX << endl;
					//offsetX = b.rw;
				}
				if( ground->Normal().x < 0 && offsetX > -b.rw && !approxEquals( offsetX, -b.rw ) ) 
				{
					//cout << "super secret fix offsetx2: " << offsetX << endl;
					//offsetX = -b.rw;
				}
				//cout << "groundinggg" << endl;
			}
			else if( hasGravReverse && tempCollision && currInput.B && currInput.LUp() && minContact.edge->Normal().y > 0 && abs( minContact.edge->Normal().x ) < wallThresh && minContact.position.y <= position.y - b.rh + b.offset.y + 1 )
			{
				hasGravReverse = false;
				hasAirDash = true;
				hasDoubleJump = true;
				reversed = true;

				b.offset.y = -b.offset.y;
				groundOffsetX = ( (position.x + b.offset.x ) - minContact.position.x) / 2; //halfway?
				ground = minContact.edge;
				edgeQuantity = minContact.edge->GetQuantity( minContact.position );
				double groundLength = length( ground->v1 - ground->v0 );
				groundSpeed = 0;
				//groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );//velocity.x;//length( velocity );
				V2d gno = ground->Normal();
				//cout << "gno: " << gno.x << ", " << gno.y << endl;
				if( -gno.y > -steepThresh )
				{
					groundSpeed = -dot( velocity, normalize( ground->v1 - ground->v0 ) );
					if( velocity.x < 0 )
					{
					//	groundSpeed = -min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					else if( velocity.x > 0 )
					{
					//	groundSpeed = -max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					//groundSpeed = 0;
				}
				else
				{
					groundSpeed = dot( velocity, normalize( ground->v1 - ground->v0 ) );
					if( velocity.x < 0 )
					{
						groundSpeed = min( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
					else if( velocity.x > 0 )
					{
						groundSpeed = max( velocity.x, dot( velocity, normalize( ground->v1 - ground->v0 ) ));
					}
				}

				cout << "groundspeed: " << groundSpeed << " .. vel: " << velocity.x << ", " << velocity.y << endl;

				movement = 0;
			
				offsetX = ( position.x + b.offset.x )  - minContact.position.x;

				if( ground->Normal().x > 0 && offsetX < b.rw && !approxEquals( offsetX, b.rw ) )					
				{
				//	cout << "super secret fix offsetx122: " << offsetX << endl;
				//	offsetX = b.rw;
				}
				if( ground->Normal().x < 0 && offsetX > -b.rw && !approxEquals( offsetX, -b.rw ) ) 
				{
				//	cout << "super secret fix offsetx222: " << offsetX << endl;
				//	offsetX = -b.rw;
				}
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

void Actor::UpdateHitboxes()
{
	double angle = 0;
	if( ground != NULL )
	{

		V2d gn = ground->Normal();
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
	}

	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			if( ground != NULL )
			{
				(*it).globalAngle = angle;
			}
			else
			{
				(*it).globalAngle = 0;
			}

			(*it).globalPosition = position + V2d( (*it).offset.x * cos( (*it).globalAngle ) + (*it).offset.y * sin( (*it).globalAngle ), 
				(*it).offset.x * -sin( (*it).globalAngle ) + (*it).offset.y * cos( (*it).globalAngle ) );

			//(*it).globalPosition = position + (*it).offset;
		
		}
	}
	
	
	hurtBody.rw = b.rw;
	hurtBody.rh = b.rh;
	hurtBody.offset = b.offset;
	cout << "hurtbody offset: " << hurtBody.offset.x << ", " << hurtBody.offset.y << endl;
	hurtBody.globalAngle = angle;
	hurtBody.globalPosition = position + hurtBody.offset;
	//hurtBody.globalPosition = position + V2d( hurtBody.offset.x * cos( hurtBody.globalAngle ) + hurtBody.offset.y * sin( hurtBody.globalAngle ), 
	//			hurtBody.offset.x * -sin( hurtBody.globalAngle ) + hurtBody.offset.y * cos( hurtBody.globalAngle ) );
	//hurtBody.globalPosition = position;

	b.globalPosition = position + b.offset;
	b.globalAngle = 0;
		
}

void Actor::UpdatePostPhysics()
{
	//if( slowMultiple > 1 )
	//{
		velocity *= (double)slowMultiple;
		groundSpeed *= slowMultiple;
		grindSpeed *= slowMultiple;

	//	cout << "post vel: " << velocity.x << ", " << velocity.y << endl;
	//}
	//if( collision )
	//	cout << "collision" << endl;
	//else
	//	cout << "no collision" << endl;

	V2d gn;

	

	//cout << "frame: " << frame << endl;
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
			if( action == AIRHITSTUN )
			{
				action = GROUNDHITSTUN;
				frame = 0;
			}
			else
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
			}
			

//			hasDoubleJump = true;
//			hasAirDash = true;
			//hasGravReverse = true;
			
			//cout << "wallcling" << endl;
			if( length( wallNormal ) > 0 && oldVelocity.y > 0 )
			//if( false )
			{
				
				if( wallNormal.x > 0)
				{
					//cout << "facing right: " << endl;
					if( currInput.LLeft() )
					{
					//	facingRight = true;
					//	action = WALLCLING;
					//	frame = 0;
					}
				}
				else
				{
					if( currInput.LRight() )
					{
					//	cout << "facing left: " << endl;
					//	facingRight = false;
					//	action = WALLCLING;
					//	frame = 0;
					}
					
				}
			}	
		}
		


		if( action == WALLCLING && abs( gn.x ) <= wallThresh ) //length( wallNormal ) == 0 )
		{
		//	action = STAND;
		//	frame = 1;
		//	action = JUMP;
		//	frame = 1;
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


		if( reversed )
		{
			if( ( action == STEEPCLIMB || action == STEEPSLIDE ) && (-gn.y <= -steepThresh || !approxEquals( abs( offsetX ), b.rw ) ) )
			{
				action = LAND2;
				frame = 0;
			}
		}
		else
		{
			
			if( ( action == STEEPCLIMB || action == STEEPSLIDE ) && (gn.y <= -steepThresh || !approxEquals( abs( offsetX ), b.rw ) ) )
			{
				action = LAND2;
				frame = 0;
			}
			else
			{

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

		if( action == GROUNDHITSTUN )
		{
			action = AIRHITSTUN;
			frame = 0;
		}

		if( action != AIRHITSTUN )
		{
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
			else if( action == WALLCLING && length( wallNormal ) == 0 )
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
			showSword1 = frame / 2 >= 2 && frame / 2 <= 10;
			sprite->setTexture( *(tileset[FAIR]->texture));

			Vector2i offset( -32, -32 );

			if( facingRight )
			{
				sprite->setTextureRect( tileset[FAIR]->GetSubRect( frame / 2 ) );
				//2 - 10
				if( showSword1 )
					fairSword1.setTextureRect( ts_fairSword1->GetSubRect( frame / 2 ) );
			}
			else
			{
				sf::IntRect ir = tileset[FAIR]->GetSubRect( frame / 2 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );

				

				if( showSword1  )
				{
					offset.x = -offset.x;
					//xoffset = -xoffset;
					sf::IntRect irSword = ts_fairSword1->GetSubRect( frame / 2 );
					fairSword1.setTextureRect( sf::IntRect( irSword.left + irSword.width, 
						irSword.top, -irSword.width, irSword.height ) );
				}
					
			}

			fairSword1.setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			fairSword1.setPosition( position.x + offset.x, position.y + offset.y );

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
			
			grindActionCurrent += grindSpeed / 20;
			while( grindActionCurrent >= grindActionLength )
			{
				grindActionCurrent -= grindActionLength;
			}
			while( grindActionCurrent < 0 )
			{
				grindActionCurrent += grindActionLength;
			}

			int grindActionInt = grindActionCurrent;




			gsdodeca.setTextureRect( tsgsdodeca->GetSubRect( grindActionInt * 5 % grindActionLength   ) );
			gstriblue.setTextureRect( tsgstriblue->GetSubRect( grindActionInt * 5 % grindActionLength ) );
			gstricym.setTextureRect( tsgstricym->GetSubRect( grindActionInt ) );
			gstrigreen.setTextureRect( tsgstrigreen->GetSubRect( grindActionInt * 5 % grindActionLength ) );
			gstrioran.setTextureRect( tsgstrioran->GetSubRect( grindActionInt ));
			gstripurp.setTextureRect( tsgstripurp->GetSubRect( grindActionInt ) );
			gstrirgb.setTextureRect( tsgstrirgb->GetSubRect( grindActionInt ) );

			

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
		//	sprite->setRotation( angle / PI * 180 );
			sprite->setRotation( 0 );
			V2d pp = grindEdge->GetPoint( grindQuantity );
			sprite->setPosition( pp.x, pp.y );


			gsdodeca.setOrigin( gsdodeca.getLocalBounds().width / 2, gsdodeca.getLocalBounds().height / 2);
			gstriblue.setOrigin( gstriblue.getLocalBounds().width / 2, gstriblue.getLocalBounds().height / 2);
			gstricym.setOrigin( gstricym.getLocalBounds().width / 2, gstricym.getLocalBounds().height / 2);
			gstrigreen.setOrigin( gstrigreen.getLocalBounds().width / 2, gstrigreen.getLocalBounds().height / 2);
			gstrioran.setOrigin( gstrioran.getLocalBounds().width / 2, gstrioran.getLocalBounds().height / 2);
			gstripurp.setOrigin( gstripurp.getLocalBounds().width / 2, gstripurp.getLocalBounds().height / 2);
			gstrirgb.setOrigin( gstrirgb.getLocalBounds().width / 2, gstrirgb.getLocalBounds().height / 2);


			gsdodeca.setPosition( pp.x, pp.y );
			gstriblue.setPosition( pp.x, pp.y );
			gstricym.setPosition( pp.x, pp.y );
			gstrigreen.setPosition( pp.x, pp.y );
			gstrioran.setPosition( pp.x, pp.y );
			gstripurp.setPosition( pp.x, pp.y );
			gstrirgb.setPosition( pp.x, pp.y );

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
	case AIRDASH:
		{
			sprite->setTexture( *(tileset[AIRDASH]->texture));

			int f = 0;
			if( currInput.LUp() )
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 2;
				}
				else
				{
					f = 1;
				}
			}
			else if( currInput.LDown() )
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 4;
				}
				else
				{
					f = 5;
				}
			}
			else
			{
				if( currInput.LLeft() || currInput.LRight() )
				{
					f = 3;
				}
				else
				{
					f = 0;
				}
			}
			if( facingRight )
			{
				
				sprite->setTextureRect( tileset[AIRDASH]->GetSubRect( f ) );
			}
			else
			{
				sf::IntRect ir = tileset[AIRDASH]->GetSubRect( f );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case STEEPCLIMB:
		{
			sprite->setTexture( *(tileset[STEEPCLIMB]->texture));
			if( (facingRight && !reversed ) || (!facingRight && reversed ) )
			{
				sprite->setTextureRect( tileset[STEEPCLIMB]->GetSubRect( frame / 4 ) );
			//	sprite->setOrigin( sprite->getLocalBounds().width - 10, sprite->getLocalBounds().height);
			}
			else
			{
				sf::IntRect ir = tileset[STEEPCLIMB]->GetSubRect( frame / 4 );
				
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
	case AIRHITSTUN:
		{
			sprite->setTexture( *(tileset[AIRHITSTUN]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[AIRHITSTUN]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[AIRHITSTUN]->GetSubRect( 0 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	case GROUNDHITSTUN:
		{
			sprite->setTexture( *(tileset[GROUNDHITSTUN]->texture));
			if( facingRight )
			{
				sprite->setTextureRect( tileset[GROUNDHITSTUN]->GetSubRect( 0 ) );
			}
			else
			{
				sf::IntRect ir = tileset[GROUNDHITSTUN]->GetSubRect( 0 );
				sprite->setTextureRect( sf::IntRect( ir.left + ir.width, ir.top, -ir.width, ir.height ) );
			}
			sprite->setOrigin( sprite->getLocalBounds().width / 2, sprite->getLocalBounds().height / 2 );
			sprite->setPosition( position.x, position.y );
			sprite->setRotation( 0 );
			break;
		}
	}

	if( slowCounter == slowMultiple )
	{
		++frame;
		slowCounter = 1;
	}
	else
		slowCounter++;

	UpdateHitboxes();
}

void Actor::HandleEdge( Edge *e )
{
	assert( queryMode != "" );

	if( ground == e )
			return;
		

	if( queryMode == "resolve" )
	{
		if( e == ground )
			return;
		Contact *c = owner->coll.collideEdge( position + b.offset , b, e, tempVel, owner->window );
		if( c != NULL )
		{
			cout << possibleEdgeCount << ", " << c->collisionPriority << " x: " << e->Normal().x <<" ," << e->Normal().y << endl;
		//	collisionNumber++;
			//if( ( c->collisionPriority <= minContact.collisionPriority && minContact.collisionPriority >= 0 ) 
			//	|| minContact.collisionPriority < -.001 && c->collisionPriority >= 0 )
			if( !col || (c->collisionPriority >= -.00001 && ( c->collisionPriority <= minContact.collisionPriority || minContact.collisionPriority < -.00001 ) ) )
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
					if( minContact.edge != NULL )
					cout << minContact.edge->Normal().x << ", " << minContact.edge->Normal().y << "... " 
						<< e->Normal().x << ", " << e->Normal().y << endl;
					minContact.collisionPriority = c->collisionPriority;
					cout << "pri: " << c->collisionPriority << endl;
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
		if ( action != GRINDBALL )
			if( ( e->Normal().y <= 0 && !reversed ) || ( e->Normal().y >= 0 && reversed ) )
				return;
		Rect<double> r( position.x + b.offset.x - b.rw, position.y /*+ b.offset.y*/ - normalHeight, 2 * b.rw, 2 * normalHeight);
		if( IsEdgeTouchingBox( e, r ) )
		{
			checkValid = false;

		}
	}
	++possibleEdgeCount;
}

void Actor::ApplyHit( HitboxInfo *info )
{
	if( receivedHit == NULL || info->damage > receivedHit->damage )
	{
		receivedHit = info;
	}
}

void Actor::Draw( sf::RenderTarget *target )
{
	if( action != GRINDBALL )
	{
		target->draw( *sprite );

		if( action == FAIR && showSword1 )
			target->draw( fairSword1 );
	}
	else
	{
		target->draw( *sprite );
		target->draw( gsdodeca );
		target->draw( gstriblue );
		target->draw( gstricym );
		target->draw( gstrigreen );
		target->draw( gstrioran );
		target->draw( gstripurp );
		target->draw( gstrirgb );
	}
}

void Actor::DebugDraw( RenderTarget *target )
{
	if( currHitboxes != NULL )
	{
		for( list<CollisionBox>::iterator it = currHitboxes->begin(); it != currHitboxes->end(); ++it )
		{
			(*it).DebugDraw( target );
		}
	}

	hurtBody.DebugDraw( target );
	b.DebugDraw( target );
}