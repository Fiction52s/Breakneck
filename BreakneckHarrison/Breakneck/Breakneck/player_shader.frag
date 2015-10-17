#version 120

//attributes from vertex shader
varying vec4 vColor;
varying vec2 vTexCoord;

//our texture samplers
uniform sampler2D u_texture;   //diffuse map
uniform sampler2D u_normals;   //normal map

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen

uniform vec3 LightPos0;        //light position, normalized
uniform vec4 LightColor0;      //light RGBA -- alpha is intensity
uniform vec3 Falloff0;         //attenuation coefficients
uniform bool On0;

uniform vec3 LightPos1;        
uniform vec4 LightColor1;      
uniform vec3 Falloff1;         
uniform bool On1;

uniform vec3 LightPos2;        
uniform vec4 LightColor2;      
uniform vec3 Falloff2;         
uniform bool On2;

uniform vec3 LightPos3;        
uniform vec4 LightColor3;
uniform vec3 Falloff3;         
uniform bool On3;

uniform vec3 LightPos4;
uniform vec4 LightColor4;
uniform vec3 Falloff4;
uniform bool On4;

uniform vec3 LightPos5;
uniform vec4 LightColor5;
uniform vec3 Falloff5;
uniform bool On5;

uniform vec3 LightPos6;
uniform vec4 LightColor6;
uniform vec3 Falloff6;
uniform bool On6;

uniform vec3 LightPos7;
uniform vec4 LightColor7;
uniform vec3 Falloff7;
uniform bool On7;

uniform vec3 LightPos8;
uniform vec4 LightColor8;
uniform vec3 Falloff8;
uniform bool On8;

uniform vec4 AmbientColor;    

uniform float zoom;
uniform bool right;

uniform float despFrame; //-1 if not desp mode


uniform bool hasPowerAirDash;
uniform bool hasPowerGravReverse;
uniform bool hasPowerBounce;
uniform bool hasPowerGrindBall;
uniform bool hasPowerTimeSlow;
uniform bool hasPowerLeftWire;
uniform bool hasPowerRightWire;
uniform bool hasPowerClones;

const int numLights = 9;
struct LightSource
{
	bool on;
	vec3 pos;
	vec4 color;
	vec3 falloff;
};
LightSource lights[numLights];

vec4 BallColors( vec4 DiffuseColor )
{		
	//blue
	if( !hasPowerAirDash && DiffuseColor.rgb == vec3( 0, 0x66/ 255.0, 0xcc / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	
	//green
	if( !hasPowerGravReverse && DiffuseColor.rgb == vec3( 0, 0xcc / 255.0, 0x44 / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	
	//yellow
	if( !hasPowerBounce && DiffuseColor.rgb == vec3( 0xff / 255.0, 0xf0 / 255.0, 0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	
	//orange
	if( !hasPowerGrindBall && DiffuseColor.rgb == vec3( 0xff / 255.0, 0xbb / 255.0, 0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	
	//red
	if( !hasPowerTimeSlow && DiffuseColor.rgb == vec3( 0xff / 255.0, 0x22 / 255.0, 0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	
	//magenta
	if( !hasPowerRightWire && DiffuseColor.rgb == vec3( 0xff / 255.0, 0, 0xff / 255.0 ) )
	{
		DiffuseColor.rgb = vec3( 0, 0, 0 );
	}
	return DiffuseColor;
}

void main() {

	lights[0].on = On0;
	lights[0].pos = LightPos0;
	lights[0].color = LightColor0;//vec4( 1, 0, 0, 1 );//LightColor;
	lights[0].falloff = Falloff0;
	
	lights[1].on = On1;
	lights[1].pos = LightPos1 ;// + vec3( .1, 0, 0 ) / zoom;
	lights[1].color = LightColor1; //vec4( 0, 1, 0, 1 );
	lights[1].falloff = Falloff1;
	
	lights[2].on = On2;
	lights[2].pos = LightPos2;// + vec3( .05, .05, 0 ) / zoom ;
	lights[2].color = LightColor2; //vec4( 0, 0, 1, 1 );
	lights[2].falloff = Falloff2;
	
	lights[3].on = On3;
	lights[3].pos = LightPos3;
	lights[3].color = LightColor3;
	lights[3].falloff = Falloff3;
	
	lights[4].on = On4;
	lights[4].pos = LightPos4;
	lights[4].color = LightColor4;
	lights[4].falloff = Falloff4;
	
	lights[5].on = On5;
	lights[5].pos = LightPos5;
	lights[5].color = LightColor5;
	lights[5].falloff = Falloff5;
	
	lights[6].on = On6;
	lights[6].pos = LightPos6;
	lights[6].color = LightColor6;
	lights[6].falloff = Falloff6;
	
	lights[7].on = On7;
	lights[7].pos = LightPos7;
	lights[7].color = LightColor7;
	lights[7].falloff = Falloff7;
	
	lights[8].on = On8;
	lights[8].pos = LightPos8;
	lights[8].color = LightColor8;
	lights[8].falloff = Falloff8;
	
    //RGBA of our diffuse color
	vec4 finalfinal = vec4( 0, 0, 0, 0 );//vec4( 1, 1, 1, 1 );  ////
	
	int numLightsOn = 0;
	for( int i = 0; i < numLights; ++i )
	{
		if( lights[i].on )
			numLightsOn++;	
	}
	
	for( int i = 0; i < numLights;  ++i )
	{
		if( !lights[i].on )
		{
			continue;
		}
		vec4 DiffuseColor = texture2D(u_texture, gl_TexCoord[0].xy);
		
		//magenta
		if( DiffuseColor.rgb == vec3( 0xff / 255.0, 0, 0xff / 255.0 ) )
		{
			DiffuseColor.rgb = vec3( 0, 0, 0 );
		}
		//orange
		if( DiffuseColor.rgb == vec3( 0xff / 255.0, 0, 0xbb / 255.0 ) )
		{
			DiffuseColor.rgb = vec3( 0, 0, 0 );
		}
		
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
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a / numLightsOn ;

		//calculate attenuation
		float Attenuation = 1.0 / ( lights[i].falloff.x + (lights[i].falloff.y*D) + (lights[i].falloff.z*D*D) );
		//float Attenuation = 1.0 / (lights[i].falloff.x + (lights[i].falloff.y * D));
		//float Attenuation = ( .00000001 + ( 500.0 - D ) / 500.0 );
		//if( D > 500 )
		//	Attenuation = 0;
		//Attenuation = 100;
		Attenuation = Attenuation;/// zoom;
		//the calculation which brings it all together
		vec3 Intensity = Ambient + Diffuse * Attenuation;
		
		vec3 FinalColor = DiffuseColor.rgb * Intensity;
		finalfinal += vec4( FinalColor, DiffuseColor.a );
		
		//gl_FragColor =  gl_Color * vec4(FinalColor, DiffuseColor.a);
	}
	vec4 doneColor = finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	if( numLightsOn == 0 )
	{
		vec4 DiffuseColor = texture2D(u_texture, gl_TexCoord[0].xy);
		
		//set up a thing to not affect ball colors
		DiffuseColor = BallColors( DiffuseColor );
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
		vec3 Intensity = Ambient;
		doneColor = vec4( DiffuseColor.rgb * Intensity, DiffuseColor.a );
	}
	if( despFrame >= 0 )
	{
		vec4 desperation = vec4( 1, 0, 0, doneColor.a );//vec4( doneColor.g, doneColor.b, doneColor.r, doneColor.a );
		gl_FragColor =  gl_Color * desperation;
		//gl_FragColor =  gl_Color * desperation;
	}
	else if( despFrame == -1 )
	{
		gl_FragColor = gl_Color * doneColor;
	}

    
}