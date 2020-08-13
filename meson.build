project('abmap', 'cpp', 'c')
qt5 = import('qt5')

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'Sql'], required : false)
dep_gl = dependency('gl', required : false)

png_dep = dependency('libpng')
boost_dep = dependency('boost')
fftw_dep = dependency('fftw3f')
dep_vag = dependency('libvag')
dep_c4x = dependency('libcluster4x')
dep_ica = dependency('libica')

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
'libsrc/Controller.h',
'libsrc/Experiment.h',
'libsrc/Explorer.h',
'libsrc/Mesh.h',
'libsrc/PDBView.h',
'libsrc/Refinement.h',
'libsrc/SurfaceView.h',
],
			moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

executable('abmap', gen_src, moc_files,
'libsrc/main.cpp',
'libsrc/Bound.cpp',
'libsrc/Controller.cpp',
'libsrc/Data.cpp',
'libsrc/Experiment.cpp',
'libsrc/Explorer.cpp',
'libsrc/Icosahedron.cpp',
'libsrc/Mesh.cpp',
'libsrc/PDBView.cpp',
'libsrc/SlipGL.cpp',
'libsrc/SlipPanel.cpp',
'libsrc/SlipObject.cpp',
'libsrc/SlipObjFile.cpp',
'libsrc/Squiggle.cpp',
'libsrc/Structure.cpp',
'libsrc/SurfaceView.cpp',
'libsrc/Refinement.cpp',
'libsrc/Result.cpp',
cpp_args: ['-std=c++11'], dependencies : [ boost_dep, dep_vag, dep_c4x, png_dep, qt5_dep, dep_ica ], 
install_rpath: vagdir, install: true)

kin_moc = qt5.preprocess(moc_headers : 
[
'kinsrc/Curve.h',
'kinsrc/CurveView.h',
'kinsrc/KineticView.h',
'kinsrc/KModel.h',
'kinsrc/Inspector.h',
],
moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])


executable('abmap.kinetics', kin_moc,
'kinsrc/main.cpp',
'kinsrc/Curve.cpp',
'kinsrc/CurveView.cpp',
'kinsrc/Inspector.cpp',
'kinsrc/KineticView.cpp',
'kinsrc/KModel.cpp',
'kinsrc/KOffset.cpp',
'kinsrc/KFirst.cpp',
'kinsrc/KSecond.cpp',
'kinsrc/KCorrectExp.cpp',
'kinsrc/KExpDecay.cpp',
'kinsrc/KLigOnOff.cpp',
cpp_args: ['-std=c++11'], dependencies : [ boost_dep, dep_vag, dep_c4x, png_dep, qt5_dep, dep_ica ], 
install_rpath: vagdir, install: true)

