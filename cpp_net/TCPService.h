#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <assert.h>
#include <stdint.h>
#include <memory>

#include "datasocket.h"
#include "typeids.h"

class EventLoop;
class DataSocket;

class ListenThread
{
public:
    typedef std::shared_ptr<ListenThread>   PTR;

    typedef std::function<void(int fd)> ACCEPT_CALLBACK;
    ListenThread();
    ~ListenThread();

    /*  ���������߳�  */
    void                                startListen(int port, const char *certificate, const char *privatekey, ACCEPT_CALLBACK callback);
    void                                closeListenThread();
#ifdef USE_OPENSSL
    SSL_CTX*                            getOpenSSLCTX();
#endif
private:
    void                                RunListen();
    void                                initSSL();
    void                                destroySSL();
private:
    ACCEPT_CALLBACK                     mAcceptCallback;
    int                                 mPort;
    bool                                mRunListen;
    std::thread*                        mListenThread;
    std::string                         mCertificate;
    std::string                         mPrivatekey;
#ifdef USE_OPENSSL
    SSL_CTX*                            mOpenSSLCTX;
#endif
};

class TcpService
{
public:
    typedef std::shared_ptr<TcpService>                                 PTR;

    typedef std::function<void (EventLoop&)>                            FRAME_CALLBACK;
    typedef std::function<void(int64_t, std::string)>                   ENTER_CALLBACK;
    typedef std::function<void(int64_t)>                                DISCONNECT_CALLBACK;
    typedef std::function<int (int64_t, const char* buffer, int len)>   DATA_CALLBACK;

public:
    TcpService();
    ~TcpService();

    /*  ����Ĭ���¼��ص�    */
    void                                setEnterCallback(TcpService::ENTER_CALLBACK callback);
    void                                setDisconnectCallback(TcpService::DISCONNECT_CALLBACK callback);
    void                                setDataCallback(TcpService::DATA_CALLBACK callback);

    void                                send(int64_t id, DataSocket::PACKET_PTR&& packet);
    void                                send(int64_t id, const DataSocket::PACKET_PTR& packet);

    /*�����Ͽ���id���ӣ�����Ȼ���յ���id�ĶϿ��ص�����Ҫ�ϲ��߼��Լ��������"����"(����ͳһ�ڶϿ��ص��������������ȹ���) */
    void                                disConnect(int64_t id);

    void                                setPingCheckTime(int64_t id, int checktime);

    void                                addDataSocket(int fd, DataSocket::PTR datasocket,
                                                        TcpService::ENTER_CALLBACK enterCallback,
                                                        TcpService::DISCONNECT_CALLBACK disConnectCallback,
                                                        TcpService::DATA_CALLBACK dataCallback);

    /*  ���������߳�  */
    void                                startListen(int port, const char *certificate = nullptr, const char *privatekey = nullptr);
    /*  ����IO�����߳�    */
    void                                startWorkerThread(int threadNum, FRAME_CALLBACK callback = nullptr);

    /*  �رշ���(�������ڴ�):���̰߳�ȫ    */
    void                                closeService();
    void                                closeListenThread();
    void                                closeWorkerThread();

    /*  ������ֹͣ�����߳��Լ���ÿ��EventLoop�˳�ѭ���������ͷ�EventLoop�ڴ� */
    void                                stopWorkerThread();

    /*  wakeupĳid���ڵ����繤���߳�:���̰߳�ȫ    */
    void                                wakeup(int64_t id);
    /*  wakeup ���е����繤���߳�:���̰߳�ȫ  */
    void                                wakeupAll();
    /*  �����ȡһ��EventLoop:���̰߳�ȫ   */
    EventLoop*                          getRandomEventLoop();
    
private:
    int64_t                             MakeID(int loopIndex);

    void                                procDataSocketClose(DataSocket::PTR);

private:
    EventLoop*                          mLoops;
    std::thread**                       mIOThreads;
    int                                 mLoopNum;
    bool                                mRunIOLoop;

    ListenThread                        mListenThread;

    TypeIDS<DataSocket::PTR>*           mIds;
    int*                                mIncIds;

    /*  ���������ص��������ڶ��߳��е���(ÿ���̼߳�һ��eventloop������io loop)(����RunListen�е�ʹ��)   */
    TcpService::ENTER_CALLBACK           mEnterCallback;
    TcpService::DISCONNECT_CALLBACK      mDisConnectCallback;
    TcpService::DATA_CALLBACK            mDataCallback;

    /*  �˽ṹ���ڱ�ʾһ���ػ����߼��̺߳������߳�ͨ����ͨ���˽ṹ�Իػ�������ز���(������ֱ�Ӵ���Channel/DataSocketָ��)  */
    
    union SessionId
    {
        struct
        {
            uint16_t    loopIndex;      /*  �Ự������eventloop��(��mLoops�е�)����  */
            uint16_t    index;          /*  �Ự��mIds[loopIndex]�е�����ֵ */
            uint32_t    iid;            /*  ����������   */
        }data;  /*  warn::so,���������֧��0xFFFF(65536)��io loop�̣߳�ÿһ��io loop���֧��0xFFFF(65536)�����ӡ�*/

        int64_t id;
    };
};

#endif