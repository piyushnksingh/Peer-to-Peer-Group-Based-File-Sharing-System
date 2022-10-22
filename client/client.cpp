#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h> 
#include <bits/stdc++.h>
#include <string.h>
#include <string>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <sys/stat.h>
#include "sha1.h"
#include <fcntl.h>
#include <fstream>
#include <unistd.h> // read(), write(), close()

#define MAX 524288 // 512KB
#define PORT 8000
#define SA struct sockaddr
using namespace std;

string c_ip = "";
string c_port = "";

pthread_t tid[60];
int thread_number = 0;

vector<string> command;

struct argument
{
    sockaddr_in cli_in_argument;
    int sock_fd_in_argument;

} * args;

struct multi_peers
{
    string port;
    string ip;
    string src_path;
    string sha;
    string dst_path;
    string filename;
} * multi;

vector<string> splitstring(string str, char delim)
{
    vector<string> vec;
    int i = 0;
    string s = "";
    while (i < str.length())
    {
        if (str[i] == delim)
        {
            vec.push_back(s);
            s = "";
            i++;
        }
        else
        {
            s = s + str[i];
            i++;
        }
    }
    vec.push_back(s);
    return vec;
}

long GetFileSize(string file_path)
{
    struct stat stat_buf;
    int rc = stat(file_path.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

void receive_chunks(int sockfd, string dst_in, string file_name)
{
    // string to_open = dst_in + "/" + file_name;
    int fd6 = open(dst_in.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd6 < 0)
    {
        cout << "Error" << endl;
        return;
    }
    char buffer7[MAX];
    while (1)
    {
        int n = read(sockfd, buffer7, MAX);
        // cout << "Received BYte" << n << endl;
        if (n <= 0)
        {
            break;
            return;
        }
        else
        {
            write(fd6, buffer7, n);
            bzero(buffer7, MAX);
        }
    }

    // cout << "-----------------------------leaving-----------------------------" << endl;
}

void func_from_multipeer(int sockfd, string src_in, string dst_in, string filename_in, string sha_in)
{
    // cout << "inside func_from_multipeer" << endl;

    char buff[MAX];
    int n;

    bzero(buff, sizeof(buff));

    string tmp = src_in;
    // cout << "()()()()()()()()()()()()()() src_in -->" << src_in << endl;
    write(sockfd, tmp.c_str(), tmp.size());

    // cout("Enter the string : ");
    // n = 0;
    // while ((buff[n++] = getchar()) != '\n')
    //     ;
    // write(sockfd, buff, sizeof(buff));
    bzero(buff, sizeof(buff));

    receive_chunks(sockfd, dst_in, filename_in);
    // cout << "&&&&&&&&&&&&&&&&&&&&&&&&&& came here &&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;

    cout << "From Server : Downloaded file Successfully." << endl;
    close(sockfd);
    // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;

    // read(sockfd, buff, sizeof(buff));
    // cout("From Server : %s", buff);
    // if ((strncmp(buff, "exit", 4)) == 0)
    // {
    //     cout("Client Exit...\n");
    //     break;
    // }
}

void *connect_multipeer(void *multilw)
{
    struct multi_peers *multi_in = (struct multi_peers *)multilw;
    string port_in = multi_in->port;
    string ip_in = multi_in->ip;
    string src_in = multi_in->src_path;
    string dst_in = multi_in->dst_path;
    string sha_in = multi_in->sha;
    string filename_in = multi->filename;

    // cout << "-------------------------------------" << endl;
    // cout << "port_in-" << port_in << endl;
    // cout << "ip_in-" << ip_in << endl;
    // cout << "src_in-" << src_in << endl;
    // cout << "dst_in-" << dst_in << endl;
    // cout << "filename_in-" << filename_in << endl;
    // cout << "--------------------------------------------" << endl;

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cout << "socket creation failed...\n";
        // log("socket creation failed...\n");
        exit(0);
    }
    else
    {
        cout << "Socket successfully created..\n";
        // log("Socket successfully created..\n")
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_in.c_str());
    servaddr.sin_port = htons(stoi(port_in));

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        cout << "INSIDE MULTIPEER connection with the server failed...\n";
        exit(0);
    }
    else
    {
        cout << "INSIDE MULTIPEER connected to the server..\n";
    }

    func_from_multipeer(sockfd, src_in, dst_in, filename_in, sha_in);
    // cout << "reached $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;

    close(sockfd);
    // cout << "reached =======================================================" << endl;

    return NULL;
}

void func(int sockfd)
{
    // cout << "func in client" << endl;
    char buff[MAX];
    int n;

    while (1)
    {
        int break_flag = 1;

        printf("Enter the string : ");

        n = 0;
        bzero(buff, MAX);

        cin.getline(buff, MAX);

        string tmpbuff = buff;

        // cout << "tmpbuff-" << tmpbuff << endl;

        string command_to_send = "";

        int i = 0;
        while (tmpbuff[i] != ' ' && i < tmpbuff.length())
        {
            // cout << "qqqqqq" << i << endl;
            command_to_send += tmpbuff[i];
            i++;
        }
        i++;
        // cout << "command_to_send-" << command_to_send << "___________" << endl;

        if (command_to_send == "upload_file")
        {
            // cout << "inside upload_file" << endl;
            string command2 = "";
            string file_path = "";
            // string file_name = "";
            string group_id = "";
            string SHA1 = "";

            int j = i;
            while (tmpbuff[j] != ' ')
            {
                file_path += tmpbuff[j];
                j++;
            }
            j++;
            while (j < tmpbuff.size())
            {
                group_id += tmpbuff[j];
                j++;
            }

            char *is_correct = realpath(file_path.c_str(), NULL);
            if (is_correct == NULL)
            {
                cout << "Error:incorrect path" << endl;
                exit(0);
            }
            else
            {
                file_path = is_correct;
                // cout << "file_path from realpath in upload-" << file_path << endl;

                // int k = tmpbuff.size() - 1;
                // while (tmpbuff[k] != '/')
                // {
                //     file_name = tmpbuff[k] + file_name;
                //     k--;
                // }

                long file_size = GetFileSize(file_path);

                ifstream t(file_path);
                if (!t.is_open())
                {
                    cout << "error while opening file!!" << endl;
                }
                else
                {
                    stringstream buffer;
                    buffer << t.rdbuf();
                    string filedata = buffer.str();
                    SHA1 = sha1(filedata);

                    // cout << "SHA1----------------->" << SHA1 << endl;
                    // cout<<"filename-"<<file_name<<endl;

                    command2 = command_to_send + " " + file_path + " " + group_id + " " + command[1] + " " + command[0] + " " + to_string(file_size) + " " + SHA1;
                    // cout << "command_to_send--->" << command_to_send << endl;
                    // cout << "command2.c_str()-" << command2.c_str() << endl;
                    // cout << "sizeof(command2)-" << command2.size() << endl;
                    write(sockfd, command2.c_str(), command2.size());
                }
            }
        }
        else if (command_to_send == "download_file")
        {
            // cout << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
            string group_id = "";
            string file_name = "";
            string destination_path = "";
            string target_path = "";

            int j = i;
            while (tmpbuff[j] != ' ')
            {
                group_id += tmpbuff[j];
                j++;
            }
            j++;
            while (tmpbuff[j] != ' ')
            {
                file_name += tmpbuff[j];
                j++;
            }
            j++;
            while (j < tmpbuff.size())
            {
                destination_path += tmpbuff[j];
                j++;
            }

            char *is_correct3 = realpath(destination_path.c_str(), NULL);
            if (is_correct3 == NULL)
            {
                cout << "Error:incorrect path" << endl;
                continue;
            }
            else
            {
                // cout << "^&#^&^&*#&*#destination_path-" << destination_path << endl;
                destination_path = is_correct3;

                // cout << "#%$^!*^*@%*$^%!&destination_path" << destination_path << endl;
                destination_path = destination_path + "/" + file_name;

                string command2 = command_to_send + " " + group_id + " " + file_name + " " + destination_path + " " + command[1] + " " + command[0];
                // cout << "command2--------->" << command2 << endl;
                write(sockfd, command2.c_str(), command2.size());

                break_flag = 0;
                bzero(buff, sizeof(buff));
                read(sockfd, buff, sizeof(buff));
                // cout << "From Server inside download :" << buff << endl;

                string received_port_ip = buff;
                // cout << "received_port_ip-" << received_port_ip << endl;
                // cout << "buff-" << buff << endl;
                string target_port = "";
                string target_ip = "";
                string target_sha1 = "";
                string target_filename = "";

                if (received_port_ip[0] == '#')
                {
                    received_port_ip.erase(0, 1);
                    // cout << "From Server : " << received_port_ip << endl;
                }
                else
                {
                    int y = 0;
                    while (received_port_ip[y] != ' ')
                    {
                        target_port += received_port_ip[y];
                        y++;
                    }
                    y++;
                    while (received_port_ip[y] != ' ')
                    {
                        target_ip += received_port_ip[y];
                        y++;
                    }
                    y++;
                    while (received_port_ip[y] != ' ')
                    {
                        target_path += received_port_ip[y];
                        y++;
                    }
                    y++;
                    while (received_port_ip[y] != ' ')
                    {
                        target_filename += received_port_ip[y];
                        y++;
                    }
                    y++;
                    while (y < received_port_ip.size())
                    {
                        target_sha1 += received_port_ip[y];
                        y++;
                    }

                    // cout << "target_port-" << target_port << endl;
                    // cout << "target_ip-" << target_ip << endl;
                    // cout << "target_path-" << target_path << endl;
                    // cout << "target_sha1-" << target_sha1 << endl;

                    pthread_t multipeer;
                    multi = (struct multi_peers *)malloc(sizeof(struct multi_peers));

                    multi->port = target_port;
                    multi->ip = target_ip;
                    multi->src_path = target_path;
                    multi->dst_path = destination_path;
                    multi->sha = target_sha1;
                    multi->filename = target_filename;

                    pthread_create(&multipeer, NULL, &connect_multipeer, multi);
                    pthread_detach(multipeer);
                }
            }
        }
        else
        {
            write(sockfd, tmpbuff.c_str(), sizeof(tmpbuff));
        }

        if (break_flag == 1)
        {
            bzero(buff, sizeof(buff));
            read(sockfd, buff, sizeof(buff));
            printf("From Server : %s\n", buff);
            if ((strncmp(buff, "exit", 4)) == 0)
            {
                printf("Client Exit...\n");
                break;
            }
        }
    }
}

void send_chunks(int sockfd, string received_src_in)
{
    // cout << "received_src_in---------->" << received_src_in << endl;
    int fd5 = open(received_src_in.c_str(), O_RDONLY);
    if (fd5 == -1)
    {
        cout << "Error in send_chunks" << endl;
        return;
    }
    char buffer5[MAX];
    int n;
    while (n = read(fd5, buffer5, MAX))
    {
        // cout << "sent" << n << "Bytes" << endl;
        // cerr << "sent" << n << "Bytes" << endl;
        if (send(sockfd, buffer5, n, 0) < 0)
        {
            perror("Error in sending file.");
            exit(1);
        }
        bzero(buffer5, MAX);
    }
    close(sockfd);
}

void *func_from_serverthread(void *pointer_sockfd)
{
    // cout << "inside func_from_serverthread" << endl;
    int connfd = *((int *)pointer_sockfd);
    // free(pointer_sockfd);

    char buff[MAX];
    int n;

    bzero(buff, MAX);

    read(connfd, buff, sizeof(buff));

    // cout("From client: %s\t To client : ", buff);
    // bzero(buff, MAX);

    // n = 0;

    // while ((buff[n++] = getchar()) != '\n')
    //     ;

    string received_src_in = buff;
    // cout << "received_src_in-" << buff << endl;

    ifstream t(received_src_in);
    if (!t.is_open())
    {
        // cout << "error while opening file!!" << endl;
    }
    else
    {
        // cout << "received_src_in in func_from_serverthread" << received_src_in << endl;
        send_chunks(connfd, received_src_in);
        bzero(buff, MAX);
    }

    // write(connfd, received_src_in.c_str(),received_src_in.size());

    // if (strncmp("exit", buff, 4) == 0)
    // {
    //     cout("Server Exit...\n");
    //     break;
    // }
    return NULL;
}

void *make_my_Server(void *clientIP)
{
    string cip = *(string *)clientIP;
    vector<string> cmd;
    cmd = splitstring(cip, ':');
    string cmd_ip = command[0], cmd_port = command[1];
    // cout << "inside server command[0] - " << command[0] << endl;
    // cout << "inside server command[1] - " << command[1] << endl;

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cout << "Inside server socket creation failed...\n";
        // log("socket creation failed...\n");
        exit(0);
    }
    else
    {
        cout << "Inside server Socket successfully created..\n";
        // log("socket successfully created...\n")
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = htonl("INADDR_ANY");
    servaddr.sin_addr.s_addr = inet_addr(cmd_ip.c_str());
    servaddr.sin_port = htons(stoi(cmd_port));
    // cout << "cmd_ip -" << cmd_ip << endl;
    // cout << "cmd_port -" << cmd_port << endl;

    int opt;

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        cout << "Inside server socket bind failed...\n";
        // log("socket bind failed...\n");
        exit(0);
    }
    else
    {
        cout << "Inside server Socket successfully binded..\n";
        // log("Socket successfully binded..\n");
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 20)) != 0)
    {
        cout << "Inside server Listen failed...\n";
        // log("Listen failed...\n");
        exit(0);
    }
    else
    {
        cout << "Inside server Server listening..\n";
        // log("Server listening..\n");
    }

    socklen_t len = sizeof(cli);

    // pthread_t process_listen;
    // args = (struct argument *)malloc(sizeof(struct argument));
    // args->cli_in_argument = cli;
    // args->sock_fd_in_argument = sockfd;

    // pthread_create(&process_listen, NULL, &process_listen_thread, args);

    while (1)
    {
        // Accept the data packet from client and verification
        // cout << "inside loop" << endl;
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            cout << "Inside server server accept failed...\n";
            // log("");
            exit(0);
        }
        else
        {
            cout << "Inside server server accept the client...\n";
            // log("server accept the client...\n");
        }

        // Function for chatting between client and server

        int *pclient = new int;
        *pclient = connfd;
        // pthread_t dummy;

        pthread_create(&tid[thread_number++], NULL, func_from_serverthread, pclient);
    }

    for (int i = 0; i <= thread_number; i++)
    {
        // cout << "hdhdhdhdhdhdhdhd------------------------>>>>>>>>\n";
        pthread_join(tid[i], NULL);
    }
    // After chatting close the socket

    close(sockfd);
    return NULL;
}

int main(int argc, char **argv)
{
    string one = argv[1];
    // cout << argv[1] << endl;
    string two = argv[2];
    // cout << argv[2] << endl;

    ifstream file(two);

    string main_ip = "";
    string main_port = "";
    string tmpq = "";

    file >> tmpq;
    // cout << "tmpq-" << tmpq << endl;
    char *token = strtok(tmpq.data(), ":");
    main_ip = token;

    token = strtok(NULL, ":");
    main_port = token;

    if (argc != 3)
    {
        cout << "invalid arguments" << endl;
        exit(1);
    }

    string client_ip = string(argv[1]);

    command = splitstring(client_ip, ':');
    string c_ip = command[0];
    string c_port = command[1];

    pthread_t serverthread;

    if (pthread_create(&serverthread, NULL, make_my_Server, (void *)&client_ip) < 0)
    {
        // log("cannot create server thread");
        cout << "faliure in creating server thread !!!!!" << endl;
        exit(EXIT_FAILURE);
    }
    // cout << "before call to server thread" << endl;
    pthread_detach(serverthread);
    // cout << "after call to server thread" << endl;

    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cout << "socket creation failed...\n";
        // log("socket creation failed...\n");
        exit(0);
    }
    else
    {
        cout << "Socket successfully created..\n";
        // log("Socket successfully created..\n")
    }
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(main_ip.c_str());
    servaddr.sin_port = htons(stoi(main_port));

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        cout << "connection with the server failed...\n";
        exit(0);
    }
    else
    {
        cout << "connected to the server..\n";
    }

    // function for chat
    // cout << "before call to func()" << endl;
    func(sockfd);
    // cout << "after call to func()" << endl;

    // close the socket
    close(sockfd);

    // cout.close();

    return 0;
}
