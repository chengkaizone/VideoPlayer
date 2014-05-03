#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#include <string.h>
#include <jni.h>
#include <pthread.h>

#include <android/log.h>
//ndk log库来输出日志在控制台显示
#define LOG_TAG "System.out.c"  //__VA_ARGS__表示可变参数
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
//转换成java类型的字符串
#define RETURN(msg) return (*env)->NewStringUTF(env, msg);

//媒体文件包信息容器---本质就是一个媒体文件
AVFormatContext *pFormatCtx;
//编解码器容器---和解码器一一对应
AVCodecContext *pCodecCtx;
//AV解码器
AVCodec *pCodec;
//视频信息
AVFrame *pFrame;
//这里是保存的视频帧的包信息--->n个AVPacket组成一个AVFrame
AVPacket packet;

//视频流
int i;
//视频流ID
int videoStreamID;
//音频流ID
int audioStreamID;
//帧数
int frameCount = 0;

int frameFinished;
//码率
float aspect_ratio;

AVFrame *pFrameRGB;
int numBytes;
//保存到缓存的图像数组
uint8_t *buffer;
// BE for Big Endian, LE for Little Endian
int dstFmt = PIX_FMT_RGB565;
//包含视频分辨率,色彩空间的容器
struct SwsContext *img_convert_ctx;
//声明宽高,比特率
int width, height, bit_rate;
//屏幕宽高
int screenWidth, screenHeight;

/*****************************************************/
/* FFmpeg API jni方法不能在c源文件中调用 */
/*****************************************************/

/** 1.注册所有编解码容器 */
void Java_org_lance_decode_FFmpeg_avRegisterAll(JNIEnv* env, jobject thiz) {
	av_register_all();
}

/** 2.打开一个媒体文件 0表示打开失败,1表示打开成功,同时初始化媒体文件 ! */
jboolean Java_org_lance_decode_FFmpeg_avOpenInputFile(JNIEnv* env, jobject thiz,
		jstring jfilePath) {
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	//LOGI("path---->%s",filePath);
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
		return 0;
	} else {
		return 1;
	}
}

/** close 关闭媒体文件容器,释放所有内存空间 */
void Java_org_lance_decode_FFmpeg_avCloseInputFile(JNIEnv* env, jobject thiz) {
	avformat_close_input(&pFormatCtx);
}

/** 初始化媒体文件 */
jboolean Java_org_lance_decode_FFmpeg_initFile(JNIEnv *env, jobject thiz,
		jstring jfilePath) {
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	//LOGI("path---->%s",filePath);
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) == 0) {
		LOGI("success open");
	} else {
		LOGE("failed open");
		return 0;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
		LOGI("success find stream info");
	} else {
		LOGE("failed find stream info");
		return 0;
	}

	//遍历所有的流信息---是否存在视频类型
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamID = i;
			break;
		}
	}

	//遍历所有的流信息---是否存在音频类型
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStreamID = i;
			break;
		}
	}

	if (videoStreamID == -1) {
		LOGE("failed find video stream");
		//-1为未知流信息
		return 0;
	} else {
		LOGI("success find video stream");
	}

	if (audioStreamID == -1) {
		LOGE("failed find audio stream");
	} else {
		LOGI("success find audio stream  %d ", audioStreamID);
	}

	//得到解码容器
	pCodecCtx = pFormatCtx->streams[videoStreamID]->codec;
	//根据译码ID找到合适的解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		LOGE("failed find decoder");
		return 0;
	} else {
		LOGI("success find decoder");
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) == 0) {
		LOGI("success codec open");
	} else {
		LOGE("failed codec open");
		return 0;
	}

	LOGI("codec name--->%s", pCodec->name);
	return 1;
}

/** 查找流信息 */
jboolean Java_org_lance_decode_FFmpeg_avFindStreamInfo(JNIEnv* env,
		jobject thiz) {
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		return 0;
	} else {
		return 1;
	}
}
/** 查找视频流 0表示返回失败 1表示成功!*/
jboolean Java_org_lance_decode_FFmpeg_findVideoStream(JNIEnv* env, jobject thiz) {
	videoStreamID = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) { //遍历所有的流信息---是否存在视频类型
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamID = i; //这里得到视频流的ID
			break;
		}
	}
	if (videoStreamID == -1) {
		return 0;
	} else {
		return 1;
	}
}
/** 查找合适的解码器 */
jboolean Java_org_lance_decode_FFmpeg_avcodecFindDecoder(JNIEnv* env,
		jobject thiz) {
	//解码器容器,用于分配和释放
	pCodecCtx = pFormatCtx->streams[videoStreamID]->codec;
	//根据译码ID找到合适的解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		return 0;
	} else {
		return 1;
	}
}
/** 打开解码器 */
jboolean Java_org_lance_decode_FFmpeg_avcodecOpen(JNIEnv* env, jobject thiz) {
	//使用给定的解码器初始化解码容器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		return 0;
	} else {
		return 1;
	}
}
/** 分配帧空间 */
void Java_org_lance_decode_FFmpeg_avcodecAllocFrame(JNIEnv* env, jobject thiz) {
	//分配一个帧空间,数据初始化为默认值---分配失败返回NULL
	pFrame = avcodec_alloc_frame();
}
/** 释放分配的空间 */
void Java_org_lance_decode_FFmpeg_avFree(JNIEnv* env, jobject thiz) {
	//释放分配的内存空间
	av_free(pFrame);
}

/** 关闭解码容器并释放与之关联的内存空间 */
void Java_org_lance_decode_FFmpeg_avcodecClose(JNIEnv* env, jobject thiz) {
	avcodec_close(pCodecCtx);
}

/*****************************************************/
/* 回调给java应用层的函数 */
/*****************************************************/
/** 获取解码器名 */
jstring Java_org_lance_decode_FFmpeg_getCodecName(JNIEnv* env, jobject thiz) {
	return (*env)->NewStringUTF(env, pCodec->name);
}
/** 获取视频文件宽度 */
jint Java_org_lance_decode_FFmpeg_getWidth(JNIEnv* env, jobject thiz) {
	width = pCodecCtx->width;
	return width;
}
/** 获取视频文件高度 */
jint Java_org_lance_decode_FFmpeg_getHeight(JNIEnv* env, jobject thiz) {
	height = pCodecCtx->height;
	return height;
}
/** 获取视频比特率 341948 */
jint Java_org_lance_decode_FFmpeg_getBitRate(JNIEnv* env, jobject thiz) {
	bit_rate = pCodecCtx->bit_rate;
	return bit_rate;
}
/** 分配帧结构缓存 只需要分配一帧图像的数组空间 */
jboolean Java_org_lance_decode_FFmpeg_allocateBuffer(JNIEnv* env, jobject thiz) {
	// Allocate an AVFrame structure
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL) {
		return 0;
	}
	//将信息保存到debugMsg中然后打印出日志
	LOGI("screen: width=%d height=%d", screenWidth, screenHeight);
	LOGI("video: width=%d height=%d", pCodecCtx->width, pCodecCtx->height);
	// 根据屏幕宽高和图片质量确定缓存区大小
	numBytes = avpicture_get_size(dstFmt, screenWidth, screenHeight);
	//numBytes=avpicture_get_size(dstFmt, pCodecCtx->width,pCodecCtx->height);
	//动态分配指定大小的内存空间保存到数组中
	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

	//分配适当的缓存部分到rgb图像帧中 AVFrame是AVPicture的超集---填充所有图像信息
	avpicture_fill((AVPicture *) pFrameRGB, buffer, dstFmt, screenWidth,
			screenHeight);

	return 1;
}
/** 设置屏幕尺寸 */
void Java_org_lance_decode_FFmpeg_setScreenSize(JNIEnv* env, jobject thiz,
		int sWidth, int sHeight) {
	screenWidth = sWidth;
	screenHeight = sHeight;
}

/* 返回一个流的下一帧 */
jbyteArray Java_org_lance_decode_FFmpeg_getNextDecodedFrame(JNIEnv* env,
		jobject thiz) {
	//解码前先释放上一帧保存的包信息
	av_free_packet(&packet);
	//它不会忽略无效数据之间的有效帧给解码器最大信息解码。返回信息保存在packet中
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoStreamID) {
			//取出视频流的部分---讲解码出的视频帧转换成图片---部分解码器在单个AVPacket支持多个帧,这样的解码器就第一帧解码。
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			LOGI("length->%d video stream:%d", frameFinished, packet.stream_index);
			if (frameFinished) {
				img_convert_ctx = sws_getCachedContext(img_convert_ctx,
						pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
						screenWidth, screenHeight, dstFmt, SWS_BICUBIC, NULL,
						NULL, NULL);
				//img_convert_ctx = sws_getContext(pCodecCtx->width,
				//		pCodecCtx->height, pCodecCtx->pix_fmt, screenWidth,
				//	screenHeight, dstFmt, SWS_BICUBIC, NULL, NULL, NULL);
				//将源片缩放到目标片中
				sws_scale(img_convert_ctx,
						(const uint8_t* const *) pFrame->data, pFrame->linesize,
						0, pCodecCtx->height, pFrameRGB->data,
						pFrameRGB->linesize);
				++frameCount;
				/* uint8_t == unsigned 8 bits == jboolean 创建图像数据数组 */
				jbyteArray nativePixels = (*env)->NewByteArray(env, numBytes);
				//使用缓存为图像数据数组赋图像数据
				(*env)->SetByteArrayRegion(env, nativePixels, 0, numBytes,
						buffer);
				return nativePixels;
			}
		}
		av_free_packet(&packet);
	}
	return NULL;
}

/* 播放指定路径的视频 该函数没有在主线程中使用 */
jstring Java_org_lance_decode_FFmpeg_play(JNIEnv* env, jobject thiz,
		jstring jfilePath) {
	//这里就返回播放路径
	char* filePath = (char *) (*env)->GetStringUTFChars(env, jfilePath, NULL);
	RETURN(filePath);
	AVFormatContext *pFormatCtx;
	int i, videoStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVPacket packet;
	int frameFinished;
	float aspect_ratio;
	struct SwsContext *img_convert_ctx;

	/* FFmpeg */
	av_register_all();
	if (avformat_open_input(&pFormatCtx, filePath, NULL, NULL) != 0) {
		RETURN("failed avformat_open_input ");
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		RETURN("failed av_find_stream_info");
	}

	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1) {
		RETURN("failed videostream == -1");
	}

	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		RETURN("Unsupported codec!");
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		RETURN("failed codec_open");
	}

	pFrame = avcodec_alloc_frame();

	/* FFmpeg */

	LOGI("decodec :%s", pCodec->name);
	LOGI("Getting into stream decode:");

	/* video stream */
	//解码前先释放上一帧保存的包信息
//		av_free_packet(&packet);
//		//它不会忽略无效数据之间的有效帧给解码器最大信息解码。返回信息保存在packet中
//		while (av_read_frame(pFormatCtx, &packet) >= 0) {
//			if (packet.stream_index == videoStreamID) {
//				//取出视频流的部分---讲解码出的视频帧转换成图片---部分解码器在单个AVPacket支持多个帧,这样的解码器就第一帧解码。
//				avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
//				LOGI("length->%d video stream:%d", frameFinished, packet.stream_index);
//				if (frameFinished) {
//					img_convert_ctx = sws_getCachedContext(img_convert_ctx,
//							pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
//							screenWidth, screenHeight, dstFmt, SWS_BICUBIC, NULL,
//							NULL, NULL);
//					//img_convert_ctx = sws_getContext(pCodecCtx->width,
//					//		pCodecCtx->height, pCodecCtx->pix_fmt, screenWidth,
//					//	screenHeight, dstFmt, SWS_BICUBIC, NULL, NULL, NULL);
//					//将源片缩放到目标片中
//					sws_scale(img_convert_ctx,
//							(const uint8_t* const *) pFrame->data, pFrame->linesize,
//							0, pCodecCtx->height, pFrameRGB->data,
//							pFrameRGB->linesize);
//					++frameCount;
//					/* uint8_t == unsigned 8 bits == jboolean 创建图像数据数组 */
//					jbyteArray nativePixels = (*env)->NewByteArray(env, numBytes);
//					//使用缓存为图像数据数组赋图像数据
//					(*env)->SetByteArrayRegion(env, nativePixels, 0, numBytes,
//							buffer);
//					return nativePixels;
//				}
//			}
//			av_free_packet(&packet);
//		}

	/* video stream */
	av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return (*env)->NewStringUTF(env, "end of main");
}

// TODO =============================附加工具类======================

/** 测试JNI */
jstring Java_org_lance_decode_FFmpeg_stringFromJNI(JNIEnv* env, jobject thiz) {
	return (*env)->NewStringUTF(env, "Helloworld from FFmpeg!");
}

/** 测试返回一个数组 */
jintArray Java_org_lance_decode_FFmpeg_jniIntArray(JNIEnv* env, jobject thiz) {
	LOGI("test return array~");
	int nativeIntArray[2];
	nativeIntArray[0] = 330;
	nativeIntArray[1] = 2;
	LOGI("0 %d", nativeIntArray[0]);
	LOGI("1 %d", nativeIntArray[1]);

	jintArray nativeReturn = (*env)->NewIntArray(env, 2);
	(*env)->SetIntArrayRegion(env, nativeReturn, 0, 2, nativeIntArray);
	return nativeReturn;
}

/** 将java的中文字符串转化成c语言的字符串 这里是用反射机制 */
char* Jstring2CStr(JNIEnv* env, jstring jstr) {
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env, "java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env, "GB2312");
	jmethodID mid = (*env)->GetMethodID(env, clsstring, "getBytes",
			"(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray) (*env)->CallObjectMethod(env, jstr, mid,
			strencode); // String .getByte("GB2312");
	jsize alen = (*env)->GetArrayLength(env, barr);
	jbyte* ba = (*env)->GetByteArrayElements(env, barr, JNI_FALSE);
	if (alen > 0) {
		rtn = (char*) malloc(alen + 1); //"\0"
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	//rtn是动态内存分配---将ba指针的内容拷贝到rtn中,所以后面需要释放的是ba指针
	(*env)->ReleaseByteArrayElements(env, barr, rtn, 0); //
	return rtn;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL; //定义JNI Env
	jint result = JNI_ERR;
	/*JavaVM::GetEnv 原型为 jint (*GetEnv)(JavaVM*, void**, jint);
	 * GetEnv()函数返回的  Jni 环境对每个线程来说是不同的，
	 *  由于Dalvik虚拟机通常是Multi-threading的。每一个线程调用JNI_OnLoad()时，
	 *  所用的JNI Env是不同的，因此我们必须在每次进入函数时都要通过vm->GetEnv重新获取
	 *
	 */
	//得到JNI Env
//	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
//		LOGE("GetEnv failed!");
//		return result;
//	}
//	LOGI("loading...");
//
//	/*开始注册
//	 * 传入参数是JNI env
//	 * 由于下面有很多class，只以register_android_media_FFMpegPlayerAndroid(env)为例子
//	 */
//
//#ifdef BUILD_WITH_CONVERTOR
//	if(register_android_media_FFMpeg(env) != JNI_OK) {
//		__android_log_print(ANDROID_LOG_ERROR, TAG, "can't load android_media_FFMpeg");
//		goto end;
//	}
//#endif
//	if (register_android_media_FFMpegAVFormatContext(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVFormatContext");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVCodecContext(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVCodecContext");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVRational(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVRational");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVInputFormat(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVInputFormat");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegUtils(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegUtils");
//		goto end;
//	}
//
//	if (register_android_media_FFMpegAVFrame(env) != JNI_OK) {
//		LOGE("can't load android_media_FFMpegAVFrame");
//		goto end;
//	}
//
//#ifdef BUILD_WITH_PLAYER
//	if(register_android_media_FFMpegPlayerAndroid(env) != JNI_OK) { //跳到----》com_media_ffmpeg_FFMpegPlayer.cpp文件
//		LOGE("can't load android_media_FFMpegPlayerAndroid");
//		goto end;
//	}
//#endif
//	LOGI("loaded!");

	result = JNI_VERSION_1_4;

	return result;
}
