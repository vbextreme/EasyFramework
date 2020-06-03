###########
# project #
###########
project('Services code make', 'c')
projectName = 'systemmake'

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories(['../../include', '/usr/include/freetype2' ])

###############
# source file #
###############

src = [ 'src/main.c', 'src/gencode.c', 'src/parseservices.c' ]

##################
# compiler flags #
##################

# warnings
add_global_arguments('-Wall', language: 'c')
add_global_arguments('-Wextra', language: 'c')
add_global_arguments('-Wuninitialized', language: 'c')
add_global_arguments('-Winit-self', language: 'c')
add_global_arguments('-Wstrict-aliasing', language: 'c')
add_global_arguments('-Wstrict-overflow', language: 'c')
add_global_arguments('-Wfloat-equal', language: 'c')
add_global_arguments('-Wvla', language: 'c')

#reentrant malloc
add_global_arguments('-pthread', language: 'c')
add_global_link_arguments('-pthread', language:'c')

#resolve RTLD_LAZY
add_global_link_arguments('-rdynamic', language:'c')

# standard
add_global_arguments('-std=gnu11', language: 'c')

# open mp
if get_option('openmp') > 0
  message('openmp enabled')
  add_global_arguments('-fopenmp', language:'c')
  add_global_link_arguments('-fopenmp', language:'c')
  add_global_arguments('-DOMP_ENABLE=1', language: 'c')
endif 
# gprof
if get_option('gprof') > 0
  add_global_arguments('-pg', language:'c')
  add_global_link_arguments('-pg', language:'c')
endif

# optimization
if( get_option('optimize') == 3 )
  message('agressive optimization enabled')
  add_global_arguments('-O3', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
  add_global_arguments('-flto', language: 'c')
  add_global_link_arguments('-flto', language: 'c')
elif( get_option('optimize') == 2 )
  message('local optimization enabled')
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-march=native', language: 'c')
  add_global_arguments('-mtune=native', language: 'c')
  add_global_arguments('-flto', language: 'c')
  add_global_link_arguments('-flto', language: 'c')
elif( get_option('optimize') == 1 )
  message('generic optimization enabled')
  add_global_arguments('-O2', language: 'c')
  add_global_arguments('-flto', language: 'c')
  add_global_link_arguments('-flto', language: 'c')
else
  message('no optimization enabled')
endif

#enable autovectorization
if get_option('autovectorization') > 0
  if( get_option('optimize') < 1 )
	error('required option optimize > 0')
  endif
  message('vectorization enabled')
  add_global_arguments('-ftree-vectorize', language:'c')
  add_global_arguments('-DVECTORIZE=1', language:'c')
endif

##########################
# libraries dependencies #
##########################
curpath = meson.source_root()
bkpath = run_command('meson.bash', 'dirname', curpath).stdout().strip().split('\n')
efpath = run_command('meson.bash', 'dirname', bkpath).stdout().strip().split('\n')

if get_option('efdbg') > 0
  efpath = join_paths(efpath, 'build-dbg')
  ef_dep = cc.find_library('ef', dirs: efpath, required: true)
  message('use ef debug')
  message(efpath)
else
  efpath = join_paths(efpath, 'build')
  ef_dep = cc.find_library('ef', dirs: efpath, required: true)
endif
libDeps = [ ef_dep ]

#libDeps += [ dependency('xcb', required: true) ] 
libDeps += [ cc.find_library('unistring', required : true) ]

#libDeps += [ dependency('freetype2', required: true) ]
#libDeps += [ dependency('fontconfig', required: true) ]
#libDeps += [ dependency('xcb-composite', required: true) ]
#libDeps += [ dependency('xcb-util', required: true) ]
#libDeps += [ dependency('xcb-image', required: true) ]
#libDeps += [ dependency('xcb-randr', required: true) ]
#libDeps += [ dependency('xkbcommon', required: true) ]
#libDeps += [ dependency('xkbcommon-x11', required: true) ]

add_global_arguments('-DXCB_ERROR_ENABLE', language: 'c')

#########################
# software dependencies #
#########################

#perl = find_program('perl', required: true)

#################
# Configuration #
#################

#################
# Custom Target #
#################

#########
# debug #
#########

opt_debug = get_option('ebug')
if opt_debug > 0
  debugstr = 'debug enabled @0@'
  debugstr = debugstr.format(opt_debug)
  message(debugstr)
  debugstr = '-DDEBUG_ENABLE=@0@'
  debugstr = debugstr.format(opt_debug)
  add_global_arguments(debugstr, language: 'c')
  add_global_arguments('-g', language: 'c')
  #add_global_arguments('-ftree-vectorizer-verbose=5', language:'c')
  #add_global_arguments('-fopt-info-loop-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-optimized', language:'c')
  #add_global_arguments('-fopt-info-vec-missed', language:'c')
endif

#enable color debug
if get_option('olor') > 0
  message('color debug enabled')
  add_global_arguments('-DDEBUG_COLOR=1', language: 'c')
endif

#enable iassert
if get_option('assert') > 0
  message('assertion enabled')
  add_global_arguments('-DASSERT_ENABLE=1', language: 'c')
endif

###########
# install #
###########

###########
# license #
###########

message('copyright vbextreme 2019')
message('released under GPLv3')

#########
# build #
#########

executable(projectName, src, include_directories: includeDir, dependencies: libDeps, install: true)


