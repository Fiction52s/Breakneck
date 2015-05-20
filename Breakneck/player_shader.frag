uniform sampler2D texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    // multiply it by the color
	if( pixel.a == 1 )
	{
	//pixel = vec4( 1, 0, 0, 1);
	}
	gl_FragColor = gl_Color * pixel;
}