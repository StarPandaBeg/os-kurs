require("os")
local procutil = require("proc-util")

-- it will be not nil if file is started from shell
if (OS.cwd() ~= nil) then
  error("cannot be run as script")
end

local pid = procutil.spawn("/system/bin/login.lua")
local ok, message = procutil.execute(pid)

if (not ok) then error(message) end