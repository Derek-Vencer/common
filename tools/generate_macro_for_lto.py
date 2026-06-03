import os
import fnmatch
import re
import sys


class FileProcessor:
    def __init__(self, search_dir, lds_file_path):
        self.search_dir = search_dir
        self.lds_file_path = lds_file_path
        self.file_index = {}
        self.o_files = []
        self.source_file_paths = []
        self.excluded_dirs = set()
        self.makefile_caches = {}
        self.modified_makefile_paths = []

    def load_file_index_in_directory(self):
        for root, dirs, files in os.walk(self.search_dir):
            dirs[:] = [d for d in dirs if not d.startswith('.') and d != 'out']
            for file in files:
                self.file_index[file] = os.path.join(root, file)

    def extract_o_files_with_wildcard_from_lds(self):
        all_o_files = set()
        pattern = r'(?:\w|/|\*|-|_|\?)+?\.o\b'
        with open(self.lds_file_path, 'r') as file:
            content = file.read()
            matches = re.findall(pattern, content)
            all_o_files.update(matches)
        self.o_files = sorted(all_o_files)

    def check_makefile_contain_specific_block(self, makefile_path):
        specific_ifneq_block = 'ifneq ($(LTO_CFLAGS),)\nKBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))\nKBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))\nendif'
        try:
            with open(makefile_path, 'r') as makefile:
                content = makefile.read()
            return specific_ifneq_block in content
        except FileNotFoundError:
            return False

    def find_excluded_dirs(self):
        for root, _, files in os.walk(self.search_dir):
            for file in files:
                if file == 'Makefile':
                    makefile_path = os.path.join(root, file)
                    if self.check_makefile_contain_specific_block(makefile_path):
                        self.excluded_dirs.add(root)

    def is_excluded(self, file_path):
        for excluded_dir in self.excluded_dirs:
            if excluded_dir in file_path:
                return True
        return False

    def find_source_file_paths_from_o_files(self):
        self.find_excluded_dirs()
        source_file_paths = set()
        source_extensions = ['.c', '.cpp']
        for o_file in self.o_files:
            base_name = os.path.splitext(o_file)[0]
            if '*' in base_name or '?' in base_name:
                for file_name, file_path in self.file_index.items():
                    for ext in source_extensions:
                        if fnmatch.fnmatch(file_name, base_name + ext):
                            if not self.is_excluded(file_path):
                                source_file_paths.add(file_path)
            else:
                for ext in source_extensions:
                    source_file_name = base_name + ext
                    if source_file_name in self.file_index:
                        file_path = self.file_index[source_file_name]
                        if not self.is_excluded(file_path):
                            source_file_paths.add(file_path)
        self.source_file_paths = sorted(source_file_paths)

    def add_macros_to_makefile(self):
        if not self.source_file_paths:
            return 0, 0

        best_pattern = re.compile(r'(\w+)best(\d+)')
        all_macros_to_add = set()

        for source_file_path in self.source_file_paths:
            relative_path = os.path.relpath(source_file_path, self.search_dir)
            parts = relative_path.split(os.sep)

            for i in range(len(parts)):
                current_dir = os.path.join(self.search_dir, *parts[:i + 1])
                makefile_path = os.path.join(current_dir, 'Makefile')

                if os.path.exists(makefile_path):
                    source_file_name = os.path.basename(source_file_path)
                    object_file_name = os.path.splitext(source_file_name)[0] + '.o'

                    best_match = best_pattern.search(object_file_name)
                    if best_match:
                        base_name = best_match.group(1)
                        macro_line = f"export CFLAGS_{base_name}$(CHIP).o += -fno-lto"
                    else:
                        macro_line = f"export CFLAGS_{object_file_name} +=  -fno-lto"

                    if makefile_path not in self.makefile_caches:
                        with open(makefile_path, 'r') as makefile:
                            original_content = makefile.read()
                        self.makefile_caches[makefile_path] = {
                            'original': original_content,
                            'to_write': set(),
                            'need_update': False
                        }

                    self.makefile_caches[makefile_path]['to_write'].add(macro_line)
                    all_macros_to_add.add(macro_line)

                    # 找到并处理 Makefile 后，不再查找子目录
                    break

        total_macro_added = len(all_macros_to_add)
        updated_count = 0

        for makefile_path, cache in self.makefile_caches.items():
            original_content = cache['original']
            to_write = cache['to_write']

            start_index = original_content.rfind("ifneq ($(LTO_CFLAGS),)")
            end_index = original_content.rfind("endif", start_index)

            if start_index != -1 and end_index != -1:
                existing_block = original_content[start_index + len("ifneq ($(LTO_CFLAGS),)"):end_index].strip().splitlines()
                existing_macros = set([line.strip() for line in existing_block if line.strip()])
            else:
                existing_macros = set()

            sorted_to_write = sorted(to_write)
            sorted_existing = sorted(existing_macros)

            if sorted_to_write != sorted_existing:
                cache['need_update'] = True
                updated_count += 1
                self.modified_makefile_paths.append(makefile_path)

            if cache['need_update']:
                if start_index != -1 and end_index != -1:
                    new_block = '\n'.join(sorted_to_write)
                    new_content = original_content[:start_index + len("ifneq ($(LTO_CFLAGS),)")] + '\n' + new_block + '\n' + original_content[end_index:]
                else:
                    new_content = original_content.rstrip() + f"\n\nifneq ($(LTO_CFLAGS),)\n" + '\n'.join(sorted_to_write) + "\nendif\n"

                with open(makefile_path, 'w') as makefile:
                    makefile.write(new_content)

        return updated_count, total_macro_added

    def save_modified_makefile_paths_to_file(self):
        with open('path.txt', 'w') as f:
            for path in self.modified_makefile_paths:
                f.write(path + '\n')

    def process_files(self):
        print("开始加载文件索引...")
        self.load_file_index_in_directory()
        indexed_files = len(set(self.file_index.values()))
        print(f"文件索引加载完成，共索引了 {indexed_files} 个不重复的文件。")

        print("开始从 .lds 文件中提取 .o 文件...")
        self.extract_o_files_with_wildcard_from_lds()
        extracted_o_files = len(self.o_files)
        print(f"提取 .o 文件完成，共提取了 {extracted_o_files} 个不重复的 .o 文件。")

        print("开始查找源码文件路径...")
        self.find_source_file_paths_from_o_files()
        found_source_files = len(self.source_file_paths)
        print(f"源码文件路径查找完成，共找到 {found_source_files} 个不重复的源码文件路径。")

        print("开始向 Makefile 文件中添加宏定义...")
        updated_count, total_macro_added = self.add_macros_to_makefile()
        if updated_count > 0:
            print(f"宏定义添加完成，共更新了 {updated_count} 个 Makefile 文件，添加了 {total_macro_added} 个不重复的宏定义。")
        else:
            print("未发现需要更新的 Makefile 文件，未添加宏定义。")

        self.save_modified_makefile_paths_to_file()


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("请在执行脚本时传入查找目录和 .lds 文件路径作为参数。")
        sys.exit(1)

    search_dir = sys.argv[1]
    lds_file_path = sys.argv[2]

    if not os.path.exists(search_dir):
        print(f"指定的查找目录 {search_dir} 不存在。")
        sys.exit(1)

    if not os.path.exists(lds_file_path):
        print(f"指定的 .lds 文件路径 {lds_file_path} 不存在。")
        sys.exit(1)

    processor = FileProcessor(search_dir, lds_file_path)
    processor.process_files()