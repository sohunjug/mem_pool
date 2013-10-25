# gcc编译tuxedo client的Makefile文件
#######################################################
FileName=test
Exe=$(FileName)
ExeLib=$(FileName).so
CC = g++
CFLAGS = -Wall -g

OBJ_PATH = objs
LINKFLAGS = -ldl -lpthread

#Cpp文件夹
SRCDIR = .

CPP_SRCDIR = $(SRCDIR)
CPP_SOURCES = $(foreach d,$(CPP_SRCDIR),$(wildcard $(d)/*.cpp) )
CPP_OBJS = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(CPP_SOURCES))
CPPLIB = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(CPP_SOURCES))

C_SRCDIR = $(SRCDIR)
C_SOURCES = $(foreach d,$(C_SRCDIR),$(wildcard $(d)/*.c) )
C_OBJS = $(patsubst %.c, $(OBJ_PATH)/%.o, $(C_SOURCES))
CLIB = $(patsubst %.c, $(OBJ_PATH)/%.o, $(C_SOURCES))

default: test init compile

lib:test init libs

$(C_OBJS):$(OBJ_PATH)/%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(CPP_OBJS):$(OBJ_PATH)/%.o:%.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(CLIB):$(OBJ_PATH)/%.o:%.c
	$(CC) -fpic -c $(CFLAGS) $< -o $@

test:
	@echo "$(FileName)"
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "CPP_OBJS: $(CPP_OBJS)"
	@echo "C_OBJS: $(C_OBJS)"

libs:$(C_LIB) $(CPP_LIB)
	$(CC) -shared -fpic $^ -o $(ExeLib) $(LINKFLAGS) 

init:
	$(foreach d,$(SRCDIR), mkdir -p $(OBJ_PATH)/$(d);)

compile:$(C_OBJS) $(CPP_OBJS)
	$(CC)  $^ -o $(Exe)  $(LINKFLAGS)

clean:
	rm -rf $(OBJ_PATH)
	rm -f $(Exe)
	rm -f $(ExeLib)
########################################################
