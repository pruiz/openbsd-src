#!/bin/sh

pthreads_root=EXEC_PREFIX
build_root=BUILD_PREFIX
src_root=SRC_PREFIX

include_dir='-I$pthreads_root/include'
lib_dir='-L$pthreads_root/lib'
libs='-lpthread -lm -lgcc -lpthread'

# Might be a good idea to also provide a way to override pthreads_root
# so that we can use this script in the build tree, before installation.
if arg="$1" ; then
    case $arg in
    -notinstalled)
        include_dir='-I$build_root/include -I$src_root/include'
        lib_dir='-L$build_root/obj'
        shift
        ;;
    esac
fi

for arg in "$@" ; do
    case $arg in
	-nostdinc)	include_dir= ;;
	-nostdlib | -c)	libs= ;;
    esac
done

# Include the -L option in any case, just in case the user provided the
# names of some libraries we've built threaded versions of.
eval exec COMPILER '"$@"' $include_dir $lib_dir $libs
