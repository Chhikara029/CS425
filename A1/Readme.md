This is a multi-threaded chat server implemented in C++. It allows multiple clients to communicate with each other through private messages, broadcasting, and group chats. The server handles user authentication using a predefined users.txt file and manages client connections using threads.

Pre-requisites:
To run this, you need a linux-based operating system(or WSL on windows or MacOS), and a c++ compiler. You also need a textfile containing all registered users along with their password in the following format:

user1 password1
user2 password2
.....

Instructions:
1)Compile the code using makefile.
2)Correctly enter your username and then password.
3)After authentication the client can do the following:
Once authenticated, they can use commands to communicate:

/msg <username> <message>: Send a private message.

/broadcast <message>: Send a message to all users.

/create_group <groupname>: Create a new group chat.

/join_group <groupname>: Join an existing group chat.

/leave_group <groupname>: Leave a group chat.

/group_msg <groupname> <message>: Send a message to a group.

/exit: Disconnect from the server

Authentication: If the username does not exist or the password corresponding to the username is incorrect, the server will send the message "Authentication Failed" and disconnect the client. If the authentication succeeds but another process is logged in with the same username, then the server will send message "user already logged in" and disconnect the client.

While typing a command, if a username types a command which does not fall in any of the types given, the server will send "invalid command".

/msg:The username field is the username of the reciever to get the private message. If there is no one logged in with that username then the sender gets the message "<username> is not logged in". Otherwise the reciever gets the message "[<senderusername>]:<msg>"

/broadcast: It sends the corresponding message to all users who are logged in except the sender. 

/create_group The group is created for anyone to join and all the users who are logged in are sent the message, "[<username>] created group <groupname>". In case a group with the same name is already created then the user is informed accordingly.

/join_group If the user has already joined the group or the group doesn't exist, then the server sends the corresponding message. Else every other member of the group is sent the message that user has joined the group.

/leave_group If the user is not in the group or the group doesn't exist then the server sends the corresponding message to the user.
Else the server sends the message "<username> has left the group <groupname>"

/group_msg: If the group exists and the user belongs to the group, then all the other users are sent the corresponding message.

Implementation and design:
For storing data regarding groups, users and sockets we used unordered_maps.
users:Stores the mapping of users to passwords.(loaded from text file).
client_names:mainatains the mapping of sockets to clients.
client_id:maintains the mapping of clients to sockets.
group_members:maintians the mapping of group to set of clients in the group

We make use of 2 mutex locks, client_mutex and group_mutex.
These locks are put at all pieces of code where these map values are being changed or accessed. For example, once authentication is done, the code checks if the client is not logged in already and logs in the user.A lock has to be put on this part to prevent two processess logging in with the same username.

Functions:
load_txt_file(filename, users):loads all the username and password data from the txt file into the users map.
check_valid_user(username, password):given a username and password, returns true if username exists and password matches the username, otherwise returns false.
send_broadcast_message(client, message): sends the message to all logged in users except the client who is calling the function.
handle_client(client_socket): first takes the username and password from the client to log in, and then handles all requests by client. For each client, there is a seperate thread running an instance of handle_client.
