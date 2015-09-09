#version 150

//our texture samplers
uniform sampler2D u_texture;   //diffuse map

uniform float zoom;
uniform vec2 resolution;
uniform vec2 pos;
uniform vec2 lightpos;
layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
	vec4 DiffuseColor = texture2D(u_texture, gl_TexCoord[0].xy);
	if( gl_FragCoord.y < 100 )
	{
		//DiffuseColor = texture2D( u_texture, vec2( gl_TexCoord[0].x, 1 );
		gl_FragColor = gl_Color * DiffuseColor * vec4( 1, 0, 0, .5 );
		//gl_FragColor = gl_Color * DiffuseColor;
	}
	else
	{
		gl_FragColor = gl_Color * DiffuseColor;
	}
}

