#pragma GCC diagnostic error "-std=c++11"
#include "Agar.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <vector>
#include <iostream>

using std::cout;
using std::endl;
using std::vector;

#define PORT 9877
#define BACKLOG 100

#define BLOCK_SIZE 1500

struct Session
{
    int fd;
    vector<uint8_t> writeBuffer;
    vector<uint8_t> readBuffer;
    void * userData;
};

struct Client
{
    bool isReady;
    float dirX;
    float dirY;
};

vector<Session> sessions;
vector<Node> playerNodes;
vector<Node> nonPlayerNodes;

void onMessage(void * data,int size,Session * session)
{
    Client * client = (Client *)(session->userData);
    uint8_t messageType = ((uint8_t*)data)[0];
    
    cout<<"onMessage,size:"<<size<<" type:"<<((unsigned int)messageType)<<endl;
    
    switch(messageType) {
        case 0:
        {
            break;
        }
        case 1:
        {
            
            break;
        }
        default:
            break;
    }
}

void onClose(Session * session)
{
    cout<<"onClose"<<session->fd<<endl;
}

void onConnect(Session * session)
{
    cout<<"onConnect:"<<session->fd<<endl;
}

void update(float delta)
{
    
}

int main(int argc, const char * argv[]) {
    cout<<"agar server"<<endl;

    signal(SIGPIPE, SIG_IGN);

    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) {
        perror("socket");
        return -1;
    }
    
    int option;
    if((option = fcntl(listenFd, F_GETFL, 0)) != -1) {
        fcntl(listenFd, F_SETFL, option | O_NONBLOCK);
    }
    else {
        perror("fcntl");
    }
    
    sockaddr_in localAddress;
    memset(&localAddress, 0, sizeof(sockaddr_in));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listenFd,
            (const struct sockaddr *)(&localAddress),
            sizeof(sockaddr_in)) != 0) {
        perror("bind");
        return -1;
    }
    
    
    if(listen(listenFd,BACKLOG) != 0) {
        perror("listen");
        return -1;
    }
    
    sockaddr_in newClientAddress;
    memset(&newClientAddress, 0, sizeof(sockaddr_in));
    socklen_t clientAddressLength;
    Session newSession;
    newSession.userData = NULL;
    
    fd_set readSet;
    fd_set writeSet;
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    int maxFD = 0;
    while(true) {
        maxFD = listenFd;
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_SET(listenFd,&readSet);
        for(auto session = sessions.begin();session != sessions.end();++session) {
            if(session->fd > maxFD) {
                maxFD = session->fd;
            }
            FD_SET(session->fd,&readSet);
            if(!session->writeBuffer.empty()) {
                FD_SET(session->fd,&writeSet);
            }
        }
        
        if(select(maxFD + 1, &readSet, &writeSet, NULL, &timeout) < 0) {
            perror("select");
        }
        if(FD_ISSET(listenFd,&readSet)) {
            int connFd;
            while((connFd = accept(listenFd, (sockaddr *)(&newClientAddress), &clientAddressLength)) >= 0) {
                if((option = fcntl(connFd, F_GETFL, 0)) != -1) {
                    fcntl(connFd, F_SETFL, option | O_NONBLOCK);
                }
                else {
                    perror("fcntl");
                }
                newSession.fd = connFd;
                sessions.push_back(newSession);
                onConnect(&sessions[sessions.size() - 1]);
            }
        }
        
        
        for(auto session = sessions.begin();session != sessions.end();++ session) {
            if(FD_ISSET(session->fd,&readSet)) {
                while(true) {
                    size_t originSize = session->readBuffer.size();
                    session->readBuffer.resize(originSize + BLOCK_SIZE);
                    ssize_t bytesRead = read(session->fd, &session->readBuffer[originSize], BLOCK_SIZE);
                    if(bytesRead < 0) {
                        if(errno == EWOULDBLOCK || errno == EAGAIN) {
                            //缓冲区中没有数据了
                            session->readBuffer.resize(originSize);
                        }
                        else {
                            //读取发生错误
                            perror("read");
                            close(session->fd);
                            session->fd = -1;
                        }
                        break;
                    }
                    else if(bytesRead == 0) {
                        //客户端发送了FIN
                        session->readBuffer.resize(originSize);
                        close(session->fd);
                        session->fd = -1;
                        break;
                    }
                    else {
                        //读到了数据
                        session->readBuffer.resize(originSize + bytesRead);
                        cout<<"received:";
                        for(size_t i=originSize;i<session->readBuffer.size();++i) {
                            cout<<((unsigned int)session->readBuffer[i])<<' ';
                        }
                        cout<<endl;
                    }
                }
            }
            if(session->fd >= 0) {
                if(FD_ISSET(session->fd,&writeSet)) {
                    while (!session->writeBuffer.empty()) {
                        ssize_t byteWritten = write(session->fd, &session->writeBuffer[0], session->writeBuffer.size());
                        if(byteWritten <= 0) {
                            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                                perror("write");
                                close(session->fd);
                                session->fd = -1;
                            }
                            break;
                        }
                        else {
                            session->writeBuffer.erase(session->writeBuffer.begin(),session->writeBuffer.begin() + byteWritten);
                        }
                    }
                }
            }
            
            if(!session->readBuffer.empty()) {
                cout<<"check:"<<((int)session->readBuffer[0])<<' '<<session->readBuffer.size()<<endl;
            }
            while(!session->readBuffer.empty() && session->readBuffer[0] <= (session->readBuffer.size() - 1)) {
                //读缓冲区中至少有一条完整的消息
                onMessage(&session->readBuffer[1],session->readBuffer[0],&(*session));
                session->readBuffer.erase(session->readBuffer.begin(),session->readBuffer.begin() + session->readBuffer[0] + 1);
            }
        }
        
        for(auto session = sessions.begin();session != sessions.end();) {
            if(session->fd < 0) {
                onClose(&(*session));
                session = sessions.erase(session);
            }
            else {
                ++ session;
            }
        }
    }
    
}
