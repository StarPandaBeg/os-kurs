local string = require("string")

Validator = {}
Validator.__index = {}

function Validator:create()
  local data = {}
  local table = {__index = self}
  return setmetatable(data, table)
end

function Validator:type(t, value)
  assert(type(value) == t, "Value must be a " .. t)
  return self
end

function Validator:number(v)
  return self:type("number", v)
end

function Validator:string(v)
  return self:type("string", v)
end

function Validator:min(v, num)
  assert(v >= num, "Value must be greater or equal " .. num)
  return self
end

function Validator:max(v, num)
  assert(v <= num, "Value must be less or equal " .. num)
  return self
end

function Validator:minlen(v, num)
  assert(string.len(v) >= num, "Value length must be greater or equal " .. num)
  return self
end

function Validator:maxlen(v, num)
  assert(string.len(v) <= num, "Value length must be less or equal " .. num)
  return self
end

return Validator:create()