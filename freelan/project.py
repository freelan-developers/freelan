"""Project related classes and functions."""

import os
import sys
import fnmatch

import file_tools
import tools

class Project(object):
    """A class to handle projects."""

    def __init__(self, path, name, libraries):
        """Create a new Project reading from the specified path."""

        super(Project, self).__init__()

        self.path = path
        self.name = name
        self.libraries = libraries

class LibraryProject(Project):
    """A class to handle library projects."""

    def __init__(self, path, name, major, minor, libraries, source_files, include_path=None):
        """Create a new LibraryProject reading from the specified path."""

        super(LibraryProject, self).__init__(path, name, libraries)

        self.major = major
        self.minor = minor
        self.source_files = source_files

        if include_path is None:
            self.include_path = os.path.join(self.path.srcnode().abspath, 'include', self.name)
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
            os.path.join(self.path.abspath, env.libdir),
            self.name,
            self.major,
            self.minor,
            self.source_files,
            **_env
        )

        # For now on, we disable shared library generation on NT systems
        if sys.platform.startswith('win32'):
            self.shared_library = []
        else:
            self.shared_library = env.FreelanSharedLibrary(
                os.path.join(self.path.abspath, env.libdir),
                self.name,
                self.major,
                self.minor,
                self.source_files,
                **_env
            )

        return self.static_library + self.shared_library

    def configure_install_environment(self, env):
        """Configure the given environment for installing the current project."""

        libraries_install = env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.libdir), self.static_library)
        libraries_install += env.Install(os.path.join(env['ARGUMENTS']['prefix'], env.libdir), self.shared_library)

        for include_file in self.include_files:
            include_file = os.path.relpath(include_file, self.path.srcnode().abspath)
            install_include_file = env.Install(os.path.dirname(os.path.join(env['ARGUMENTS']['prefix'], include_file)), include_file)
            libraries_install.extend(install_include_file)

        return libraries_install

    def configure_documentation_environment(self, env):
        """Configure the given environment for generating the documentation."""

        documentation = env.Doxygen('doxyfile')
        env.Depends(documentation, self.include_files)

        return documentation

    def Sample(self, path, libraries=None):
        """Build a sample project at the given path, or in the current directory if no path is specified."""

        if libraries is None:
            libraries = self.libraries[:]

        name = os.path.basename(os.path.abspath(path.abspath))

        return SampleProject(path, self, name, libraries, None)

    def __get_files(self):
        """Get the project source files."""

        return self.include_files + self.source_files

    files = property(__get_files, None, None, "All the project source files.")

class ProgramProject(Project):
    "A class to handle program projects."""

    def __init__(self, path, name, major, minor, libraries, source_files, include_path=None):
        """Create a new ProgramProject reading from the specified path."""

        super(ProgramProject, self).__init__(path, name, libraries)

        self.major = major
        self.minor = minor
        self.source_files = source_files

        if include_path is None:
            self.include_path = os.path.join(self.path.srcnode().abspath, 'src')
        else:
            self.include_path = include_path

        # Scan for include files
        self.include_files = []

        for root, directories, files in os.walk(self.include_path):
            self.include_files.extend(file_tools.filter(files, ['*.h', '*.hpp']))

    def configure_environment(self, env):
        """Configure the given environment for building the current project."""

        _env = {
            'LIBS': self.libraries
        }

        self.program = env.FreelanProgram(
            os.path.join(self.path.abspath, env.bindir),
            self.name,
            self.source_files,
            **_env
        )

        if self.path.variant_dirs:
            self.program = env.Install(os.path.join(self.path.srcnode().abspath, env.bindir), self.program)

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

    def __init__(self, path, parent_project, name, libraries, source_files=None):
        """Create a new sample project."""

        super(SampleProject, self).__init__(path, name, libraries)

        self.parent_project = parent_project

        # Scan for source files
        if source_files is None:
            self.source_files = []

            for root, directories, files in os.walk(self.path.srcnode().abspath):
                self.source_files.extend(file_tools.filter(files, ['*.c', '*.cpp']))
        else:
            self.source_files = source_files

    def configure_environment(self, env):
        """Configure the given environment for building the current project."""

        parent_library = self.parent_project.static_library

        _env = {
            'CPPPATH': [self.path.abspath, os.path.join(self.parent_project.path.abspath, 'include')],
            'LIBPATH': [self.path.abspath, os.path.join(self.parent_project.path.abspath, env.libdir)],
            'LIBS': [self.parent_project.name + env.static_suffix] + self.libraries
        }

        sample = env.FreelanProgram(
            self.path.abspath,
            self.name,
            self.source_files,
            **_env
        )

        if self.path.variant_dirs:
            sample = env.Install(self.path.srcnode().abspath, sample)

        return sample
