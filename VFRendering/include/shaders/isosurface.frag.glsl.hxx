#ifndef ISOSURFACE_FRAG_GLSL_HXX
#define ISOSURFACE_FRAG_GLSL_HXX

static const std::string ISOSURFACE_FRAG_GLSL = R"LITERAL(
#version 330

in vec3 vfPosition;
in vec3 vfDirection;
in vec3 vfNormal;
out vec4 fo_FragColor;

vec3 colormap(vec3 direction);
bool is_visible(vec3 position, vec3 direction);
float lighting(vec3 position, vec3 normal);

void main(void) {
  if (is_visible(vfPosition, vfDirection)) {
    vec3 color = colormap(normalize(vfDirection));
    fo_FragColor = vec4(color * lighting(vfPosition, normalize(vfNormal)), 1.0);
  } else {
    discard;
  }
}

)LITERAL";

#endif

