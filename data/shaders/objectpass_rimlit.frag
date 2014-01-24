#version 130
uniform sampler2D Albedo;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
uniform vec2 screen;
uniform vec3 ambient;

noperspective in vec3 normal;
in vec2 uv;

out vec4 FragColor;

void main() {
	float rim = 1.0 - dot(normal, vec3(0., 0., -1));
	rim = smoothstep(0.5, 1.5, rim) * 0.35;

	vec4 color = texture(Albedo, uv);
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 LightFactor = ao * ambient + DiffuseComponent + SpecularComponent * (1. - color.a);
    FragColor = vec4(color.xyz * LightFactor + rim, 1.);
}

