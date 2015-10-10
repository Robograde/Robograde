#new_shader vertex
#version 430
layout (location = 0) in vec3 pos_xyz; //Position
layout (location = 1) in vec2 wh;
layout (location = 2) in vec4 uv_tl_br; //Texture coords for top-left and bottom-right
layout (location = 3) in vec4 colour;

//out vec2 texCoords;

out VS_OUT
{
	vec3 xyz;
	vec2 wh;
	vec4 tl_br;
	vec4 colour;
} vsOutputs;

void main() 
{
	vsOutputs.xyz = pos_xyz;
	vsOutputs.wh = wh;
	vsOutputs.tl_br = uv_tl_br;
	vsOutputs.colour = colour;
}

#end_shader

#new_shader geometry
#version 430
layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in VS_OUT {
	vec3 xyz;
	vec2 wh;
	vec4 tl_br; // UVs for top left and bottom right corners
	vec4 colour;
} vsInputs[];

out GS_OUT {
	vec2 texCoord;
	vec4 colour;
} gsOutputs;

void main()
{
	gsOutputs.colour = vsInputs[0].colour;
	
	vec3	pos = vsInputs[0].xyz;
	float	width = vsInputs[0].wh.x;
	float	height = vsInputs[0].wh.y;

	vec2	tl = vsInputs[0].tl_br.xy;
	vec2	br = vsInputs[0].tl_br.zw;

	gl_Position = vec4( pos.x, pos.y - height, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( tl.x, br.y );
	EmitVertex();

	gl_Position = vec4( pos.x + width, pos.y - height, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( br.x, br.y );
	EmitVertex();

	gl_Position = vec4( pos.x, pos.y, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( tl.x, tl.y );
	EmitVertex();

	gl_Position = vec4( pos.x + width, pos.y, pos.z, 1.0f );
	gsOutputs.texCoord = vec2( br.x, tl.y );
	EmitVertex();

	EndPrimitive();
}

#end_shader


#new_shader fragment
#version 430

in GS_OUT
{
	vec2 texCoord;
	vec4 colour;
} gsInputs;


uniform sampler2D texture;
//uniform vec4 gColor;

out vec4 frag_colour;

void main() 
{
	frag_colour = texture2D(texture, gsInputs.texCoord).r * gsInputs.colour;
	//frag_colour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

#end_shader


