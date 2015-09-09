#version 150

uniform sampler2D u_texture;
uniform sampler2D u_normals;   //normal map

uniform vec2 topLeft;
uniform float zoom;

layout(origin_upper_left) in vec4 gl_FragCoord;

//this makes things weird sometimes. need y coords reversed in a way.

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
uniform vec3 LightPos;        //light position, normalized
uniform vec4 LightColor;      //light RGBA -- alpha is intensity
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform vec3 Falloff;         //attenuation coefficients

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
	fc = fc * vec2( 960, 540 ) / Resolution;
	vec2 pixelPos = vec2( fc.x * zoom, fc.y * zoom );
	//vec2 pixelPos = vec2( fc.x / zoom, fc.y / zoom );
    vec2 pos = mod( topLeft + pixelPos, size ) / vec2( size );
	gl_FragColor = texture2D( u_texture, pos );
	
	lights[0].on = true;
	lights[0].pos = LightPos;
	lights[0].color = vec4( 1, 0, 0, 1 );//LightColor;
	lights[0].falloff = Falloff;
	
	lights[1].on = true;
	lights[1].pos = LightPos + vec3( .1, 0, 0 ) / zoom;
	lights[1].color = vec4( 0, 1, 0, 1 );
	lights[1].falloff = Falloff;
	
	lights[2].on = true;
	lights[2].pos = LightPos + vec3( .05, -.05, 0 ) / zoom;
	lights[2].color = vec4( 0, 0, 1, 1 );
	lights[2].falloff = Falloff;
	
	vec4 finalfinal = vec4( 0, 0, 0, 0 );
	
	
	
	for( int i = 0; i < numLights;  ++i )
	{
		vec4 DiffuseColor = texture2D(u_texture, pos);
		vec3 NormalMap = texture2D(u_normals, pos).rgb;
		vec3 LightDir = vec3(lights[i].pos.xy - (vec2( gl_FragCoord.x, gl_FragCoord.y) / Resolution.xy), lights[i].pos.z);
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
	gl_FragColor =  gl_Color * finalfinal;//vec4(finalfinal, DiffuseColor.a);
	
	
}

