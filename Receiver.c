#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

#define SERVER_PORT 5060
#define BUFFER_SIZE 16
#define BUNDLE 32768

int main() {
    
    // Open the listening socket
    int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }

    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_port = htons(SERVER_PORT);  // network order (makes byte order consistent)

    // Bind the socket to the port with any IP at this port
    int bindResult = bind(listeningSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if (bindResult == -1) {
        printf("Bind failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }else printf("executed Bind() successfully\n");

    // Make the socket listen.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections = 3
    int listenResult = listen(listeningSocket, 3);
    if (listenResult == -1) {
        printf("listen() failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }else printf("Waiting for incoming TCP-connections...\n");

    // Accept and incoming connection
    struct sockaddr_in clientAddress;  
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t len_clientAddress = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *) &clientAddress, &len_clientAddress);
    if (clientSocket == -1) {
        printf("listen failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }else printf("A new client connection accepted\n");

    int fileSize;
    int dummy = 0;
    recv(clientSocket, &fileSize, sizeof(int), 0);
    send(clientSocket, &dummy, sizeof(int), 0);
    
    

    long timeOfPartA[1000];
    bzero(timeOfPartA, 1000);
    long timeOfPartB[1000];
    bzero(timeOfPartB, 1000);
    long counter = 0;

    int running = 1;
    while(running)
    {
        
        char cc_algo[BUFFER_SIZE];
        printf("Changing to cubic...\n");
        strcpy(cc_algo, "cubic");
        socklen_t len = strlen(cc_algo);
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, cc_algo, len) == -1){
            perror("setsockopt");
            return -1;
        }

        char buffer[fileSize/2];
        int totalbytes = 0;

        printf("Waiting for part A...\n");

        struct timeval current_time;
        gettimeofday(&current_time,NULL);
        long before_partA_sec = current_time.tv_sec;
        long before_partA_mic = current_time.tv_usec;
        long total_time_before_partA = before_partA_sec*1000000 + before_partA_mic;

        while (totalbytes < (fileSize/2))
        {
            int bytesgot = recv(clientSocket, buffer+totalbytes, sizeof(char), 0);
            totalbytes += bytesgot;

            if (bytesgot == 0){
                printf("Connection with sender closed, exiting...\n");
                running = 0;
                break;
            }
        }

        if (running == 0) break;

        gettimeofday(&current_time,NULL);
        long after_partA_sec = current_time.tv_sec;
        long after_partA_mic = current_time.tv_usec;
        long total_time_after_partA = after_partA_sec*1000000 + after_partA_mic;
        long total_time_partA = total_time_after_partA - total_time_before_partA;
        timeOfPartA[counter]=total_time_partA;

        printf("Got part A\n");

        printf("Sending authntication check\n");

        int xor = 2421 ^ 7494;
        send(clientSocket, &xor, sizeof(int), 0);

        printf("Authontication sent\n");

        printf("Changeing to reno..\n");

        strcpy(cc_algo, "reno");
        len = strlen(cc_algo);
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, cc_algo, len) == -1){
            perror("setsockopt");
            return -1;
        }

        printf("Waiting for part B...\n");

        totalbytes = 0;

        gettimeofday(&current_time,NULL);
        long before_partB_sec = current_time.tv_sec;
        long before_partB_mic = current_time.tv_usec;
        long total_time_before_partB = before_partB_sec*1000000 + before_partB_mic;

        
        while (totalbytes < (fileSize/2))
        {
            int bytesgot = recv(clientSocket, buffer+totalbytes, sizeof(char), 0);
            totalbytes += bytesgot;

            if (bytesgot == 0){
                printf("Connection with sender closed, exiting...\n");
                running = 0;
                break;
            }
        }

        if (running == 0) break;
        
        gettimeofday(&current_time,NULL);
        long after_partB_sec = current_time.tv_sec;
        long after_partB_mic = current_time.tv_usec;
        long total_time_after_partB = after_partB_sec*1000000 + after_partB_mic;
        
        long total_time_partB = total_time_after_partB - total_time_before_partB;
        timeOfPartB[counter]=total_time_partB;
        counter++;

        printf("Got part B\n");
        recv(clientSocket, &dummy, sizeof(int), 0);
    }

    printf("Exit\n");
    close(clientSocket);
    close(listeningSocket);

    long total_time_of_A = 0;
    long total_time_of_B = 0;
    
    for(int i=0; i<counter; i++){
        printf("Run time of part A, number %d : (%ld second,%ld microseconed)\n",i+1 ,(timeOfPartA[i]/1000000),(timeOfPartA[i]%1000000));
        printf("Run time of part B, number %d : (%ld second,%ld microseconed)\n",i+1,(timeOfPartB[i]/1000000),(timeOfPartB[i]%1000000));

        total_time_of_A += timeOfPartA[i];
        total_time_of_B += timeOfPartB[i];
    }
    
    long average_time_of_A = (total_time_of_A/counter);
    long average_time_of_B = (total_time_of_B/counter);

    printf("The average time of part A is: (%ld second,%ld microsecond)\n",(average_time_of_A/1000000),(average_time_of_A%1000000));
    printf("The average time of part B is: (%ld second,%ld microsecond)\n",(average_time_of_B/1000000),(average_time_of_B%1000000));

    return 0;
}