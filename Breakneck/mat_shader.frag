uniform sampler2D u_texture;
uniform vec2 topLeft;
uniform float zoom;
uniform vec2 resolution;
layout(origin_upper_left) in vec4 gl_FragCoord;

void main()
{
	float size = 32.0;
	vec2 fc = gl_FragCoord.xy;
	fc = fc * vec2( 960, 540 ) / resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
    vec2 pos = mod( topLeft + pixelPos, size ) / vec2( size );
	gl_FragColor = texture2D( u_texture, pos );
	
}

