import os

"""
helper functions for files
"""


def getext(path: str) -> str:
    """Extract just the extension from a path"""
    return os.path.splitext(path)[1].strip(".")


def fileExists(path: str) -> bool:
    """check if a file exists"""
    return os.path.exists(path) and os.path.isfile(path)


def dirExists(path: str) -> bool:
    """check if a directory exists"""
    return os.path.exists(path) and os.path.isdir(path)


def makeabs(path: str) -> str:
    """turn a path into an absolute path"""
    path = os.path.expanduser(path)
    path = os.path.expandvars(path)
    path = os.path.realpath(path)
    path = os.path.abspath(path)
    path = os.path.normpath(path)
    return path
