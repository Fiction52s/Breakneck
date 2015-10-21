#version 120

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map

uniform vec2 topLeft;
uniform float zoom;

//layout(origin_upper_left) in vec4 gl_FragCoord;

//this makes things weird sometimes. need y coords reversed in a way.

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


uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 


const int numLights = 9;
struct LightSource
{
	bool on;
	vec3 pos;
	vec4 color;
	vec3 falloff;
};
LightSource lights[numLights];

void main()
{
	float size = 96.0;
	vec2 fc = gl_FragCoord.xy;
	fc.y = 1 - fc.y;
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	//vec2 pixelPos = vec2( fc.x / zoom, fc.y / zoom );
    vec2 pos = mod( topLeft + pixelPos, size ) / vec2( size );
	gl_FragColor = texture2D( u_texture, pos );
	
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
	
	vec4 finalfinal = vec4( 0, 0, 0, 0 );
	
	
	int numLightsOn = 0;
	for( int i = 0; i < numLights; ++i )
	{
		if( lights[i].on )
			numLightsOn = numLightsOn + 1;
	}
	
	for( int i = 0; i < numLights;  ++i )
	{
		if( !lights[i].on )
		{
			continue;
		}
		//noLights = false;
		vec4 DiffuseColor = texture2D(u_texture, pos);
		vec3 NormalMap = texture2D(u_normals, pos).rgb;
		vec2 fragC = gl_FragCoord.xy;
		fragC.y = 1 - fragC.y;
		vec3 LightDir = vec3(lights[i].pos.xy - (vec2( fragC.x, fragC.y) / Resolution.xy), lights[i].pos.z);
		LightDir.x *= Resolution.x / Resolution.y;
		float D = length(LightDir) * zoom;		
		vec3 N = normalize(NormalMap * 2.0 - 1.0);
		vec3 L = normalize(LightDir);
		vec3 Diffuse = (lights[i].color.rgb * lights[i].color.a) * max(dot(N, L), 0.0);
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a / numLights;
		float Attenuation = 1.0 / ( lights[i].falloff.x + (lights[i].falloff.y*D) + (lights[i].falloff.z*D*D) );
	//	float Attenuation = 1.0 / ( 500 - D );//(lights[i].falloff.x + (lights[i].falloff.y * D));
	//	if( D > 500 )
	//		Attenuation = 0;
			
		//float Attenuation = ( .00001 + ( 500 - D ) / 500 );
		//float Attenuation = .00001;//( .000000000001 + ( 500.0 - D ) / 500.0 );
		//if( D > 500 )
		//	Attenuation = 0;
		Attenuation = Attenuation;// / zoom;
		vec3 Intensity = Ambient + Diffuse * Attenuation;
		vec3 FinalColor = DiffuseColor.rgb * Intensity;
		
		finalfinal += vec4( FinalColor, DiffuseColor.a );
	}
	
	if( numLightsOn == 0 )
	{
		vec4 DiffuseColor = texture2D(u_texture, pos);
		
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a;
		float Intensity = 1;//.000001;
		finalfinal = vec4( DiffuseColor.rgb, DiffuseColor.a ); //* Intensity, DiffuseColor.a );
	}
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

