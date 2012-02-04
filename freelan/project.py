"""Project related classes and functions."""

import os
import sys
import fnmatch

import file_tools
import tools

class Project(object):
    """A class to handle projects."""

    def __init__(self, name, libraries, path=None):
        """Create a new Project reading from the specified path."""

        super(Project, self).__init__()

        self.name = name
        self.libraries = libraries

        if path is None:
            self.abspath = os.getcwd()
        else:
            self.abspath = os.path.normpath(os.path.join(os.getcwd(), path))

        if not hasattr(os, 'related'):
            if not self.abspath.startswith(os.getcwd()):
                raise ValueError('Invalid path: ' + self.abspath)
            self.path = self.abspath[len(os.getcwd()):] or '.'
        else:
            self.path = os.path.relpath(self.abspath)

class LibraryProject(Project):
    """A class to handle library projects."""

    def __init__(self, name, major, minor, libraries, source_files, include_path=None, path=None):
        """Create a new LibraryProject reading from the specified path."""

        super(LibraryProject, self).__init__(name, libraries, path)

        self.major = major
        self.minor = minor
        self.source_files = source_files

        if include_path is None:
            self.include_path = os.path.join(self.path, 'include', self.name)
        else:
            self.include_path = include_path

        # Scan for include files
        self.include_files = []

        for root, directories, files in os.walk(self.include_path):
            self.include_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.h', '*.hpp'])]

    def configure_environment(self, env):
        """Configure the given environment for building the current project."""

        _env = {
            'CPPPATH': [self.include_path],
            'LIBS': self.libraries
        }

        self.static_library = env.FreelanStaticLibrary(
            os.path.join(self.path, env.libdir),
            self.name,
            self.major,
            self.minor,
            self.source_files,
            **_env
        )

        self.shared_library = env.FreelanSharedLibrary(
            os.path.join(self.path, env.libdir),
            self.name,
            self.major,
            self.minor,
            self.source_files,
            **_env
        )

        if env.link == 'static':
            return self.static_library
        else:
            return self.shared_library

    def configure_install_environment(self, env):
        """Configure the given environment for installing the current project."""

        if env.link == 'static':
            libraries_install = env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.libdir), self.static_library)
        else:
            libraries_install = env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.libdir), self.shared_library)

        for include_file in self.include_files:
            libraries_install += env.Install(os.path.dirname(os.path.join(env['ARGUMENTS']['prefix'], include_file)), include_file)

        return libraries_install

    def configure_documentation_environment(self, env):
        """Configure the given environment for generating the documentation."""

        documentation = env.Doxygen('doxyfile')
        env.Depends(documentation, self.include_files)

        return documentation

    def Sample(self, libraries=None, path=None):
        """Build a sample project at the given path, or in the current directory if no path is specified."""

        if libraries is None:
            libraries = self.libraries[:]

        if path is None:
            name = os.path.basename(os.path.abspath(os.getcwd()))
        else:
            name = os.path.basename(os.path.abspath(path))

        return SampleProject(self, name, libraries, None, path)

    def __get_files(self):
        """Get the project source files."""

        return self.include_files + self.source_files

    files = property(__get_files, None, None, "All the project source files.")

class ProgramProject(Project):
    "A class to handle program projects."""

    def __init__(self, name, major, minor, libraries, source_files, include_path=None, path=None):
        """Create a new ProgramProject reading from the specified path."""

        super(ProgramProject, self).__init__(name, libraries, path)

        self.major = major
        self.minor = minor
        self.source_files = source_files

        if include_path is None:
            self.include_path = os.path.join(self.path, 'src')
        else:
            self.include_path = include_path

        # Scan for include files
        self.include_files = []

        for root, directories, files in os.walk(self.include_path):
            self.include_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.h', '*.hpp'])]

    def configure_environment(self, env):
        """Configure the given environment for building the current project."""

        _env = {
            'LIBS': self.libraries
        }

        self.program = env.FreelanProgram(
            os.path.join(self.path, env.bindir),
            self.name,
            self.source_files,
            **_env
        )

        return self.program

    def configure_install_environment(self, env):
        """Configure the given environment for installing the current project."""

        return env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.bindir), self.program)

    def __get_files(self):
        """Get the project source files."""

        return self.include_files + self.source_files

    files = property(__get_files, None, None, "All the project source files.")

class SampleProject(Project):
    """A class to handle samples."""

    def __init__(self, parent_project, name, libraries, source_files=None, path=None):
        """Create a new sample project."""

        super(SampleProject, self).__init__(name, libraries, path)

        self.parent_project = parent_project

        # Scan for source files
        if source_files is None:
            self.source_files = []

            for root, directories, files in os.walk(self.path):
                self.source_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.c', '*.cpp'])]
        else:
            self.source_files = source_files

    def configure_environment(self, env):
        """Configure the given environment for building the current project."""

        if env.link == 'static':
            parent_library = self.parent_project.static_library
        else:
            parent_library = [self.parent_project.name]

        _env = {
            'CPPPATH': [self.path, os.path.join(self.parent_project.abspath, 'include')],
            'LIBPATH': [self.path, os.path.join(self.parent_project.abspath, env.libdir)],
            'LIBS': self.libraries + parent_library
        }

        sample = env.FreelanProgram(
            self.path,
            self.name,
            self.source_files,
            **_env
        )

        return sample

