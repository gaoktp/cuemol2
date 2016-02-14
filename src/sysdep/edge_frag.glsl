// -*-Mode: C++;-*-
//
//  Default fragment shader for OpenGL
//

uniform float frag_alpha;
uniform float frag_zdisp;

void main (void)
{
  vec4 color;

  color = gl_Color;
  float z = gl_FogFragCoord;

  float fog;
  fog = (gl_Fog.end - z) * gl_Fog.scale;
  fog = clamp(fog, 0.0, 1.0);
  color = vec4(mix( vec3(gl_Fog.color), vec3(color), fog), color.a * frag_alpha);

  gl_FragColor = color;

  gl_FragDepth = gl_FragCoord.z + frag_zdisp;
}

