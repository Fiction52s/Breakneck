#include "Light.h"
#include <assert.h>
#include <iostream>
#include "GameSession.h"

using namespace sf;
using namespace std;
Light::Light( GameSession *own, sf::Vector2i &p, Color &c )
	:color( c ), pos( p ), next( NULL )
{
	owner = own;
	if (!sh.loadFromFile("light_shader.frag", sf::Shader::Fragment))
	//if (!sh.loadFromMemory(fragmentShader, sf::Shader::Fragment))
	{
		cout << "LIGHt SHADER NOT LOADING CORRECTLY" << endl;
		assert( 0 && "light shader not loaded" );
	}
	sh.setParameter( "pos", 0, 0 );
	sh.setParameter( "lightpos", 0, -300 );

	cs.setRadius( 100 );
	cs.setFillColor( color );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	//cs.setPosition( 0, -300 );
	cs.setPosition( p.x, p.y );
}

void Light::HandleQuery( QuadTreeCollider * qtc )
{
	qtc->HandleEntrant( this );
}

bool Light::IsTouchingBox( sf::Rect<double> &r )
{
	sf::FloatRect bounds = cs.getGlobalBounds();
	sf::Rect<double> r2( bounds.left, bounds.top, bounds.width, bounds.height );
	if( r.intersects( r2 ) )
		return true;

	return false;
}

void Light::Draw( RenderTarget *target )
{
	sh.setParameter( "pos", owner->cam.pos.x, owner->cam.pos.y );
	target->draw( cs, &sh );
}