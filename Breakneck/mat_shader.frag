uniform sampler2D u_texture;
//uniform vec2 resolution;  
uniform vec2 vel;
//uniform float random;

void main()
{
    // lookup the pixel in the texture
	vec2 blah = gl_FragCoord.xy;
	blah.x = blah.x / 200;
	blah.y = blah.y / 200;

    vec4 pixel = texture2D(u_texture, vec2(vel.x / 100, vel.y / 100 ));
	
	
    // multiply it by the color
	if( pixel.a == 1 )
	{
	//pixel = vec4( 1, 1, 0, 1);
	}
	gl_FragColor = pixel;//gl_Color * pixel;
}