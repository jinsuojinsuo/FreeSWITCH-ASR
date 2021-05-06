#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "pthread.h"
#include "NlsClient.h"	
#include <stdlib.h>
#include <string.h>
#ifdef _WINDOWS_COMPILE_
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
#define FRAME_SIZE 640

NlsSpeechCallback* gCallback = NULL;
NlsClient* gNlc = NULL;

struct ParamStruct {
	std::string filename;
	std::string config;
	int threadcount;
	int count;
	std::string id;
	std::string key;
	bool isSendData;
};

void* func(void* arg)
{
	int loop = 0;
	while (loop ++ < 1) {
        try {
            ParamStruct* tst = (ParamStruct*)arg;
            if (tst == NULL) {
                std::cout << "filename is not valid" << endl;
                return 0;
            }

            ifstream fs;
            fs.open(tst->filename.c_str(), ios::binary | ios::in);
            if (!fs) {
                throw ("file isn't exist");
            }

            /*******************第一种调用方*****************************/
            //创建一句话识别  NlsRequest，第一个参数为callback对象，第二个参数为config.txt文件
            //request对象在一个会话周期内可以重复使用.
            //会话周期是一个逻辑概念. 比如Demo中，指读取, 发送完整个音频文件数据的时间.
            //音频文件数据发送结束时, 可以delete request.
            //request方法调用，比如create, start, sendAudio, stop, delete必须在
            //同一线程内完成，跨线程使用可能会引起异常错误.
            //下面是以读取config.txt方式创建request
            NlsRequest* request = gNlc->createAsrRequest(gCallback, tst->config.c_str());
            /***********************************************************/

            /*******************第二种调用方式*************************
            //以参数设置方式创建request
            NlsRequest* request = nlc.createAsrRequest(gCallback, NULL);
            request->SetParam("AppKey","nls-service");
            request->SetParam("AsrInEnable","true");
            request->SetParam("ResponseMode","normal");
            request->SetParam("Format","pcm");
            request->SetParam("Url","wss://nls.dataapi.aliyun.com:443");
            ******************/

            if (request == NULL) {
                std::cout << "createAsrRequest fail" << endl;
                return 0;
            }

            request->Authorize(tst->id.c_str(), tst->key.c_str());

            //start()为非异步操作，发送start指令之后，会等待服务端响应、或超时之后才返回
            if (request->Start() < 0) {
                std::cout << "start fail" << endl;
                delete request;
                request = NULL;
                return 0;
            }

            if (tst->isSendData) {
                while (!fs.eof()) {
                    vector<char> data(FRAME_SIZE, 0);
                    fs.read(&data[0], sizeof(char) * FRAME_SIZE);
                    int nlen = fs.gcount();
                    nlen = request->SendAudio((char*)&data[0], nlen);
                    if (nlen <= 0) {
                        std::cout << "send data fail" << endl;
                        break;
                    }

//如果是实时语音数据, 此处不需要sleep操作.
//如果是文件数据或压缩数据, 此处需要sleep操作.
//16K采样率, 16位, 单通道, 每ms数据量32byte.Demo中FRAME_SIZE为640，所以需sleep 20ms.
#ifdef _WINDOWS_COMPILE_
                    Sleep(20);
#else
                    usleep(20 * 1000);
#endif
                }
            }
            fs.close();

            //音频文件数据发送结束(一个会话周期结束), 此时调用stop().
            //stop()非异步，在接受到服务端响应，或者超时之后，才会返回.
            //返回之后，可以调用delete request
                request->Stop();

            delete request;
            request = NULL;
        } catch (const char* e) {
            std::cout << e << endl;
        }
	}

	return 0;

}

void OnResultDataRecved(NlsEvent* str,void* para=NULL) {
	std::cout << "Id:" << (*(int*)para) << " " << str->getResponse() << endl;
}

void OnOperationFailed(NlsEvent* str, void* para=NULL) {
	std::cout << "Id:" << (*(int*)para) << " " << str->getErrorMessage() << endl;
}

void OnChannelCloseed(NlsEvent* str, void* para=NULL) {
	std::cout << "Id:" << (*(int*)para) << " " << str->getResponse() << endl;
}

int main(int arc, char* argv[]) {
	try {

		if (arc <= 7) {
			cout << "param is not valid. Usage: demo test.wav config.txt 1 1 [your-id] [your-scret] 1" << endl;
			return -1;
		}

        int id = 123434;
		gCallback = new NlsSpeechCallback();
		gCallback->setOnMessageReceiced(OnResultDataRecved,&id);
		gCallback->setOnOperationFailed(OnOperationFailed,&id);
		gCallback->setOnChannelClosed(OnChannelCloseed,&id);

		gNlc = NlsClient::getInstance();
		int ret = gNlc->setLogConfig("log-asr.txt",3);

        ParamStruct pa;
		pa.filename = argv[1];
		pa.config = argv[2];
		pa.threadcount = atoi(argv[3]);
		pa.count = atoi(argv[4]);
		pa.id = argv[5];
		pa.key = argv[6];
		pa.isSendData = strcmp(argv[7], "0") == 0 ? false : true;
		const int ThreadCount = pa.threadcount;

        vector<pthread_t> tarr(ThreadCount);
		for (int i = 0; i < pa.count; i++) {
			for (int j = 0; j < ThreadCount; j++)
				pthread_create(&tarr[j], NULL, &func, (void*)&pa);
			for (int j = 0; j < ThreadCount; j++)
				pthread_join(tarr[j], NULL);
		}

		if(gNlc != NULL) {
			delete gNlc;
			gNlc = NULL;
		}

		delete gCallback;
		gCallback = NULL;

	} catch (exception& e) {
		cout << e.what() << endl;
	}

	return 0;

}

