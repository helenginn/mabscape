#ifndef __Blot__Patch_vsh__
#define __Blot__Patch_vsh__

inline std::string Patch_vsh()
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
	"uniform vec3 centre;\n"\
	"uniform vec3 direction;\n"\
	"uniform mat4 patchmat;\n"\
	"\n"\
	"varying vec4 vColor;\n"\
	"varying vec4 vPos;\n"\
	"varying vec2 vTex;\n"\
	"varying float vDot;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	 vColor = color;\n"\
	"    vec4 pos = vec4(position, 1.0);\n"\
	"	 vec4 ray = pos - vec4(centre, 1.0);\n"\
	"    float dotdir = dot(direction, direction);\n"\
	"    float dotray = dot(vec3(ray), direction);\n"\
	"	 float dist = dotdir / dotray;\n"\
	"	 vec4 oldray = ray;\n"\
	"	 if (dist > 0.)\n"\
	"	 {\n"\
	"	    ray *= dist;\n"\
	"	 }\n"\
	"	 mat4 rot = model;\n"\
	"	 rot[0][3] = 0.;\n"\
	"	 rot[1][3] = 0.;\n"\
	"	 rot[2][3] = 0.;\n"\
	"	 rot[3][3] = 1.;\n"\
	"	 oldray = patchmat * oldray;\n"\
	"	 ray = projection * patchmat * ray;\n"\
	"	 ray[2] -= oldray[2] * 0.01;\n"\
	"	 gl_Position = ray;\n"\
	"	 vPos = gl_Position;\n"\
	"}";
	return str;
}

#endif
