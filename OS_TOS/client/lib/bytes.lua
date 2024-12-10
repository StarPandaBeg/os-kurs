local math = require("math")
local string = require("string")

local function int2_to_bytes(num)
  local byte1 = math.floor(num / 256)
  local byte2 = num % 256
  return string.char(byte1, byte2)
end

local function int4_to_bytes(num)
  local byte1 = math.floor(num / 16777216)
  local byte2 = math.floor(num / 65536) % 256
  local byte3 = math.floor(num / 256) % 256
  local byte4 = num % 256
  return string.char(byte1, byte2, byte3, byte4)
end

local function bytes_to_int2(byte_string)
  local byte1, byte2 = string.byte(byte_string, 1, 2)
  local num = byte1 * 256 + byte2
  return num
end

local function bytes_to_int4(byte_string)
  local byte1, byte2, byte3, byte4 = string.byte(byte_string, 1, 4)
  local num = byte1 * 16777216 + byte2 * 65536 + byte3 * 256 + byte4
  return num
end

local export = {
  int2_to_bytes = int2_to_bytes,
  int4_to_bytes = int4_to_bytes,
  bytes_to_int2 = bytes_to_int2,
  bytes_to_int4 = bytes_to_int4,
}
return export