import sys

files = sys.argv[1:]
for f in files:
    with open(f, "r") as fd:
        print(f'#line 0 "{f}"')
        print(fd.read())
