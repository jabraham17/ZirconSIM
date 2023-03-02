import subprocess as sp
from typing import List, Tuple
import shlex


def execute(args: List[str], stdin: str = None) -> Tuple[int, str]:
    cp = sp.run(
        args, stdout=sp.PIPE, stderr=sp.STDOUT, input=stdin, text=True, errors="ignore"
    )
    return (cp.returncode, cp.stdout)


# def getResult(cmd: str):
#     """execute a simple command and get the result"""
#     cmd_args = shlex.split(cmd)
#     return sp.get(cmd_args)
