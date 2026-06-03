import os
import sys


def remove_ifneq_block(file_path):
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()

        endif_index = -1
        ifneq_index = -1
        for i in range(len(lines) - 1, -1, -1):
            line = lines[i].strip()
            if line == "endif" and endif_index == -1:
                endif_index = i
            elif line.startswith("ifneq ($(LTO_CFLAGS),)") and ifneq_index == -1 and endif_index != -1:
                ifneq_index = i
                break

        if ifneq_index != -1 and endif_index != -1:
            new_lines = lines[:ifneq_index] + lines[endif_index + 1:]
            if new_lines and new_lines[-1].strip() == "":
                new_lines = new_lines[:-1]
            with open(file_path, 'w') as file:
                file.writelines(new_lines)
    except Exception:
        pass


def process_path_file(directory):
    path_file_path = os.path.join(directory, 'path.txt')
    if not os.path.exists(path_file_path):
        return

    try:
        with open(path_file_path, 'r') as path_file:
            for line in path_file:
                file_path = line.strip()
                if not os.path.isabs(file_path):
                    file_path = os.path.join(directory, file_path)
                if os.path.exists(file_path):
                    remove_ifneq_block(file_path)
    except Exception:
        pass


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("请传入一个目录作为参数，例如: python script.py /your/target/directory")
        sys.exit(1)

    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"{directory} 不是一个有效的目录，请检查。")
        sys.exit(1)

    print(f"开始处理目录 {directory} 下的 path.txt 文件")
    process_path_file(directory)
    print(f"完成对目录 {directory} 下 path.txt 文件中路径对应文件的处理")