import sys
files = sys.argv[1:]
for f in files:
    with open(f, 'r') as fd:
        print(fd.read())
