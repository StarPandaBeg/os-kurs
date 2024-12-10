local proc = require("proc")
local os = require("os-raw")
local ros = require("os-real")
local strutil = require("string-util")

OS = {}
OS.__index = OS

function OS.envget(key, default)
  return _G.TOS_ENV[key] or default
end

function OS.envset(key, value)
  _G.TOS_ENV[key] = value
  proc.env(proc.pid(), key, value)
end

function OS.cwd()
  return OS.envget("SHELL_CWD", nil)
end

function OS.exec()
  return _G.TOS_EXEC
end

function OS.args()
  local argc = OS.envget("SHELL_ARGC", "0")
  local argv = OS.envget("SHELL_ARGV", "")
  return tonumber(argc), strutil.split(argv, ";")
end

function OS.exists(path)
  return os.exists(path)
end

function OS.is_file(path)
  return os.is_file(path)
end

function OS.is_catalog(path)
  return os.is_catalog(path)
end

function OS.listdir(path)
  return os.listdir(path)
end

function OS.mkdir(path)
  return os.mkdir(path)
end

function OS.filedata(path)
  return os.filedata(path)
end

function OS.clear()
  os.clear()
end

function OS.chmod(path, permission)
  os.chmod(path, permission)
end

function OS.chown(path, uid, gid)
  if (gid == nil) then
    os.chown(path, uid)
  else
    os.chown(path, uid, gid)
  end
end

function OS.date(format, time)
  return ros.date(format, time)
end

function OS.rm(path)
  os.rm(path)
end

function OS.touch(path)
  os.touch(path)
end

function OS.link(target, link)
  os.link(target, link)
end

function OS.can(path, what)
  return os.can(path, what)
end

function OS.disk()
  return os.disk()
end

function OS.sleep(ms)
  return os.sleep(ms)
end