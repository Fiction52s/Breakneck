#version 120

//our texture samplers
uniform sampler2D u_texture;   //diffuse map

uniform float zoom;
uniform vec2 resolution;
uniform vec2 pos;
uniform vec2 lightpos;
uniform float newscreen;
//layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	
	if( pixelPos.y < 540 * newscreen )
	{
		vec4 DiffuseColor = texture2D( u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y - (1 - newscreen) ) );
		gl_FragColor = gl_Color * DiffuseColor * vec4( 1, 0, 0, .5 );
		//gl_FragColor = gl_Color * DiffuseColor;
	}
	else
	{
		vec4 DiffuseColor = texture2D(u_texture, vec2( gl_TexCoord[0].x, gl_TexCoord[0].y + newscreen) );
		gl_FragColor = gl_Color * DiffuseColor;
	}
}

