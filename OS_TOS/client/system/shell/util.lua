local colors = require("colors")
local man = require("man")
local table = require("table")
local proc = require("proc")
local users = require("/system/lib/users")
local strutil = require("string-util")
local procutil = require("proc-util")

path = '/'

function set_path(path)
  _G.path = path
end

function print_error(data)
  print(colors("%{red}" .. data))
end

function run_external(p, argc, argv)
  local pid = procutil.spawn(p)

  proc.env(pid, "SHELL_ARGC", tostring(argc))
  proc.env(pid, "SHELL_ARGV", table.concat(argv, ";"))
  proc.env(pid, "SHELL_CWD", path)
  
  local ok, message = proc.execute(pid)
  if (not ok) then error(message, 0) end
end

function fif(condition, if_true, if_false)
  if condition then return if_true else return if_false end
end

function uid()
  local info = proc.info()
  return info.uid
end

function user()
  return users:from_uid(uid())
end

function shell_welcome()
  local welcome = user().name .. "@tos:"

  local p = path
  local home = user().home
  if (path:sub(1, #home) == home) then
    p = "~" .. path:sub(#home + 1)
  end
  return welcome .. p
end


_G.colors = colors
_G.man = man
_G.parse_args = strutil.parse_args