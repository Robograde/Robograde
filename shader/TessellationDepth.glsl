#new_shader vertex
#version 430
layout (location = 0) in vec3 posL;
layout (location = 1) in vec3 normalL;
layout (location = 2) in vec2 texCoord;
uniform mat4 world;

out vec3 posW_CS_in;
out vec3 normalW_CS_in;
out vec2 texCoord_CS_in;



void main(){
	posW_CS_in = (world * vec4(posL,1)).xyz;
	normalW_CS_in = (world * vec4(normalL,0)).xyz;
	texCoord_CS_in = texCoord;
}
#end_shader


#new_shader fragment
#version 430
layout(early_fragment_tests) in;
out vec4 FragmentColor;
void main()
{
	discard;
}
#end_shader

#new_shader control
#version 430
layout (vertices = 3) out;

uniform vec4 tessFactor;

in vec3 posW_CS_in[];
in vec3 normalW_CS_in[];
in vec2 texCoord_CS_in[];

out vec3 posW_EV_in[];
out vec3 normalW_EV_in[];
out vec2 texCoord_EV_in[];

void main()
{
	// Set output
	posW_EV_in[gl_InvocationID] = posW_CS_in[gl_InvocationID];
	normalW_EV_in[gl_InvocationID] = normalW_CS_in[gl_InvocationID];
	texCoord_EV_in[gl_InvocationID] = texCoord_CS_in[gl_InvocationID];

		// Calculate the tessellation factors here
		//float distanceFromEye = length(posW_CS_in[gl_InvocationID] - eyePosition);
		//int tessFactor = int(max(16-(distanceFromEye), 1));
		//distanceFromEye = abs(posW_CS_in[gl_InvocationID + 3] - eyePosition);
		//int nextTessFactor = int(max(16-(distanceFromEye), 1));

		gl_TessLevelOuter[0] = tessFactor.x;
		gl_TessLevelOuter[1] = tessFactor.y;
		gl_TessLevelOuter[2] = tessFactor.z;
		gl_TessLevelInner[0] = tessFactor.w;

}

#end_shader

#new_shader evaluation
#version 430
layout(triangles, equal_spacing, cw) in;
layout(std140, binding = 0) uniform SceneInputs
{
	mat4 g_ViewProj;
	vec4 g_CamPos;
};
layout (location = 0, binding = 0) uniform sampler2D heightMap;
uniform float heightScale;
uniform float horizontalScale;
uniform int heightmapRepeat;

in vec3 posW_EV_in[];
in vec3 normalW_EV_in[];
in vec2 texCoord_EV_in[];

out vec3 posW;
out vec3 normalW;
out vec2 texCoord;

vec3 interpolate3D(vec3 v0, vec3 v1, vec3 v2)
{
	return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y)
	* v1 + vec3(gl_TessCoord.z) * v2;
}

vec2 interpolate2D(vec2 v0, vec2 v1, vec2 v2)
{
	return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y)
	* v1 + vec2(gl_TessCoord.z) * v2;
}

void main()
{
	// Interpolate the attributes of the output vertex using the barycentric coordinates
	posW = interpolate3D(posW_EV_in[0], posW_EV_in[1], posW_EV_in[2]);
	normalW = interpolate3D(normalW_EV_in[0], normalW_EV_in[1], normalW_EV_in[2]);
	normalW = normalize(normalW);

	texCoord = interpolate2D(texCoord_EV_in[0], texCoord_EV_in[1], texCoord_EV_in[2]);
	//posW += normalW;

	float t,b,l,r,tl,tr,bl,br;

	float heightOffset = texture( heightMap, texCoord.st * heightmapRepeat).r;
	bl = texture( heightMap, vec2(-0.01,-0.01) + texCoord.st ).r;
	l  = texture( heightMap, vec2(-0.01,0) + texCoord.st ).r;
	tl = texture( heightMap, vec2(-0.01,0.01) + texCoord.st ).r;
	t  = texture( heightMap, vec2(0,0.01) + texCoord.st ).r;
	tr = texture( heightMap, vec2(0.01,0.01) + texCoord.st ).r;
	r  = texture( heightMap, vec2(0.01,0) + texCoord.st ).r;
	br = texture( heightMap, vec2(0.01,-0.01) + texCoord.st ).r;
	b  = texture( heightMap, vec2(0,-0.01) + texCoord.st ).r;


// ------------------------------------------------
// Broken code for calculating normals.
// Leaving it in in case I want to fix it later.

//	vec3 botToTop = normalize(vec3(0,t-b,1));
//	vec3 leftToRight = normalize(vec3(1,r-l,0));
//	vec3 newNormal = -cross(botToTop, leftToRight);
//	normalW = normalize(newNormal);
// ------------------------------------------------

// 	Removed during debug - should be put back afterwards
	heightOffset = heightOffset + bl + l + tl + tr + r + br + b + t;
	heightOffset /= 9;

	gl_Position = g_ViewProj * vec4(posW, 1.0f);
}

#end_shader
