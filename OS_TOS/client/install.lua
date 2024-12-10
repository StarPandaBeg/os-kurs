require("os")
local io = require("io")
local users = require("/system/lib/users")
local groups = require("/system/lib/groups")
local validator = require("validators")

-- Prevent from recreating root user if it already exist
if (users:exists(0)) then return end

OS.clear()
print("Welcome to TOS installer\n")

local rpassword = ""
while (1) do
  io.IO.write("Enter new passsword for root: ")
  password = io.IO.read()
  local status, data = pcall(function() validator:minlen(password, 4) end)
  if (status) then break end
  print(data)
end

groups:add({
  gid = 0,
  name = "root"
})
groups:adduser(0, 0)

users:add({
  uid = 0,
  gid = 0,
  name = "root",
  password = "root",
  home = "/root"
})

-- Remove self after first run
OS.rm("/install.lua")