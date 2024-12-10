FileAttr = {}
FileAttr.__index = FileAttr

local BIT_DIRECTORY = 15
local BIT_DELETED = 14
local BIT_SUID = 11
local BIT_SGID = 10
local BIT_STICKY = 9
local BIT_OWNER_R = 8
local BIT_OWNER_W = 7
local BIT_OWNER_X = 6
local BIT_GROUP_R = 5
local BIT_GROUP_W = 4
local BIT_GROUP_X = 3
local BIT_WORLD_R = 2
local BIT_WORLD_W = 1
local BIT_WORLD_X = 0

local function is_set(value, index)
  return (value & (1 << index)) ~= 0
end

local function set(value, index, flag)
  if flag then
    return value | (1 << index)
  else
    return value & ~(1 << index)
  end
end

function FileAttr:create(value)
  value = value or 0
  local data = {
    value = value
  }
  local metatable = {
    __index = self
  }
  return setmetatable(data, metatable)
end

function FileAttr:is_dir()
  return is_set(self.value, BIT_DIRECTORY)
end

function FileAttr:is_file()
  return not self:is_dir()
end

function FileAttr:deleted()
  return not is_set(self.value, BIT_DELETED)
end

function FileAttr:suid()
  return is_set(self.value, BIT_SUID)
end

function FileAttr:sgid()
  return is_set(self.value, BIT_SGID)
end

function FileAttr:sticky()
  return is_set(self.value, BIT_STICKY)
end

function FileAttr:owner(index)
  return is_set(self.value, BIT_OWNER_R - index)
end

function FileAttr:group(index)
  return is_set(self.value, BIT_GROUP_R - index)
end

function FileAttr:world(index)
  return is_set(self.value, BIT_WORLD_R - index)
end

function FileAttr:can(who, what)
  local offset = 2 + 3 * (2 - who)
  return is_set(self.value, offset - what)
end

function FileAttr:set_dir(flag)
  self.value = set(self.value, BIT_DIRECTORY, flag)
  return self
end

function FileAttr:set_file(flag)
  return self:set_dir(not flag)
end

function FileAttr:set_deleted(flag)
  self.value = set(self.value, BIT_DELETED, not flag)
  return self
end

function FileAttr:set_suid(flag)
  self.value = set(self.value, BIT_SUID, flag)
  return self
end

function FileAttr:set_sgid(flag)
  self.value = set(self.value, BIT_SGID, flag)
  return self
end

function FileAttr:set_sticky(flag)
  self.value = set(self.value, BIT_STICKY, flag)
  return self
end

function FileAttr:set_owner(index, flag)
  self.value = set(self.value, BIT_OWNER_R - index, flag)
  return self
end

function FileAttr:set_group(index, flag)
  self.value = set(self.value, BIT_GROUP_R - index, flag)
  return self
end

function FileAttr:set_world(index, flag)
  self.value = set(self.value, BIT_WORLD_R - index, flag)
  return self
end

function FileAttr:set_can(who, what, flag)
  local offset = 2 + 3 * (2 - who)
  self.value = set(self.value, offset - what, flag)
  return self
end

function FileAttr:value()
  return self.value
end

local export = {
  FileAttr=FileAttr,
}
return export