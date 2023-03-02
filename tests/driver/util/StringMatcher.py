from __future__ import annotations
import sys
import util.FileHelper as FileHelper
from typing import Dict, Generator, List, Set, Union
from dataclasses import dataclass
import os
import enum
import re
import argparse as ap
import multiprocessing as mp
import glob
import itertools


class StringMatcher:
    _pattern: Union[str, re.Pattern]
    _original_pattern: str
    # global flags for the matched
    _flags: StringMatcher.Flags

    class Flags(enum.IntFlag):
        """treat a bare string as a regex"""

        REGEX = enum.auto()
        """make the match case insensitive"""
        IGNORE_CASE = enum.auto()
        """matches the string exactly"""
        EXACT = enum.auto()
        """ignores leading and trailing whitespace in a match"""
        IGNORE_WHITESPACE = enum.auto()

    def __init__(self, pattern: str, flags: StringMatcher.Flags = 0):
        self._pattern = pattern
        self._original_pattern = pattern
        self._flags = flags

        # if a regex, precompile it
        if self._flags & StringMatcher.Flags.REGEX:
            compile_flags = (
                re.IGNORECASE if self._flags & StringMatcher.Flags.IGNORE_CASE else 0
            )
            self._pattern = re.compile(self._pattern, flags=compile_flags)

        # if its a plain string and ignore case, pre lower the pattr
        if (
            not (self._flags & StringMatcher.Flags.REGEX)
            and self._flags & StringMatcher.Flags.IGNORE_CASE
        ):
            self._pattern = self._pattern.lower()

    def __eq__(self, other: str) -> bool:
        return isinstance(other, str) and bool(self.match(other))

    def match(self, string: str) -> Union[List[str], None]:
        """
        matches string against the pattern
        returns the matched part of the string, or NONE
        for regex, returns a list of groups
        """
        # serves as dispatcher for internal methods

        # both internal methods need to remove whitespace to do this
        if self._flags & StringMatcher.Flags.IGNORE_WHITESPACE:
            string = string.strip()

        return self._regex_match(string) if self.isRegex else self._str_match(string)

    def _regex_match(self, string: str) -> Union[List[str], None]:
        """internal method, performs regex match"""

        m = None
        if self._flags & StringMatcher.Flags.EXACT:
            m = self._pattern.fullmatch(string)
        else:
            m = self._pattern.search(string)

        if m:
            g = list(m.groups())
            if len(g) == 0:
                return m.group(0)  # return the matched string
            else:
                return g  # return the list of matched strings
        else:
            return None

    def _str_match(self, string: str) -> Union[List[str], None]:
        """internal method, performs string match"""

        # if IGNORE_CASE, need to lower string
        if self._flags & StringMatcher.Flags.IGNORE_CASE:
            string = string.lower()

        matches = False
        if self._flags & StringMatcher.Flags.EXACT:
            matches = self._pattern == string
        else:
            matches = self._pattern in string

        if matches:
            return self._original_pattern
        else:
            return None

    def isRegex(self):
        return isinstance(self._pattern, re.Pattern)
