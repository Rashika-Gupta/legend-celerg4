Error using NERSC
```
In file included from /global/u2/i/ianwang1/legend-resum/celeritas/src/corecel/sys/Stream.cc:20:0:
/global/u2/i/ianwang1/legend-resum/celeritas/src/corecel/sys/detail/AsyncMemoryResource.hh:12:14: fatal error: thrust/mr/memory_resource.h: No such file or directory
 #    include <thrust/mr/memory_resource.h>
              ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
compilation terminated.
make[2]: *** [src/corecel/CMakeFiles/corecel.dir/build.make:790: src/corecel/CMakeFiles/corecel.dir/sys/Stream.cc.o] Error 1
make[2]: *** Waiting for unfinished jobs....
make[1]: *** [CMakeFiles/Makefile2:351: src/corecel/CMakeFiles/corecel.dir/all] Error 2
make: *** [Makefile:136: all] Error 2
```