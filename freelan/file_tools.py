"""Files related functions."""

import fnmatch
import re

def filter(files, patterns):
    """Filter a list of files, keeping only those which match one of the specified patterns."""
    
    if not isinstance(patterns, basestring):
        regex = re.compile('|'.join(fnmatch.translate(pattern) for pattern in patterns))

        return [file for file in files if regex.match(file)]
    else:
        return fnmatch.filter(files, patterns)
