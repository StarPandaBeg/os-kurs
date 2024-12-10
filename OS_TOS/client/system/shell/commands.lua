require("os")
require("util")
local pathlib = require("path")
local proc = require("proc")

local cd = require("command/cd")
local clear = require("command/clear")
local pwd = require("command/pwd")
local who = require("command/who")

local internal_commands = {
  cd = cd,
  clear = clear,
  pwd = pwd,
  who = who,
}

local function locate_exec(name, p)
  local path1 = pathlib.normalize(name, p)
  local path2 = pathlib.normalize(name .. ".lua", p)

  local exists1 = OS.is_file(path1)
  local exists2 = OS.is_file(path2)
  if (not exists1 and not exists2) then return nil end
  return fif(exists1, path1, path2)
end

local function locate_in(command, p)
  local path1 = pathlib.normalize(command, p)
  local exists1 = OS.exists(path1)
  local epath = nil

  if (exists1 and OS.is_catalog(path1)) then
    epath = locate_exec("module", path1)
  else
    epath = locate_exec(command, p)
  end

  if (not epath) then return nil end
  return function(argc, argv)
    run_external(epath, argc, argv)
  end
end

local function locate_cwd(command)
  return locate_in(command, path)
end

local function locate_bin(command)
  return locate_in(command, "/bin")
end

local function locate_internal(command)
  return internal_commands[command]
end

local locators = {
  locate_cwd,
  locate_bin,
  locate_internal
}

local function locate(command)
  for _, locator in ipairs(locators) do
    local l = locator(command)
    if (l ~= nil) then return l end
  end
  return nil
end

return locate