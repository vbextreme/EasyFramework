###########
# project #
###########
project('Test EasyFramework', 'c')
projectName = 'testef'

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories(['../include'])

###############
# source file #
###############

src = [ 'src/main.c', 'src/data.c' ]
#@src
src += [ 'src/err.c' ]
src += [ 'src/file.c' ]
src += [ 'src/ftp.c' ]
src += [ 'src/fuzzys.c' ]
src += [ 'src/hash.c' ]
src += [ 'src/imap.c' ]
src += [ 'src/json.c' ]
src += [ 'src/mem.c' ]
src += [ 'src/mth.c' ]
src += [ 'src/oauth.c' ]
src += [ 'src/os.c' ]
src += [ 'src/promise.c' ]
src += [ 'src/rbhash.c' ]
src += [ 'src/socket.c' ]
src += [ 'src/str.c' ]
src += [ 'src/thr.c' ]
src += [ 'src/trie.c' ]
src += [ 'src/vector.c' ]
#@end

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

if get_option('efdbg') > 0
  ef_dep = cc.find_library('ef', dirs: ['/home/vbextreme/Project/c/ef/build-dbg'], required: true)
  message('use ef debug')
else
  ef_dep = cc.find_library('ef', dirs: ['/home/vbextreme/Project/c/ef/build'], required: true)
endif

#math_dep = cc.find_library('m', required : true)
#dynamic_dep = cc.find_library('dl', required : true)
#rlt_dep = cc.find_library('rt', required: true)
#rl_dep = dependency('readline', required: true)
#sql_dep = dependency('sqlite3', required: true)
#curl_dep = dependency('libcurl', required: true)
#pcre2_dep = cc.find_library('pcre2-8', required : true)
#utf8_dep = cc.find_library('unistring', required : true)
#tar_dep = cc.find_library('tar', required: true)
#zlib_dep = dependency('zlib', required: true)
#git2_dep = dependency('libgit2', required: true)
#libDeps = [math_dep, dynamic_dep, curl_dep, pcre2_dep, utf8_dep, tar_dep, zlib_dep, git2_dep, rl_dep, sql_dep ]
#libDeps = [ math_dep, dynamic_dep, rlt_dep ]
libDeps = [ ef_dep ]

#tcc_dep = cc.find_library('tcc', required: false)
#if tcc_dep.found()
#  add_global_arguments('-DSCRIPTC_TCC', language:'c')
#  message('enable scriptc tcc')
#  libDeps += [ tcc_dep ]
#endif


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


