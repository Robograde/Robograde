#new_shader vertex
#version 430
#define MAX_DECALS 1024
layout (location = 0) in vec3 PosL;

layout(std140, binding = 0) uniform SceneInputs
{
    mat4 g_ViewProj;
    vec4 g_CamPos;
};

struct Decal
{
  mat4  World;
  mat4  InvWorld;
  vec4  Tint;
  uint  Texture;
  uint pad;
  uint pad2;
  uint pad3;
};

struct TextureFrame{
  float S;
  float T;
  float NormWidth;
  float NormHeight;
};

layout(std430, binding = 12) buffer DecalBuffer
{
  Decal gDecals[MAX_DECALS];
  TextureFrame gDecalFrames[MAX_DECALS];
};

out vec4 PosH;
out vec3 PosW;
flat out uint InstanceID;

void main()
{
  PosW = (gDecals[gl_InstanceID].World * vec4(PosL.xyz,1)).xyz;
  PosH = g_ViewProj * vec4(PosW,1);
  gl_Position = PosH;
  InstanceID = gl_InstanceID;
}
#end_shader

#new_shader fragment
#version 430
#define MAX_DECALS 1024
in vec4 PosH;
in vec3 PosW;
flat in uint InstanceID;
uniform sampler2D gDecalAlbedo;
uniform sampler2D gDepth;
uniform mat4 gInvViewProj;
uniform vec2 gScreenSize;

struct TextureFrame{
  float S;
  float T;
  float NormWidth;
  float NormHeight;
};

struct Decal
{
  mat4  World;
  mat4  InvWorld;
  vec4  Tint;
  uint  Texture;
  uint pad;
  uint pad2;
  uint pad3;
};

layout(std430, binding = 12) buffer DecalBuffer
{
  Decal gDecals[MAX_DECALS];
  TextureFrame gDecalFrames[MAX_DECALS];
};

layout(location = 0) out vec4 FragmentColor;
layout(location = 1) out vec4 FragmentGlow;

void main()
{
  vec2 screenuv = gl_FragCoord.xy / gScreenSize.xy;
  //vec2 screenuv = PosH.xy / PosH.w;

  float depth = texture(gDepth, screenuv).x * 2.0f - 1.0f;
  vec4 sPos = vec4(screenuv * 2 - 1, depth, 1);
  sPos = gInvViewProj * sPos;
  vec4 GbufferPosW = vec4(sPos.xyz / sPos.w, 1.0f);

  vec4 localPos = gDecals[InstanceID].InvWorld * GbufferPosW;
  localPos = vec4(localPos.xyz / localPos.w,1);

  bvec2 inBox = lessThan(abs(localPos.xy), vec2(1.0f));

  if(all(inBox))
  {
    vec2 uv = localPos.xy * 0.5f + 0.5f;
    TextureFrame frame = gDecalFrames[gDecals[InstanceID].Texture];
    vec2 texcoord;
    vec2 tex = clamp(fract(uv.xy),0.0,1.0);
    texcoord.x = float(frame.S + frame.NormWidth * uv.x);
    texcoord.y = float(frame.T + frame.NormHeight * uv.y);

    vec4 diffuse = texture(gDecalAlbedo, texcoord) * gDecals[InstanceID].Tint;
    FragmentColor = diffuse;
    //FragmentGlow = vec4(gDecals[InstanceID].Tint.xyz,diffuse.a);
    FragmentGlow = vec4(FragmentColor * 0.5);
  }
  else
  {
    discard;
  }
}
#end_shader
