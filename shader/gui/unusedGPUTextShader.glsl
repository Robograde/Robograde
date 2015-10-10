#new_shader vertex
#version 330

//out vec2 texCoords;

out VS_OUT
{
	vec2 pos;
} vsOutputs;

void main() 
{
	//texCoords = vp_vt.zw;

	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
	
	//vsOutputs.pos = vec2(0); //vpos;
}

#end_shader

#new_shader geometry
#version 430
//#define MaxTextLength 256

layout(points) in;
layout(triangle_strip, max_vertices=512) out;

//In and outputs
in VS_OUT {
	vec2 pos;
} vsInputs[];

out GS_OUT {
	vec2 texCoord;
} gsOutputs;

//Structures
struct CharacterInfo
{
	int ax;
	int ay;
	int bw;
	int bh;
	int bl;
	int bt;
	float tx;
};

//Per font uniforms
layout( std140, binding = 1 ) uniform CharInfoBuffer
{
	CharacterInfo CharInfo[ 128 ]; 
};


uniform vec2 WindowSize;
uniform vec2 FontAtlasSize;

//Per text Uniforms
layout( std430, binding = 2 ) buffer TextBuffer
{
	int Text[ 512 ];
};

uniform vec2 TextPosition;
uniform int TextLength;


void main()
{
	float x = (((TextPosition.x)* 2) - 1);
	float y = -(((TextPosition.y)* 2) - 1);
	
	
	vec2		pos;
	float	width;
	float	height;

	vec2	tl;
	vec2	br;
	//vec2	FontAtlasSize = vec2( 1280, 700 );
	        
	float sx = 2.0 / WindowSize.x;
	float sy = 2.0 / WindowSize.y;
	
	float x2, y2, w, h;
	//int c;
	
	for( int i = 0; i < TextLength; i++ )
	{
		//int a = Text[0];
		int c = Text[i];
		
		x2 = x + CharInfo[c].bl * sx;
		y2 = -y - CharInfo[c].bt * sy;
		w = CharInfo[c].bw * sx;
		h = CharInfo[c].bh * sy;
		
		//Advance cursor
		x += CharInfo[c].ax * sx;
		y += CharInfo[c].ay * sy;
		
		
		//pos = vec2( x2, -y2 );
		//width = w;
		//height = h;
		
		
		pos = vec2( 0 + (0.1 * i), 0 + (0.1 * i) );
		width = 0.5;
		height = 0.5;
		

	
		//tl = vec2( CharInfo[c].tx, 0.0 );
		//br = vec2 ( CharInfo[c].bh / FontAtlasSize.x, CharInfo[c].tx + CharInfo[c].bw / FontAtlasSize.y );
		
		//Create vertices
		gl_Position = vec4( pos.x, pos.y, 0.0f, 1.0f );
		//gsOutputs.texCoord = vec2( tl.x, tl.y );
		EmitVertex();
	
		gl_Position = vec4( pos.x + width, pos.y, 0.0f, 1.0f );
		//gsOutputs.texCoord = vec2( br.x, tl.y );
		EmitVertex();
	
		gl_Position = vec4( pos.x, pos.y - height, 0.0f, 1.0f );
		//gsOutputs.texCoord = vec2( tl.x, br.y );
		EmitVertex();
	
		gl_Position = vec4( pos.x + width, pos.y - height, 0.0f, 1.0f );
		//gsOutputs.texCoord = vec2( br.x, br.y );
		EmitVertex();
	
		EndPrimitive();
	}
}

#end_shader


#new_shader fragment
#version 330

in GS_OUT
{
	vec2 texCoord;
} gsInputs;


uniform sampler2D texture;
uniform vec4 gColor;

out vec4 frag_colour;

void main() 
{


	frag_colour = texture2D(texture, gsInputs.texCoord).r * gColor;

	//frag_colour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
}

#end_shader


