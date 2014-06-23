"""
Generate a defines file.
"""

import os
import argparse
import inspect
import datetime

from subprocess import check_output, CalledProcessError
from distutils.version import StrictVersion
from collections import namedtuple
from SCons.Errors import BuildError


class Defines(object):
    """
    Handles defines.
    """

    def __init__(self):
        self._repository_root = None
        self._repository_version = None
        self._version = None
        self._date = None

    @property
    def no_git(self):
        return int(os.environ.get('FREELAN_NO_GIT', '0'))

    @property
    def local_path(self):
        return os.path.dirname(os.path.realpath(__file__))

    @property
    def repository_root(self):
        if self._repository_root is None:
            try:
                if self.no_git:
                    self._repository_root = self.local_path
                else:
                    self._repository_root = os.path.abspath(check_output(['git', 'rev-parse', '--show-toplevel']).rstrip())
            except CalledProcessError, OSError:
                self._repository_root = os.path.abspath(os.path.dirname(inspect.getfile(inspect.currentframe())))

        return self._repository_root

    @property
    def repository_version(self):
        if self._repository_version is None:
            try:
                if self.no_git:
                    if not 'FREELAN_NO_GIT_VERSION' in os.environ:
                        raise BuildError(errstr='You must specify FREELAN_NO_GIT_VERSION when FREELAN_NO_GIT is specified.')

                    self._repository_version = os.environ['FREELAN_NO_GIT_VERSION'].rstrip()
                else:
                    self._repository_version = check_output(['git', 'describe', '--dirty=-modified']).rstrip()
            except CalledProcessError, OSError:
                self._repository_version = "<unspecified version>"

        return self._repository_version

    @property
    def version_file_path(self):
        return os.path.join(self.repository_root, 'VERSION')

    @property
    def version(self):
        if self._version is None:
            self._version = namedtuple('Version', ['major', 'minor', 'patch'])(*StrictVersion(open(self.version_file_path).read()).version)

        return self._version

    @property
    def date(self):
        if self._date is None:
            self._date = datetime.date.today().strftime('%a %d %b %Y')

        return self._date

    @property
    def template_file_path(self):
        return os.path.join(self.repository_root, 'defines.hpp.template')

    @property
    def defines_file_name(self):
        return os.path.splitext(os.path.basename(self.template_file_path))[0]

    def replace_template_variables(self, content):
        """
        Replace the template variables.

        Return the content.
        """

        return content.format(defines=self)

    def emitter(self, target, source, env):
        """
        Modifies the dependencies.
        """

        env.Depends(target, env.Value(self.repository_version))
        env.Depends(target, env.Value(self.version))
        env.Depends(target, env.Value(self.date))

        return target, source

    def action(self, target, source, env):
        """
        Generate the defines file.
        """

        output = self.replace_template_variables(source[0].get_contents())

        with open(target[0].abspath, 'wb') as out:
            out.write(output)

    def register_into(self, env):
        """
        Register into the specified environment.
        """

        env.Append(BUILDERS={'GenerateDefines': env.Builder(
            action=self.action,
            emitter=self.emitter,
        )})

    def generate_defines(self, target):
        """
        Generate the defines.hpp file.
        """

        with open(self.template_file_path, 'rb') as source_file:
            output = self.replace_template_variables(source_file.read())

        try:
            with open(target, 'rb') as target_file:
                current_content = target_file.read()
        except IOError:
            current_content = None

        if output != current_content:
            with open(target, 'wb') as target_file:
                target_file.write(output)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate a defines file.')
    parser.add_argument('target', help='The target file to generate from the template.')
    args = parser.parse_args()

    defines = Defines()
    defines.generate_defines(args.target)
