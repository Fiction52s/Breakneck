#include "Light.h"
#include <assert.h>
#include <iostream>
#include "GameSession.h"

using namespace sf;
using namespace std;

Light::Light( GameSession *own )
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
	cs.setFillColor( Color::Red );
	cs.setOrigin( cs.getLocalBounds().width / 2, cs.getLocalBounds().height / 2 );
	cs.setPosition( 0, -300 );
}

void Light::Draw( RenderTarget *target )
{
	sh.setParameter( "pos", owner->cam.pos.x, owner->cam.pos.y );
	target->draw( cs, &sh );
}