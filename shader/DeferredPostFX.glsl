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
uniform sampler2D   InputTex;
uniform sampler2D   NormalTex;
uniform sampler2D   DepthTex;
uniform sampler2D   RoughMetalTex;
uniform sampler2D   EmissiveTex;
uniform sampler2D	GlowTex;
uniform bool        Split;
out vec4 			      FinalColor;

void main()
{
	if(Split)
	{
		if(texcoord.x > 0.5  && texcoord.y > 0.5)
		{
			FinalColor = texture2D(InputTex,texcoord * 2.0 - 1.0);
		}
		else if(texcoord.x < 0.5  && texcoord.y > 0.5)
		{
			float depth = texture2D(DepthTex, vec2(texcoord.x * 2.0, texcoord.y * 2.0 - 1.0)).r * 2.0 - 1.0;
			FinalColor = vec4(depth,depth,depth,1);
		}
		else if(texcoord.x > 0.5  && texcoord.y < 0.5)
		{
			FinalColor = vec4(texture2D(RoughMetalTex, vec2(texcoord.x * 2.0 - 1.0, texcoord.y * 2.0)).xyz, 1);
		}
		else
		{
			FinalColor = texture2D(NormalTex,texcoord * 2.0);
		}
	}
	else
	{
		FinalColor = texture2D(InputTex,texcoord) + texture2D(GlowTex,texcoord);// + texture2D(EmissiveTex,texcoord);
	}

	//FinalColor = pow(FinalColor,vec4( 1.0 / 2.2));
	//FinalColor = vec4(texcoord,1,1);
}
#end_shader
