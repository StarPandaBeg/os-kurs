local math = require("math")
local string = require("string")
local table = require("table")

local function split(inputstr, sep)
  if sep == nil then
    sep = "%s"
  end
  local t = {}
  for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
    table.insert(t, str)
  end
  return t
end

local function parse_args(args, aargs, flags)
    flags = flags or {}
    aargs = aargs or {}

    local parsed_args = {}
    local i = 1
    local required_arg_count = 0

    local flags_available = {h=true, help=true}
    local flags_with_no_params = {h=true, help=true}
    for _, flag in ipairs(flags) do
        if (flag.short) then flags_available[flag.short] = true end
        if (flag.long) then flags_available[flag.long] = true end
        if flag.option then
            if (flag.short) then flags_with_no_params[flag.short] = true end
            if (flag.long) then flags_with_no_params[flag.long] = true end
        end
    end

    while i <= #args do
        local arg = args[i]

        if arg:sub(1, 2) == "--" then
            local equal_pos = arg:find("=")
            if equal_pos then
                local key = arg:sub(3, equal_pos - 1)
                if flags_with_no_params[key] then
                    error("Option " .. key .. "shouldn't have a value")
                end
                local value = arg:sub(equal_pos + 1)
                parsed_args[key] = value
            else
                local key = arg:sub(3)
                if flags_with_no_params[key] then
                    parsed_args[key] = true
                elseif i + 1 <= #args and args[i + 1]:sub(1, 1) ~= "-" then
                    parsed_args[key] = args[i + 1]
                    i = i + 1
                elseif (flags_available[key] ~= nil) then
                    error("Option " .. key .. " should have a value!")
                end
            end
        elseif arg:sub(1, 1) == "-" then
            if #arg > 2 then
                for j = 2, #arg do
                    local key = arg:sub(j, j)
                    parsed_args[key] = true
                end
            else
                local key = arg:sub(2)
                if flags_with_no_params[key] then
                    parsed_args[key] = true
                elseif i + 1 <= #args and args[i + 1]:sub(1, 1) ~= "-" then
                    parsed_args[key] = args[i + 1]
                    i = i + 1
                elseif (flags_available[key] ~= nil) then
                    error("Option " .. key .. " should have a value!")
                end
            end
        else
            table.insert(parsed_args, arg)
        end
        i = i + 1
    end

    return parsed_args
end

local function serialize(tbl)
    local result = "{"
    for k, v in pairs(tbl) do
        local key
        if type(k) == "string" then
            key = "[" .. string.format("%q", k) .. "]"
        else
            key = "[" .. tostring(k) .. "]"
        end
        if type(v) == "table" then
            result = result .. key .. "=" .. serialize(v) .. ","
        else
            local value = type(v) == "string" and string.format("%q", v) or tostring(v)
            result = result .. key .. "=" .. value .. ","
        end
    end
    result = result .. "}"
    return result
end

local function deserialize(str)
    local func, err = load("return " .. str)
    if not func then
        error("Unable to deserialize data: " .. err)
    end
    return func()
end

local function rpad(str, length, sym)
    sym = sym or " "
    if #str >= length then
        return str
    else
        return str .. string.rep(sym, length - #str)
    end
end

local function lpad(str, length, sym)
    sym = sym or " "
    if #str >= length then
        return str
    else
        return string.rep(sym, length - #str) .. str
    end
end

local function replace_char(str, r, pos)
    return table.concat{str:sub(1,pos-1), r, str:sub(pos+1)}
end

local function remove_char(str, pos)
    if pos < 1 or pos > #str then
        return str
    end
    local result = str:sub(1, pos - 1) .. str:sub(pos + 1)
    return result
end

local function insert_char(str, r, pos)
    if pos < 1 or pos > #str + 1 then
        return str
    end
    local result = str:sub(1, pos - 1) .. r .. str:sub(pos)
    return result
end

local export = {
  split = split,
  parse_args = parse_args,
  serialize = serialize,
  deserialize = deserialize,
  lpad = lpad,
  rpad = rpad,
  replace_char = replace_char,
  remove_char = remove_char,
  insert_char = insert_char,
}
return export