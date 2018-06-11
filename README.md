# cpp
Just to test some C++ codes. Checking dynamic C++ libraries search mechanism in osx

1. Compile C++ codes - using 
make -f makefile

2. Above produces two libraries libsum.dylib, libadd.dylib and test.exe
   Note that libsum.dylib itself depends on libsum.dylib.
    
3. I try here to tweak the paths and explore the dynamic loading mechanism.


```
	
jvsingh: ~/work/cpp/dynamic-lib-test  -> mv test.exe  ../
jvsingh: ~/work/cpp/dynamic-lib-test  -> cd ../
jvsingh: ~/work/cpp  -> ./test.exe 
dyld: Library not loaded: ./sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/cpp/./test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/cpp  -> otool -L test.exe 
test.exe:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/cpp  -> install_name_tool -change "./sum/libsum.dylib" "./dynamic-lib-test/sum/libsum.dylib" test.exe
jvsingh: ~/work/cpp  -> otool -L test.exe 
test.exe:
	./dynamic-lib-test/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/cpp  -> ./test.exe 
dyld: Library not loaded: ./add/libadd.dylib
  Referenced from: /Users/jvsingh/work/cpp/dynamic-lib-test/sum/libsum.dylib
  Reason: image not found
Abort trap: 6

jvsingh: ~/work/cpp  -> otool -L ./dynamic-lib-test/sum/libsum.dylib
./dynamic-lib-test/sum/libsum.dylib:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	./add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/cpp  -> install_name_tool -change "./add/libadd.dylib" "./dynamic-lib-test/./add/libadd.dylib" ./dynamic-lib-test/sum/libsum.dylib
jvsingh: ~/work/cpp  -> otool -L ./dynamic-lib-test/sum/libsum.dylib
./dynamic-lib-test/sum/libsum.dylib:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	./dynamic-lib-test/./add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/cpp  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7

```
