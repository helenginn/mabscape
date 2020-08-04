#ifndef __abmap__White__
#define __abmap__White__

inline std::string White_vsh()
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
	"    vPos = projection * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 vColor = color;\n"\
	"}";
	return str;
}

#endif
