-- Users table

require("os")
local bytes = require("bytes")
local crypto = require("crypto")
local io = require("io")
local math = require("math")
local validator = require("validators")
local strutil = require("string-util")

UsersTable = {}
UsersTable.__index = UsersTable

local function userread(file)
  local uid = bytes.bytes_to_int2(file:read(2))
  local gid = bytes.bytes_to_int2(file:read(2))
  local name = file:read(30):gsub("%s*$", "")
  local password = file:read(64)
  local home = file:read(30):gsub("%s*$", "")
  return {
    uid = uid,
    gid = gid,
    name = name,
    password = password,
    home = home
  }
end

local function userwrite(file, user)
  file:write(bytes.int2_to_bytes(user.uid))
  file:write(bytes.int2_to_bytes(user.gid))
  file:write(strutil.rpad(user.name, 30))
  file:write(user.password)
  file:write(strutil.rpad(user.home, 30))
end

local function safe(user)
  return {
    uid = user.uid,
    gid = user.gid,
    name = user.name,
    home = user.home
  }
end

local function load(file, callback)
  local fsize = file:size()
  local entries = fsize / 128
  if (entries ~= math.floor(entries)) then error('users data is corrupted') end

  for i=1,entries,1 do
    local user = userread(file)
    callback(user)
  end
end

function UsersTable:create()
  local data = {
    path = "/etc/passwd",
    data = {},
    names = {},
    total = 0
  }
  local metatable = {
    __index = self
  }
  return setmetatable(data, metatable)
end

function UsersTable:load()
  local mode = "w+"
  if (OS.is_file(self.path)) then mode = "r" end

  local file = io.File:open(self.path, mode)
  self.total = 0
  load(file, function(entry)
    self.data[entry.uid] = entry
    self.names[entry.name] = entry.uid
    self.total = self.total + 1
  end)
  file:close()
end

function UsersTable:sync()
  local file = io.File:open(self.path, "w")
  for uid,data in pairs(self.data) do
    userwrite(file, data)
  end
  file:close()
end

function UsersTable:add(user)
  local uid = user.uid
  local gid = user.gid
  local name = user.name
  local password = user.password
  local home = user.home

  if (not home or home:sub(1,1) ~= '/') then home = "/home/" .. (home or name) end

  if (uid ~= nil) then validator:number(uid):min(uid, 0):max(uid, 65535) end
  validator:number(gid):min(gid, 0):max(gid, 65535)
  validator:string(name):minlen(name, 1):maxlen(name, 30)
  validator:string(password):minlen(name, 1):maxlen(name, 30)
  validator:string(home):minlen(name, 4):maxlen(name, 30)

  if (OS.exists(home)) then
    if (OS.is_file(home)) then error("invalid home directory") end
  end

  self:load()
  if (uid == nil) then uid = self.total end

  if (uid ~= nil and self.data[uid] ~= nil) then error("uid " .. uid .. " is already used") end
  if (self.names[name] ~= nil) then error("name " .. name .. " is already used") end

  epassword = crypto.sha256(password)
  if (not OS.exists(home)) then
    OS.mkdir(home)
    OS.chmod(home, 755)
    OS.chown(home, uid)
  end

  local newuser = {
    uid = uid,
    gid = gid,
    name = name,
    password = epassword,
    home = home
  }
  self.data[uid] = newuser
  self.names[name] = uid
  self.total = self.total + 1

  local file = io.File:open(self.path, "a")
  userwrite(file, newuser)
  file:close()

  return uid
end

function UsersTable:remove(uid)
  if (not self:exists(uid)) then error('user not found') end

  local name = self:from_uid(uid)
  self.names[name] = nil
  self.data[uid] = nil
  self.total = self.total - 1
  self:sync()
end

function UsersTable:exists(uid)
  return self.data[uid] ~= nil
end

function UsersTable:from_uid(uid)
  if (not self:exists(uid)) then return nil end
  local user = safe(self.data[uid])
  return user
end

function UsersTable:from_name(name)
  local uid = self.names[name]
  if (uid == nil) then return nil end
  local user = self:from_uid(uid)
  return user
end

function UsersTable:list()
  local users = {}
  for uid, data in pairs(self.data) do
    users[uid] = safe(data)
  end
  return users
end

function UsersTable:passwd(uid, password)
  if (not self:exists(uid)) then error('user not found') end
  local epassword = crypto.sha256(password)
  local entry = self.data[uid]
  entry.password = epassword
  self:sync()
end

function UsersTable:check(uid, password)
  if (not self:exists(uid)) then error('user not found') end
  local epassword = crypto.sha256(password)
  return epassword == self.data[uid].password
end

local table = UsersTable:create()
table:load()
return table