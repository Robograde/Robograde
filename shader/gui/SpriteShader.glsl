#new_shader vertex
#version 430
layout (location = 0) in vec3 pos_xyz; //Position
layout (location = 1) in float textureID;
layout (location = 2) in vec4 uv_tl_br; //Texture coords for top-left and bottom-right
layout (location = 3) in vec4 colour;
layout (location = 4) in vec2 size; //Width and height


out VS_OUT
{
	vec3 xyz;
	vec2 size;
	vec4 tl_br;
	vec4 colour;
	flat int textureID;
} vsOutputs;

/*struct SpriteMaterial
{
	vec4 Colour;
	ivec4 TextureIDPadd3;
}

layout( std430, binding =0 ) buffer SpriteMaterials
{
	SpriteMaterial g_Materials[];
}*/

void main() 
{
	vsOutputs.xyz = pos_xyz;
	vsOutputs.size = size;
	vsOutputs.tl_br = uv_tl_br;
	vsOutputs.colour = colour;
	//vsOutputs.colour = vec4( float(textureID) / 12.0, 0,  0, 1);
	vsOutputs.textureID = int( textureID );
}

#end_shader

#new_shader geometry
#version 430
layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in VS_OUT {
	vec3 xyz;
	vec2 size;
	vec4 tl_br; // UVs for top left and bottom right corners
	vec4 colour;
	flat int textureID;
} vsInputs[];

out GS_OUT {
	vec2 texCoord;
	vec4 colour;
	flat int textureID;
} gsOutputs;

void main()
{
	
	vec3	pos = vsInputs[0].xyz;
	float	width = vsInputs[0].size.x;
	float	height = vsInputs[0].size.y;

	vec2	tl = vsInputs[0].tl_br.xy;
	vec2	br = vsInputs[0].tl_br.zw;

	gsOutputs.colour = vsInputs[0].colour;
	gsOutputs.textureID = vsInputs[0].textureID;

	gl_Position = vec4( pos.x, pos.y - height, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( tl.x, tl.y );
	EmitVertex();

	gl_Position = vec4( pos.x + width, pos.y - height, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( br.x, tl.y );
	EmitVertex();

	gl_Position = vec4( pos.x, pos.y , pos.z, 1.0f );
	gsOutputs.texCoord = vec2( tl.x, br.y );
	EmitVertex();

	gl_Position = vec4( pos.x + width, pos.y , pos.z, 1.0f );
	gsOutputs.texCoord = vec2( br.x, br.y );
	EmitVertex();

	EndPrimitive();
}

#end_shader


#new_shader fragment
#version 430
#define MAX_SPRITES 512

in GS_OUT
{
	vec2 texCoord;
	vec4 colour;
	flat int textureID;
} gsInputs;

struct TextureFrame
{
	float S;
	float T;
	float NormWidth;
	float NormHeight;
};

layout( binding = 0 ) uniform sampler2D texture;

layout( binding = 1 ) uniform sampler2D specialTexture;
layout( binding = 2 ) uniform sampler2D specialTexture2;

layout( std140,binding = 2 ) uniform TextureBuffer
{
	TextureFrame g_TextureFrames[MAX_SPRITES * 4];
};

//uniform vec4 colour;

out vec4 frag_colour;

void main() 
{
	int textureID = gsInputs.textureID;
	vec4 outColour;
	
	if( textureID >= 0 )
	{
		TextureFrame frame = g_TextureFrames[gsInputs.textureID];
		vec2 texCoord;
		vec2 tex = clamp( fract( gsInputs.texCoord.xy ), 0.0, 1.0 );
		texCoord.x = float( frame.S + frame.NormWidth * tex.x ); // mod(frame.NormWidth * TexOut.x ,frame.NormWidth);  //mix(frame.S, frame.S + frame.NormWidth, TexOut.x);
		texCoord.y = float( frame.T + frame.NormHeight * tex.y ); 

	
		outColour = texelFetch( texture, ivec2( texCoord * textureSize( texture, 0 ) ), 0) * gsInputs.colour;
		//outColour = texture2D( texture, texCoord ) * gsInputs.colour;
		
		
	}
	else if( textureID == -1 )//TextureHandle
	{
		outColour = texture2D( specialTexture, gsInputs.texCoord ) * gsInputs.colour;
	}
	else //TextureHandle2
	{
		outColour = texture2D( specialTexture2, gsInputs.texCoord ) * gsInputs.colour;
	}

	if( outColour.a < 0.2 )
		discard;
	
	//frag_colour = gsInputs.colour;
	//frag_colour = vec4( gsInputs.texCoord, 0, 1 );
	frag_colour = outColour;
	
	//frag_colour = vec4( frame.S, frame.T, 0, 1 );
	//frag_colour = vec4( gsInputs.textureID  / 1200000.0, 0.0, 0.0, 1.0   );
}

#end_shader
