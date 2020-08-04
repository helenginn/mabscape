#ifndef __Blot__Image_fsh__
#define __Blot__Image_fsh__

inline std::string Pencil_fsh() 
{
	std::string str = 
	"varying vec4 vColor;\n"\
	"varying vec2 vTex;\n"\
	"varying vec4 vPos;\n"\
	"varying float vTime;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"	gl_FragColor = vColor;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}


#endif
