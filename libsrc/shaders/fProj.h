#ifndef __patch_f__
#define __patch_f__

inline std::string Patch_fsh() 
{
	std::string str = 
	"varying vec4 vColor;\n"\
	"varying vec2 vTex;\n"\
	"varying vec4 vPos;\n"\
	"varying float vTime;\n"\
	"varying float vDot;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    if (vColor[2] < 0.99) {\n"\
	"//		discard;\n"\
	"	 }\n"\
	"	gl_FragColor = vColor;\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}


#endif
