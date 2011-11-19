# NSIS Support for SCons
# Based on http://www.scons.org/wiki/NsisSconsTool by Mike Elkins, January 2004

"""
This tool provides SCons support for the Nullsoft Scriptable Install System
a windows installer builder available at http://nsis.sourceforge.net/home

In addition, if you set NSISDEFINES to a dictionary, those variables will be
passed to NSIS.
"""

import SCons.Builder
import SCons.Util
import SCons.Scanner

import os.path
import glob
import platform

is_linux = (platform.system() == 'Linux')

def nsis_parse(sources, keyword, multiple, nsis_flags):
  """
  A function that knows how to read a .nsi file and figure
  out what files are referenced, or find the 'OutFile' line.

  sources is a list of nsi files.
  keyword is the command ('File' or 'OutFile') to look for
  multiple is true if you want all the args as a list, false if you
  just want the first one.
  """
  stuff = []
  current_ignored = 0
  for s in sources:
    c = s.get_contents()
    linenum = 0
    for l in c.split('\n'):
      linenum += 1
      try:
        semi = l.find(';')
        if (semi != -1):
          l = l[:semi]
        hash = l.find('#')
        if (hash != -1):
          l = l[:hash]
        # Look for the keyword
        l = l.strip()
        spl = l.split(None,1)
        if len(spl) == 1 and current_ignored > 0 and spl[0].capitalize() == '!endif':
          current_ignored -= 1
        elif len(spl) > 1:
          if current_ignored > 0 and spl[0].capitalize() in [ '!ifdef', '!ifmacrodef', '!ifndef' ]:
            current_ignored += 1
          elif current_ignored == 0 and spl[0].capitalize() == '!ifdef' and spl[1] not in nsisdefines:
            current_ignored += 1
          elif current_ignored == 0 and spl[0].capitalize() == '!ifndef' and spl[1] in nsisdefines:
            current_ignored += 1
          elif current_ignored == 0 and spl[0].capitalize() == keyword.capitalize():
            arg = spl[1]
            if keyword.capitalize() == 'File' and arg.lower().startswith('/oname') and len(spl) > 1:
              arg = spl[2]
            if arg.startswith('"') and arg.endswith('"'):
              arg = arg[1:-1]
            if multiple:
              stuff += [ arg ]
            else:
              return arg
      except:
        print "in %(source)s, line %(linenum)d\n" % { 'source': s, 'linenum': linenum }
        raise
  return stuff


def nsis_path(filename, nsisdefines, rootdir):
  """
  Do environment replacement, and prepend with the SCons root dir if
  necessary
  """
  # We can't do variables defined by NSIS itself (like $INSTDIR),
  # only user supplied ones (like ${FOO})
  varPos = filename.find('${')
  while varPos != -1:
    endpos = filename.find('}',varPos)
    assert endpos != -1
    if not nsisdefines.has_key(filename[varPos+2:endpos]):
      raise KeyError ("Could not find %s in NSISDEFINES" % filename[varPos+2:endpos])
    val = nsisdefines[filename[varPos+2:endpos]]
    if type(val) == list:
      if varPos != 0 or endpos+1 != len(filename):
        raise Exception("Can't use lists on variables that aren't complete filenames")
      return val
    filename = filename[0:varPos] + val + filename[endpos+1:]
    varPos = filename.find('${')
  return filename


def nsis_scanner(node, env, path, source_dir = None):
  """
  The scanner that looks through the source .nsi files and finds all lines
  that are the 'File' command, fixes the directories etc, and returns them.
  """
  nodes = node.rfile()
  if not node.exists():
    return []
  nodes = []
  if source_dir is None:
    try:
      source_dir = env['NSISSRCDIR']
    except:
      source_dir = node.get_dir()
  for include in nsis_parse([node],'file',1, env['NSISDEFINES']):
    exp = nsis_path(include,env['NSISDEFINES'],source_dir)
    if type(exp) != list:
      exp = [exp]
    for p in exp:
      for filename in env.Glob( os.path.abspath(
        os.path.join(str(source_dir),p))):
          # Why absolute path?  Cause it breaks mysteriously without it :(
          nodes.append(filename)
  for include in nsis_parse([node],'!include',1, env['NSISDEFINES']):
    exp = nsis_path(include,env['NSISDEFINES'],source_dir)
    if type(exp) != list:
      exp = [exp]
    for p in exp:
      if p not in [ 'LogicLib.nsh', 'MUI2.nsh' ]:
        # get ../bin/makensis and go up two directories
        nsis_install_location = os.path.dirname(os.path.dirname(env['NSIS']))
        filename = os.path.abspath(os.path.join(nsis_install_location, 'share', 'nsis', 'Include', p))
        if not os.path.isfile(filename):
          filename = os.path.abspath(os.path.join(str(source_dir),p))
        # Why absolute path?  Cause it breaks mysteriously without it :(
        nodes.append(filename)
        nodes += nsis_scanner(env.File(filename), env, path, source_dir = source_dir)
  return nodes


def nsis_emitter( source, target, env ):
  """
  The emitter changes the target name to match what the command actually will
  output, which is the argument to the OutFile command.
  """
  nsp = nsis_parse(source,'outfile',0, env['NSISDEFINES'])
  if not nsp:
    return (target,source)
  x  = (
    nsis_path(nsp,env['NSISDEFINES'],''),
    source)
  return x

def quoteIfSpaced(text):
  if ' ' in text:
    return '"'+text+'"'
  else:
    return text

def toString(item, env):
  if type(item) == list:
    ret = ''
    for i in item:
      if ret:
        ret += ' '
      val = toString(i,env)
      if ' ' in val:
        val = "'"+val+"'"
      ret += val
    return ret
  else:
    # For convenience, handle #s here
    if str(item).startswith('#'):
      item = env.File(item).get_abspath()
    return str(item)

def runNSIS(source, target, env, for_signature):
  ret = env['NSIS']+" "
  if env.has_key('NSISFLAGS'):
    for flag in env['NSISFLAGS']:
      ret += flag
      ret += ' '
  if env.has_key('NSISDEFINES'):
    for d in env['NSISDEFINES']:
      if is_linux:
        ret += '-D'+d
      else:
        ret += '/D'+d
      if env['NSISDEFINES'][d]:
        ret +='='+quoteIfSpaced(toString(env['NSISDEFINES'][d],env))
      ret += ' '
  if is_linux:
    ret += '-- '
  for s in source:
    ret += quoteIfSpaced(str(s))
  return ret

def generate(env):
  """
  This function adds NSIS support to your environment.
  """
  env['BUILDERS']['Installer'] = SCons.Builder.Builder(generator=runNSIS,
                                 src_suffix='.nsi',
                                 emitter=nsis_emitter)
  env.Append(SCANNERS = SCons.Scanner.Scanner( function = nsis_scanner,
             skeys = ['.nsi','.nsh']))
  if not env.has_key('NSISDEFINES'):
    env['NSISDEFINES'] = {}
  env['NSIS'] = find_nsis(env)

def find_nsis(env):
  """
  Try and figure out if NSIS is installed on this machine, and if so,
  where.
  """
  if SCons.Util.can_read_reg:
    # If we can read the registry, get the NSIS command from it
    try:
      k = SCons.Util.RegOpenKeyEx(SCons.Util.hkey_mod.HKEY_LOCAL_MACHINE,
                                  'SOFTWARE\\NSIS')
      val, tok = SCons.Util.RegQueryValueEx(k,None)
      ret = val + os.path.sep + 'makensis.exe'
      if os.path.exists(ret):
        return '"' + ret + '"'
      else:
        return None
    except:
      pass # Couldn't find the key, just act like we can't read the registry

  # Hope it's on the path
  return env.WhereIs('makensis' + env['PROGSUFFIX'])

def exists(env):
  """
  Is NSIS findable on this machine?
  """
  if find_nsis(env) != None:
    return 1
  return 0
