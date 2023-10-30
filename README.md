# Final Project Team 43

## Assumptions
- IP address and port of naming server is known to all storage servers.
- Only text files are stored.
- Always setup and clean SS using python script only.
- Python script is just used to make arbitrary copies of the storage server and assign each server a unique port to communicate with NFS and to communicate with Clients and list them down in ss_config.txt (Assigning each SSi the port number for NFS = 1000 x (i + 1) + 500 and for client 1000 x (i + 1) + 501) Assuming all the port numbers fall under the permissible range otherwise just change the port assigning logic. (Using 500 offset because ports till 1024 are reserved)
- Sample command to create 10 SS : ```python3 setup.py 10``` This will create 10 SS
- setup_ss.py just creates n number of SS
- start_ss.py starts all the servers in different terminal windows
> Remember to change the operating system name in start_ss.py before running it