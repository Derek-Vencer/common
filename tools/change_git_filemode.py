import os


def modify_git_config(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()

        modified = False
        new_lines = []
        for line in lines:
            if 'filemode =' in line:
                if 'filemode = true' in line:
                    new_line = line.replace('filemode = true', 'filemode = false')
                    new_lines.append(new_line)
                    modified = True
                else:
                    new_lines.append(line)
            else:
                new_lines.append(line)

        if modified:
            with open(file_path, 'w', encoding='utf-8') as file:
                file.writelines(new_lines)
    except Exception as e:
        print("Error processing {file_path}: {e}")


def find_and_modify_git_configs(root_dir):
    for root, dirs, files in os.walk(root_dir):
        if '.git' in dirs:
            git_dir = os.path.join(root, '.git')
            config_path = os.path.join(git_dir, 'config')
            if os.path.isfile(config_path):
                modify_git_config(config_path)


if __name__ == "__main__":
    current_dir = os.getcwd()
    find_and_modify_git_configs(current_dir)