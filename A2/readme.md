# CS425 A2
In this assignment we implement a DNS resolution system that supports both iterative and recursive lookups.

# Group Members
1. Nipun Nohria (220717)
2. Mridul Gupta (220672)
3. Harshit (220436)

# Features
1. Recursive Querying
-This method uses the system's default DNS resolver which fetches the query recursively.
-The host queries the local server, which then queries the root server. The root server queries the TLD which queries the author
atative server. The resolved IP address is propagated back to the host.

2. Iterative Querying
-The local server itself queries the name servers successively, starting with the root server.

Additionally, the application also displays the NS hostnames at each level and the execution time.

# Instructions
1. Ensure that python and it's library dnspython are installed in your system.
2. To run the script, type the following command in the terminal:
   python3 dns_server.py <mode> <domain>
   mode can be "recursive" or "iterative". For example, 
   python3 dns_server.py iterative google.command


# Changes made to code
1. For the TODO in send DNS query, we just return the response to dns.query.udp(query, server) . 
2. For the TODO in extract_next_nameserver, we run a for loop on each line of the response and use
 the library resolver function to resolve the host names to address.
3. In the function for iterative dns lookup, used if-else logic to move to the next stage of resolution after next set of hostnames is recieved.
4. In the function for recursive dns lookup, added answer = dns.resolver.resolve(domain, "NS") to resolve the host names from
 
