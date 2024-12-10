require("os")
local io = require("io")
local procutil = require("proc-util")
local users = require("/system/lib/users")

local uid = nil

::start::
OS.clear()
IO.write("Welcome to TOS v1.0.0\n\n")

::login::

IO.write("tos login: ")
local login = IO.read()
IO.write("tos password: ")
local password = IO.read()

user = users:from_name(login)
if (user == nil) then
  IO.write("\nInvalid credentials. Try again.\n")
  goto login
end
if (not users:check(user.uid, password)) then
  IO.write("\nInvalid credentials. Try again.\n")
  goto login
end

OS.clear()
local pid = procutil.suspawn("/system/shell/module.lua", user.uid, user.uid)
local ok, message = procutil.execute(pid)

if (not ok) then
  error("Unable to open system shell: " .. message)
end

users:load()
goto start