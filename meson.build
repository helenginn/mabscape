
project('mabscape', 'cpp', 'c', default_options : ['cpp_std=c++17'])

qt6 = import('qt6')

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])
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

run_command(hash_script)

gen_src = custom_target(
  'commit-output',
  input : ['libsrc/commit.h.pre'],
  output : ['commit.h'],
  command : [hash_script, '@INPUT@', '@OUTPUT@'],
  build_by_default: true,
  build_always_stale: true
)

moc_files = qt6.preprocess(
  moc_headers : [
    'libsrc/ColourOptions.h',
    'libsrc/Controller.h',
    'libsrc/Experiment.h',
    'libsrc/Explorer.h',
    'libsrc/InputDialogue.h',
    'libsrc/Metadata.h',
    'libsrc/PDBView.h',
    'libsrc/Refinement.h',
    'libsrc/SurfaceView.h',
    'libsrc/Structure.h',
  ],
  moc_extra_arguments: ['-DMAKES_MY_MOC_HEADER_COMPILE']
)
inc_libsrc = include_directories('libsrc')
executable(
  'mabscape',
  gen_src,
  moc_files,
  'libsrc/main.cpp',
  'libsrc/Antibody.cpp',
  'libsrc/Bound.cpp',
  'libsrc/ColourOptions.cpp',
  'libsrc/Cone.cpp',
  'libsrc/Controller.cpp',
  'libsrc/Data.cpp',
  'libsrc/Experiment.cpp',
  'libsrc/Explorer.cpp',
  'libsrc/InputDialogue.cpp',
  'libsrc/Genes.cpp',
  'libsrc/Metadata.cpp',
  'libsrc/PDBView.cpp',
  'libsrc/Square.cpp',
  'libsrc/Squiggle.cpp',
  'libsrc/Structure.cpp',
  'libsrc/SurfaceView.cpp',
  'libsrc/Refinement.cpp',
  'libsrc/Result.cpp',
  include_directories: inc_libsrc,
  dependencies : [
    boost_dep,
    helen3d_dep,
    helencore_dep,
    dep_vag,
    dep_c4x,
    png_dep,
    qt6_dep,
    dep_ica,
    dep_ccp4,
    dep_vgeom,
    m_dep,
    dep_gl
  ],
  install: true
)
