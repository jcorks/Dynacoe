LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_CFLAGS := -DDC_BACKENDS_GLES_X11 -DDC_BACKENDS_GLESFRAMEBUFFER_X11 -DDC_BACKENDS_NOINPUT -DDC_BACKENDS_NOAUDIO -DDC_BACKENDS_NODISPLAY

DYNACOE_ROOT_SRC         = ../../DynacoeSrc/srcs/Dynacoe/
DYNACOE_ROOT_INC         = $(NDK_APP_PROJECT_PATH)/../DynacoeSrc/includes/

DYNACOE_GIT_LINEAR_COUNT = $(shell git rev-list --count HEAD)
DYNACOE_GIT_HASH         = $(shell git rev-parse HEAD)



LOCAL_MODULE    := libdynacoe

LOCAL_SRC_FILES := \
    $(DYNACOE_ROOT_SRC)/Components/Clock.cpp \
    $(DYNACOE_ROOT_SRC)/Components/DataTable.cpp \
    $(DYNACOE_ROOT_SRC)/Components/GUI.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Mutator.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Object2D.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Render2D.cpp \
    $(DYNACOE_ROOT_SRC)/Components/RenderLight.cpp \
    $(DYNACOE_ROOT_SRC)/Components/RenderMesh.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Scheduler.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Sequencer.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Shape2D.cpp \
    $(DYNACOE_ROOT_SRC)/Components/StateControl.cpp \
    $(DYNACOE_ROOT_SRC)/Components/Text2D.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Chain.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Filesys.cpp \
    $(DYNACOE_ROOT_SRC)/Util/FilesysWin.cpp \
    $(DYNACOE_ROOT_SRC)/Util/FilesysUnix.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Iobuffer.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Math.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Parallaxizer.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Random.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Table.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Time.cpp \
    $(DYNACOE_ROOT_SRC)/Util/TransformArray.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Transform.cpp \
    $(DYNACOE_ROOT_SRC)/Util/TransformMatrix.cpp \
    $(DYNACOE_ROOT_SRC)/Util/Vector.cpp \
    $(DYNACOE_ROOT_SRC)/AudioBlock.cpp \
    $(DYNACOE_ROOT_SRC)/Camera.cpp \
    $(DYNACOE_ROOT_SRC)/Color.cpp \
    $(DYNACOE_ROOT_SRC)/Component.cpp \
    $(DYNACOE_ROOT_SRC)/Dynacoe.cpp \
    $(DYNACOE_ROOT_SRC)/Entity.cpp \
    $(DYNACOE_ROOT_SRC)/FontAsset.cpp \
    $(DYNACOE_ROOT_SRC)/Image.cpp \
    $(DYNACOE_ROOT_SRC)/Interpreter.cpp \
    $(DYNACOE_ROOT_SRC)/Material.cpp \
    $(DYNACOE_ROOT_SRC)/Mesh.cpp \
    $(DYNACOE_ROOT_SRC)/Model.cpp \
    $(DYNACOE_ROOT_SRC)/Particle.cpp \
    $(DYNACOE_ROOT_SRC)/Sequence.cpp \
    $(DYNACOE_ROOT_SRC)/Shader.cpp \
    $(DYNACOE_ROOT_SRC)/Spatial.cpp \
    $(DYNACOE_ROOT_SRC)/Variable.cpp \
    $(DYNACOE_ROOT_SRC)/BuiltIn/DataGrid.cpp \
    $(DYNACOE_ROOT_SRC)/BuiltIn/FillBar.cpp \
    $(DYNACOE_ROOT_SRC)/BuiltIn/InputBox.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodeFontBasic.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodeOBJ.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodeOGG.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodeParticle.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodePLY.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodePNG.cpp \
    $(DYNACOE_ROOT_SRC)/Decoders/DecodeWAV.cpp \
    $(DYNACOE_ROOT_SRC)/Encoders/EncodePNG.cpp \
    $(DYNACOE_ROOT_SRC)/Extensions/Decode3D.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Assets.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Console.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Debugger.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Graphics.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Input.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/Sound.cpp \
    $(DYNACOE_ROOT_SRC)/Modules/ViewManager.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Backend.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/AudioManager/NoAudio_Multi.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Display/NoDisplay_Multi.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/InputManager/NoInput_Multi.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/InputManager/InputDevice.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES/Light_ES.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES/RenderBuffer_ES.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES/Renderer2D_ES.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES/StaticRenderer.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Renderer/RendererES/Texture_ES.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Framebuffer/GLRenderTarget.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Framebuffer/GLRenderTarget_FBO.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Framebuffer/GLRenderTarget_Legacy.cpp \
    $(DYNACOE_ROOT_SRC)/Backends/Framebuffer/OpenGLFB.cpp 




LOCAL_C_INCLUDES += $(DYNACOE_ROOT_INC) \
	$(NDK_APP_PROJECT_PATH)/jni/deps/freetype/include/ \
	$(NDK_APP_PROJECT_PATH)/jni/deps/ogg/include/ \
        $(NDK_APP_PROJECT_PATH)/jni/deps/vorbis/include/ \
	$(NDK_APP_PROJECT_PATH)/jni/deps/png/include/

#LOCAL_SHARED_LIBRARIES := -lz
LOCAL_EXPORT_LDLIBS := -lm -lpng -lz -lfreetype -lvorbisfile -lvorbis -logg
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/.

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
