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
	"    float green = abs(dot(normalize(norm4), normalize(lightpos)));"\
	"    float adot = (asin(green) + 0. * 12.56);\n"\
	"	 if (adot != adot) adot = 0.;\n"\
	"	 while (adot > 6.28) adot -= 6.28;\n"\
	"    float dot = abs(sin(adot));\n"\
	"    float gdot = abs(sin(adot + 3.14));\n"\
	"    vPos = projection * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 float red = 0.4 *((1.0 - color[0]) * dot) + color[0];\n"\
	"    green *= color[1] * 2.;\n"\
	"    float blue = color[2];\n"\
	"	 if (color[1] < 0.01)\n"\
	"	 {\n"\
	"//		 red = origdot;\n"\
	"	 }\n"\
	"	 vColor = vec4(red, green, blue, color[3]);\n"\
	"}";
	return str;
}

#endif
