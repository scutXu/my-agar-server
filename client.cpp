#pragma GCC diagnostic error "-std=c++11"
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <vector>

using std::vector;
using std::cout;
using std::endl;
using std::cin;

#define SERVER_PORT 9877

void onConnect()
{
    cout<<"onConnect"<<endl;
}

int main(int argc, const char * argv[]) {
    cout<<"agar client"<<endl;
    int connFD = socket(AF_INET, SOCK_STREAM, 0);
    if(connFD < 0) {
        perror("socket");
        return -1;
    }
    
    
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(sockaddr_in));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    cout<<"start connecting to server..."<<endl;
    if((connect(connFD,(const sockaddr *)(&servAddr),sizeof(servAddr))) != 0) {
        close(connFD);
        connFD = -1;
        perror("connect");
        return -1;
    }
    else {
        onConnect();
    }
        
    /*int option;
    if((option = fcntl(connFD, F_GETFL, 0)) != -1) {
        fcntl(connFD, F_SETFL, option | O_NONBLOCK);
    }
    else {
        perror("fcntl");
        return -1;
    }*/
    
    vector<uint8_t> message(10);
    message[0] = 9;
    message[1] = 0;
    ssize_t numByteWritten = write(connFD, &message[0], 10);
    cout<<numByteWritten<<endl;
    
    message.resize(15);
    message[0] = 14;
    message[1] = 1;
    if(write(connFD, &message[0], 15) < 0) {
        perror("write");
    }

    
    close(connFD); 
    return 0;
}
