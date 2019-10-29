import re
import glob
import sys

add = []
separator = '|'

for file in glob.iglob('**/CMakeFiles', recursive=True):
    add.append(file)

for file in glob.iglob('**/Makefile', recursive=True):
    add.append(file)

for file in glob.iglob('**/CMakeCache.txt', recursive=True):
    add.append(file)

for file in glob.iglob('**/CMakeLists.txt', recursive=True):
    add.append(file)

for file in glob.iglob('**/cmake_install.cmake', recursive=True):
    add.append(file)

for file in glob.iglob('**/Release', recursive=True):
    add.append(file)

with open(str(sys.argv[1]), 'r+') as f:
    first = True
    old = f.readlines()
    f.seek(0)
    for line in old:
        bingo = re.findall(r'.*\<entry excluding=\"(.*)\" flags=\"VALUE_WORKSPACE_PATH\|RESOLVED\".*', line)
        if bingo and first:
            first = False
            exclude = re.split('\|',bingo[0])
            for path in exclude:
                if ("CMakeFiles" in path) or \
                ("Makefile" in path) or \
                ("CMakeCache.txt" in path) or \
                ("CMakeLists.txt" in path) or \
                ("cmake_install.cmake" in path) or \
                ("Release" in path):
                    continue
                else:
                    add.append(path)
            add = separator.join(add)
            line = re.sub(r'(.*)\<entry excluding=\"(.*)\" flags=\"VALUE_WORKSPACE_PATH\|RESOLVED\"(.*)', r'\1<entry excluding="' + add + r'" flags="VALUE_WORKSPACE_PATH|RESOLVED"\3', line)
            f.write(line)
        else:
            f.write(line)
