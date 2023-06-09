#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "proto.h"
#include "string.h"

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
void enleve_space (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void affichage() {
    printf("\r%s", "> ");
    fflush(stdout);
}


void msg_recu() {
    char receiveMessage[LENGTH_SEND] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            affichage();
        } else if (receive == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void msg_envoi() {
    char message[LENGTH_MSG] = {};
    while (1) {
        affichage();
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            enleve_space(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                affichage();
            } else {
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Entrez votre nom : ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        enleve_space(nickname, LENGTH_NAME);
    }
    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nLe nom doit faire entre 2 et 30 caracatères\n");
        exit(EXIT_FAILURE);
    }

    // Creation du socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Erreur de création du sockets.");
        exit(EXIT_FAILURE);
    }

    // information du socket 
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    // Connexion au Serveur
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Erreur de connexion au serveur\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connexion au serveur : %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("Vous êtes: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, LENGTH_NAME, 0);

    pthread_t msg_envoi_thread;
    if (pthread_create(&msg_envoi_thread, NULL, (void *) msg_envoi, NULL) != 0) {
        printf ("Erreur de création thread\n");
        exit(EXIT_FAILURE);
    }

    pthread_t msg_recu_thread;
    if (pthread_create(&msg_recu_thread, NULL, (void *) msg_recu, NULL) != 0) {
        printf ("Erreur de création thread\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\n\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
