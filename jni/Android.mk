

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= andfix.cpp\
					art/art_method_replace.cpp \
					art/art_method_replace_5_0.cpp \
					art/art_method_replace_5_1.cpp \
					art/art_method_replace_6_0.cpp \
					dalvik/dalvik_method_replace.cpp \

LOCAL_CFLAGS	:= -std=gnu++11 -fpermissive -DDEBUG -O0

LOCAL_C_INCLUDES := 

LOCAL_SHARED_LIBRARIES := 
    
LOCAL_LDLIBS    := -llog

LOCAL_STATIC_LIBRARIES := 

LOCAL_MODULE:= andfix

include $(BUILD_SHARED_LIBRARY)
