#ifndef __Blot__Image_vsh__
#define __Blot__Image_vsh__

inline std::string Pencil_vsh()
{
	std::string str = 
	"attribute vec3 normal;\n"\
	"attribute vec3 position;\n"\
	"attribute vec4 color;\n"\
	"attribute vec4 extra;\n"\
	"attribute vec2 tex;\n"\
	"\n"\
	"uniform mat4 model;\n"\
	"uniform mat4 projection;\n"\
	"uniform float time;\n"\
	"\n"\
	"varying vec4 vColor;\n"\
	"varying vec4 vPos;\n"\
	"varying vec2 vTex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"	 vec4 norm4 = vec4(normal[0], normal[1], normal[2], 1.0);\n"\
	"	 pos = model * pos;\n"\
	"	 vec4 lightpos = vec4(pos[0], pos[1], pos[2], 1);\n"\
	"    float origdot = abs(dot(normalize(norm4), normalize(lightpos)));"\
	"    float adot = (asin(origdot) + time * 12.56);\n"\
	"	 if (adot != adot) adot = 0.;\n"\
	"	 while (adot > 6.28) adot -= 6.28;\n"\
	"    float dot = abs(sin(adot));\n"\
	"    float gdot = abs(sin(adot + 3.14));\n"\
	"    vPos = projection * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 float red = 0.4 *((1.0 - color[0]) * dot) + color[0];\n"\
	"	 float green = 0.4 *((1. - color[1]) * gdot) + color[1];\n"\
	"//	 red = max(red, 0.); red = min(red, 1.);\n"\
	"//	 green = max(green, 0.); green = min(green, 1.);\n"\
	"	 vColor = vec4(red, origdot, color[2], color[3]);\n"\
	"}";
	return str;
}

#endif
