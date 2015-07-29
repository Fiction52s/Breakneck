uniform sampler2D u_texture;
//uniform vec2 resolution;  
uniform vec2 topLeft;
//uniform float random;

void main()
{
    // lookup the pixel in the texture
	vec2 blah = gl_FragCoord.xy;
	//blah.x = 0;
	//blah.y = 0;
	blah.x = mod( (blah.x ), 16 );
	
	
	blah.y = mod( (blah.y ), 16 );
	//blah.y = blah.y / 200;
	
	
    vec4 pixel = texture2D(u_texture, blah).bgra;
	
	
    // multiply it by the color
	if( pixel.a == 1 )
	{
	//pixel = vec4( 1, 1, 0, 1);
	}
	gl_FragColor = pixel;//gl_Color * pixel;
	
	vec2 test = ( mod( topLeft + vec2( gl_FragCoord.x, -gl_FragCoord.y), 32 ), mod( topLeft + vec2( gl_FragCoord.x, -gl_FragCoord.y ), 32 ) );
	test = test / vec2( 32, 32 );
	gl_FragColor = texture2D( u_texture, test );
}