#version 330 core

in vec3 lightColor;

in vec3 fragmentNormal;

in vec3 lightPosition;
in vec3 fragmentPosition;

in vec3 viewPosition;

in vec3 materialAmbient;
in vec3 materialDiffuse;
in vec3 materialSpecular;
in float materialShininess;

out vec4 color;

/* based on http://learnopengl.com/#!Lighting/Materials */

void main() {
  vec3 ambient, diffuse, specular, lightDirection, normal,
    viewDirection, reflectionDirection, result;
  float diff, spec;

  // Ambient
  ambient = vec3(lightColor.x * materialAmbient.x,
    lightColor.y * materialAmbient.y,
    lightColor.z * materialAmbient.z);

  // Diffuse
  normal = normalize(fragmentNormal);
  lightDirection = normalize(lightPosition - fragmentPosition);
  diff = max(dot(normal, lightDirection), 0.0);
  diffuse = vec3(diff * lightColor.x * materialDiffuse.x,
    diff * lightColor.y * materialDiffuse.y,
    diff * lightColor.z * materialDiffuse.z);

  // Specular
  viewDirection = normalize(viewPosition - fragmentPosition);
  reflectionDirection = -1 * lightDirection + 2 *
    dot(lightDirection, normal) * normal;
  spec = pow(max(dot(viewDirection, reflectionDirection), 0.0),
    1.f / materialShininess);
  specular = vec3(spec * lightColor.x * materialSpecular.x,
    spec * lightColor.y * materialSpecular.y,
    spec * lightColor.z * materialSpecular.z);

  result = ambient + diffuse + specular;
  color = vec4(result, 1.f);
}
