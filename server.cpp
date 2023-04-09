#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#define __USE_POSIX
// #define __USE_BSD
#include <signal.h>
#include <setjmp.h>
#include <unordered_map>
#include <stack>
#include <string>
#include <atomic>
#include <nlohmann/json.hpp>
#include <iostream>
#include <iomanip>
#include <list>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <map>
#include <sys/select.h>
#include <memory>
#include <fstream>
#include <sys/sendfile.h>
#include <dirent.h>
#include "database.h"
#include "cutil.h"
#define portAgraph 6667
#define portAdata 6666
#define portBdata 7777
#define portBwarn 7778
#define portTick 6668
#define portBother 7779
#define MESSAE_LENGTH 1000
#define REQUEST_LENGTH 1000
#define ANUM 100
#define BNUM 3000
#define BCBEFORE "not verified yet"
#define BBBEFORE "not connect yet"
// B:connect fd others->connect fd warn->connect fd data->connect board
#define B1 "step1"
#define B2 "step2"
#define STORE_HOUR 3
#define GLEN_32 153666
// for search of SOL_TCP,TCP_KEEPIDLE,TCP_KEEPCNT,TCP_KEEPINTVL
#include <netinet/tcp.h>
#define ERROR_ACTION(X)                                   \
    do                                                    \
    if ((X) == -1)                                        \
    {                                                     \
        printf("error %s:%d", strerror(errno), __LINE__); \
        exit_database();                                  \
        exit(1);                                          \
    }\
    while(0)
#define PHOTOS "photos"
#define FACES "faces"
#define VOICES "voices"
#define DATA_CONF "data.conf"
#define FACE_CONF "data.xml"
#define FACE_DEFAULT "/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_alt2.xml"
#define DATA_DEFAULT "data.conf"
using namespace std;
using namespace nlohmann;
string user_dir;
static int signum;
time_t clock_after;
using namespace std;
// for epoll_wait
struct epoll_event a_data_event[ANUM];
struct epoll_event a_graph_event[ANUM];
struct epoll_event a_video_event[ANUM];
struct epoll_event b_connect_event[BNUM];
struct epoll_event b_data_event[BNUM];
pthread_t Aticks[ANUM];
template <class T>
class WrapStack
{
private:
    pthread_mutex_t dlock;
    stack<T> vdata;

public:
    WrapStack()
    {
        DEBUG("wrapStack inited!");
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapStack()
    {
        pthread_mutex_destroy(&dlock);
    }
    auto push(int dat)
    {
        return vdata.push(dat);
    }
    auto pop(void)
    {
        return vdata.pop();
    }
    auto top(void)
    {
        return vdata.top();
    }
    void lock(void)
    {
        cout << "stackV lock" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        cout << "stackV unlock" << endl;
        pthread_mutex_unlock(&dlock);
    }
    auto empty(void)
    {
        return vdata.empty();
    }
};
WrapStack<int> freeV;

struct Num
{
private:
    static int curA;
    static int curB;
    pthread_mutex_t lockA;
    pthread_mutex_t lockB;

public:
    void increaseA()
    {
        pthread_mutex_lock(&lockA);
        curA++;
        pthread_mutex_unlock(&lockA);
    }
    void decreaseA()
    {
        pthread_mutex_lock(&lockA);
        curA--;
        pthread_mutex_unlock(&lockA);
    }
    void increaseB()
    {
        pthread_mutex_lock(&lockB);
        curB++;
        pthread_mutex_unlock(&lockB);
    }
    void decreaseB()
    {
        pthread_mutex_lock(&lockB);
        curB--;
        pthread_mutex_unlock(&lockB);
    }
    Num()
    {
        curA = 0;
        curB = 0;
        pthread_mutex_init(&lockA, NULL);
        pthread_mutex_init(&lockB, NULL);
    }
    ~Num()
    {
        pthread_mutex_destroy(&lockA);
        pthread_mutex_destroy(&lockB);
    }
};
int Num::curA;
int Num::curB;
pthread_mutex_t BTestlock;
class BNodeInfo
{
public:
    static int id;
    int fd_data; // push,modified to pull
    int fd_other;
    int fd_warn;
    struct sockaddr_in clientData;
    string client_name;
    string board_name;
    string faces;
    bool shut_fd_warn = false;
    BNodeInfo()
    {
        pthread_mutex_lock(&BTestlock);
        id++;
        cout << "---B(id)---" + to_string(id) << endl;
        pthread_mutex_unlock(&BTestlock);
    }
    ~BNodeInfo()
    {
        pthread_mutex_lock(&BTestlock);
        id--;
        cout << "---~B(id)---()" + to_string(id) + " " + board_name << endl;
        pthread_mutex_unlock(&BTestlock);
    }
};
int BNodeInfo::id;
template <class V>
class WrapList
{
private:
    pthread_mutex_t dlock;
    list<V> data;

public:
    WrapList()
    {
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapList()
    {
        pthread_mutex_destroy(&dlock);
    }
    auto push_back(V dat)
    {
        return data.push_back(dat);
    }
    auto back()
    {
        return data.back();
    }
    auto begin()
    {
        return data.begin();
    }
    auto end()
    {
        return data.end();
    }
    auto pop_back()
    {
        return data.push_back();
    }
    auto remove(V dat)
    {
        return data.remove(dat);
    }
    auto erase(decltype(data.begin()) dat)
    {
        return data.erase(dat);
    }
    void lock(void)
    {
        // cout << "nodesA locked" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        // cout << "nodesA unlocked" << endl;
        pthread_mutex_unlock(&dlock);
    }
};
class ANodeInfo
{
public:
    int fd_data;
    int vcode;
    struct sockaddr_in client_data;
    string message;
    string name;
    string position = "anonymous";
    string temp;
    string humi;
    string light;
    string smoke;
    string state;
    string work_dir;
    string type;
    unsigned int wood_time = 0;
    // ANodeInfo *data_node;
    WrapList<BNodeInfo *> connection;
    string face_conf, data_conf;
    string faces, photos, voices;
    double high_temp, high_humi, wrong_light, wrong_smoke;
    unique_ptr<pthread_t> threadVal = unique_ptr<pthread_t>(new pthread_t());

    ANodeInfo()
    {
        // pthread_rwlock_init(&rw_lock, NULL);
    }
    ~ANodeInfo()
    {
        // pthread_rwlock_destroy(&rw_lock);
    }
};
bool operator==(const ANodeInfo &a, const ANodeInfo &b)
{
    return a.name == b.name;
}
template <class KEY, class VALUE>
class WrapMap
{
private:
    pthread_mutex_t dlock;
    unordered_map<KEY, VALUE> nodes;

public:
    WrapMap()
    {
        DEBUG("WrapMap inited!");
        pthread_mutex_init(&dlock, NULL);
    }
    ~WrapMap()
    {
        // cout << "debug:wrap hash_map destroyed" << endl;
        pthread_mutex_destroy(&dlock);
    }
    auto find(string key)
    {
        // cout << "debug:find" << endl;
        return nodes.find(key);
    }
    auto insert(pair<KEY, VALUE> &dat)
    {
        // cout << "debug:insert" << endl;
        return nodes.insert(dat);
    }
    auto emplace(KEY key, VALUE val)
    {
        // cout << "debug:emplace" << endl;
        // nodes.insert({key, val});
        return nodes.emplace(key, val);
    }
    auto erase(KEY key)
    {
        // cout << "debug:erase" << endl;
        // DEBUG(key.c_str());
        return nodes.erase(key);
    }
    auto begin()
    {
        // cout << "debug:begin" << endl;
        return nodes.begin();
    }
    auto end()
    {
        // cout << "debug:end" << endl;
        return nodes.end();
        // cout << "debug:after end" << endl;
    }
    auto count(string key)
    {
        // cout << "debug:count" << endl;
        // pthread_mutex_lock(&lock);
        return nodes.count(key);
        // pthread_mutex_unlock(&lock);
    }
    auto size()
    {
        // cout << "debug:size" << endl;
        return nodes.size();
    }
    void lock(void)
    {
        // cout << "debug:lock" << endl;
        pthread_mutex_lock(&dlock);
    }
    void unlock(void)
    {
        // cout << "debug:unlock" << endl;
        pthread_mutex_unlock(&dlock);
    }
};
string randomName(void)
{
    char tmp[20];
    time_t time_now = time(NULL);
    sprintf(tmp, "%ld", time_now);
    return tmp;
}
WrapMap<string, ANodeInfo *> nodesA;
// nodesA是多线程共享资源，访问必须加锁，对其中的链接访问也需要加锁
struct tickInfo
{
    int fd_data;
    int fd_graph;
    int fd_tick;
    string type;
    string name;
    struct sockaddr_in client_tick;
    unique_ptr<pthread_t> tickThread = unique_ptr<pthread_t>(new pthread_t());
};
// 不同板子的连接共享一份数据是不行的，在一个进程多个连接时必须想办法把数据和每个连接绑定
unsigned int curA,
    curB;
Num numer;
char *ip_addr = "0.0.0.0";
int listenAdata,listenBdata, listenBwarn, listenBother;
// #define exit ::exit
// #define free ::free
// #define memset ::memset
void clean_sock(void)
{

}

void setup_tcp_keepalive(int socketfd, int minutes)
{
    int keepalive = 1;           // enable tcp keepalive
    int keepidle = 60 * minutes; // tcp_keepalive_time
    int keepcnt = 2;             // tcp_keepalive_probes
    int keepintvl = 30;          // tcp_keepalive_intvl
    setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt));
    setsockopt(socketfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl));
}

sigjmp_buf env;
void timeOut(int signo)
{
    cout << "fuck!!!" << endl;
    siglongjmp(env, 1);
}
#define MAX_POLICIES 100
unordered_map<string,void *(*)(void *)> policy_dict;

void *raspi(void *args)
{

}
void *B_L475E_IOT01A(void *args)
{
    ANodeInfo *data = (ANodeInfo *)args;
    int fd_data = data->fd_data;
    char message_box[MESSAE_LENGTH];
    int len, rlen, n;
    string position;
    
    n = recv(fd_data, &rlen, sizeof(rlen), MSG_WAITALL);
    string state,temp,humi;
    if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<sizeof(rlen))
    {
        FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
        goto clean_end;
    }
    else if(n<0)
    {
        FTDEBUG("B-L475E-IOT01A.log", "n<0","");
        perror("recv error in b-l475e-iot01a");
        exit(1);
    }
    len = ntohl(rlen);
    n = recv(fd_data, message_box, len, MSG_WAITALL);
    if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<len)
    {
        FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
        goto clean_end;
    }
    else if(n<0)
    {
        FTDEBUG("B-L475E-IOT01A.log", "n<0","");
        perror("recv error in b-l475e-iot01a");
        exit(1);
    }
    message_box[len] = '\0';
    position = message_box;
    while(1)
    {
        n = recv(fd_data,&rlen,sizeof(rlen),MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<sizeof(rlen))
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        len = ntohl(rlen);
        printf("len=%d\n",len);
        n = recv(fd_data,message_box,len,MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<len)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        message_box[len] = '\0';    
        printf("message_box:%s\n",message_box);
        state = message_box;
        n = recv(fd_data,&rlen,sizeof(rlen),MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<sizeof(rlen))
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        len = ntohl(rlen);
        n = recv(fd_data,message_box,len,MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<len)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        message_box[len] = '\0';
        printf("message_box:%s\n",message_box);
        temp = trim(message_box,'0');
        n = recv(fd_data,&rlen,sizeof(rlen),MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<sizeof(rlen))
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        len = ntohl(rlen);
        n = recv(fd_data,message_box,len,MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) )|n<len)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<=0", "n=%d,errno=%d,%s", n,errno,strerror(errno));
            goto clean_end;
        }
        else if(n<0)
        {
            FTDEBUG("B-L475E-IOT01A.log", "n<0","");
            perror("recv error in b-l475e-iot01a");
            exit(1);
        }
        message_box[len] = '\0';
        printf("message_box:%s\n",message_box);
        humi = trim(message_box,'0');
        nodesA.lock();
        data->position = position;
        data->temp = temp;
        data->humi = humi;
        data->state = state;
        data->light = "NA";
        data->smoke = "NA";
        nodesA.unlock();
        unsigned int wood_time = data->wood_time;
        unsigned int sec;
        if (wood_time == 0)
        {
            json j;
            j["name"] = data->name;
            j["position"] = data->position;
            j["temp"] = temp;
            j["humi"] = humi;
            j["light"] = "NA";
            j["smoke"] = "NA";
            j["state"] = state;
            data->wood_time = time(NULL);
            puts(j.dump().c_str());
            printf("\n\n\njust for one time should it be\n\n\n");
            save_board_data(j.dump());
        }
        else
        {
            clock_after = time(NULL);
            sec = difftime(clock_after, wood_time);
        }
        int res;
        if (sec >= 60 * 3)
        {
            json j;
            j["name"] = data->name;
            j["position"] = data->position;
            j["temp"] = temp;
            j["humi"] = humi;
            j["light"] = "NA";
            j["smoke"] = "NA";
            j["state"] = state;
            printf("\n\n\nsecsecsec:%d\n\n\n", sec);
            save_board_data(j.dump());
            data->wood_time = clock_after;
        }
        double temp,humi;
        try
        {
            temp = stod(data->temp);
            humi = stod(data->humi);
        }
        catch (exception &e)
        {
            cout << e.what() << endl;
            cout<< "in " << __LINE__ <<"float transfer error" <<endl;
            cout<<data->temp<<endl;
            cout<<data->humi<<endl;
            exit(1);
        }
        if (temp > data->high_temp || humi > data->high_humi)
        {
            json reply;
            reply["type"] = "data";
            reply["boardName"] = data->name;
            reply["temp"] = data->temp;
            reply["humi"] = data->humi;
            reply["light"] = data->light;
            reply["smoke"] = data->smoke;
            reply["position"] = data->position;
            reply["state"] = data->state;
            char time_in[100];
            time_t time_in_1 = time(NULL);
            asctime_r(localtime(&time_in_1), time_in);
            len = strlen(time_in);
            time_in[len - 1] = '\0';
            reply["time"] = time_in;
            string reply_string = reply.dump();
            DEBUG("before send data to B");
            nodesA.lock();
            for (auto m = data->connection.begin(); m != data->connection.end(); m++)
            {
                int fd_tmp = (*m)->fd_warn;
                int len_tmp = reply_string.size();
                len_tmp = htonl(len_tmp);
                n = send(fd_tmp, &len_tmp, sizeof(len_tmp), 0);
                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send len to client failed");
                    exit(1);
                }
                n = send(fd_tmp, reply_string.c_str(), reply_string.size(), 0);
                if (n <0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send data to client failed");
                    exit(1);
                }
            }
            nodesA.unlock();
            DEBUG(reply_string.c_str());
        }
    }
    clean_end:
    nodesA.lock();
    //timing sequence is really important
    for (auto b = data->connection.begin(); b != data->connection.end(); b++)
    {
        DEBUG("in Adata sending");
        json j;
        j["type"] = "cmd";
        j["content"] = "breset";
        string a = j.dump();
        int len = a.size();
        len = htonl(len);
        int m = send((*b)->fd_warn, &len, sizeof(len), 0);
        if (m <= 0 && errno != EPIPE)
        {
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            exit(1);
        }
        m = send((*b)->fd_warn, a.c_str(), a.size(), 0);
        if (m <= 0 && errno != EPIPE)
        {
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            exit(1);
        }
        DEBUG(a.c_str());
    }
    nodesA.erase(data->name);
    nodesA.unlock();
    close(data->fd_data);
    delete data;
    numer.decreaseA();
}
void *stm32F103(void *args)
{
    ANodeInfo *data = (ANodeInfo *)args;
    int fd_data = data->fd_data;
    int len, rlen, n;
    char temp[18], humi[18], light[2], smoke[2];
    temp[17] = humi[17] = '\0';
    light[1] = smoke[1] = '\0';
    char message_box[MESSAE_LENGTH];
    string position;
    n = recv(fd_data, &rlen, sizeof(rlen), MSG_WAITALL);
    FTDEBUG("stm32.log", "para", "high_temp=%f,high_humi=%f", data->high_temp, data->high_humi);
    if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
    {
        FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
        goto clean_end;
    }
    else if(n<0)
    {
        perror("stm32f1 recv err");
        exit(1);
    }
    len = ntohl(rlen);
    n = recv(fd_data, message_box, len, MSG_WAITALL);
    if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
    {
        FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
        goto clean_end;
    }
    else if(n<0)
    {
        perror("stm32f1 recv err");
        exit(1);
    }
    message_box[len] = '\0';
    position = message_box;
    while (1)
    {
        n = recv(fd_data, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
            goto clean_end;
        }
        else if(n<0)
        {
            perror("stm32f1 recv err");
            exit(1);
        }
        len = ntohl(rlen);
        if (len != 36)
        {
            FTDEBUG("stm32.log", "len rectify", "stm32 recv data len!=36:len=%d", len);
            break;
        }
        n = recv(fd_data, temp, 17, MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
            goto clean_end;
        }
        else if(n<0)
        {
            perror("stm32f1 recv err");
            exit(1);
        }
        else if (n < 17)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 17:n=%d", n);
            break;
        }
        n = recv(fd_data, humi, 17, MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
            goto clean_end;
        }
        else if(n<0)
        {
            perror("stm32f1 recv err");
            exit(1);
        }
        else if (n < 17)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 17:n=%d", n);
            break;
        }
        n = recv(fd_data, light, 1, MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
            goto clean_end;
        }
        else if(n<0)
        {
            perror("stm32f1 recv err");
            exit(1);
        }
        else if (n < 1)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 1:n=%d", n);
            break;
        }
        n = recv(fd_data, smoke, 1, MSG_WAITALL);
        if (n == 0|((n<0) && (errno==ECONNRESET|errno==110) ))
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv position len n < 0:m=%d", n);
            goto clean_end;
        }
        else if(n<0)
        {
            perror("stm32f1 recv err");
            exit(1);
        }
        else if (n < 1)
        {
            FTDEBUG("stm32.log", "n<0", "stm32 recv data n < 1:n=%d", n);
            break;
        }
        trim(temp);
        trim(humi);
        FTDEBUG("stm32.log", "data", "recv temp = %s;humi = %s\n", temp, humi);
        // data->writeVal();
        nodesA.lock();
        data->temp = temp;
        data->humi = humi;
        data->light = light;
        data->smoke = smoke;
        data->state = "NA";
        data->position = position;
        nodesA.unlock();
        // data->freeLock();
        unsigned int wood_time = data->wood_time;

        if (wood_time == 0)
        {
            json j;
            j["name"] = data->name;
            j["position"] = data->position;
            j["temp"] = temp;
            j["humi"] = humi;
            j["light"] = light;
            j["smoke"] = smoke;
            j["state"] = "NA";
            wood_time = data->wood_time = time(NULL);
            printf("\n\n\njust for one time should it be\n\n\n");
            save_board_data(j.dump());
        }
        else 
        {
            clock_after = time(NULL);
            unsigned int sec = difftime(clock_after, wood_time);
            if (sec >= 60 * 60 * STORE_HOUR)
            {
                json j;
                j["name"] = data->name;
                j["position"] = data->position;
                j["temp"] = temp;
                j["humi"] = humi;
                j["light"] = light;
                j["smoke"] = smoke;
                j["state"] = "NA";
                printf("\n\n\nsecsecsec:%d\n\n\n", sec);
                save_board_data(j.dump());
                data->wood_time = clock_after;
            }
        }
        double temp;
        double humi;
        double light;
        double smoke;
        int res;
        try
        {
            temp = stod(data->temp);
            humi = stod(data->humi);
            light = stod(data->light);
            smoke = stod(data->smoke);
        }
        catch (exception &e)
        {
            cout << e.what() << endl;
            cout<< "in " << __LINE__ <<"float transfer error" <<endl;
            cout << data->temp <<endl;
            cout << data->humi <<endl;
            cout << data->light <<endl;
            cout << data->smoke <<endl;
            //exit(1);
            continue;
        }
        if (temp > data->high_temp || humi > data->high_humi || light == data->wrong_light || smoke == data->wrong_smoke)
        {
            json reply;
            reply["type"] = "data";
            reply["boardName"] = data->name;
            reply["temp"] = data->temp;
            reply["humi"] = data->humi;
            reply["light"] = data->light;
            reply["smoke"] = data->smoke;
            reply["position"] = data->position;
            reply["state"] = data->state;
            char time_in[100];
            time_t time_in_1 = time(NULL);
            asctime_r(localtime(&time_in_1), time_in);
            len = strlen(time_in);
            time_in[len - 1] = '\0';
            reply["time"] = time_in;
            string reply_string = reply.dump();
            DEBUG("before send data to B");
            nodesA.lock();
            for (auto m = data->connection.begin(); m != data->connection.end(); m++)
            {
                int fd_tmp = (*m)->fd_warn;
                int len_tmp = reply_string.size();
                len_tmp = htonl(len_tmp);
                n = send(fd_tmp, &len_tmp, sizeof(len_tmp), 0);
                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send len to client failed");
                    exit(1);
                }
                n = send(fd_tmp, reply_string.c_str(), reply_string.size(), 0);
                if (n <0 && (errno == EPIPE | errno == ECONNRESET))
                {
                    continue;
                }
                else if (n <= 0)
                {
                    perror("send data to client failed");
                    exit(1);
                }
            }
            nodesA.unlock();
            DEBUG(reply_string.c_str());
        }
    }
clean_end:
    nodesA.lock();
    //timing sequence is really important
    for (auto b = data->connection.begin(); b != data->connection.end(); b++)
    {
        DEBUG("in Adata sending");
        json j;
        j["type"] = "cmd";
        j["content"] = "breset";
        string a = j.dump();
        int len = a.size();
        len = htonl(len);
        int m = send((*b)->fd_warn, &len, sizeof(len), 0);
        if (m <= 0 && errno != EPIPE)
        {
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            exit(1);
        }
        m = send((*b)->fd_warn, a.c_str(), a.size(), 0);
        if (m <= 0 && errno != EPIPE)
        {
            printf("send breset failed in %d:%s\n", __LINE__, strerror(errno));
            exit(1);
        }
        DEBUG(a.c_str());
    }
    nodesA.erase(data->name);
    nodesA.unlock();
    close(data->fd_data);
    delete data;
    numer.decreaseA();
}
void dictSetup(void)
{
    policy_dict.emplace("raspi",raspi);
    policy_dict.emplace("stm32F103",stm32F103);
    policy_dict.emplace("B-L475E-IOT01A",B_L475E_IOT01A);
}
WrapMap<string, BNodeInfo *> sockMapB;
void *Bdata(void *arg)
{
    printf("Bdata:%d\n", syscall(__NR_gettid));
    struct epoll_event ev;
    int epfd = (long)arg;
    int nfds;
    int n;
    int fd;
    int i;
    int len;
    char message_box[MESSAE_LENGTH];
    while (1)
    {
        DEBUG("Bdata working");
        nfds = epoll_wait(epfd, b_data_event, BNUM, -1);
        DEBUG("Bconn epoll shit");
        switch (nfds)
        {
        case -1:
            // debug interrupt
            if (errno == 4)
            {
                errno = 0;
                continue;
            }
            perror("epoll wait failed in Bread");
            printf("%d\n", errno);
            exit_database();
            exit(1);
        case 0:
            continue;
        default:
            for (i = 0; i < nfds; i++)
            {
                fd = b_data_event[i].data.fd;
                if (b_data_event[i].events & EPOLLIN)
                {
                    n = recv(fd, &len, sizeof(len), MSG_WAITALL);
                    if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        continue;
                    }
                    else if (n < 0)
                    {
                        printf("(line %d)n=%d ", __LINE__, n);
                        perror("（recv err)");
                        exit(1);
                    }
                    len = ntohl(len);
                    n = recv(fd, message_box, len, MSG_WAITALL);
                    if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        continue;
                    }
                    else if (n < 0)
                    {
                        printf("(line %d)n=%d ", __LINE__, n);
                        perror("（recv err)");
                        exit(1);
                    }
                    message_box[len] = '\0';
                    json j1 = json::parse(message_box);
                    string board_name = j1["board name"];
                    json j;
                    nodesA.lock();
                    auto p = nodesA.find(board_name);
                    // nodesA.unlock();
                    if (p == nodesA.end())
                    {
                        DEBUG("did not find!");
                        // forget this is vicious
                        nodesA.unlock();
                        continue;
                    }

                    // p->second->readVal();
                    j["name"] = p->second->name;
                    j["position"] = p->second->position;
                    j["temp"] = p->second->temp;
                    j["humi"] = p->second->humi;
                    j["light"] = p->second->light;
                    j["smoke"] = p->second->smoke;
                    j["action"] = p->second->state;
                    string tmp = p->second->name;
                    nodesA.unlock();
                    // p->second->freeLock();
                    string msg = j.dump();
                    len = msg.length();
                    len = htonl(len);
                    n = send(fd, &len, sizeof(len), 0);
                    if ((n<0) && ((errno == ECONNRESET) | (errno == EPIPE)))
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        continue;
                    }
                    else if (n <= 0)
                    {
                        cout << "send data failed " << tmp << " in line " << __LINE__ << endl;
                        exit(1);
                    }
                    n = send(fd, msg.c_str(), msg.length(), 0);
                    if ((n<0) && ((errno == ECONNRESET) | (errno == EPIPE)))
                    {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        continue;
                    }
                    else if (n <= 0)
                    {
                        cout << "send data failed " << tmp << " in line " << __LINE__ << endl;
                        exit(1);
                    }
                }
                else if (b_data_event[i].events & EPOLLHUP)
                {
                    perror("epoll hup in B hup");
                    exit_database();
                    exit(1);
                }
                else if (b_data_event[i].events & EPOLLERR)
                {
                    perror("epoll err in B data");
                    exit_database();
                    exit(1);
                }
            }
        }
    }
}
void *Bconnect(void *arg)
{
    printf("Bconnect:%d\n", syscall(__NR_gettid));
    struct epoll_event ev;
    int epfd = (long)arg;
    int nfds;
    int n;
    char request[REQUEST_LENGTH];
    char message_box[MESSAE_LENGTH];
    BNodeInfo *info;
    int fd;
    int i;
    int len;
    int rlen;
    while (1)
    {
        DEBUG("Bconn working");
        nfds = epoll_wait(epfd, b_connect_event, BNUM, -1);
        DEBUG("Bconn epoll shit");
        switch (nfds)
        {
        case -1:
            // debug interrupt
            if (errno == 4)
            {
                continue;
            }
            perror("epoll wait failed in Bread");
            printf("%d\n", errno);
            exit_database();
            exit(1);
        case 0:
            continue;
        default:
            for (i = 0; i < nfds; i++)
            {
                if (b_connect_event[i].events & EPOLLIN)
                {
                    info = (BNodeInfo *)b_connect_event[i].data.ptr;
                    printf("in Bcon:%d,ptr:%x\n", __LINE__, info);
                    sockMapB.lock();
                    fd = info->fd_other;
                    sockMapB.unlock();
                    n = recv(fd, &len, 4, MSG_WAITALL);
                    if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
                    {
                        char *p = (char *)malloc(25);
                        time_t now;
                        now = time(NULL);
                        printf("(n==%d)%s:socket error:connection with board [%s:%d]:%s\n",n,info->client_name.c_str(), inet_ntop(AF_INET, &info->clientData.sin_addr, p, 20), ntohs(info->clientData.sin_port), strerror(errno));
                        free(p);
                        ERROR_ACTION(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev));
                        sockMapB.lock();
                        string board_name = info->board_name;
                        sockMapB.erase(info->client_name);
                        if (info->shut_fd_warn)
                        {
                            close(info->fd_warn);
                        }
                        // 顺序不能反：运气好见到了反了的话，在刚关闭socket还未删节点时出问题：bad file descriptor
                        delete info;
                        sockMapB.unlock();
                        if (board_name != BBBEFORE)
                        {
                            nodesA.lock();
                            auto c = nodesA.find(info->board_name);
                            // nodesA.unlock();
                            if (c != nodesA.end())
                            {
                                // c->second->connection.lock();
                                c->second->connection.remove(info);
                                // c->second->connection.unlock();
                            }
                            nodesA.unlock();
                        }
                        //errono is thread safe
                        //epoll automatically removes the fined connection from epfd watch queue
                        DEBUG("%d,%s",errno,strerror(errno));
                        int n_ = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        DEBUG("%d,%s",errno,strerror(errno));
                        DEBUG("((%d,%d,%d))",n_,fd,epfd);
                        close(fd);
                        DEBUG("%d,%s",errno,strerror(errno));
                        numer.decreaseB();
                    }
                    else if (n < 0 && errno != ECONNRESET)
                    {
                        perror("recv failed in Bread");
                        exit_database();
                        exit(1);
                    }

                    else
                    {
                        len = ntohl(len);
                        n = recv(fd, request, len, MSG_WAITALL);
                        request[len] = '\0';
                        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
                        {
                            errno = 0;
                            char *p = (char *)malloc(25);
                            time_t now;
                            now = time(NULL);
                            printf("(n==%d\n)%s:socket error:connection with board [%s:%d]:%s\n",n,info->client_name.c_str(), inet_ntop(AF_INET, &info->clientData.sin_addr, p, 20), ntohs(info->clientData.sin_port), strerror(errno));
                            free(p);
                            ERROR_ACTION(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev));
                            sockMapB.lock();
                            string board_name = info->board_name;
                            sockMapB.erase(info->client_name);
                            if (info->shut_fd_warn)
                            {
                                close(info->fd_warn);
                            }
                            delete info;
                            sockMapB.unlock();
                            if (info->board_name != BBBEFORE)
                            {
                                nodesA.lock();
                                auto c = nodesA.find(info->board_name);
                                // nodesA.unlock();
                                if (c != nodesA.end())
                                {
                                    c->second->connection.remove(info);
                                }
                                nodesA.unlock();
                            }
                            close(fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            numer.decreaseB();
                        }
                        else if (n < 0 && errno != ECONNRESET)
                        {
                            perror("recv failed in Bread");
                            exit_database();
                            exit(1);
                        }
                        else
                        {
                            json j = json::parse(request);
                            FDEBUG("bconnect.log", "<request>\n\n%s\n\n<\\request>", request);
                            string type = j["type"];
                            if (type == "connect")
                            {

                                string boardName = j["board name"];
                                info->board_name = boardName;
                                nodesA.lock();
                                auto c = nodesA.find(boardName);

                                // nodesA.unlock();
                                if (c != nodesA.end())
                                {
                                    c->second->connection.push_back(info);
                                }
                                else
                                {
                                    nodesA.unlock();
                                    sockMapB.lock();
                                    sockMapB.erase(info->client_name);
                                    if (info->shut_fd_warn)
                                    {
                                        close(info->fd_warn);
                                    }
                                    delete info;
                                    sockMapB.unlock();
                                    close(fd);
                                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                                    continue;
                                }

                                info->faces = c->second->faces;
                                int vcode = c->second->vcode;
                                string high_temp = to_string(c->second->high_temp);
                                string high_humi = to_string(c->second->high_humi);
                                string wrong_light = to_string(c->second->wrong_light);
                                string wrong_smoke = to_string(c->second->wrong_smoke);
                                string board_type = c->second->type;
                                nodesA.unlock();
                                vcode = htonl(vcode);
                                n = send(fd, &vcode, 4, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }

                                int len = high_temp.length();
                                int rlen = htonl(len);
                                printf("high_temp=%s,len=%d\n", high_temp.c_str(), rlen);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, high_temp.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = high_humi.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                // 这里exception的好处就来了，这么多只需写一个try-catch,而且和主逻辑分离
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, high_humi.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = wrong_light.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, wrong_light.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = wrong_smoke.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, wrong_smoke.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                len = board_type.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(rlen), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, board_type.c_str(), len, 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                            }
                            else if (type == "month data")
                            {
                                string name = j["board name"];
                                time_t time_now = time(NULL);
                                struct tm *now = localtime(&time_now);
                                string month_data = get_month_data(now->tm_mon + 1, name);
                                int len = month_data.size();
                                len = htonl(len);
                                n = send(fd, &len, sizeof(len), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                                n = send(fd, month_data.c_str(), month_data.size(), 0);
                                if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
                                {
                                    errno = 0;
                                }
                                else if (n <= 0)
                                {
                                    printf("send failed in %d:%s\n", __LINE__, strerror(errno));
                                    exit_database();
                                    exit(1);
                                }
                            }
                            else if (type == "delete month data")
                            {
                                string name = j["board name"];
                                time_t time_now = time(NULL);
                                struct tm *now = localtime(&time_now);
                                delete_data(now->tm_mon + 1, name);
                            }
                            else if (type == "face")
                            {
                                // nodesA相关的清理在data做
                                string time = j["time"];
                                string fileName = info->faces + "/" + time + ".jpg";
                                FDEBUG("face-transfer.log", "file name:%s", fileName.c_str());
                                int fd_tmp = open(fileName.c_str(), O_RDONLY);
                                if (fd >= 0)
                                {
                                    len = lseek(fd_tmp, 0, SEEK_END);
                                    rlen = htonl(len);
                                    FDEBUG("face-transfer.log", "len=%d", len);
                                    n = send(fd, &rlen, sizeof(rlen), 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    lseek(fd_tmp, 0, SEEK_SET);
                                    n = sendfile(fd, fd_tmp, 0, len);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    close(fd_tmp);
                                }
                                else
                                {
                                    const char *note = "sys error:picture not found,please contact 649535675@qq.com";
                                    len = strlen(note);
                                    rlen = htonl(len);
                                    n = send(fd, &rlen, sizeof(rlen), 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                    n = send(fd, note, len, 0);
                                    if (n <= 0 && errno != EPIPE && errno != ECONNRESET)
                                    {
                                        FDEBUG("face-transfer.log", "send err:%s", fileName.c_str());
                                        exit(1);
                                    }
                                }
                            }
                            else if (type == "all faces")
                            {
                                vector<string> files;
                                string a;
                                DIR *face_dir = opendir(info->faces.c_str());
                                while (1)
                                {
                                    struct dirent *face_d = readdir(face_dir);
                                    if (face_d == NULL)
                                        break;
                                    string tmp = face_d->d_name;
                                    if (tmp == "." || tmp == "..")
                                    {
                                        continue;
                                    }
                                    tmp = tmp.substr(0, tmp.find_last_of("."));
                                    files.push_back(tmp);
                                }
                                closedir(face_dir);
                                json j;
                                j["data"] = files;
                                j["num"] = files.size();
                                string faces_data = j.dump();
                                len = faces_data.length();
                                rlen = htonl(len);
                                n = send(fd, &rlen, sizeof(len), 0);
                                if (n <= 0 && errno != EPIPE && errno != EPIPE)
                                {
                                    DEBUG("send err");
                                    exit(1);
                                }
                                FDEBUG("face-all.log", "\n\n\n%s\n\n\n", faces_data.c_str());
                                n = send(fd, faces_data.c_str(), len, 0);
                                if (n <= 0 && errno != EPIPE && errno != EPIPE)
                                {
                                    DEBUG("send err");
                                    exit(1);
                                }
                            }
                            else if (type == "delete faces")
                            {
                                FTDEBUG("face-delete.log", "delete", "%s", "in delete all");
                                rmAll(info->faces.c_str());
                            }
                            else if (type == "delete face")
                            {
                                FTDEBUG("face-delete.log", "delete", "%s", "in delete");
                                string time = j["time"];
                                // 注意下面这句由于要给system函数在终端执行，少一个字母也不行
                                string fileName = info->faces + "/" + time + ".jpg";
                                DEBUG("rm file name:%s",fileName.c_str());
                                rm(fileName.c_str());
                            }
                        }
                    }
                }
                else if (b_connect_event[i].events & EPOLLHUP)
                {
                    perror("epoll hup in B connection");
                    exit_database();
                    exit(1);
                }
                else if (b_connect_event[i].events & EPOLLERR)
                {

                    perror("epoll err in B connection");
                    exit_database();
                    exit(1);
                }
            }
        }
    }
}
void *AThread(void *arg)
{
    printf("AThread:%d\n", syscall(__NR_gettid));
    struct sockaddr_in server_data,client_data;
    struct epoll_event ev1, ev2;
    pthread_t adata, agraph;
    int connfdData;
    int epfdData, epfdGraph;
    if (epfdData == -1 | epfdGraph == -1)
    {
        perror("AThread epfd create failed");
        exit(1);
    }
    ANodeInfo * a_info_1;
    listenAdata = socket(AF_INET, SOCK_STREAM, 0);
    if (listenAdata == -1)
    {
        perror("AThread error create TCP socket");
        exit(1);
    }
    memset(&server_data, 0, sizeof(server_data));
    server_data.sin_family = AF_INET;
    server_data.sin_port = htons(portAdata);
    socklen_t client_data_addr_len = sizeof(client_data);
    if (inet_aton(ip_addr, &server_data.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenAdata, (struct sockaddr *)&server_data, sizeof(server_data)) == -1)
    {
        perror("error while trying to bind on portAdata");
        exit(1);
    }
    if (listen(listenAdata, ANUM * 30) == -1)
    {
        printf("%d\n", listenAdata);
        perror("error while trying to listen to Adata");
        exit(1);
    }
    int len_tmp;
    char message_buffer[200], type_buffer[200];
    char reply[200];
    int n;
    while (1)
    {
        DEBUG("ATH working");
        connfdData = accept(listenAdata, (struct sockaddr *)&client_data, &client_data_addr_len);
        setup_tcp_keepalive(connfdData,1);
        if (connfdData < 0)
        {
            perror("error accepting from board(data)");
            continue;
            // exit(1);
        }
        //bug here:if we suspend for 10s on board, data sequence is fucked up!
        n = recv(connfdData, &len_tmp, sizeof(len_tmp), MSG_WAITALL);
        //errno==110:time out
        if (n == 0 | (n<0 &&( errno == ECONNRESET | errno == EAGAIN|errno==110))|n<sizeof(len_tmp))
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            errno = 0;
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        len_tmp = ntohl(len_tmp);
        n = recv(connfdData, message_buffer, len_tmp, MSG_WAITALL);
        if (n < 0 && errno != ECONNRESET && errno != EAGAIN && errno!=110)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", errno, strerror(errno));
            exit(1);
        }
        else if (n == 0 | n<0 | n<len_tmp)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            errno = 0;
            close(connfdData);
            continue;
        }
        message_buffer[n] = 0;
        string s = message_buffer;
        
        // recv type
        FTDEBUG("AThread.log", "message_buffer", "%s", message_buffer);
        n = recv(connfdData, &len_tmp, sizeof(len_tmp), MSG_WAITALL);
        if (n == 0 | (n<0 &&( errno == ECONNRESET | errno == EAGAIN | errno==110))|n<sizeof(len_tmp))
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "(%s)AThread recv==0|errno == ECONNRESET", "errno=%d,%s", message_buffer, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", message_buffer, errno, strerror(errno));
            exit(1);
        }
        len_tmp = ntohl(len_tmp);

        n = recv(connfdData, type_buffer, len_tmp, MSG_WAITALL);
        if (n < 0 && errno != ECONNRESET && errno != EAGAIN &&errno !=110)
        {
            FTDEBUG("AThread.log", "AThread recv<0", "errno=%d,%s", message_buffer, errno, strerror(errno));
            exit(1);
        }
        if (n == 0 | n<0 | n<len_tmp)
        {
            FTDEBUG("AThread.log", "AThread recv==0|errno == ECONNRESET", "errno=%d,%s", errno, strerror(errno));
            FTDEBUG("A.log", "(%s)AThread recv==0|errno == ECONNRESET", "errno=%d,%s", message_buffer, errno, strerror(errno));
            errno = 0;
            DEBUG("let me know");
            close(connfdData);
            continue;
        }

        type_buffer[n] = 0;
        // puts(type_buffer);
        string t = type_buffer;
        int rcode, code = -1;
        code = -1;
        // TODO
        nodesA.lock();
        n = nodesA.count(message_buffer);
        // printf("in 1432:%s,%d\n",message_buffer,n);
        nodesA.unlock();
        if (n != 0)
        {
            FTDEBUG("A.log", "AThread code==-1(stm32)", "(%s)code=%d", message_buffer, code);
            FTDEBUG("AThread.log", "AThread code==-1(stm32)", "(%s)code=%d", message_buffer, code);
            close(connfdData);
            continue;
        }
        a_info_1 = new ANodeInfo;
        a_info_1->vcode = code;
        a_info_1->client_data = client_data;
        a_info_1->fd_data = connfdData;
        a_info_1->wood_time = 0;
        a_info_1->name = message_buffer;
        a_info_1->type = type_buffer;
        a_info_1->work_dir = user_dir + "/" + a_info_1->name;
        string face_conf = user_dir + "/" + s + "/" + FACE_CONF;
        string data_conf = user_dir + "/" + s + "/" + DATA_CONF;
        string face_dir = user_dir + "/" + s + FACES;
        string data_conf_from = user_dir + "/" + DATA_DEFAULT;
        if ((access(a_info_1->work_dir.c_str(), F_OK)))
        {
            mkdir(a_info_1->work_dir.c_str(), 0777);
        }
        a_info_1->data_conf = data_conf;
        if (access(data_conf.c_str(), F_OK))
        {
            // if(symlink(data_conf_from.c_str(), data_conf.c_str())<0)
            // {
            //     perror("data conf symlink err");
            //     exit(1);
            // }
            char cmd[50];
            sprintf(cmd,"cp \"%s\" \"%s\"",data_conf_from.c_str(),data_conf.c_str());
            system(cmd);
        }
        ifstream inf(data_conf);
        inf.exceptions(ios::eofbit | ios::badbit | ios::failbit);
        inf >> a_info_1->high_temp >> a_info_1->high_humi >> a_info_1->wrong_light >> a_info_1->wrong_smoke;
        inf.close();
        a_info_1->face_conf = face_conf;
        a_info_1->faces = user_dir + "/" + s + "/" + FACES;
        a_info_1->photos = user_dir + "/" + s + "/" + PHOTOS;
        if (access(face_conf.c_str(), F_OK))
        {
            symlink(FACE_DEFAULT, face_conf.c_str());
        }
        if (access(a_info_1->faces.c_str(), F_OK))
        {
            mkdir(a_info_1->faces.c_str(), 0777);
        }
        if (access(a_info_1->photos.c_str(), F_OK))
        {
            mkdir(a_info_1->photos.c_str(), 0777);
        }

        nodesA.lock();
        nodesA.emplace(message_buffer, a_info_1);
        nodesA.unlock();
        auto p = policy_dict.find(t);
        if(p==policy_dict.end())
        {
            printf("unknown policy required:%s(line %d)\n",t,__LINE__);
            exit(1);
        }
        ERROR_ACTION(pthread_create(a_info_1->threadVal.get(), NULL, p->second, a_info_1));
        numer.increaseA();
    }
    DEBUG("............................ATH quiting..................................");
}
int epfdConnect, epfdData;

void *BThreadWarn(void *args)
{
    struct sockaddr_in clientWarnIn;
    socklen_t client_addr_warn_len = sizeof(clientWarnIn);
    BNodeInfo *info;
    const char *sync = "warn socket accept over";
    while (1)
    {
        char p_out[25];
        int connfdWarn = accept(listenBwarn, (struct sockaddr *)&clientWarnIn, &client_addr_warn_len);
        inet_ntop(AF_INET, &clientWarnIn.sin_addr, p_out, 20);
        FTDEBUG("BThreadWarn.log", "connect", "[%s:%d]",p_out, ntohs(clientWarnIn.sin_port));
        int n, len, rlen;
        n = recv(connfdWarn, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThreadWarn.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdWarn);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThreadWarn.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn recv rlen failed");
            exit(1);
        }
        len = ntohl(rlen);
        char message_box[200];
        n = recv(connfdWarn, message_box, len, MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThreadWarn.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdWarn);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThreadWarn.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn recv rlen failed");
            exit(1);
        }
        message_box[len] = '\0';
        string board_name = message_box;
        sockMapB.lock();
        auto info = sockMapB.find(board_name);
        if (info != sockMapB.end())
        {
            
        }
        else
        {
            FTDEBUG("BThreadWarn.log", "client name not found", "%s", board_name.c_str());
            close(connfdWarn);
            sockMapB.unlock();
            continue;
        }
        sockMapB.unlock();
        
        info->second->fd_warn = connfdWarn;
        info->second->shut_fd_warn = true;
        
        len = strlen(sync);
        rlen = htonl(len);
        n = send(connfdWarn, &rlen, sizeof(rlen), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThreadWarn.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThreadWarn.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn send sync message failed");
            exit(1);
        }
        n = send(connfdWarn, sync, len, 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThreadWarn.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThreadWarn.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn send sync message failed");
            exit(1);
        }
    }
}
void *BThreadData(void *args)
{
    struct sockaddr_in clientDataIn;
    socklen_t client_addr_data_len = sizeof(clientDataIn);
    BNodeInfo *info;
    const char *sync = "data socket accept over";
    struct epoll_event ev1;
    while (1)
    {
        int connfdData = accept(listenBdata, (struct sockaddr *)&clientDataIn, &client_addr_data_len);
        int n, len, rlen;
        n = recv(connfdData, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThreadData.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThreadData.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn recv rlen failed");
            exit(1);
        }
        len = ntohl(rlen);
        char message_box[200];
        n = recv(connfdData, message_box, len, MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThreadData.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThreadData.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn recv rlen failed");
            exit(1);
        }
        message_box[len] = '\0';
        string board_name = message_box;
        sockMapB.lock();
        auto info = sockMapB.find(board_name);
        if (info != sockMapB.end())
        {
    
        }
        else
        {
            FTDEBUG("BThreadData.log", "client name not found", "%s", board_name.c_str());
            close(connfdData);
            continue;
        }
        info->second->fd_data = connfdData;

        sockMapB.unlock();
        len = strlen(sync);
        rlen = htonl(len);
        n = send(connfdData, &rlen, sizeof(rlen), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThreadData.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThreadData.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn send sync message failed");
            exit(1);
        }
        n = send(connfdData, sync, len, 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThreadData.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdData);
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThreadData.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("connfdWarn send sync message failed");
            exit(1);
        }
        ev1.events = EPOLLIN | EPOLLERR;
        ev1.data.fd = connfdData;
        epoll_ctl(epfdData, EPOLL_CTL_ADD, connfdData, &ev1);
    }
}
void *BThread(void *arg)
{
    #define LOGIN_SUCCESS 0
    #define LOGIN_FAILURE 1
    printf("BThread:%d", syscall(__NR_gettid));
    BNodeInfo *info_1, *info_2;
    pthread_t bconnect, bdata;
    pthread_t pBwarn,pBdata;
    int fdpro;
    epfdConnect = epoll_create(BNUM);
    epfdData = epoll_create(BNUM);
    ERROR_ACTION(epfdConnect);
    ERROR_ACTION(epfdData);
    struct sockaddr_in serverData, serverWarn, serverOther, clientData, clientWarn, clientOther;
    struct epoll_event ev1, ev2;
    int connfdData, connfdWarn, connfdOther;
    ERROR_ACTION(listenBdata = socket(AF_INET, SOCK_STREAM, 0));
    ERROR_ACTION(listenBwarn = socket(AF_INET, SOCK_STREAM, 0));
    ERROR_ACTION(listenBother = socket(AF_INET, SOCK_STREAM, 0));
    pthread_create(&bconnect, NULL, Bconnect, (void *)epfdConnect);
    pthread_create(&bdata, NULL, Bdata, (void *)epfdData);
    memset(&serverData, 0, sizeof(serverData));
    serverData.sin_family = AF_INET;
    serverData.sin_port = htons(portBdata);
    memset(&serverWarn, 0, sizeof(serverWarn));
    serverWarn.sin_family = AF_INET;
    serverWarn.sin_port = htons(portBwarn);
    memset(&serverOther, 0, sizeof(serverOther));
    serverOther.sin_family = AF_INET;
    serverOther.sin_port = htons(portBother);
    socklen_t client_addr_data_len = sizeof(clientData);
    socklen_t client_addr_warn_len = sizeof(clientWarn);
    socklen_t client_addr_other_len = sizeof(clientOther);
    int len, rlen, n;
    if (inet_aton(ip_addr, &serverData.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBdata, (struct sockaddr *)&serverData, sizeof(serverData)) == -1)
    {
        perror("error while trying to bind on portBdata\n");
        exit(1);
    }
    if (listen(listenBdata, BNUM) == -1)
    {
        printf("%d\n", listenBdata);
        perror("error while trying to listen to Bdata\n");
        exit(1);
    }
    if (inet_aton(ip_addr, &serverWarn.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBwarn, (struct sockaddr *)&serverWarn, sizeof(serverWarn)) == -1)
    {
        perror("error while trying to bind on portA\n");
        exit(1);
    }
    if (listen(listenBwarn, 1) == -1)
    {
        printf("%d\n", listenBwarn);
        perror("error while trying to listen to B\n");
        exit(1);
    }
    if (inet_aton(ip_addr, &serverOther.sin_addr) == 0)
    {
        perror("address transferring error");
        exit(1);
    }
    if (bind(listenBother, (struct sockaddr *)&serverOther, sizeof(serverOther)) == -1)
    {
        perror("error while trying to bind on portA\n");
        exit(1);
    }
    if (listen(listenBother, 1) == -1)
    {
        printf("%d\n", listenBother);
        perror("error while trying to listen to B\n");
        exit(1);
    }
    pthread_create(&pBwarn, NULL, BThreadWarn, NULL);
    pthread_create(&pBdata, NULL, BThreadData, NULL);
    while (1)
    {
        DEBUG("BTH working");
        connfdOther = accept(listenBother, (sockaddr *)&clientOther, &client_addr_other_len);
        if (connfdOther < 0)
        {
            FTDEBUG("BThread.log", "accept3<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            exit(1);
        }
        char message_box[200];
        setup_tcp_keepalive(connfdOther,60);
        n = recv(connfdOther, &rlen, sizeof(rlen), MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThread.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdOther);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThread.log", "n<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            exit(1);
        }
        len = ntohl(rlen);
        FTDEBUG("BThread.log", "len recved", "len=%d", len);
        n = recv(connfdOther, message_box, len, MSG_WAITALL);
        if (n == 0 | ((n < 0) && (errno == ECONNRESET)))
        {
            FTDEBUG("BThread.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdOther);
            continue;
        }
        else if (n < 0)
        {
            FTDEBUG("BThread.log", "n<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            exit(1);
        }
        message_box[len] = '\0';
        FTDEBUG("BThread.log", "message", "message=%s", message_box);
        string client_name = message_box;
        sockMapB.lock();
        n = sockMapB.count(client_name);
        int sendStatus = htonl(LOGIN_SUCCESS);
        if(n!=0)
        {
            sendStatus = htonl(LOGIN_FAILURE);
            send(connfdOther,&sendStatus,4,0);
            close(connfdOther);
            sockMapB.unlock();
            continue;
        }
        n = send(connfdOther,&sendStatus,4,0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThread.log", "send<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdOther);
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThread.log", "send<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            exit(1);
        }
        sockMapB.unlock();
        json Bdata;
        nodesA.lock();
        int num = nodesA.size();
        Bdata["num"] = num;
        json m;
        json j = json::array();
        int i = 0;
        for (auto a = nodesA.begin(); a != nodesA.end(); a++)
        {
            m["name"] = a->second->name;
            cout << "name=" + a->second->name << endl;
            m["position"] = a->second->position;
            cout << "pos=" + a->second->position << endl;
            m["type"] = a->second->type;
            cout << "type=" + a->second->type << endl;
            j[i] = m; // here core dump,looking for reason
            i++;
        }
        nodesA.unlock();
        Bdata["nodes"] = j;
        string data_string = Bdata.dump();
        len = data_string.length(), rlen;
        len = htonl(len);
        n = send(connfdOther, &len, sizeof(len), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThread.log", "send<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdOther);
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThread.log", "send<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            exit(1);
        }
        n = send(connfdOther, data_string.c_str(), data_string.size(), 0);
        if (n < 0 && (errno == EPIPE | errno == ECONNRESET))
        {
            FTDEBUG("BThread.log", "send<0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            close(connfdOther);
            // delete info_2;
            continue;
        }
        else if (n <= 0)
        {
            FTDEBUG("BThread.log", "send<=0", "n=%d,errno=%d,%s", n, errno, strerror(errno));
            perror("faile in Bconn:");
            printf("errno=%d\n", errno);
            exit_database();
            exit(1);
        }
        info_1 = new BNodeInfo;
        ev1.events = EPOLLIN | EPOLLERR;
        ev1.data.ptr = info_1;
        info_1->fd_other = connfdOther;
        info_1->client_name = client_name;
        info_1->board_name = BBBEFORE;
        //no client name confliction detection results in error:fd_x = 0 on AdataThreads, socket operation on non-socket!
        sockMapB.lock();
        sockMapB.emplace(client_name, info_1);
        sockMapB.unlock();
        epoll_ctl(epfdConnect, EPOLL_CTL_ADD, connfdOther, &ev1);
        numer.increaseB();
    }
}
pthread_mutex_t freelock;
int ack;
void sigPipeHandler(int signo)
{
    signum = signo;
    printf("[recv SIGPIPE!]\n");
}
int main(void)
{
    pthread_mutex_init(&BTestlock, NULL);
    printf("main:%d\n", syscall(__NR_gettid));
    for (int i = 0; i < ANUM; i++)
    {
        FTDEBUG("vode.log", "intilialize", "vode=%d", i);
        freeV.push(i);
    }
    struct passwd *cur_user = getpwuid(getuid());
    user_dir = cur_user->pw_dir;
    struct sigaction sigpipe, sigalarm;
    database_init();
    atexit(clean_sock);
    dictSetup();
    pthread_t pA, pB;
    pthread_create(&pA, NULL, AThread, NULL);
    pthread_create(&pB, NULL, BThread, NULL);
    pthread_join(pA, NULL);
    pthread_join(pB, NULL);
    while (1)
        ;
}
