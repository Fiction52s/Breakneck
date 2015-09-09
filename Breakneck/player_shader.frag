#version 120

//attributes from vertex shader
varying vec4 vColor;
varying vec2 vTexCoord;

//our texture samplers
uniform sampler2D u_texture;   //diffuse map
uniform sampler2D u_normals;   //normal map

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
uniform vec3 LightPos;        //light position, normalized
uniform vec4 LightColor;      //light RGBA -- alpha is intensity
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform vec3 Falloff;         //attenuation coefficients
uniform float zoom;
uniform bool right;

const int numLights = 3;
struct LightSource
{
	bool on;
	vec3 pos;
	vec4 color;
	vec3 falloff;
};
LightSource lights[numLights];



void main() {

	lights[0].on = true;
	lights[0].pos = LightPos;
	lights[0].color = vec4( 1, 0, 0, 1 );//LightColor;
	lights[0].falloff = Falloff;
	
	lights[1].on = true;
	lights[1].pos = LightPos + vec3( .1, 0, 0 ) / zoom;
	lights[1].color = vec4( 0, 1, 0, 1 );
	lights[1].falloff = Falloff;
	
	lights[2].on = true;
	lights[2].pos = LightPos + vec3( .05, .05, 0 ) / zoom ;
	lights[2].color = vec4( 0, 0, 1, 1 );
	lights[2].falloff = Falloff;
	
    //RGBA of our diffuse color
	vec4 finalfinal = vec4( 0, 0, 0, 0 );//vec4( 1, 1, 1, 1 );  ////
	
	for( int i = 0; i < numLights;  ++i )
	{
		vec4 DiffuseColor = texture2D(u_texture, gl_TexCoord[0].xy);

		//RGB of our normal map
		vec3 NormalMap = texture2D(u_normals, gl_TexCoord[0].xy).rgb;
		
		if( !right )
		{
			NormalMap.r = 1-NormalMap.r;
		}
		
		//The delta position of light
		vec3 LightDir = vec3(lights[i].pos.xy - (gl_FragCoord.xy / Resolution.xy), lights[i].pos.z);
		
		//Correct for aspect ratio
		LightDir.x *= Resolution.x / Resolution.y;

		//Determine distance (used for attenuation) BEFORE we normalize our LightDir
		float D = length(LightDir) * zoom;

		//normalize our vectors
		vec3 N = normalize(NormalMap * 2.0 - 1.0);
		//N.r = -N.r;
		//N.r = 1-N.r;
		vec3 L = normalize(LightDir);
		

		//Pre-multiply light color with intensity
		//Then perform "N dot L" to determine our diffuse term
		vec3 Diffuse = (lights[i].color.rgb * lights[i].color.a) * max(dot(N, L), 0.0);

		//pre-multiply ambient color with intensity
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a / numLights ;

		//calculate attenuation
		float Attenuation = 1.0 / ( lights[i].falloff.x + (lights[i].falloff.y*D) + (lights[i].falloff.z*D*D) );
		//Attenuation = 100;
		Attenuation = Attenuation * 2 / zoom;
		//the calculation which brings it all together
		vec3 Intensity = Ambient + Diffuse * Attenuation;
		
		vec3 FinalColor = DiffuseColor.rgb * Intensity;
		finalfinal += vec4( FinalColor, DiffuseColor.a );
		
		//gl_FragColor =  gl_Color * vec4(FinalColor, DiffuseColor.a);
	}
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
    
}