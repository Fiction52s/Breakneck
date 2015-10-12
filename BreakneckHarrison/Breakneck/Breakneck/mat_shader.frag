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


uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 


const int numLights = 3;
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
	
	vec4 finalfinal = vec4( 0, 0, 0, 0 );
	
	
	bool noLights = true;
	for( int i = 0; i < numLights;  ++i )
	{
		if( !lights[i].on )
		{
			continue;
		}
		noLights = false;
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
		vec3 Ambient = AmbientColor.rgb * AmbientColor.a / numLights ;
		float Attenuation = 1.0 / ( lights[i].falloff.x + (lights[i].falloff.y*D) + (lights[i].falloff.z*D*D) );
		Attenuation = Attenuation * 2;
		vec3 Intensity = Ambient + Diffuse * Attenuation;
		vec3 FinalColor = DiffuseColor.rgb * Intensity;
		
		finalfinal += vec4( FinalColor, DiffuseColor.a );
	}
	
	if( noLights )
	{
		vec4 DiffuseColor = texture2D(u_texture, pos);
		finalfinal = DiffuseColor;
	}
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

