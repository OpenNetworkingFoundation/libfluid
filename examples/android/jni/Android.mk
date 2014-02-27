LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent
LOCAL_SRC_FILES := libevent/.libs/libevent.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libevent/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libevent_pthreads
LOCAL_SRC_FILES := libevent/.libs/libevent_pthreads.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libevent/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfluid_base
LOCAL_SRC_FILES := libfluid_base/.libs/libfluid_base.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libfluid_base
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfluid_msg
LOCAL_SRC_FILES := libfluid_msg/.libs/libfluid_msg.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libfluid_msg
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ofcontroller
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := AndroidController.cc
LOCAL_STATIC_LIBRARIES := libfluid_base libfluid_msg libevent libevent_pthreads
LOCAL_LDLIBS := -llog
LOCAL_CFLAGS += -std=c++11

include $(BUILD_SHARED_LIBRARY)
