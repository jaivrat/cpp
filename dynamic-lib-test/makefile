
all: compile_add compile_sum compile_exec

compile_add: ./add/add.cpp
	@echo  "Step 01 - add.."
	g++ -c ./add/add.cpp -o ./add/add.o -I/add
	g++ -dynamiclib -o ./add/libadd.dylib ./add/add.o
	@echo  "-------------------------------------------------------"


compile_sum: ./sum/sum.cpp
	@echo  "Step 02 - add.."
	g++ -c ./sum/sum.cpp -o ./sum/sum.o -I./sum  -I./add
	g++ -dynamiclib -o ./sum/libsum.dylib ./sum/sum.o  -L./add -ladd
	@echo  "-------------------------------------------------------"


compile_exec: ./sum/sum.cpp
	@echo  "Step 03 - executable.."
	g++ -o  test.exe test.cpp -I./sum  -L./sum -lsum
	@echo  "-------------------------------------------------------"

clean:
	rm ./add/add.o
	rm ./sum/sum.o
	rm ./sum/libsum.dylib
	rm ./add/libadd.dylib
	rm test.exe
