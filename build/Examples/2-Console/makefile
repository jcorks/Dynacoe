DYNACOE_ROOT        = ../../../
DYNACOE_LIB_PATH    = $(DYNACOE_ROOT)/build/lib/

# Basic makefile for Dynacoe

OUTPUT_NAME = console

SRCS = main.cpp
# SRCS = testmain.cpp
INCS = 
FLGS = $(shell cat $(DYNACOE_LIB_PATH)lib_compileropts)
LIBS = 







#--------------------
#--------------------
#--------------------


CC = g++
LD = -std=c++11


# Define Dynacoe assets
#DYNACOE_INPUT_BACKEND_LIBS_GAINPUT = -lgainputstatic
DYNACOE_INC_PATHS   = /DynacoeSrc/includes/  /$(shell cat $(DYNACOE_LIB_PATH)lib_incpaths)
DYNACOE_LIB_PATHS   = $(shell cat $(DYNACOE_LIB_PATH)lib_libpaths)   
DYNACOE_LIB_NAME    = -ldynacoe 
DYNACOE_LIBS        =  $(shell cat $(DYNACOE_LIB_PATH)build_libs) 


DYNACOE_INC_PATHS := $(patsubst %,-I$(DYNACOE_ROOT)%, $(DYNACOE_INC_PATHS))
DYNACOE_LIB_PATHS := $(patsubst %,-L$(DYNACOE_ROOT)%, $(DYNACOE_LIB_PATHS)) -L$(DYNACOE_LIB_PATH)





# Gather proper vars

TEMP := $(LIBS)
LIBS := $(DYNACOE_LIB_NAME) $(DYNACOE_LIBS)


TEMP := $(INCS)
INCS := $(DYNACOE_INC_PATHS) $(INCS)

USER_OBJS    := $(patsubst %.cpp,%.o, $(SRCS))
DYNACOE_OBJS := $(patsubst %.cpp,%.o, $(DYNACOE_SRCS))

ALL_SRCS := $(SRCS) $(DYNACOE_SRCS)

LOCAL_USER_OBJS    := $(notdir $(USER_OBJS))
LOCAL_DYNACOE_OBJS := $(notdir $(DYNACOE_OBJS))

# Compile objects - main target



all: $(LOCAL_USER_OBJS)
	$(CC) $(OS_FLAGS)  $(LD) $(FLGS) $(DYNACOE_LIB_PATHS)  $(LOCAL_USER_OBJS) -o $(OUTPUT_NAME)  $(LIBS)  


# The lbrary 
$(DYNACOE_LIB_NAME) :
	$(MAKE) -F ./lib/


# each object file
%.o: %.cpp
	$(CC) $(OS_FLAGS) $(FLGS) $(LD)  $(INCS) -c $(filter %$(patsubst %.o,%.cpp,$@), $(ALL_SRCS))


	
clean:
	rm -f *.o $(OUTPUT_NAME)
