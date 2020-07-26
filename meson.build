project('abmap', 'cpp', 'c')
qt5 = import('qt5')

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Sql'], required : false)
dep_gl = dependency('gl', required : false)

png_dep = dependency('libpng')
boost_dep = dependency('boost')
fftw_dep = dependency('fftw3f')
dep_vag = dependency('libvag')

vagdir = dep_vag.get_pkgconfig_variable('prefix') + '/lib64'

message('Vagdir: ' + vagdir)

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

# Run the version and hash generator
hash_script = find_program('get_hash.sh')

# This will produce the dependency file.
run_command('get_hash.sh')

gen_src = custom_target('commit-output',
                        input : ['libsrc/commit.h.pre'],
                        output : ['commit.h'],
                        command : [hash_script, '@INPUT@', '@OUTPUT@'],
                        build_by_default: true,
                        build_always_stale: true)

moc_files = qt5.preprocess(moc_headers : 
['libsrc/SlipGL.h',
'libsrc/SurfaceView.h',
],
			moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

executable('abmap', gen_src, moc_files,
'libsrc/main.cpp',
'libsrc/SlipGL.cpp',
'libsrc/SlipPanel.cpp',
'libsrc/SlipObject.cpp',
'libsrc/SurfaceView.cpp',
cpp_args: ['-std=c++11'], dependencies : [ boost_dep, dep_vag, png_dep, qt5_dep ], 
install_rpath: vagdir, install: true)

