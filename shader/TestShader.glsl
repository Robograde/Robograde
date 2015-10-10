#new_shader vertex
#version 330
in vec3 posL;
in vec2 texcoord;
in vec3 normalL;
in vec3 tangentL;

uniform mat4 world;
uniform mat4 wvp;

out vec2 tex;
out vec3 normalW;
out vec3 tangentW;
out vec3 posW;

void main(){
  gl_Position = wvp * vec4(posL,1);
  tex = texcoord;
  normalW = (world * vec4(normalL,0)).xyz;
  tangentW = (world * vec4(tangentL,0)).xyz;
  posW = (world * vec4(posL,1)).xyz;
}
#end_shader

#new_shader fragment
#version 330
in vec2 tex;
in vec3 normalW;
in vec3 tangentW;
in vec3 posW;
#ifdef USE_DIFFUSE_TEXTURE
uniform sampler2D albedoTex;
#endif

out vec4 FragmentColor;
void main(){

#ifdef USE_DIFFUSE_TEXTURE
  FragmentColor = texture(albedoTex,tex);
#endif
#ifdef RETURN_RED
  FragmentColor = vec4(1,0,0,1);
#endif
  FragmentColor =  vec4(1,1,1,1);
}
#end_shader