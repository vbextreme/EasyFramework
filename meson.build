###########
# project #
###########
project('EasyFramework', 'c', version: '1.2.3', license: 'gplv3')
projectName = 'ef'

cc = meson.get_compiler('c')

###########
# include #
###########

includeDir = include_directories(['./include', '/usr/include/cairo' ])

###############
# source file #
###############

src =  [ 'ef/err/err.c' ]

src += [ 'ef/memory/memory.c']
src += [ 'ef/memory/heap.c' ]
src += [ 'ef/memory/shared.c' ]
src += [ 'ef/memory/protect.c' ]

src += [ 'ef/datastructure/vector.c' ]
src += [ 'ef/datastructure/rbuffer.c' ]
src += [ 'ef/datastructure/list.c' ]
src += [ 'ef/datastructure/hashalg.c', 'ef/datastructure/rbhash.c' ]
src += [ 'ef/datastructure/phq.c' ]
src += [ 'ef/datastructure/fzs.c' ]
src += [ 'ef/datastructure/trie.c' ]

src += [ 'ef/opt/optex.c' ]

src += [ 'ef/math/math.c' ]
src += [ 'ef/math/gnn.c' ]

src += [ 'ef/string/string.c' ]
src += [ 'ef/string/regex.c' ]
src += [ 'ef/string/utf8.c' ]

src += [ 'ef/times/delay.c' ]

src += [ 'ef/file/file.c' ]
src += [ 'ef/file/dir.c' ]
src += [ 'ef/file/path.c' ]
src += [ 'ef/file/fd.c' ]
src += [ 'ef/file/stream.c' ]
src += [ 'ef/file/gz.c' ]
src += [ 'ef/file/tar.c' ]
src += [ 'ef/file/deadpoll.c' ]
src += [ 'ef/file/config.c' ]

src += [ 'ef/os/proc.c' ]
src += [ 'ef/os/sys.c' ]
src += [ 'ef/os/spawn.c' ]
src += [ 'ef/os/inutility.c' ]
src += [ 'ef/os/vectorization.c' ]
src += [ 'ef/os/signal.c' ]

src += [ 'ef/concurrency/threads.c' ]
src += [ 'ef/concurrency/promise.c' ]

src += [ 'ef/parser/json.c' ]

src += [ 'ef/www/socket.c' ]
src += [ 'ef/www/www.c' ]
src += [ 'ef/www/http.c' ]
src += [ 'ef/www/ftp.c' ]
src += [ 'ef/www/imap.c' ]
src += [ 'ef/www/tig.c' ]
src += [ 'ef/www/oauth.c' ]

src += [ 'ef/term/terminfo.c' ]
src += [ 'ef/term/termlink.c' ]
src += [ 'ef/term/pty.c' ]
src += [ 'ef/term/termmode.c' ]
src += [ 'ef/term/terminput.c' ]
src += [ 'ef/term/termreadline.c' ]
src += [ 'ef/term/tui.c' ]
src += [ 'ef/term/tuiRoot.c' ]
src += [ 'ef/term/tuiWindow.c' ]
src += [ 'ef/term/tuiLabel.c' ]
src += [ 'ef/term/tuiButton.c' ]
src += [ 'ef/term/tuiText.c' ]
src += [ 'ef/term/tuiList.c' ]

src += [ 'ef/graphics/image.c' ]
src += [ 'ef/graphics/imageFiles.c' ]
src += [ 'ef/graphics/imagePng.c' ]
src += [ 'ef/graphics/imageJpeg.c' ]
src += [ 'ef/graphics/imageSvg.c' ]
src += [ 'ef/graphics/imageGif.c' ]
src += [ 'ef/graphics/qdbmp.c' ]
src += [ 'ef/graphics/imageBmp.c' ]
src += [ 'ef/graphics/media.c' ]
src += [ 'ef/graphics/ft.c' ]
src += [ 'ef/graphics/xorg.c' ]
src += [ 'ef/graphics/gui.c' ]
src += [ 'ef/graphics/guiResources.c' ]
src += [ 'ef/graphics/guiLabel.c' ]
src += [ 'ef/graphics/guiButton.c' ]
src += [ 'ef/graphics/guiDiv.c' ]
src += [ 'ef/graphics/guiText.c' ]

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
add_global_arguments('-Wduplicated-branches', language: 'c')
add_global_arguments('-Wduplicated-cond', language: 'c')
add_global_arguments('-Wlogical-op', language: 'c')
#add_global_arguments('-fanalyzer', language: 'c')
#add_global_arguments('-Wcast-qual', language: 'c')


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
  add_global_arguments('-O0', language: 'c')
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

libDeps  = [ cc.find_library('m', required : true) ]
libDeps += [ cc.find_library('dl', required : true) ]
libDeps += [ cc.find_library('rt', required: true) ]
libDeps += [ cc.find_library('pcre2-8', required : true) ]
libDeps += [ cc.find_library('unistring', required : true) ]
libDeps += [ cc.find_library('tar', required: true) ]
libDeps += [ cc.find_library('png', required : true) ]
libDeps += [ cc.find_library('jpeg', required: true) ]
libDeps += [ cc.find_library('gif', required: true) ]
libDeps += [ dependency('librsvg-2.0', required: true) ]
libDeps += [ dependency('zlib', required: true) ]
libDeps += [ dependency('gnutls', required: true) ]
libDeps += [ dependency('libcurl', required: true) ]
libDeps += [ dependency('libgit2', required: true) ]
libDeps += [ dependency('libavcodec', required: true) ]
libDeps += [ dependency('libavdevice', required: true) ]
libDeps += [ dependency('libavfilter', required: true) ]
libDeps += [ dependency('libavformat', required: true) ]
libDeps += [ dependency('libavutil', required: true) ]
libDeps += [ dependency('libswresample', required: true) ]
libDeps += [ dependency('libswscale', required: true) ]
libDeps += [ dependency('freetype2', required: true) ]
libDeps += [ dependency('fontconfig', required: true) ]
libDeps += [ dependency('xcb', required: true) ] 
libDeps += [ dependency('xcb-xrm', required: true) ] 
libDeps += [ dependency('xcb-composite', required: true) ]
libDeps += [ dependency('xcb-util', required: true) ]
libDeps += [ dependency('xcb-image', required: true) ]
libDeps += [ dependency('xcb-randr', required: true) ]
libDeps += [ dependency('xcb-shape', required: true) ]
libDeps += [ dependency('xkbcommon', required: true) ]
libDeps += [ dependency('xkbcommon-x11', required: true) ]

xcb_error_dep = dependency('xcb-errors', required: false)
if xcb_error_dep.found()
  message('enable xcb_error')
  add_global_arguments('-DXCB_ERROR_ENABLE', language: 'c')
  libDeps += [ xcb_error_dep ]
  pkg_xcb_error='-DXCB_ERROR_ENABLE'
else
  message('disable xcb_error')
  pkg_xcb_error=''
endif

#rl_dep = dependency('readline', required: true)
#sql_dep = dependency('sqlite3', required: true)

#tcc_dep = cc.find_library('tcc', required: false)
#if tcc_dep.found()
#  add_global_arguments('-DSCRIPTC_TCC', language:'c')
#  message('enable scriptc tcc')
#  libDeps += [ tcc_dep ]
#endif


#########################
# software dependencies #
#########################

tic = find_program('tic', required: true)

#################
# Configuration #
#################

#################
# Custom Target #
#################

tic_src = custom_target('tic',
  output: 'ef-term-extend',
  input: 'ti/ef-term-extend.ti',
  command: [tic, '-x', '-o', '@OUTPUT@', '@INPUT@']
)
src += [ tic_src ]

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
  add_global_arguments('-fstack-protector-strong', language: 'c')
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

#######
# pkg #
#######

pkg = import('pkgconfig')
pkg.generate(
  name: 'EasyFramework',
  description : 'easy framework library.',
  libraries: [ '-lef', '-lunistring', '-pthread' ],
  extra_cflags: [ '-I/usr/include/freetype2', pkg_xcb_error ],
  version: meson.project_version()
)

###########
# install #
###########

install_subdir('include/ef', install_dir: 'include')
install_subdir('ef-term-extend/e', install_dir: get_option('terminfo')) 

if get_option('man') > 0
  manc = run_command('allman')
  manf = manc.stdout().strip().split('\n')
  install_man(manf)  
endif

###########
# license #
###########

message('copyright vbextreme 2000')
message('released under GPLv3')

#########
# build #
#########

shared_library(projectName, src, include_directories: includeDir, dependencies: libDeps, install: true)


