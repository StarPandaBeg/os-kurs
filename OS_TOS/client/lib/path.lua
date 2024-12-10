local table = require("table")
local proc = require("proc")
local users = require("/system/lib/users")

local function normalize(path, cwd)
  local is_absolute = path:sub(1, 1) == "/"
  local is_home = path:sub(1, 1) == "~"

  local uid = proc.info().uid
  local user = users:from_uid(uid)
  local homedir = user.home

  local path_parts = {}
  if is_absolute then
    path_parts = {""}
  elseif is_home then
    path = path:sub(2)
    for part in homedir:gmatch("[^/]+") do
      table.insert(path_parts, part)
    end
  else
    for part in cwd:gmatch("[^/]+") do
      table.insert(path_parts, part)
    end
  end

  for part in path:gmatch("[^/]+") do
    if part == "." then
    elseif part == ".." then
      if #path_parts > 0 then
        table.remove(path_parts)
      end
    else
      table.insert(path_parts, part)
    end
  end

  local normalized_path = table.concat(path_parts, "/")
  if normalized_path:sub(1, 1) ~= "/" then
    normalized_path = "/" .. normalized_path
  end

  if normalized_path == "" or normalized_path:find("^%.%.") then
    error("invalid path")
  end
  return normalized_path
end

function filename(path)
  return path:match("([^/]+)$")
end

local export = {
  normalize = normalize,
  filename = filename
}
return export