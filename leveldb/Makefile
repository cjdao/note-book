.PHONY:default all clean
SRC:=leveldb_example.cc
OBJ:=$(SRC:.cc=.o)
EXE:=leveldb_example

# default target
default:all

all: $(EXE)
	
$(EXE):$(OBJ)
	@g++ -static -L/home/clouder/Code/leveldb/ -o $@ $^ -lleveldb -lpthread
	@echo "Build $(EXE) finished."	

%.o:%.cc
	@echo "Building object $@ ....."
	@g++  -I/home/clouder/Code/leveldb/include  -o $@ -c $^ 

clean:
	@rm -rf $(EXE) $(OBJ)

