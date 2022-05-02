#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform bool fogEnabled;
uniform float lightColorCoeff;

void main()
{
    color = texture(skybox, textureCoordinates);
    if (fogEnabled) {
        color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    color = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), color, lightColorCoeff);
}
