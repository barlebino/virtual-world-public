#version 330 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform vec3 light_color;
uniform vec3 light_position;
// Coordinates of the camera
uniform vec3 view_position;

uniform vec3 material_ambient;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

uniform mat4 perspective;
// Rotates all objects in world to accommodate eye direction
uniform mat4 eye_direction;
// Move camera position to (0, 0, 0), and move
// everything relative to the camera position
uniform mat4 centering;
// Move object to its location in the world
uniform mat4 placement;

out vec3 lightColor;
// Normal of the fragment after being transformed to world space
out vec3 fragmentNormal;
out vec3 lightPosition;
// Position of the fragment after being transformed to world space
out vec3 fragmentPosition;
// Location of the camera
out vec3 viewPosition;

out vec3 materialAmbient;
out vec3 materialDiffuse;
out vec3 materialSpecular;
out float materialShininess;

void main() {
  gl_Position = perspective * eye_direction * centering *
    placement * vertPos;

  lightColor = light_color;

  // Direction of the normal in world space
  fragmentNormal = mat3(placement) * vertNor;

  lightPosition = light_position;
  fragmentPosition = (placement * vertPos).xyz;

  viewPosition = view_position;

  materialAmbient = material_ambient;
  materialDiffuse = material_diffuse;
  materialSpecular = material_specular;
  materialShininess = material_shininess;
}
