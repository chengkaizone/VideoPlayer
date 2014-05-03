LOCAL_PATH := $(call my-dir)

#这里是先处理的代码
include $(CLEAR_VARS)
#生成库的名字
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := libffmpeg.so
#先建立以上库
include $(PREBUILT_SHARED_LIBRARY)

#目标库
include $(CLEAR_VARS)
LOCAL_MODULE := VideoPlayer
LOCAL_SRC_FILES := VideoPlayer2.c

#同时生成动态库
LOCAL_SHARED_LIBRARIES += ffmpeg

#引入ndk依赖库 log
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
