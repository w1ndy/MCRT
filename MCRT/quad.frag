#version 130

in vec2 texcoord;
uniform sampler2D tex;
out vec4 color;

void main(void) {
	color = texture(tex, texcoord);
}
