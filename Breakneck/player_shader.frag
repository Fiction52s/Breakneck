uniform sampler2D texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
//227733
    // multiply it by the color
	
    gl_FragColor = gl_Color * pixel;
	//gl_FragColor = gl_Color * vec4( 0, 0, 1, 0 );
}