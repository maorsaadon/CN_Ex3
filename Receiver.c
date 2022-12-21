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


int main() {

    // (1)
    // Open the listening socket
    int listeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listeningSocket == -1) {
        printf("Could not create listening socket : %d", errno);
        return 1;
    }

    // for reusing of port
    int enableReuse = 1;
    int ret = setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int));
    if (ret < 0) {
        printf("setsockopt() failed with error code : %d", errno);
        return 1;
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    struct sockaddr_in serverAddress;

    /*
    fills the first 'sizeof(serverAddress)' bytes of the memory 
    area pointed to by &serverAddress with the constant 0.
    */
    memset(&serverAddress, 0, sizeof(serverAddress));

    //address family, AF_INET(while using TCP) undighned 
    serverAddress.sin_family = AF_INET;
    //any IP at this port (Address to accept any incoming messages)
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    //change the port number from little endian => network endian(big endian): 
    serverAddress.sin_port = htons(SERVER_PORT);  // network order (makes byte order consistent)

    // (2)
    // Bind the socket to the port with any IP at this port
    int bindResult = bind(listeningSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    //checking
    if (bindResult == -1) {
        printf("Bind failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    } else printf("executed Bind() successfully\n");

    // (3)
    // Make the socket listen.
    // 500 is a Maximum size of queue connection requests
    // number of concurrent connections = 3
    int listenResult = listen(listeningSocket, 3);
    //checking
    if (listenResult == -1) {
        printf("listen() failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    } else printf("Waiting for incoming TCP-connections...\n");

    // Accept and incoming connection
    struct sockaddr_in clientAddress;

    // (4)
    /*
    fills the first 'sizeof(clientAddress)' bytes of the memory 
    area pointed to by &clientAddress with the constant 0.
    */
    memset(&clientAddress, 0, sizeof(clientAddress));

    //saving the size of clientAddress in socklen_t variable
    socklen_t len_clientAddress = sizeof(clientAddress);

    // (5)
    //accept a connection on a socket
    int clientSocket = accept(listeningSocket, (struct sockaddr *) &clientAddress, &len_clientAddress);
    //checking
    if (clientSocket == -1) {
        printf("listen failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    } else printf("A new client connection accepted\n");


    // (7)
    int fileSize; // the file size that we receiv from the sender
    int signal = 0; // agreed sign

    // receive the file size from the sender
    recv(clientSocket, &fileSize, sizeof(int), 0);
    //send the agreed sign to the sender
    send(clientSocket, &signal, sizeof(int), 0);

    long timeOfPartA[1000];//long array to save the run time of sending partA
    bzero(timeOfPartA, 1000);//make a zero array
    long timeOfPartB[1000];//long array to save the run time of sending partB
    bzero(timeOfPartB, 1000);//make a zero array
    long counter = 0;//present the number of sending the whole file

    int running = 1;//stop condition
    while (running) {

        char cc_algo[BUFFER_SIZE];//char array for changing the algorithem
        printf("Changing to cubic...\n");
        strcpy(cc_algo, "cubic");//copy the string "cubic" into cc_algo
        socklen_t len = strlen(cc_algo);//saving the size of the str in the cc_algo in socklen_t variable
        //checking & defult the cubic algorithem
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, cc_algo, len) == -1) {
            perror("setsockopt");
            return -1;
        }

        char buffer[fileSize / 2];//char array for receiving the half of the file
        int totalbytes = 0;//present the bytes that have been received

        printf("Waiting for part A...\n");

        struct timeval current_time;//struct for saving current time
        gettimeofday(&current_time, NULL);//saving the current time
        long before_partA_sec = current_time.tv_sec;//time in second
        long before_partA_mic = current_time.tv_usec;//time in microsecond
        long total_time_before_partA = before_partA_sec * 1000000 + before_partA_mic;//total time befor


        while (totalbytes < (fileSize / 2)) {
            //receive the first part of the file
            int bytesgot = recv(clientSocket, buffer + totalbytes, sizeof(char), 0);
            totalbytes += bytesgot;
            //checking
            if (bytesgot == 0) {
                printf("Connection with sender closed, exiting...\n");
                running = 0;
                break;
            }
        }

        if (running == 0) break; // quit the program if somthing wrong

        gettimeofday(&current_time, NULL);//saving the current time
        long after_partA_sec = current_time.tv_sec;//time in second
        long after_partA_mic = current_time.tv_usec;//time in microsecond
        long total_time_after_partA = after_partA_sec * 1000000 + after_partA_mic;//total time after
        long total_time_partA = total_time_after_partA - total_time_before_partA;//tatal time that took part A
        timeOfPartA[counter] = total_time_partA;//saving the time in the array

        printf("Got part A\n");

        printf("Sending authntication check\n");

        int receiver_xor = 2421 ^ 7494; //the receiver xor
        send(clientSocket, &receiver_xor, sizeof(int),
             0);//the receiver send his xor to the sender for the authentication

        printf("Authontication sent\n");

        printf("Changeing to reno..\n");

        strcpy(cc_algo, "reno");//copy the string "reno" into cc_algo
        len = strlen(cc_algo);//saving the size of the str in the cc_algo in socklen_t variable
        //checking & defult the cubic algorithem
        if (setsockopt(listeningSocket, IPPROTO_TCP, TCP_CONGESTION, cc_algo, len) == -1) {
            perror("setsockopt");
            return -1;
        }

        printf("Waiting for part B...\n");

        totalbytes = 0;

        gettimeofday(&current_time, NULL);//saving the current time
        long before_partB_sec = current_time.tv_sec;//time in second
        long before_partB_mic = current_time.tv_usec;//time in microsecond
        long total_time_before_partB = before_partB_sec * 1000000 + before_partB_mic;//total time befor


        while (totalbytes < (fileSize / 2)) {
            //receive the second part of the file
            int bytesgot = recv(clientSocket, buffer + totalbytes, sizeof(char), 0);
            totalbytes += bytesgot;
            //checking
            if (bytesgot == 0) {
                printf("Connection with sender closed, exiting...\n");
                running = 0;
                break;
            }
        }

        if (running == 0) break;//quit the program if somthing wrong

        gettimeofday(&current_time, NULL);//saving the current time
        long after_partB_sec = current_time.tv_sec;//time in second
        long after_partB_mic = current_time.tv_usec;//time in microsecond
        long total_time_after_partB = after_partB_sec * 1000000 + after_partB_mic;//total time after
        long total_time_partB = total_time_after_partB - total_time_before_partB;//tatal time that took part B
        timeOfPartB[counter] = total_time_partB;//saving the time in the array
        counter++;//add 1 for the counter after we finished one receinging of the whole file

        printf("Got part B\n");
        recv(clientSocket, &signal, sizeof(int), 0);//receiving from the sender the agreed sign
    }

    printf("Exit\n");
    close(clientSocket);//close the clientSocket
    close(listeningSocket);//listeningSocket

    long total_time_of_A = 0;//present the tatal time for receiving part A(for all times)
    long total_time_of_B = 0;//present the tatal time for receiving part A(for all times)

    for (int i = 0; i < counter; i++) {
        printf("Run time of part A, number %d : (%ld second,%ld microseconed)\n", i + 1, (timeOfPartA[i] / 1000000),
               (timeOfPartA[i] % 1000000));
        printf("Run time of part B, number %d : (%ld second,%ld microseconed)\n", i + 1, (timeOfPartB[i] / 1000000),
               (timeOfPartB[i] % 1000000));

        //sum the time of each receiving (part A/B)
        total_time_of_A += timeOfPartA[i];
        total_time_of_B += timeOfPartB[i];
    }

    //calculate the average
    long average_time_of_A = (total_time_of_A / counter);
    long average_time_of_B = (total_time_of_B / counter);

    printf("The average time of part A is: (%ld second,%ld microsecond)\n", (average_time_of_A / 1000000),
           (average_time_of_A % 1000000));
    printf("The average time of part B is: (%ld second,%ld microsecond)\n", (average_time_of_B / 1000000),
           (average_time_of_B % 1000000));

    return 0;
}