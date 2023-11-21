def generate_large_text_file(file_path, file_size_gb):
    chunk_size = 1 * 1024 * 1024  # 1MB chunk size
    target_file_size = file_size_gb * 1024 * 1024 * 1024  # Convert GB to bytes

    with open(file_path, 'w') as file:
        while file.tell() < target_file_size:
            chunk_text = 'abcde ' * chunk_size
            file.write(chunk_text)

file_path = "large_text_file.txt"
file_size_gb = 0.5

generate_large_text_file(file_path, file_size_gb)
print("done")
