-- Groups table

require("os")
local bytes = require("bytes")
local io = require("io")
local table = require("table")
local validator = require("validators")
local strutil = require("string-util")

GroupsTable = {}
GroupsTable.__index = GroupsTable

local function groupread(file)
  local gid = bytes.bytes_to_int2(file:read(2))
  local name = file:read(30):gsub("%s*$", "")
  local usize = bytes.bytes_to_int4(file:read(4))
  local users = {}
  for i=1,usize,1 do
    local uid = bytes.bytes_to_int2(file:read(2))
    table.insert(users, uid)
  end

  return {
    gid = gid,
    name = name,
    usize = usize,
    users = users
  }
end

local function groupwrite(file, group)
  file:write(bytes.int2_to_bytes(group.gid))
  file:write(strutil.rpad(group.name, 30))
  file:write(bytes.int4_to_bytes(group.usize))
  for i,v in ipairs(group.users) do
    file:write(bytes.int2_to_bytes(v))
  end
end

local function table_contains(table, element)
  for _, value in pairs(table) do
    if value == element then
      return true
    end
  end
  return false
end

local function table_remove(table, element)
  for i, value in pairs(table) do
    if value == element then
      table.remove(table, i)
      return true
    end
  end
  return false
end

local function load(file, callback)
  while (not file:eof()) do
    local entry = groupread(file)
    callback(entry)
  end
end

function GroupsTable:create()
  local data = {
    path = "/etc/group",
    data = {},
    names = {},
    total = 0
  }
  local metatable = {
    __index = self
  }
  return setmetatable(data, metatable)
end

function GroupsTable:load()
  local mode = "w+"
  if (OS.is_file(self.path)) then mode = "r" end

  local file = io.File:open(self.path, mode)
  self.total = 0
  load(file, function(entry)
    self.data[entry.gid] = entry
    self.names[entry.name] = entry.gid
    self.total = self.total + 1
  end)
  file:close()
end

function GroupsTable:sync()
  local file = io.File:open(self.path, "w")
  for gid,data in pairs(self.data) do
    groupwrite(file, data)
  end
  file:close()
end

function GroupsTable:add(group)
  local gid = group.gid
  local name = group.name

  if (gid ~= nil) then validator:number(gid):min(gid, 0):max(gid, 65535) end
  validator:string(name):minlen(name, 1):maxlen(name, 30)

  self:load()
  if (gid == nil) then gid = self.total end

  if (gid ~= nil and self.data[gid] ~= nil) then error("gid " .. gid .. " is already used") end
  if (self.names[name] ~= nil) then error("name " .. name .. " is already used") end

  local newgroup = {
    gid = gid,
    name = name,
    usize = 0,
    users = {}
  }
  self.data[gid] = newgroup
  self.names[name] = gid
  self.total = self.total + 1

  local file = io.File:open(self.path, "a")
  groupwrite(file, newgroup)
  file:close()

  return gid
end

function GroupsTable:remove(gid)
  if (not self:exists(gid)) then error('group not found') end

  local name = self:from_gid(gid)
  self.names[name] = nil
  self.data[gid] = nil
  self.total = self.total - 1
  self:sync()
end

function GroupsTable:exists(gid)
  return self.data[gid] ~= nil
end

function GroupsTable:from_gid(gid)
  if (not self:exists(gid)) then return nil end
  return self.data[gid]
end

function GroupsTable:from_name(name)
  local gid = self.names[name]
  if (gid == nil) then return nil end
  local group = self:from_gid(gid)
  return group
end

function GroupsTable:list()
  return self.data
end

function GroupsTable:adduser(gid, uid)
  self:load()
  if (not self:exists(gid)) then error('group not found') end
  local group = self.data[gid]
  if (table_contains(group.users, uid)) then return end

  group.usize = group.usize + 1
  table.insert(group.users, uid)
  self.data[gid] = group
  self:sync()
end

function GroupsTable:removeuser(gid, uid)
  self:load()
  if (not self:exists(gid)) then error('group not found') end
  local group = self.data[gid]
  if (not table_contains(group.users, uid)) then return end

  group.usize = group.usize - 1
  table_remove(group.users, uid)
  self.data[gid] = group
  self:sync()
end

function GroupsTable:listuser(uid) 
  local groups = {}
  for gid,data in pairs(self.data) do
    if (table_contains(data.users, uid)) then
      table.insert(groups, gid)
    end
  end
  return groups
end

local table = GroupsTable:create()
table:load()
return table