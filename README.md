# cpp

Purpose: Some clarity on dynamic linking on macOS, rpath(relative path), loader_path, executable path, LD_LIBRARY_PATH and DYLD_LIBRARY_PATH


I got some issues while using a Machine Learning library on my MacBook Pro, which keeps on failing for C++ dynamic linking errors. I thought to create this **practical** tutorial based on https://www.mikeash.com/pyblog/friday-qa-2009-11-06-linking-and-install-names.html


This will give you how to create library in C++, dynamically link to the executable and search paths on Mac.
```
  System Version:	macOS 10.13.2 (17C88)
  Kernel Version:	Darwin 17.3.0
```   
  

1. Compile C++ codes - using 
make -f makefile
```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7


```

2. Above produces two libraries libsum.dylib, libadd.dylib and test.exe
   Note that libsum.dylib itself depends on libsum.dylib.

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.exe 
test.exe:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./sum/libsum.dylib
./sum/libsum.dylib:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	./add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./add/libadd.dylib
./add/libadd.dylib:
	./add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
```
    
3. I try here to tweak the paths and explore the dynamic loading mechanism. 


```
	
jvsingh: ~/work/cpp/dynamic-lib-test  -> mv test.exe  ../
jvsingh: ~/work/cpp/dynamic-lib-test  -> cd ../
jvsingh: ~/work/cpp  -> ./test.exe 
dyld: Library not loaded: ./sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/cpp/./test.exe
  Reason: image not found
Abort trap: 6

```

###### Hacks using **install_name_tool** to change the names of loaded libraries in the executable


```
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

##### It can be resolved using DYLD_LIBRARY_PATH
(Also note that LD_LIBRARY_PATH had no effect)

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv test.exe ../
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> cd ../
jvsingh: ~/work/github/cpp  -> otool -L test.exe 
test.exe:
	./sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp  -> ./test.exe 
dyld: Library not loaded: ./sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/./test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp  -> export LD_LIBRARY_PATH=~/work/github/cpp/dynamic-lib-test/sum
jvsingh: ~/work/github/cpp  -> ./test.exe 
dyld: Library not loaded: ./sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/./test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp  -> export DYLD_LIBRARY_PATH=~/work/github/cpp/dynamic-lib-test/sum
jvsingh: ~/work/github/cpp  -> ./test.exe 
dyld: Library not loaded: ./add/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/sum/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp  -> export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/Users/jvsingh/work/github/cpp/dynamic-lib-test/add
jvsingh: ~/work/github/cpp  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp  -> 

```


### Using Executable path
compile using makefile.executable_path
```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile.executable_path
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o -install_name @executable_path/lib/libadd.dylib
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd -install_name @executable_path/lib/libsum.dylib
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 

```

Dependency :

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.exe 
test.exe:
	@executable_path/lib/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
dyld: Library not loaded: @executable_path/lib/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 

```

#### Move the libsum into desired folder:

```

jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./sum/libsum.dylib ./lib/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
dyld: Library not loaded: @executable_path/lib/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./lib/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
```

##### Now, the complaint for libadd.dylib (libsum.dylib depends on libadd.dylib).

#### Move libadd as well - 

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./add/libadd.dylib ./lib/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
```

#### Lets move whole structure to a new location:
```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test.exe 
dyld: Library not loaded: @executable_path/lib/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./test1/test2/test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv  lib test1/test2/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
```
So we see above that how @executable_path works.


### LOADER PATH method

Note below that libadd.dylib is loaded by sum/libsum.dylib so we have relative path wrt this library as @loader_path/../add/libadd.dylib

similarly libsum.dylib is loaded by test.exe , sp we have loader path 

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile.loader_path
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o -install_name @loader_path/../add/libadd.dylib
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd -install_name @loader_path/sum/libsum.dylib
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump: 'test.': No such file or directory
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.exe 
test.exe:
	@loader_path/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./sum/libsum.dylib 
./sum/libsum.dylib:
	@loader_path/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	@loader_path/../add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./add/libadd.dylib 
./add/libadd.dylib:
	@loader_path/../add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 

```

Lets change relative positions, It works :

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p test1/test2/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p test1/test2/sum
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p test1/test2/add
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv test.exe test1/test2/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./sum/libsum.dylib test1/test2/sum/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./add/libadd.dylib test1/test2/add/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 

```
As expected, the below fails - when paths are distrubed
```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv test1/test2/add test1/test2/fff
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test.exe 
dyld: Library not loaded: @loader_path/../add/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./test1/test2/sum/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  ->
```


### RPATH Method

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile.rpath 
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o -install_name @rpath/add/libadd.dylib
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd -install_name @rpath/sum/libsum.dylib
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum -rpath @executable_path/../../cpp/dynamic-lib-test
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.exe 
test.exe:
	@rpath/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./sum/libsum.dylib 
./sum/libsum.dylib:
	@rpath/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	@rpath/add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./add/libadd.dylib 
./add/libadd.dylib:
	@rpath/add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
```

#### Let us try moving executable to some complex relative paths deep inside

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p test1/test2/test3/test4
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv test.exe test1/test2/test3/test4/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe 
dyld: Library not loaded: @rpath/sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./test1/test2/test3/test4/test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p ./test1/test2/test3/test4/sum
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./sum/libsum.dylib ./test1/test2/test3/test4/sum/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe 
dyld: Library not loaded: @rpath/sum/libsum.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/./test1/test2/test3/test4/test.exe
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ls -lrt ./test1/test2/test3/test4/../../cpp/dynamic-lib-test
ls: ./test1/test2/test3/test4/../../cpp/dynamic-lib-test: No such file or directory
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p  ./test1/test2/test3/test4/../../cpp/dynamic-lib-test
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p  ./test1/test2/test3/test4/../../cpp/dynamic-lib-test/sum
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./test1/test2/test3/test4/sum ./test1/test2/test3/test4/../../cpp/dynamic-lib-test/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ls -lrt ./test1/test2/test3/test4/../../cpp/dynamic-lib-test/*
total 32
-rwxr-xr-x  1 jvsingh  staff  15924 Jun 11 19:46 libsum.dylib
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe 
dyld: Library not loaded: @rpath/add/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/test1/test2/cpp/dynamic-lib-test/sum/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> find ./test1/test2/test3/test4/
./test1/test2/test3/test4/
./test1/test2/test3/test4//test.exe
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> find ./test1/
./test1/
./test1//test2
./test1//test2/cpp
./test1//test2/cpp/dynamic-lib-test
./test1//test2/cpp/dynamic-lib-test/sum
./test1//test2/cpp/dynamic-lib-test/sum/libsum.dylib
./test1//test2/test3
./test1//test2/test3/test4
./test1//test2/test3/test4/test.exe
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p ./test1//test2/cpp/dynamic-lib-test/add
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv ./add/libadd.dylib  ./test1//test2/cpp/dynamic-lib-test/add/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> 

```

Note that rpath can also be resolved by **DYLD_LIBRARY_PATH**

```
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> make -f makefile.rpath
Step 01 - add..
g++ -c ./add/add.cpp -o ./add/add.o -I/add
g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o -install_name @rpath/add/libadd.dylib
-------------------------------------------------------
Step 02 - add..
g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd -install_name @rpath/sum/libsum.dylib
-------------------------------------------------------
Step 03 - executable..
g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum -rpath @executable_path/../../cpp/dynamic-lib-test
-------------------------------------------------------
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L test.exe 
test.exe:
	@rpath/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> otool -L ./sum/libsum.dylib ; otool -L ./add/libadd.dylib 
./sum/libsum.dylib:
	@rpath/sum/libsum.dylib (compatibility version 0.0.0, current version 0.0.0)
	@rpath/add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
./add/libadd.dylib:
	@rpath/add/libadd.dylib (compatibility version 0.0.0, current version 0.0.0)
	/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 400.9.0)
	/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.0.0)
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test.exe 
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mkdir -p test1/test2/test3/test4
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ls -lrt test1/test2/test3/test4
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> mv test.exe test1/test2/test3/test4/
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe 
dyld: Library not loaded: @rpath/add/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/sum/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> export DYLD_LIBRARY_PATH=/Users/jvsingh/work/github/cpp/dynamic-lib-test/sum
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe
dyld: Library not loaded: @rpath/add/libadd.dylib
  Referenced from: /Users/jvsingh/work/github/cpp/dynamic-lib-test/sum/libsum.dylib
  Reason: image not found
Abort trap: 6
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/Users/jvsingh/work/github/cpp/dynamic-lib-test/add
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> ./test1/test2/test3/test4/test.exe
 Called sum with 2 and 5
 Called add with 2 and 5
Calculated result 7
jvsingh: ~/work/github/cpp/dynamic-lib-test  -> echo $DYLD_LIBRARY_PATH
/Users/jvsingh/work/github/cpp/dynamic-lib-test/sum:/Users/jvsingh/work/github/cpp/dynamic-lib-test/add

```
So it works :)
