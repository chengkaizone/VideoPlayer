LOCAL_PATH := $(call my-dir)

#�������ȴ���Ĵ���
include $(CLEAR_VARS)
#���ɿ������
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := libffmpeg.so
#�Ƚ������Ͽ�
include $(PREBUILT_SHARED_LIBRARY)

#Ŀ���
include $(CLEAR_VARS)
LOCAL_MODULE := VideoPlayer
LOCAL_SRC_FILES := VideoPlayer2.c

#ͬʱ���ɶ�̬��
LOCAL_SHARED_LIBRARIES += ffmpeg

#����ndk������ log
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
