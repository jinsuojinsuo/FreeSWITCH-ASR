#pragma once
#ifdef _WINDOWS_COMPILE_
#ifndef  ASR_API
#define ASR_API _declspec(dllexport)
#endif
#else
#define ASR_API
#endif

#include <iostream>
#include <vector>
#include <map>
#include "pthread.h"

class INlsRequestParam;
class Nls;

class ASR_API NlsEvent {
public:
	enum EventType {
		Normal = 0,
		Error = 1,
		Close =2,
		Binary =3
	};

	NlsEvent(NlsEvent& );
	NlsEvent(std::string msg, int code, EventType type);
	NlsEvent(std::vector<unsigned char> data, int code, EventType type);
	~NlsEvent();

	/**
    * @brief 获取状态码
    * @note 正常情况为0或者200，失败时对应失败的错误码。错误码参考SDK文档说明。
    * @return string
    */
	std::string getStausCode();

    /**
    * @brief 获取云端返回的结果
    * @note json格式
    * @return string
    */
	std::string getResponse();

    /**
    * @brief 获取NlsRequest操作过程中出现失败时的错误信息
    * @note 在Failed回调函数中使用
    * @return string
    */
    std::string getErrorMessage();

    /**
    * @brief 获取云端返回的二进制数据
    * @note 仅用于tts语音合成功能
    * @return vector<unsigned char>
    */
	std::vector<unsigned char> getBinaryData();

    /**
    * @brief 获取当前所发生Event的类型
    * @note Event值参照NlsClient.h说明
    * @return EventType
    */
	EventType getMsgType();

    /**
    * @brief 获取当前request对象id
    * @return string
    */
	std::string getId();

    /**
    * @brief 设置当前request对象id
    * @return
    */
    void setId(std::string id);

private:
	int _errorcode;
	std::string _msg;
	EventType _msgtype;
	std::string _identifyId;
	std::vector<unsigned char> _binaryData;
};

typedef void(*NlsCallbackMethod)(NlsEvent*,void*);

class ASR_API NlsSpeechCallback {
public:
	NlsSpeechCallback();
	~NlsSpeechCallback();

    /**
    * @brief 设置结果消息接受回调函数
    * @param _event	回调方法
    * @param para	用户传入参数, 默认为NULL
    * @return void
    */
	void setOnMessageReceiced(NlsCallbackMethod onMessageReceived,void*para=NULL);

    /**
    * @brief 设置错误回调函数
    * @note 在请求过程中出现错误时，触发该回调。用户可以在事件的消息头中检查状态码和状态消息，以确认失败的具体原因。
    * @param _event	回调方法
    * @param para	用户传入参数, 默认为NULL
    * @return void
    */
    void setOnOperationFailed(NlsCallbackMethod _event, void*para=NULL);

    /**
    * @brief 设置通道关闭回调函数
    * @note 在请求过程中通道关闭时，触发该回调
    * @param _event	回调方法
    * @param para	用户传入参数, 默认为NULL
    * @return void
    */
    void setOnChannelClosed(NlsCallbackMethod _event, void*para=NULL);

    /**
    * @brief 二进制音频数据接收回调函数
    * @note 仅在tts语音合成中触发该回调
    * @param _event	回调方法
    * @param para	用户传入参数, 默认为NULL
    * @return void
    */
    void setOnBinaryDataReceived(NlsCallbackMethod _event, void*para=NULL);


    NlsCallbackMethod _onMessageReceived;
	NlsCallbackMethod _onOperationFailed;
	NlsCallbackMethod _onChannelClosed;
	NlsCallbackMethod _onBinaryDataReceived;
	std::map < NlsEvent::EventType, void*> _paramap;
};

class ASR_API NlsRequest {
public:
	NlsRequest(NlsSpeechCallback*, INlsRequestParam* rtnr, int requestmode = 3);
	~NlsRequest();

    /**
    * @brief 参数设置
    * @param value appKey字符串
    * @return 成功则返回0，否则返回-1
    */
	int SetParam(const char* str_key, const char* str_value);	// add format

    /**
    * @brief 启动Request
    * @note 阻塞操作，等待服务端响应、或10秒超时才会返回
    * @return 成功则返回0，否则返回-1
    */
	int Start();

    /**
    * @brief 会与服务端确认关闭，正常停止SpeechRecognizerRequest链接操作
    * @note 阻塞操作，等待服务端响应才会返回
    * @return 成功则返回0，否则返回-1
    */
	int Stop();

    /**
    * @brief 不会与服务端确认关闭，直接关闭SpeechRecognizerRequest链接
    * @note 正常情况下，建议使用stop结束操作
    * @return 成功则返回0，否则返回-1
    */
	int Cancel();

    /**
    * @brief 授权校验
    * @param id
    * @param scret
    * @return 成功则返回实际发送长度，失败返回-1
    */
	int Authorize(const char* id, const char* scret);

    /**
    * @brief 发送语音数据
    * @param data	语音数据
    * @param dataSize	语音数据长度
    * @return 成功则返回实际发送长度，失败返回-1
    */
	int SendAudio(char* data, size_t num_byte);

private:
	Nls* _session;
	NlsSpeechCallback* _callbck;
	int _mode;
};

class ASR_API NlsClient {
public:
	~NlsClient();

    /**
    * @brief 设置日志文件与存储路径
    * @param logOutputFile	日志文件
    * @param logLevel	日志级别，默认1（Error : 1、Warn : 2、Debug : 3）
    * @return 成功则返回0，失败返回-1
    */
	int setLogConfig(const char* logOutputFile, int loglevel);

    /**
    * @brief 创建实时语音识别Request对象
    * @param onResultReceivedEvent	事件回调接口
    * @param config	配置文件
    * @return 成功返回NlsRequest对象，否则返回NULL
    */
	NlsRequest* createRealTimeRequest(NlsSpeechCallback* onResultReceivedEvent, const char* config);

    /**
    * @brief 创建一句话识别Request对象
    * @param onResultReceivedEvent	事件回调接口
    * @param config	配置文件
    * @return 成功返回NlsRequest对象，否则返回NULL
    */
	NlsRequest* createAsrRequest(NlsSpeechCallback* onResultReceivedEvent, const char* config);

    /**
    * @brief 创建语音合成Request对象
    * @param onResultReceivedEvent	事件回调接口
    * @param config	配置文件
    * @return 成功返回NlsRequest对象，否则返回NULL
    */
	NlsRequest* createTtsRequest(NlsSpeechCallback* onResultReceivedEvent, const char* config);

    /**
    * @brief 创建NLu Request对象
    * @param onResultReceivedEvent	事件回调接口
    * @param config	配置文件
    * @return 成功返回NlsRequest对象，否则返回NULL
    */
    NlsRequest* createNluRequest(NlsSpeechCallback* onResultReceivedEvent, const char* config);

    /**
    * @brief NlsClient对象实例
    * @param sslInitial	是否初始化openssl 线程安全，默认为true
    * @return NlsClient对象
    */
    static NlsClient* getInstance(bool sslInitial = true);

private:
	NlsClient();
	static pthread_mutex_t _mtx;
	static bool _isInitializeSSL;
	static NlsClient* _instance;
};
