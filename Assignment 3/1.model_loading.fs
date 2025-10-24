#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    // Use the texture from the model
    FragColor = texture(texture_diffuse1, TexCoords);
}