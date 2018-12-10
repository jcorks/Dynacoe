FLAGS = $(shell cat lib_compileropts)
DYNACOE_ROOT        = ../..
BACKENDS			= $(shell cat lib_defines)

      
#--------------------
#--------------------
#--------------------


CC = g++  -fmax-errors=5 

# Define Dynacoe assets

DYNACOE_GIT_LINEAR_COUNT = $(shell git rev-list --count HEAD)
DYNACOE_GIT_HASH         = $(shell git rev-parse HEAD)

DYNACOE_SRC_ROOT    = $(DYNACOE_ROOT)/DynacoeSrc/srcs/Dynacoe/
DYNACOE_INC_PATHS   = $(shell cat lib_incpaths) /DynacoeSrc/includes/
DYNACOE_LIB_PATHS   = $(shell cat lib_libpaths)


DYNACOE_INC_PATHS := $(patsubst %,-I$(DYNACOE_ROOT)%, $(DYNACOE_INC_PATHS))
DYNACOE_LIB_PATHS := $(patsubst %,-L$(DYNACOE_ROOT)%, $(DYNACOE_LIB_PATHS))

DYNACOE_LIB_NAME    = libdynacoe.a



DYNACOE_SRCS := $(filter %.cpp %.c, $(shell echo $(DYNACOE_ROOT)/DynacoeSrc/Dynacoe/* \
                          $(DYNACOE_SRC_ROOT)/BuiltIn/* \
                          $(DYNACOE_SRC_ROOT)/Components/* \
                          $(DYNACOE_SRC_ROOT)/Util/* \
                          $(DYNACOE_SRC_ROOT)/Modules/* \
                          $(DYNACOE_SRC_ROOT)/Internal/* \
                          $(DYNACOE_SRC_ROOT)/Encoders/* \
                          $(DYNACOE_SRC_ROOT)/Decoders/* \
                          $(DYNACOE_SRC_ROOT)/Backends/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Display/* \
                          $(DYNACOE_SRC_ROOT)/Backends/InputManager/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Renderer/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Renderer/LegacyGL/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Renderer/ShaderGL/* \
			  $(DYNACOE_SRC_ROOT)/Backends/Renderer/RendererES/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Renderer/SoftRender/* \
                          $(DYNACOE_SRC_ROOT)/Backends/AudioManager/* \
                          $(DYNACOE_SRC_ROOT)/Backends/AudioManager/RtAudio/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Framebuffer/* \
                          $(DYNACOE_SRC_ROOT)/Backends/Framebuffer/OpenGL3/* \
                          $(DYNACOE_SRC_ROOT)/Extensions/* \
                          $(DYNACOE_SRC_ROOT)/Overlay/* \
                          $(DYNACOE_SRC_ROOT)/* \
                          ))






# Gather proper vars

TEMP := $(LIBS)

DYNACOE_OBJS := $(patsubst %.cpp,%.o, $(DYNACOE_SRCS))
LOCAL_DYNACOE_OBJS := $(notdir $(DYNACOE_OBJS))

# Compile objects - main target

# The library 
library: $(LOCAL_DYNACOE_OBJS)
	echo $(DYNACOE_SRCS)
	ar cr  $(DYNACOE_LIB_NAME) $(LOCAL_DYNACOE_OBJS)
	echo rm -rf ./*.o


# each object file
%.o: $(filter %$(patsubst %.o,%.cpp,%), $(DYNACOE_SRCS))
	echo $(filter %$(patsubst %.o,%.cpp,$@), $(DYNACOE_SRCS)) $@
	$(CC) $(FLAGS) $(DEF) $(BACKENDS) $(DYNACOE_INC_PATHS) -DDYNACOE_GIT_LINEAR_COUNT=\"$(DYNACOE_GIT_LINEAR_COUNT)\" -DDYNACOE_GIT_HASH=\"$(DYNACOE_GIT_HASH)\" -c $(filter %$(patsubst %.o,%.cpp,$@), $(DYNACOE_SRCS))


# clean everything
clean:
	rm -rf ./*.o $(DYNACOE_LIB_NAME)

	
