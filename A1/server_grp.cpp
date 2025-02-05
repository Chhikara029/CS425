#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>

#define BUFFER_SIZE 1024

using namespace std;

mutex cout_mutex;   // Mutex to synchronize console output
mutex client_mutex; // Mutex to synchronize access to client data structures
mutex group_mutex;  // Mutex to synchronize access to group data structures

unordered_map<string, string> users;                        // username -> password
unordered_map<int, string> client_names;                    // socket -> name who are logged in
unordered_map<string, int> client_id;                       // name -> socket
unordered_map<string, unordered_set<string>> group_members; // group_name -> set of sockets
bool check_valid_user(const string &username, const string &password);
void send_broadcast_message(int client, const string message)
{
    unique_lock<std::mutex> client_lock(client_mutex);
    for (auto client_pair : client_names)
    {
        if (client_pair.first != client)
        {
            send(client_pair.first, message.c_str(), message.size(), 0);
        }
    }
    client_lock.unlock();
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    string username, password;
    while (true)
    {
        send(client_socket, "Enter user name : ", 18, 0);
        int bytes = recv(client_socket, buffer, BUFFER_SIZE, 0);
        username = buffer;
        // cout << bytes << endl;
        if (bytes<= 0)
        {
            close(client_socket);
            return;
        }
        else
            break;
    }

    memset(buffer, 0, BUFFER_SIZE);
    while (true)
    {
        send(client_socket, "Enter password : ", 18, 0);
        int bytes_recieved=recv(client_socket, buffer, BUFFER_SIZE, 0);
        if(bytes_recieved<=0){
            close(client_socket);
            return;
        }
        password = buffer;
        if (password.empty())
        {
            continue;
        }
        else
            break;
    }

    if (!check_valid_user(username, password))
    {
        send(client_socket, "Error: Authentication failed.", 30, 0);
        close(client_socket);
        return;
    }

    unique_lock<std::mutex> client_lock(client_mutex);
    if (client_id.find(username) != client_id.end())
    {
        
        send(client_socket, "Error: User already logged in.", 30, 0);
        client_lock.unlock();
        close(client_socket);
        return;
    }
    client_names[client_socket] = username;
    client_id[username] = client_socket;
    client_lock.unlock();
    send(client_socket, "Authentication successful.", 28, 0);
    send_broadcast_message(client_socket, username + " has joined the chat.");

    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_recieved=recv(client_socket, buffer, BUFFER_SIZE, 0);
        if(bytes_recieved<=0){
            unique_lock<std::mutex> client_lock(client_mutex);
            client_names.erase(client_socket);
            client_id.erase(username);
            client_lock.unlock();
            send_broadcast_message(client_socket, username + " has exited the chat.");
            break;
        }
        string command = buffer;
        if (command.empty())
        {
            continue;
        }
        if (command == "/exit")
        {
            unique_lock<std::mutex> client_lock(client_mutex);
            client_names.erase(client_socket);
            client_id.erase(username);
            client_lock.unlock();
            send_broadcast_message(client_socket, username + " has exited the chat.");
            break;
        }
        if ("/msg" == command.substr(0, 4) && command[4]==' ')
        {
            // parse the command into /msg <username> <message>
            istringstream iss(command);
            vector<string> tokens;
            string token;
            while (getline(iss, token, ' '))
            {
                tokens.push_back(token);
            }
            if (tokens.size() < 3)
            {
                send(client_socket, "Error: Invalid command.", 24, 0);
                continue;
            }
            string receiver = tokens[1];
            string message = "[" + username + "]" + " : ";
            unique_lock<std::mutex> client_lock(client_mutex);
            if (client_id.find(receiver) == client_id.end())
            {
                string msg = "Error:" + receiver + " has not logged in.";
                send(client_socket, msg.c_str(), msg.size(), 0);
                client_lock.unlock();
                continue;
            }
            for (int i = 2; i < tokens.size(); i++)
            {
                message += " " + tokens[i];
            }
            bool receiver_found = false;
            send(client_id[receiver], message.c_str(), message.size(), 0);
            client_lock.unlock();
        }
        else if ("/broadcast" == command.substr(0, 10)  && command[10]==' ')
        {
            send_broadcast_message(client_socket, "[" + username + "]" + " : " + command.substr(11));
        }
        else if ("/create_group" == command.substr(0, 13) && command[13]==' ')
        {
            string group_name = command.substr(14);
            unique_lock<std::mutex> group_lock(group_mutex);
            if (group_members.find(group_name) != group_members.end())
            {
                send(client_socket, "Error: Group already exists.", 29, 0);
                group_lock.unlock();
                continue;
            }
            group_members[group_name].insert(username);
            group_lock.unlock();
            send_broadcast_message(client_socket, username + " has created group " + group_name);
        }
        else if ("/join_group" == command.substr(0, 11) && command[11]==' ')
        {
            int flag=0;
            string group_name = command.substr(12);
            unique_lock<std::mutex> client_lock(client_mutex);
            unique_lock<std::mutex> group_lock(group_mutex);
            if(group_members.find(group_name)==group_members.end())
            {
                string message="Error: Group does not exist";
                send(client_socket, message.c_str(), message.size(), 0);
            }
            else if(group_members[group_name].find(username)!=group_members[group_name].end())
            {
                send(client_socket, "Error: You are already a member of this group.", 46, 0);
            }
            else
            {
                group_members[group_name].insert(username);
                flag=1;
            }
            client_lock.unlock();
            group_lock.unlock();
            if(flag)
            send_broadcast_message(client_socket, username + " has joined group " + group_name);
        }
        else if ("/leave_group" == command.substr(0, 12)  && command[12]==' ')
        {
            int flag=0;
            string group_name = command.substr(13);
            unique_lock<std::mutex> client_lock(client_mutex);
            unique_lock<std::mutex> group_lock(group_mutex);
            if(group_members.find(group_name)==group_members.end())
            {
                string message="Error: Group does not exist";
                send(client_socket, message.c_str(), message.size(), 0);
            }
            else if(group_members[group_name].find(username)==group_members[group_name].end())
            {
                send(client_socket, "Error: You are not a member of this group.", 42, 0);
            }
            else
            {
            group_members[group_name].erase(username);
            flag=1;
            }
            client_lock.unlock();
            group_lock.unlock();
            if(flag)
            send_broadcast_message(client_socket, username + " has left group " + group_name);
        }
        else if ("/group_msg" == command.substr(0, 10)  && command[10]==' ')
        {
            istringstream iss(command);
            vector<string> tokens;
            string token;
            while (getline(iss, token, ' '))
            {
                tokens.push_back(token);
            }
            string group_name = tokens[1];
            string message = "[ Group " + group_name + " : " + username + "]"
                                                                          " : ";
            for (int i = 2; i < tokens.size(); i++)
            {
                message += " " + tokens[i];
            }
            unique_lock<std::mutex> client_lock(client_mutex);
            unique_lock<std::mutex> group_lock(group_mutex);
            if (group_members.find(group_name) == group_members.end())
            {
                send(client_socket, "Error: Group does not exist.", 28, 0);
                client_lock.unlock();
                group_lock.unlock();
                continue;
            }
            if (group_members[group_name].find(username) == group_members[group_name].end())
            {
                send(client_socket, "Error: You are not a member of this group.", 42, 0);
                client_lock.unlock();
                group_lock.unlock();
                continue;
            }
            for (auto member : group_members[group_name])
            {
                if (member == username)
                {
                    continue;
                }
                if (client_id.find(member) != client_id.end())
                {
                    send(client_id[member], message.c_str(), message.size(), 0);
                }
            }
            client_lock.unlock();
            group_lock.unlock();
        }
        else
        {
            send(client_socket, "Error: Invalid command.", 24, 0);
        }
        
    }

    close(client_socket);
    return;
}

void load_txt_file(const string &filename, unordered_map<string, string> &users)
{
    ifstream file(filename); // Open the file

    if (!file)
    { // Check if file opened successfully
        cerr << "Error opening " << filename << " file!" << endl;
        return;
    }

    string line, stored_username, stored_password;

    while (getline(file, line))
    {                                          // Read each line
        size_t delimiter_pos = line.find(':'); // Find the position of ':'
        if (delimiter_pos == string::npos)
        {
            continue; // Skip lines without the expected format
        }

        // Extract username and password
        stored_username = line.substr(0, delimiter_pos);
        stored_password = line.substr(delimiter_pos + 1);

        // Store in the map
        users[stored_username] = stored_password;
    }
}

bool check_valid_user(const string &username, const string &password)
{
    if (users.find(username) != users.end() && users[username] == password)
    {
        return true;
    }
    return false;
}

int main()
{
    // server code
    int server_socket;
    sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        cerr << "Error setting socket options." << endl;
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (::bind(server_socket, (sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        cerr << "Error binding socket." << endl;
        return 1;
    }

    if (listen(server_socket, 3) < 0)
    {
        cerr << "Error listening on socket." << endl;
        return 1;
    }

    cout << "Server is listening on port 12345." << endl;

    load_txt_file("users.txt", users);
    while (true)
    {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0)
        {
            cerr << "Accept failed." << endl;
            continue;
        }
        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    return 0;
}
