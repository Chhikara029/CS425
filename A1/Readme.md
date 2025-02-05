This is a multi-threaded chat server implemented in C++. It allows multiple clients to communicate with each other through private messages, broadcasting, and group chats. The server handles user authentication using a predefined users.txt file and manages client connections using threads.

## Pre-requisites:
To run this, you need a linux-based operating system(or WSL on windows or MacOS), and a c++ compiler. You also need a textfile containing all registered users along with their password in the following format:

```
user1:password1
user2:password2
```
## Instructions:
1) Compile the code using makefile.
2) Run the server code
3) In another terminal, run the client and correctly enter your username and then password.
4) After authentication the client can do the following:

### Commands Supported
Once authenticated, they can use commands to communicate:

```
/msg <username> <message>: Send a private message.

/broadcast <message>: Send a message to all users.

/create_group <groupname>: Create a new group chat.

/join_group <groupname>: Join an existing group chat.

/leave_group <groupname>: Leave a group chat.

/group_msg <groupname> <message>: Send a message to a group.

/exit: Disconnect from the server
```

## Design/Implementation

##### Authentication:
If the username does not exist or the password corresponding to the username is incorrect, the server will send the message "Error Authentication Failed" and disconnect the client. If the authentication succeeds but another client is logged in with the same username, then the server will send message "Error: user already logged in" and disconnect the client. Otherwise the server will send the message "Authentication Successful" and log in that client with the username.

While typing a command, if a client types a command which does not fall in any of the types given, the server will send "invalid command".

- /msg:
The username field is the username of the reciever to get the private message. If there is no one logged in with that username then the sender gets the message "Error: {username} is not logged in". Otherwise the reciever gets the message "{senderusername}: {msg}>"

- /broadcast:
It sends the corresponding message to all users who are logged in except the sender. 

- /create_group:
The group is created for anyone to join and all the users who are logged in are sent the message, "[{username}] created group {groupname}". In case a group with the same name is already created then the user gets the message "Group already exists".

- /join_group:
 If the user has already joined the group or the group doesn't exist, then the server sends the corresponding error message. Else the message that the user has joined the group is broadcasted to all the users.

- /leave_group:
 If the user is not in the group or the group doesn't exist then the server sends the corresponding message to the user. Else the server broadcasts the message "{username} has left the group {groupname}"

- /group_msg:
If the group exists and the user belongs to the group, then all the other users are sent the corresponding message.


### Implementation and design:

#### Data Management
For storing data regarding groups, users and sockets we used unordered_maps.
- users: Stores the mapping of users to passwords.(loaded from text file).
- client_names: maintains the mapping of sockets to clients.
- client_id: maintains the mapping of clients to sockets.
- group_members: maintains the mapping of group to set of clients in the group

#### Lock Implementation
We make use of 2 mutex locks, client_mutex and group_mutex.
These locks are put at all pieces of code where these map values are being changed or accessed. For example, once authentication is done, the code checks if the client is not logged in already and logs in the user.A lock has to be put on this part to prevent two processess logging in with the same username.

### Threading
There is a main thread which is responsible for accepting connections from clients. Each time a client socket is connected, a new thread is created which handles the requests for that socket.
### Functions:

- load_txt_file(filename, users):loads all the username and password data from the txt file into the users map.
- check_valid_user(username, password):given a username and password, returns true if username exists and password matches the username, otherwise returns false.
- send_broadcast_message(client, message): sends the message to all logged in users except the client who is calling the function.
- handle_client(client_socket): first takes the username and password from the client to log in, and then handles all requests by client. For each client, there   is a seperate thread running an instance of handle_client.

#### Notes
- Once a group is created, it remains created as long as the server is running(even if all members have left the group).
- If a user disconnects, they still remain a part of any groups they were a part of, but they won't have access to the messages sent while they were offline.

#### Server Code Flow
```
                                             +-----------------------------------------------------------+
                                             |                         Start                             |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                   Create Server Socket                    |
                                             |                    Set socket options                     |
                                             |                 Bind server to port 12345                 |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |             Listen for client connections                 | <---
                                             +-----------------------------------------------------------+    |
                                                                         |                                    |
                                                                         v                                    |
                                             +-----------------------------------------------------------+    |
                                             |                  Accept client connection                 |    |
                                             +-----------------------------------------------------------+    |
                                                                         |                                    |
                                                                         v                                    |
                                             +-----------------------------------------------------------+    |
                                             |          Create a new thread to handle the client         |----|
                                             +-----------------------------------------------------------+
                                                                         | corresponding thread
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                     Receive Username                      |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |               +----------------------------+              |
                                             |               |          Error?            |              |
                                             |               +----------------------------+              |
                                             +---------------------------+-------------------------------+
                                                                         |
                                                                         v
                                              +-----------------------------------------------------------+
                                              |                          Yes                              |
                                              |                +--------------------+                     |
                                              |                |   Authentication   |                     |
                                              |                |   Error: Username  |                     |
                                              |                |   Invalid/Empty    |                     |
                                              |                +--------------------+                     |
                                              |                          |                                |
                                              |                          v                                |
                                              |                   Close Connection                        |
                                              +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                    Receive Password                       |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |               +----------------------------+              |
                                             |               |          Error?            |              |
                                             |               +----------------------------+              |
                                             +---------------------------+-------------------------------+
                                                                         |
                                                                         v
                                              +-----------------------------------------------------------+
                                              |                         Yes                               |
                                              |                +--------------------+                     |
                                              |                |   Authentication   |                     |
                                              |                |   Error: Password  |                     |
                                              |                |   Invalid/Empty    |                     |
                                              |                +--------------------+                     |
                                              |                           |                               |
                                              |                           v                               |
                                              |                    Close Connection                       |
                                              +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                  Check Credentials                        |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |               +----------------------------+              |
                                             |               |    Credentials Valid?      |              |
                                             |               +----------------------------+              |
                                             +---------------------------+-------------------------------+
                                                                         |
                                                                         v
                                              +-----------------------------------------------------------+
                                              |                         No                                |
                                              |                 +--------------------+                    |
                                              |                 |   Authentication   |                    |
                                              |                 |   Error: Invalid   |                    |
                                              |                 |   Credentials      |                    |
                                              |                 +--------------------+                    |
                                              |                           |                               |
                                              |                           v                               |
                                              |                    Close Connection                       |
                                              +-----------------------------------------------------------+
                                                                          |
                                                                          v
                                              +------------------------------------------------------------+
                                              |                     Proceed to Chat                        |
                                              +------------------------------------------------------------+
                                                                          |
                                                                          v
                                             +-----------------------------------------------------------+
                                             |                     Process Commands                      |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |               +---------------------------+               |
                                             |               | /msg <username>           |               |
                                             |               | /broadcast <message>      |               |
                                             |               | /create_group <group_name>|               |
                                             |               | /join_group <group_name>  |               |
                                             |               | /leave_group <group_name> |               |
                                             |               | /group_msg <group_name>   |               |
                                             |               | /exit                     |               |
                                             |               +---------------------------+               |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                  Exit Command or error?                   |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |             +--------------------------+                  |

                                             |             | - Remove user from lists |                  |
                                             |             | - Send exit broadcast    |                  |
                                             |              +-------------------------+                  |
                                             +-----------------------------------------------------------+
                                                                         |
                                                                         v
                                             +-----------------------------------------------------------+
                                             |                      End  Thread                          |
                                             +-----------------------------------------------------------+
```

## Testing

### Testing Process

We primarily conducted **manual testing** to verify the functionality of the client and server interactions. The key focus areas were authentication, message broadcasting, command handling, and concurrency.

#### **Manual Testing Approach**
1. **Basic Functionality**:
   - Launched the server and multiple clients.
   - Logged in with different credentials and tested authentication.
   - Sent various commands to check proper execution.

2. **Concurrency Testing**:
   - Ran multiple clients simultaneously to evaluate server stability.
   - Observed how the server handled concurrent message broadcasts.

3. **Edge Cases Tested**:
   - Attempted incorrect login credentials to verify authentication failure.
   - Sent malformed commands to test error handling.
   - Tested abrupt client disconnections and their impact on the server.

### Major Identified Bugs

#### **1. Server Shutdown on Ctrl+C Before Authentication**
- **Issue**: If a client is launched but terminates (e.g., via `Ctrl+C`) before authentication, the server shuts down unexpectedly.
- **Cause**: The server does not handle unexpected client disconnections properly at the authentication stage.
- **Solution**: Whenever the recv protocol ges a negative return value, the server code sets the client data to not logged in and that thread returns.

#### **2. Incorrect Command Handling**
- **Issue**: If user enters an invalid command like `/broadcastabc`, the server incorrectly interprets it as `/broadcast bc` and sends `bc` instead of throwing an error.
- **Cause**: The command parsing logic only checks for the prefix (`/broadcast`) but does not validate additional characters.
- **Solution**: We ensured that the code properly parses the string.


## Limitations of code/design

#### Scalability
- As the buffer size is defined as 1024, the maximum message size is 1024 characted. Messages longer than this size will be truncated.

- The server creates a new thread for each client connection, leading to potential scalability issues.

- There is no limit on the number of concurrent clients, but excessive connections may exhaust system resources.

#### Security Concerns

##### No encryption:
- Passwords and messages are sent in plaintext over TCP.
- Anyone sniffing the network (e.g., using Wireshark) can read credentials.
##### Prone to brute force:
- The system is vulnerable to brute-force attacks on passwords.
##### Denial of Service (DoS) Attack Risk:
- A malicious client could keep reconnecting, consuming thread resources.

## Team Members
| Name   | Roll No.   | Email   |
|------------|------------|------------|
| Harshit | 220436 | harshit22@iitk.ac.in |
| Mridul Gupta | 220672 | mridulg22@iitk.ac.in |
| Nipun Nohria | 220717 | nipun22@iitk.ac.in |

## Contribution
In the assignment, all members contributed equally, with specific roles as follows:
- Design: Nipun and Mridul handled the design along with inputs from Harshit.
- Implementation: Most of the code was jointly written by all. Harshit created the initial draft of the code. Mridul and Nipun jointly tested and implemented the features mentioned in the assignment.
- Testing: Mridul and Nipun took the lead in testing the solution along with Harshit.
- Readme: All three members (Harshit, Mridul, Nipun) contributed to preparing the Readme.

## Sources

Codes and examples from class notes

## Declaration

We all declare that we have not indulged in any form of plagiarism

