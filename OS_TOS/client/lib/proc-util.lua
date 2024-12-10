local proc = require("proc")
local users = require("/system/lib/users")
local groups = require("/system/lib/groups")

local function suspawn(path, uid, euid)
  local pid = proc.suspawn(path, uid, euid)
  local user = users:from_uid(uid)
  local g = groups:listuser(uid)
  proc.group(pid, user.gid)
  proc.groups(pid, g)
  return pid
end

local function spawn(path)
  local info = proc.info()
  local uid = info.uid
  return suspawn(path, uid, uid)
end

local export = {
  spawn = spawn,
  suspawn = suspawn,
  execute = proc.execute
}
return export