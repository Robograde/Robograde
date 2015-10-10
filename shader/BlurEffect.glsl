#new_shader vertex
#version 330
void main()
{}
#end_shader
#new_shader geometry
#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
out vec2 texcoord;
void main()
{
	gl_Position = vec4( 1.0, 1.0, 0.0, 1.0 );
	texcoord = vec2( 1.0, 1.0 );
	EmitVertex();
	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0 );
	texcoord = vec2( 0.0, 1.0 );
	EmitVertex();
	gl_Position = vec4( 1.0,-1.0, 0.0, 1.0 );
	texcoord = vec2( 1.0, 0.0 );
	EmitVertex();
	gl_Position = vec4(-1.0,-1.0, 0.0, 1.0 );
	texcoord = vec2( 0.0, 0.0 );
	EmitVertex();
	EndPrimitive();
}
#end_shader
#new_shader fragment
#version 330
in vec2 texcoord;

uniform sampler2D gInputTex;
uniform float gWeights[3];
uniform bool gVerticaly;
layout(location = 0) out vec4 FinalColor;
void main(){
	ivec2 texSize = textureSize(gInputTex,0);
	ivec2 tex = ivec2(texcoord * texSize);
	FinalColor = vec4(0);
	ivec2 offset;
	if(gVerticaly){
		offset = ivec2(0,1);
	}else{
		offset = ivec2(1,0);
	}
	for(int i = -2; i <= 2; i++ ){
		FinalColor += texelFetch(gInputTex, tex + offset * i, 0) * gWeights[abs(i)];
	}
}
#end_shader
