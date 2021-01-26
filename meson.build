project('mabscape', 'cpp', 'c')
qt5 = import('qt5')

qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets'], required : false)
dep_gl = dependency('gl', required : false)

png_dep = dependency('libpng')
boost_dep = dependency('boost')
fftw_dep = dependency('fftw3f')
dep_vag = dependency('libvag')
dep_vgeom = dependency('libvgeom', required: false)
dep_ccp4 = dependency('libccp4')
dep_c4x = dependency('libcluster4x')
dep_ica = dependency('libica')
helen3d_dep = dependency('helen3d')
helencore_dep = dependency('helencore')

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
[#'libsrc/SlipGL.h',
'libsrc/Controller.h',
'libsrc/Experiment.h',
'libsrc/Explorer.h',
'libsrc/InputDialogue.h',
'libsrc/Metadata.h',
'libsrc/PDBView.h',
'libsrc/PatchView.h',
'libsrc/Refinement.h',
'libsrc/SurfaceView.h',
'libsrc/Structure.h',
],
			moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])

executable('mabscape', gen_src, moc_files,
'libsrc/main.cpp',
'libsrc/Arrow.cpp',
'libsrc/Antibody.cpp',
'libsrc/Bound.cpp',
'libsrc/Controller.cpp',
'libsrc/Data.cpp',
'libsrc/Experiment.cpp',
'libsrc/Explorer.cpp',
'libsrc/InputDialogue.cpp',
'libsrc/Genes.cpp',
'libsrc/Metadata.cpp',
'libsrc/Patch.cpp',
'libsrc/PatchView.cpp',
'libsrc/PDBView.cpp',
'libsrc/Squiggle.cpp',
'libsrc/Structure.cpp',
'libsrc/SurfaceView.cpp',
'libsrc/Refinement.cpp',
'libsrc/Result.cpp',
cpp_args: ['-std=c++11'], dependencies : [ boost_dep, helen3d_dep,
helencore_dep, dep_vag, dep_c4x, png_dep, qt5_dep, dep_ica, 
dep_ccp4, dep_vgeom ], 
install: true)

kin_moc = qt5.preprocess(moc_headers : 
[
'kinsrc/Curve.h',
'kinsrc/CurveView.h',
'kinsrc/KineticView.h',
'kinsrc/Kintroller.h',
'kinsrc/KModel.h',
'kinsrc/Inspector.h',
],
moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE'])


executable('mabscape.kinetics', kin_moc,
'kinsrc/main.cpp',
'kinsrc/Curve.cpp',
'kinsrc/CurveView.cpp',
'kinsrc/Inspector.cpp',
'kinsrc/KineticView.cpp',
'kinsrc/Kintroller.cpp',
'kinsrc/KModel.cpp',
'kinsrc/KOffset.cpp',
'kinsrc/KFirst.cpp',
'kinsrc/KSecond.cpp',
'kinsrc/KCorrectExp.cpp',
'kinsrc/KExpDecay.cpp',
'kinsrc/KLigOnOff.cpp',
cpp_args: ['-std=c++11', '-Wall'], dependencies : [ helencore_dep, boost_dep, dep_vag, dep_c4x, png_dep, qt5_dep, dep_ica ], 
install: true)

