import os
import sys
import subprocess
import shutil
import re

def update_header_and_config_files(i: int, base_dir: str) -> None:
    # Now assigning the port numbers to this Storage server
    input_file = base_dir + "/headers.h"    # File in which these macros are stored
    # Defining regex to identify the line
    pattern1 = r'^\s*#define\s*MY_NFS_PORT_NO.*$'   
    pattern2 = r'^\s*#define\s*MY_CLIENT_PORT_NO.*$'
    
    # Read the content of the input file
    with open(input_file, 'r') as file:
        lines = file.readlines()

    # Modify the lines to replace #define with #ifndef
    modified_lines = [re.sub(pattern1, f'#define MY_NFS_PORT_NO {1000 * (i + 1) + 500}', line) for line in lines]
    modified_lines = [re.sub(pattern2, f'#define MY_CLIENT_PORT_NO {1000 * (i + 1) + 501}', line) for line in modified_lines]
    
    # Write the modified content back to the same file
    with open(input_file, 'w') as file:
        file.writelines(modified_lines)
        
    with open("ss_config.txt", "a") as config_file:
        config_file.write(f"SS {i + 1}:\nNFS_PORT_NO_SS{i + 1} = {1000 * (i + 1) + 500}\nCLIENT_PORT_NO_SS_{i + 1} = {1000 * (i + 1) + 501}\n\n")

def run_clean(i: int) -> None:
    os.chdir(f"SS{i + 1}")
    subprocess.run(['make', 'clean'],  check = True)
    os.chdir("..")

def compile_make(i: int) -> None:
    os.chdir(f"SS{i + 1}")
    subprocess.run(['make'],  check = True)
    os.chdir("..")
    
def copy_folder_n_times(num_of_ss: int, cwd: str) -> None:
    source_dir = cwd + "/SS_to_copy"

    for i in range(num_of_ss):
        dest_dir = cwd + f"/SS{i + 1}"
        try:
            shutil.copytree(source_dir, dest_dir)
        except FileExistsError as e:
            pass
        finally:
            pass
    
def create_dir(path: str) -> None:
    # Try to create the directory and if it already exists then just ignore and continue
    try:
        os.mkdir(path)
    except FileExistsError as e:
        pass
    finally:
        pass
    
def create_file(path: str, curr_file_index: int) -> None:
    try:
        with open(path, "x") as file:
            file.write(f"This is sample text in SS {i + 1} and file {curr_file_index}")
    except FileExistsError:
        pass
    
# Clearing the config file
with open("ss_config.txt", "w") as config_file:
    pass

num_of_ss: int = 10     # Number of storage servers available
num_of_dir: int = 3     # Number of dir in each test_dir
max_num_of_files_in_dir: int = 2   # Number of files in each dir
total_num_of_files: int = 5 # Total number of test files in the SS

# Flag to check whether we need to compile all the make files or clean them
run_clean: int = 0

# If any of these arguments are not provided as command line arguments then the above values will be taken as default
if len(sys.argv) >= 2:
    num_of_ss = int(sys.argv[1])

if len(sys.argv) >= 3:
    num_of_dir = sys.argv[2]
    # If clean is passed as the second argument then cleans all the compiled files
    if (num_of_dir == "clean"):
        run_clean = 1
    else:
        num_of_dir = int(num_of_dir)

if len(sys.argv) >= 4:
    max_num_of_files_in_dir = int(sys.argv[3])
    
if len(sys.argv) >= 5:
    total_num_of_files = int(sys.argv[4])

# Getting the path to current working directory
cwd = os.getcwd()

# Copying the SS directory into SS1, SS2, SS3 ... till SSn
copy_folder_n_times(num_of_ss, cwd)

# Looping through all the storage server folders to create test directories and files
for i in range(num_of_ss):
    # If run_clean is 0 then we run the makefile to compile the code
    if run_clean == 0:
        paths_to_be_stored: list = list()  # Stores the list of all the paths accessible to the SS which will later be written onto the paths.txt file
        
        curr_file_index: int = 1
        
        base_dir = os.path.join(cwd, f"SS{i + 1}")
        
        update_header_and_config_files(i, base_dir)
        
        test_dir_path = os.path.join(base_dir, f"SS{i + 1}_test_dir")
        create_dir(test_dir_path)
        
        # Creating multiples directories in the test directory
        for j in range(num_of_dir):
            dir_path = os.path.join(test_dir_path, f"SS{i + 1}_dir{j + 1}")
            create_dir(dir_path)
            
            # Creating sample files with sample text in each dir
            for k in range(max_num_of_files_in_dir):
                file_path = os.path.join(dir_path, f"SS{i + 1}_file{curr_file_index}.txt")
                if curr_file_index <= total_num_of_files:
                    
                    path_to_store = f"./SS{i + 1}_dir{j + 1}/SS{i + 1}_file{curr_file_index}.txt"
                    paths_to_be_stored.append(path_to_store)
                    create_file(file_path, curr_file_index)
                    curr_file_index += 1
        
        # Writing stored paths onto the file
        with open(f"./SS{i + 1}/paths.txt", "w") as paths_file:
            for path in paths_to_be_stored:
                paths_file.write(path)
                paths_file.write("\n")
                
        # Compiling all the makefiles
        compile_make(i)
        
    else:
        run_clean(i)
