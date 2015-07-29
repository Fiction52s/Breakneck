uniform sampler2D u_texture;
uniform vec2 resolution;  
uniform vec2 topLeft;
//uniform float random;
layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
	float size = 32.0;
    vec2 pos = mod( topLeft + gl_FragCoord.xy, size ) / vec2( size );
	gl_FragColor = texture2D( u_texture, pos );// * vec4( 1, 1, 1, .5 );
}