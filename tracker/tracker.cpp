#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <bits/stdc++.h>
#include <pthread.h> 
#include <arpa/inet.h>
#include <sys/types.h>
#include <mutex>
#include <unistd.h> // read(), write(), close()
#define MAX 524288
#define SA struct sockaddr
using namespace std;

pthread_t tid[60];
int thread_number = 0;

struct upload_file_info
{
    string file_path;
    string user_name;
    string file_name;
    long file_size;
    string group_id;
    string portno;
    string ip;
    string SHA1;
};

map<string, string> username_password;
map<string, bool> login_map;
unordered_map<string, vector<string>> groupname_members;
map<string, vector<string>> list_of_pending_joins;
unordered_map<string, vector<upload_file_info>> upload_file_map;
unordered_map<string, vector<pair<string, string>>> downloads_map;
unordered_map<string, vector<pair<string, string>>> filename_ip_port;
mutex m1, m2;

void createuser(string username, string password)
{
    username_password[username] = password;
}

void login(string str)
{
    login_map[str] = 1;
}

int creategroup(string group_id, string admin)
{
    if (groupname_members.size() != 0 && groupname_members.find(group_id) != groupname_members.end())
    {
        // cout << "group name already exists\n";
        return -1;
    }
    else
    {
        groupname_members[group_id].push_back(admin);
        return 1;
    }
}

int joingroup(string group_id, string user)
{
    if (groupname_members.size() == 0 || groupname_members.find(group_id) == groupname_members.end())
    {
        // cout << "No group with this group name\n";
        return -1;
    }
    else
    {
        if (find(list_of_pending_joins[group_id].begin(), list_of_pending_joins[group_id].end(), user) == list_of_pending_joins[group_id].end())
        {
            list_of_pending_joins[group_id].push_back(user);
        }
        return 1;
    }
}

int listrequest(string group_id)
{
    if (groupname_members.size() == 0 || groupname_members.find(group_id) == groupname_members.end())
    {
        // cout << "no group with this group name\n";
        return -1;
    }
    else
    {
        return 1;
    }
}

int leavegroup(string group_id, string user)
{
    if (groupname_members.size() == 0 || groupname_members.find(group_id) == groupname_members.end())
    {
        // cout << "No group with this group name\n";
        return -1;
    }
    else
    {
        auto itr = find(groupname_members[group_id].begin(), groupname_members[group_id].end(), user);
        if (itr == groupname_members[group_id].end())
        {
            // cout << "username not exists in this group\n";
            return -1;
        }
        else
        {
            groupname_members[group_id].erase(itr);
            if (groupname_members[group_id].size() == 0)
            {
                groupname_members.erase(group_id);
            }

            if (login_map.find(user) != login_map.end())
            {
                login_map[user] = 0;
                login_map.erase(user);
            }

            for (int i = 0; i < upload_file_map[group_id].size(); i++)
            {
                if (upload_file_map[group_id][i].user_name == user)
                {
                    upload_file_map[group_id].erase(upload_file_map[group_id].begin() + i);
                    i--;
                }
            }

            return 1;
        }
    }
}

int acceptrequest(string group_id, string user)
{
    auto itr = find(list_of_pending_joins[group_id].begin(), list_of_pending_joins[group_id].end(), user);
    if (itr != list_of_pending_joins[group_id].end())
    {
        groupname_members[group_id].push_back(user);
        list_of_pending_joins[group_id].erase(itr);
        return 1;
    }
    else
    {
        // cout << "username not exists in this group\n";
        return -1;
    }
}

// Function designed for chat between client and server.
void *func(void *pointer_sockfd)
{
    // cout << "func" << endl;
    int connfd = *((int *)pointer_sockfd);
    free(pointer_sockfd);

    char buff[MAX];
    int n;
    string main_user = "";

    while (1)
    {
        // cout << " map of username_password" << endl;
        // for (auto i : username_password)
        // {
        //     cout << i.first << " " << i.second << "----->";
        // }

        // cout << endl;
        // cout << "login_map" << endl;
        // for (auto i : login_map)
        // {
        //     cout << i.first << " " << i.second << "----->";
        // }

        // cout << endl;
        // cout << "group name" << endl;
        // for (auto i : groupname_members)
        // {
        //     cout << i.first << "----->";
        //     for (auto it : i.second)
        //     {
        //         cout << it << " " << endl;
        //     }
        //     cout << endl;
        // }
        // cout << endl;
        // cout << "list_of_pending_joins" << endl;
        // for (auto i : list_of_pending_joins)
        // {
        //     cout << i.first << "----->";
        //     for (auto it : i.second)
        //     {
        //         cout << it << " " << endl;
        //     }
        //     cout << endl;
        // }
        // cout << endl;

        // cout << "upload_file_map-" << endl;
        // for (auto it : upload_file_map)
        // {
        //     cout << it.first << "------>";
        //     for (auto itr : it.second)
        //     {
        //         cout << "debug print file_path:" << itr.file_path << "-----///////////////-" << endl;
        //         cout << "debug print group_id:" << itr.group_id << "----///////////////--" << endl;
        //         cout << "debug print portno:" << itr.portno << "----///////////////--" << endl;
        //         cout << "debug print ip:" << itr.ip << "----///////////////--" << endl;
        //         cout << "debug print file_size:" << itr.file_size << "---///////////////---" << endl;
        //         cout << "debug print file_name:" << itr.file_name << "///////////////------" << endl;
        //         cout << "debug print SHA1:" << itr.SHA1 << "--///////////////----" << endl;
        //     }
        //     cout << endl;
        // }
        // cout << endl;
        // cout << endl;
        // cout << endl;

        bzero(buff, MAX);

        // read the message from client and copy it in buffer
        int a = read(connfd, buff, sizeof(buff));
        if (a <= 0)
        {
            return NULL;
        }
        // cout << buff << endl;
        string buff_string = buff;
        string command = "";
        // cout << buff << endl;

        int i = 0;
        while (buff_string[i] != ' ' && i < buff_string.length())
        {
            command += buff_string[i];
            i++;
        }
        i++;

        // cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        // cout << "command-" << command << "***" << endl;
        // cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

        if (command == "create_user")
        {
            m1.lock();
            string user_name = "";
            string password = "";

            int j = i;
            while (buff_string[j] != ' ')
            {
                user_name += buff_string[j];
                j++;
            }

            j++;
            while (j < buff_string.size())
            {
                password += buff_string[j];
                j++;
            }

            if (password.size() == 0)
            {
                write(connfd, "empty password field re-enter username nad password\n", sizeof("empty password field re-enter username nad password\n"));
            }
            else
            {
                createuser(user_name, password);
                write(connfd, "user created\n", sizeof("user created\n"));
            }
            m1.unlock();
        }
        else if (command == "login")
        {
            m2.lock();
            string user_name = "";
            string password = "";

            // cout << "user_name -" << user_name << endl;

            // cout << "password -" << password << endl;

            int j = i;
            while (buff_string[j] != ' ')
            {
                user_name += buff_string[j];
                j++;
            }
            j++;
            while (j < buff_string.size())
            {
                password += buff_string[j];
                j++;
            }

            if (username_password.find(user_name) == username_password.end())
            {
                // cout << "Invalid username" << endl;
                write(connfd, "Invalid username\n", sizeof("Invalid username\n"));
            }
            else if (username_password[user_name] != password)
            {
                // cout << "Invalid password" << endl;
                write(connfd, "Invalid password\n", sizeof("Invalid password\n"));
            }
            else
            {
                login(user_name);
                main_user = user_name;
                // cout << "login_map[user_name] - " << login_map[user_name] << endl;
                write(connfd, "logged_in\n", sizeof("logged_in\n"));
            }
            m2.unlock();
        }
        else if (command == "logout")
        {
            // cout << "inside" << endl;
            login_map[main_user] = 0;
            login_map.erase(main_user);
            // cout << "logged out" << endl;

            write(connfd, "logged out\n", sizeof("logged out\n"));
        }
        else if (command == "create_group")
        {
            // cout << "main_user" << main_user << endl;
            if (main_user.size() != 0 && login_map[main_user] == 1)
            {
                string group_id = "";

                int j = i;
                while (j < buff_string.size())
                {
                    group_id += buff_string[j];
                    j++;
                }
                // cout << "inside ----------------->group_id-" << group_id << endl;

                if (creategroup(group_id, main_user) == 1)
                {
                    write(connfd, "new group created\n", sizeof("new group created\n"));
                }
                else
                {
                    write(connfd, "group name already exists\n", sizeof("group name already exists\n"));
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else if (command == "join_group")
        {
            // cout << "main_user" << main_user << endl;
            if (main_user != "" && login_map[main_user] == 1)
            {
                string group_id = "";
                int j = i;
                while (j < buff_string.size())
                {
                    group_id += buff_string[j];
                    j++;
                }

                if (joingroup(group_id, main_user) == -1)
                {
                    write(connfd, "No group with this group name\n", sizeof("No group with this group name\n"));
                }
                else
                {
                    if (find(groupname_members[group_id].begin(), groupname_members[group_id].end(), main_user) != groupname_members[group_id].end())
                    {
                        write(connfd, "Already in Group \n", sizeof("Already in Group \n"));
                    }
                    else
                    {
                        write(connfd, "waiting for admin \n", sizeof("waiting for admin \n"));
                    }
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else if (command == "leave_group")
        {
            // cout << "main_user" << main_user << endl;
            if (main_user.size() != 0 && login_map[main_user] == 1)
            {
                string group_id = "";
                int j = i;
                while (j < buff_string.size())
                {
                    group_id += buff_string[j];
                    j++;
                }

                if (leavegroup(group_id, main_user) == -1)
                {
                    write(connfd, "user not exists / group not exists\n", sizeof("user not exixts / group not exists\n"));
                }
                else
                {
                    write(connfd, "user leaved group\n", sizeof("user leaved group\n"));
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else if (command == "list_request")
        {
            // cout << "main_user" << main_user << endl;

            string group_id = "";
            int j = i;
            while (j < buff_string.size())
            {
                group_id += buff_string[j];
                j++;
            }

            if (groupname_members.find(group_id) != groupname_members.end())
            {
                string admin = groupname_members[group_id][0];
                if (login_map.size() > 0 && (main_user == admin))
                {
                    if (listrequest(group_id) == -1)
                    {
                        write(connfd, "No group with this group name\n", sizeof("No group with this group name\n"));
                    }
                    else
                    {
                        string tmpbuff = "";
                        for (int it = 0; it < list_of_pending_joins[group_id].size(); it++)
                        {
                            tmpbuff += list_of_pending_joins[group_id][it];
                            tmpbuff += ",";
                        }
                        if (tmpbuff.size() > 0)
                        {
                            tmpbuff.pop_back();
                            tmpbuff.push_back('.');
                            tmpbuff.push_back('\n');
                            write(connfd, tmpbuff.c_str(), tmpbuff.size());
                            tmpbuff.erase();
                        }
                        else
                        {
                            write(connfd, "no requests\n", sizeof("no requests\n"));
                        }
                    }
                }
                else
                {
                    write(connfd, "admin not logged in or group doesn't exist.\n", sizeof("admin not logged in or group doesn't exist.\n"));
                }
            }
            else
            {
                write(connfd, "group doesn't exist.\n", sizeof("group doesn't exist.\n"));
            }
        }
        else if (command == "accept_request")
        {
            // cout << "main_user" << main_user << endl;

            string group_id = "";
            string user = "";

            int j = i;
            while (buff_string[j] != ' ')
            {
                group_id += buff_string[j];
                j++;
            }
            j++;
            while (j < buff_string.size())
            {
                user += buff_string[j];
                j++;
            }
            if (groupname_members.size() != 0 && groupname_members.find(group_id) != groupname_members.end())
            {
                // cout << "group_id:" << group_id << endl;
                string admin = groupname_members[group_id][0];
                // cout << "Admin:" << admin << endl;
                // cout << "user:" << user << endl;
                if (admin == main_user)
                {
                    if (acceptrequest(group_id, user) == -1)
                    {
                        write(connfd, "username not exists in this group\n", sizeof("username not exists in this group\n"));
                    }
                    else
                    {
                        write(connfd, "user accepted\n", sizeof("user accepted\n"));
                    }
                }
                else
                {
                    write(connfd, "admin not logged in.\n", sizeof("admin not logged in.\n"));
                }
            }
            else
            {
                write(connfd, "No group with this name.\n", sizeof("No group with this name.\n"));
            }
        }
        else if (command == "list_groups")
        {
            // cout << "main_user" << main_user << endl;
            if (main_user.size() != 0 && login_map[main_user] == 1)
            {
                string allkeys = "";
                for (auto itr = groupname_members.begin(); itr != groupname_members.end(); itr++)
                {
                    allkeys += itr->first;
                    allkeys += ",";
                }
                if (allkeys.size() >= 1)
                {
                    allkeys.pop_back();
                    allkeys.push_back('.');
                    allkeys.push_back('\n');
                    write(connfd, allkeys.c_str(), allkeys.size());
                    allkeys.erase();
                }
                else if (allkeys.size() == 0)
                {
                    write(connfd, "no groups\n", sizeof("no groups\n"));
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else if (command == "upload_file")
        {
            string file_path = "";
            string group_id = "";
            string file_name = "";
            long file_size = 0;
            string portno = "";
            string ip = "";
            string SHA1 = "";

            // cout << "buff_string ----------->" << buff_string << endl;

            int j = i;
            while (buff_string[j] != ' ')
            {
                file_path += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                group_id += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                portno += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                ip += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                file_size += buff_string[j];
                j++;
            }
            j++;
            while (j < buff_string.size())
            {
                SHA1 += buff_string[j];
                j++;
            }

            int k = file_path.size() - 1;
            while (file_path[k] != '/')
            {
                file_name = file_path[k] + file_name;
                k--;
            }

            char *is_correct2 = realpath(file_path.c_str(), NULL);
            if (is_correct2 == NULL)
            {
                // cout << "Error:incorrect path" << endl;
                write(connfd, "Error:incorrect path\n", sizeof("Error:incorrect path\n"));
                // exit(EXIT_FAILURE);
            }
            file_path = is_correct2;

            // cout << "inside upload_file file_path:" << file_path << "---------" << endl;
            // cout << "inside upload_file group_id:" << group_id << "---------" << endl;
            // cout << "inside upload_file portno:" << portno << "---------" << endl;
            // cout << "inside upload_file ip:" << ip << "---------" << endl;
            // cout << "inside upload_file file_size:" << file_size << "---------" << endl;
            // cout << "inside upload_file file_name:" << file_name << "---------" << endl;
            // cout << "inside upload_file SHA1:" << SHA1 << "---------" << endl;

            // cout << "sizes---->" << endl;
            // cout << "inside upload_file file_path:" << file_path.size() << "---------" << endl;
            // cout << "inside upload_file group_id:" << group_id.size() << "---------" << endl;
            // cout << "inside upload_file portno:" << portno.size() << "---------" << endl;
            // cout << "inside upload_file ip:" << ip.size() << "---------" << endl;
            // cout << "inside upload_file file_size:" << file_size << "---------" << endl;
            // cout << "inside upload_file file_name:" << file_name.size() << "---------" << endl;
            // cout << "inside upload_file SHA1:" << SHA1.size() << "---------" << endl;

            if (groupname_members.find(group_id) != groupname_members.end())
            {
                if (find(groupname_members[group_id].begin(), groupname_members[group_id].end(), main_user) != groupname_members[group_id].end())
                {
                    upload_file_info var1;
                    var1.group_id = group_id;
                    var1.file_path = file_path;
                    var1.file_size = file_size;
                    var1.ip = ip;
                    var1.portno = portno;
                    var1.SHA1 = SHA1;
                    var1.user_name = main_user;
                    var1.file_name = file_name;

                    // cout << "___________________INSIDE UPLOAD_________________________" << endl;

                    // cout << "var1.group_id =" << group_id << endl;
                    // cout << "var1.file_path =" << file_path << endl;
                    // cout << "var1.file_size =" << file_size << endl;
                    // cout << "var1.ip =" << ip << endl;
                    // cout << "var1.portno =" << portno << endl;
                    // cout << "var1.SHA1 =" << SHA1 << endl;
                    // cout << "var1.user_name =" << main_user << endl;
                    // cout << "var1.file_name =" << file_name << endl;

                    upload_file_map[group_id].push_back(var1);

                    filename_ip_port[file_name].push_back({ip, portno});

                    write(connfd, "uploaded file successfully\n", sizeof("uploaded file successfully\n"));
                }
                else
                {
                    write(connfd, "not a user \n", sizeof("not a user \n"));
                }
            }
            else
            {
                write(connfd, "groupname not found.....\n", sizeof("groupname not found.....\n"));
            }
        }
        else if (command == "download_file")
        {
            string group_id = "";
            string file_name = "";
            string dst_path = "";
            string appended_port = "";
            string appended_ip = "";

            int j = i;
            while (buff_string[j] != ' ')
            {
                group_id += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                file_name += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                dst_path += buff_string[j];
                j++;
            }
            j++;
            while (buff_string[j] != ' ')
            {
                appended_port += buff_string[j];
                j++;
            }
            j++;
            while (j < buff_string.size())
            {
                appended_ip += buff_string[j];
                j++;
            }
            // cout << "buff_string        -" << buff_string << endl;

            // cout << "==========> group_id" << group_id << endl;
            // cout << "==========> file_name" << file_name << endl;
            // cout << "==========> dst_path" << dst_path << endl;
            // cout << "==========> appended_port" << appended_port << endl;
            // cout << "==========> appended_ip" << appended_ip << endl;

            if (groupname_members.find(group_id) != groupname_members.end())
            {
                string tmp = dst_path;
                // cout << "tmp-" << tmp << endl;
                // cout << "dst_path.size()-" << dst_path.size();
                int i = dst_path.size() - 1;

                while (dst_path[i] != '/')
                {
                    // cout << "pop<" << endl;
                    // cout << dst_path[i];
                    tmp.pop_back();
                    i--;
                }
                tmp.pop_back();
                // cout << "tmp----------->" << tmp << endl;
                if (realpath(tmp.c_str(), NULL) == NULL)
                {
                    write(connfd, "#invalid path...\n", sizeof("#invalid path...\n"));
                }
                else
                {
                    bool flag_tmp = 0;
                    string to_sent_final = "";
                    if (find(groupname_members[group_id].begin(), groupname_members[group_id].end(), main_user) != groupname_members[group_id].end())
                    {
                        for (auto itr : upload_file_map)
                        {
                            // cout << "((((((((((((((((((((((((((((" << endl;
                            for (auto itrye : itr.second)
                            {
                                // cout << ")))))))))))))))))))))))))))))))" << endl;
                                if (itrye.file_name == file_name)
                                {
                                    if (login_map.find(itrye.user_name) != login_map.end())
                                    {
                                        // cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                                        string to_sent = "";
                                        // string tmp = itrye.file_path + "/" + itrye.file_name;
                                        to_sent = itrye.portno + " " + itrye.ip + " " + itrye.file_path + " " + itrye.file_name + " " + itrye.SHA1;
                                        to_sent_final = to_sent;

                                        for (auto i : filename_ip_port[itrye.file_name])
                                        {
                                            pair<string, string> pp;
                                            pp.first = i.first;   // ip
                                            pp.second = i.second; // port
                                            to_sent_final = to_sent_final + "$" + pp.second + " " + pp.first;
                                        }
                                        upload_file_info var2;
                                        var2.group_id = group_id;
                                        var2.file_path = dst_path;
                                        var2.file_size = itrye.file_size;
                                        var2.ip = appended_ip;
                                        var2.portno = appended_port;
                                        var2.SHA1 = itrye.SHA1;
                                        var2.user_name = main_user;
                                        var2.file_name = file_name;

                                        // cout << "___________________INSIDE DOWNLOAD___________" << endl;
                                        // cout << "var2.group_id =" << group_id << endl;
                                        // cout << "var2.file_path =" << dst_path << endl;
                                        // cout << "var2.file_size =" << itrye.file_size << endl;
                                        // cout << "var2.ip =" << appended_ip << endl;
                                        // cout << "var2.portno =" << appended_port << endl;
                                        // cout << "var2.SHA1 =" << itrye.SHA1 << endl;
                                        // cout << "var2.user_name =" << main_user << endl;
                                        // cout << "var2.file_name =" << file_name << endl;

                                        upload_file_map[group_id].push_back(var2);
                                        flag_tmp = 1;
                                        downloads_map[main_user].push_back({group_id, file_name});
                                        write(connfd, to_sent.c_str(), to_sent.size());
                                    }
                                    else
                                    {
                                        flag_tmp = 1;
                                        write(connfd, "#seeder not logged in.....", sizeof("#seeder not logged in....."));
                                    }
                                }
                            }
                        }
                        if (flag_tmp == 0)
                        {
                            write(connfd, "#file not found.....\n", sizeof("#file not found.....\n"));
                        }
                    }
                    else
                    {
                        write(connfd, "#user not in group.....\n", sizeof("#user not in group.....\n"));
                    }
                }
            }
            else
            {
                write(connfd, "#groupname not found.....\n", sizeof("#groupname not found.....\n"));
            }
        }
        else if (command == "list_files")
        {
            string group_id = "";
            int j = i;
            while (j < buff_string.size())
            {
                group_id += buff_string[j];
                j++;
            }

            string allkeys = "";

            if (upload_file_map.find(group_id) != upload_file_map.end())
            {
                for (auto h : upload_file_map[group_id])
                {
                    allkeys += h.file_name;
                    allkeys += ",";
                }
            }
            if (allkeys.size() >= 1)
            {
                allkeys.pop_back();
                allkeys.push_back('.');
                allkeys.push_back('\n');
                write(connfd, allkeys.c_str(), allkeys.size());
                allkeys.erase();
            }
            else if (allkeys.size() == 0)
            {
                write(connfd, "groupname not found.....\n", sizeof("groupname not found.....\n"));
            }
        }
        else if (command == "show_downloads")
        {
            // cout << "ASDFGHJKLKJHGFDSA" << endl;
            string tmp = "\n[D]\n[C] ";

            if (login_map.find(main_user) != login_map.end())
            {
                if (downloads_map.find(main_user) == downloads_map.end())
                {
                    write(connfd, "no downloads.....\n", sizeof("no downloads.....\n"));
                }
                else
                {
                    vector<pair<string, string>> iterater = downloads_map[main_user];
                    for (auto itr : iterater)
                    {
                        // cout << itr.first << endl;
                        // cout << itr.second << endl;
                        tmp = tmp + "<" + itr.first + "," + itr.second + "> ";
                    }

                    write(connfd, tmp.c_str(), tmp.size());
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else if (command == "stop_share")
        {
            string group_id = "";
            string file_name = "";

            // cout << "buff_string ----------->" << buff_string << endl;

            int j = i;
            while (buff_string[j] != ' ')
            {
                group_id += buff_string[j];
                j++;
            }
            j++;
            while (j < buff_string.size())
            {
                file_name += buff_string[j];
                j++;
            }

            if (login_map.find(group_id) != login_map.end())
            {
                if (upload_file_map.find(group_id) != upload_file_map.end())
                {
                    for (int i = 0; i < upload_file_map[group_id].size(); i++)
                    {
                        if (upload_file_map[group_id][i].file_name == file_name)
                        {
                            upload_file_map[group_id].erase(upload_file_map[group_id].begin() + i);
                            i--;
                        }
                    }
                }
                else
                {
                    write(connfd, "no group found.....\n", sizeof("no group found.....\n"));
                }
            }
            else
            {
                write(connfd, "user not logged in.....\n", sizeof("user not logged in.....\n"));
            }
        }
        else
        {
            write(connfd, "Enter valid command\n", sizeof("Enter valid command\n"));
        }

        bzero(buff, MAX);
    }
    return NULL;
}

struct argument
{
    sockaddr_in cli_in_argument;
    int sock_fd_in_argument;

} * args;

void *process_listen_thread(void *arguments)
{
    // cout << "process_listen_thread" << endl;
    int connfd;
    struct argument *args1 = (struct argument *)arguments;
    sockaddr_in cli = args1->cli_in_argument;
    int sockfd = args1->sock_fd_in_argument;
    socklen_t len = sizeof(cli);

    while (1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            cout << "server accept failed...\n";
            exit(0);
        }
        else
            cout << "server accept the client...\n";

        int *pclient = new int;
        *pclient = connfd;

        pthread_create(&tid[thread_number++], NULL, func, pclient);
    }

    for (int i = 0; i <= thread_number; i++)
    {
        pthread_join(tid[i], NULL);
    }
}

vector<string> command_str;

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
// Driver function
int main(int argc, char **argv)
{

    if (argc != 3)
    {
        cout << "invalid arguments" << endl;
        // log("invalid arguments");
        exit(1);
    }

    string one = argv[1];
    // cout << argv[1] << endl;
    string two = argv[2];
    // cout << argv[2] << endl;

    ifstream file(one);

    string main_ip = "";
    string main_port = "";
    string tmpq = "";

    file >> tmpq;
    char *token = strtok(tmpq.data(), ":");
    main_ip = token;

    token = strtok(NULL, ":");
    main_port = token;

    int sockfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        cout << "socket creation failed...\n";

        exit(0);
    }
    else
        cout << "Socket successfully created..\n";

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(main_ip.c_str());
    servaddr.sin_port = htons(stoi(main_port));
    // servaddr.sin_addr.s_addr = inet_addr(cli_ip.c_str());
    // servaddr.sin_port = htons(stoi(cli_port));

    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        cout << "socket bind failed...\n";
        exit(0);
    }
    else
        cout << "Socket successfully binded..\n";

    if ((listen(sockfd, 20)) != 0)
    {
        cout << "Listen failed...\n";
        exit(0);
    }
    else
    {
        cout << "Server listening..\n";
    }
    socklen_t len = sizeof(cli);

    pthread_t process_listen;

    args = (struct argument *)malloc(sizeof(struct argument));
    args->cli_in_argument = cli;
    args->sock_fd_in_argument = sockfd;

    pthread_create(&process_listen, NULL, &process_listen_thread, args);

    while (1)
    {
        string is_quit;
        getline(cin, is_quit);
        if (is_quit == "quit")
        {
            break;
        }
    }

    close(sockfd);

    return 0;
}
