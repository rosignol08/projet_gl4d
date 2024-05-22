#version 330
uniform vec4 lumpos;
uniform sampler2D myTexture;
uniform int hasTexture;
uniform int sky;
uniform vec4 diffuse_color;
uniform vec4 specular_color;
uniform vec4 ambient_color;
uniform vec4 emission_color;
uniform float shininess;


in vec2 vsoTexCoord;
in vec3 vsoNormal;
in vec4 vsoModPosition;

out vec4 fragColor;


void main(void) {
  if(sky != 0) {
    float i = texture(myTexture, vsoTexCoord).r;
    i = pow(i, 1.0);
    fragColor = vec4(vec3(i), 1.0) + vec4(0.0, 1.0, 1.0, 1.0);
    return;
  }
  //limière vsoModPosition.xyz - 
  vec3 lum  = normalize(vsoModPosition.xyz - lumpos.xyz);
  float diffuse = clamp(dot(normalize(vsoNormal), -lum), 0.0, 10.0);
  // Définir la couleur de la lumière
  vec4 light_color = vec4(0.976, 0, 0.988, 1.0); // rose
  //pour du toon shading
  diffuse = float(int(diffuse * 4.0 - 0.01)) / 3.0;
  vec3 lightDirection = vec3(lumpos - vsoModPosition);
  
  vec4 specularReflection = specular_color * pow(max(0.0, dot(normalize(reflect(-lightDirection, vsoNormal)), normalize(vec3(-vsoModPosition)))), shininess);
  
  // Définir la couleur ambiante rose avec une intensité réduite
  vec4 ambient_color = vec4(0.976, 0, 0.988, 1.0); // Exemple de couleur rose : 0.7, 0.4, 0.6, 1.0 ou mieux : 0.773, 0.063, 0.78, 1.0
  // Calcul de la réflexion diffuse
  vec4 diffuseReflection = ambient_color*0.2 + diffuse_color * diffuse ;
  fragColor = vec4(diffuseReflection.rgb, texture(myTexture, vsoTexCoord).a);
  //fragColor = diffuseReflection + specularReflection;
  if(hasTexture != 0)
    fragColor *= texture(myTexture, vsoTexCoord);
}


//#version 330
//uniform sampler2D tex;
//uniform int sky, fog, useColor; // Ajout de useColor
//uniform float texRepeat;
//uniform vec4 uColor; // Couleur uniforme
//
//in vec2 vsoTexCoord;
//out vec4 fragColor;
//
//void main(void) {
//  const float SQRT2 = 1.442695, fog_density2 = 0.01;
//  const vec4 fog_color = vec4(0.2, 0.5, 0.5, 1.0);
//
//  if(useColor != 0) {
//    // Utiliser la couleur uniforme si useColor est actif
//    fragColor = uColor;
//  } else if(sky != 0) {
//    // Logique pour le ciel
//    float x = 0.02 + 0.96 * abs(2.0 * vsoTexCoord.x - 1);
//    float y = clamp(2.0 * (1.0 - vsoTexCoord.y), 0, 1);
//    vec4 texc = vec4(0, 0.5, 0.5, 1.0) + pow(texture(tex, vec2(x, 0.96 * y)).rrrr, vec4(4.0));
//    if(fog != 0) {
//      float ffactor = 1.0 - pow(y, 30);
//      fragColor = mix(fog_color, texc, ffactor);
//    } else {
//      fragColor = texc;
//    }
//  } else {
//    // Logique pour les textures normales
//    vec4 texc = texture(tex, texRepeat * vsoTexCoord);
//    if(fog != 0) {
//      float z = gl_FragCoord.z / gl_FragCoord.w;
//      float ffactor = exp(-fog_density2 * z * z);
//      ffactor = clamp(ffactor, 0.0, 1.0);
//      fragColor = mix(fog_color, texc, ffactor);
//    } else {
//      fragColor = texc;
//    }
//  }
//}