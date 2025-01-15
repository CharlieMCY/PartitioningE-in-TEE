import os
import glob
 
def count_lines_in_folder(folder_path):
    total_lines = 0
    for path in glob.glob(os.path.join(folder_path + '/*')):
        if os.path.isdir(path):
            total_lines += count_lines_in_folder(path)
    for file_path in glob.glob(os.path.join(folder_path, '*.c')):
        with open(file_path, 'r') as file:
            total_lines += sum(1 for line in file)
    for file_path in glob.glob(os.path.join(folder_path, '*.h')):
        with open(file_path, 'r') as file:
            total_lines += sum(1 for line in file)
    return total_lines
 

project_path = 'benchmark'
 
for path in glob.glob(os.path.join(project_path + '/*')):
    if os.path.isdir(path):
        lines = count_lines_in_folder(path)
        print(path + ':' + str(lines))