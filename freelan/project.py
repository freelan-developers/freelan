"""Project related classes and functions."""

import os
import sys
import fnmatch

try:
    import json
except ImportError:
    import simplejson as json

import file_tools
import tools

class Project(object):
    """A class to handle project attributes."""

    def __init__(self, path=None):
        """Create a new Project reading from the specified path."""

        super(Project, self).__init__()

        if path is None:
            self.abspath = os.getcwd()
        else:
            self.abspath = os.path.normpath(os.path.join(os.getcwd(), path))

        if not hasattr(os, 'related'):
            if not self.abspath.startswith(os.getcwd()):
                raise ValueError('Invalid path: ' + self.abspath)
            self.path = self.abspath[len(os.getcwd()):]
        else:
            self.path = os.path.relpath(self.abspath)

        # Set the project file
        self.project_file = os.path.join(self.path, 'project.json')

        # Load the project file
        self.attributes = json.loads(open(self.project_file).read())

    def get_libraries(self, platform=None):
        """Return the list of used libraries."""

        if platform is None:
            platform = sys.platform

        return sorted(set(self.attributes['libraries'].get('*', []) + self.attributes['libraries'].get(platform, [])))

    libraries = property(get_libraries)

class LibraryProject(Project):
    """A class to handle library project attributes."""

    def __init__(self, path=None, include_path=None, source_path=None):
        """Create a new LibraryProject reading from the specified path."""

        super(LibraryProject, self).__init__(path)

        if include_path is None:
            self.include_path = os.path.join(self.path, 'include', self.attributes['name'])
        else:
            self.include_path = include_path

        if source_path is None:
            self.source_path = os.path.join(self.path, 'src')
        else:
            self.source_path = source_path

        # Scan for include files
        self.include_files = []

        for root, directories, files in os.walk(self.include_path):
            self.include_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.h', '*.hpp'])]

        # Scan for source files
        self.source_files = []

        for root, directories, files in os.walk(self.source_path):
            self.source_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.c', '*.cpp'])]

    def configure_environment(self, env):
        libraries = env.FreelanLibrary(
            os.path.join(self.path, env.libdir),
            self.attributes['name'],
            self.attributes['major'],
            self.attributes['minor'],
            self.include_path,
            self.source_files,
            self.libraries
        )

        libraries_install = env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.libdir), libraries)

        for include_file in self.include_files:
            libraries_install += env.Install(os.path.dirname(os.path.join(env['ARGUMENTS']['prefix'], include_file)), include_file)

        documentation = env.Doxygen('doxyfile')
        env.AlwaysBuild(documentation)
        indentation = env.AStyle(self.source_files + self.include_files)
        env.AlwaysBuild(indentation)

        env.Alias('build', libraries)
        env.Alias('install', libraries_install)
        env.Alias('doc', documentation)
        env.Alias('indent', indentation)

        env.Default('build')
