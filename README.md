# Final Project Team 43

## Assumptions
- IP address and port of naming server is known to all storage servers.
- Only text files are stored.
- Always setup and clean SS using python script only.
- Python script is just used to make arbitrary copies of the storage server and assign each server a unique port to communicate with NFS and to communicate with Clients, and also a unique SS_ID; and list them down in ss_config.txt (Assigning each SSi the port number for NFS = 1000 x (i + 1) + 500 and for client 1000 x (i + 1) + 501) Assuming all the port numbers fall under the permissible range otherwise just change the port assigning logic. (Using 500 offset because ports till 1024 are reserved)
- Sample command to create 10 SS : ```python3 setup.py 10``` This will create 10 SS
- setup_ss.py just creates n number of SS
- start_ss.py starts all the servers in different terminal windows
> Remember to change the operating system name in start_ss.py before running it
- The outer make file it to run both the python scripts together run as ```make n=3``` where n specifies the number of storage server you want to create and to delete all the files again just run ```make n=3 clean``` it would clear all the SS folders and related files. **Make sure that the value of n is same in both the cases**
- Using TCP to register SS with NFS instead of UDP because UDP can only transmit 64k at a time and TCP can transmit 1 Gigs and according to our requirement we need to send large amount of data which was not getting sent throught UDP so we had to switch to TCP. We can use UDP but for that we would have to first break our registration request into smaller parts and send each part separately and then assemble it again it is just some extra work which we could do and would do if time permits but as for now we are using TCP to register with NFS.
- If a client initiates a write request to a file on which some other client is already writing then the new client will have to wait till the previous one completes writing, after that only the new client's write request would proceed.
- Assuming all the accessible files are in the storage folder only.
- If you are running ss.c file in some other folder and running it there then you will have to manually update the pwd macro to the pwd of that headers.h
- Assuming that only files can be copied. As there can be multiple storage servers with same folder names but it is guranteed to have a unique file path for each file which may not be unique for each subpath or path to each folder so it would not be possible to figure out what the client exactly wants and which folder is to be copied.
- Assuming if a file is being copied, there won't be already a file with exactly the same relative path, and if there is it's data will be overwritten
- Assuming all the files ends with .txt (all are text files) and no folder name ends with .txt
- Got the basic tries code from chatGPT prompt : "Write me a tries code in C to store directory structure such that each node of trie contains name of a file/directory and points to next files in the directory. Write code for effective search, insert and delete", and then changed it according to our requirement.