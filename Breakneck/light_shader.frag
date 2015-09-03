#version 150

uniform float zoom;
uniform vec2 resolution;
uniform vec2 pos;
uniform vec2 lightpos;

void main()
{
	vec4 yo = vec4( 1, 0, 0, 1 );
	float quant = 100 / length( pos );//+ gl_FragCoord.xy - lightpos );
	yo.a = quant;
	gl_FragColor = yo;
	
}

