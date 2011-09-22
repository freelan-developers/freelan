"""Project related classes and functions."""

import os
import fnmatch
import json
import file_tools

class Project(object):
    """A class to handle project attributes."""

    def __init__(self, path=None):
        """Create a new Project reading from the specified path."""

        super(Project, self).__init__()

        if path is None:
            path = os.getcwd()

        # Set the project file
        self.project_file = os.path.join(path, 'project.json')

        # Load the project file
        attributes = json.loads(open(self.project_file).read())
        
        # Add attributes to the Project instance
        for key, value in attributes.items():
            if key == 'project_file':
                raise RuntimeError('Invalid \'%s\' attribute in %s: keyword is reserved.' % (key, self.project_file))

            self.__setattr__(key, value)

class LibraryProject(Project):
    """A class to handle library project attributes."""

    def __init__(self, path=None, include_dir=None, source_dir=None):
        """Create a new LibraryProject reading from the specified path."""
        super(LibraryProject, self).__init__(path)

        if include_dir is None:
            include_dir = os.path.join('include', self.name)
        if source_dir is None:
            source_dir = os.path.join('src')

        # Scan for include files
        self.include_files = []

        for root, directories, files in os.walk(include_dir):
            self.include_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.h', '*.hpp'])]

        # Scan for source files
        self.source_files = []

        for root, directories, files in os.walk(source_dir):
            self.source_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.c', '*.cpp'])]
