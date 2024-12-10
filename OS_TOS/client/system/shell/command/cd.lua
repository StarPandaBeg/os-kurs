require("os")
require("util")
local pathlib = require("path")

local args = {
  {name="DIRECTORY", description="selected directory. default to user home directory", required=false}
}
local man_obj = man.man("cd", "Change working directory", args)

local function cd(argc, argv)
  local argt = parse_args(argv)
  if (man.try_print(argt, man_obj)) then return end
  if (#argt > 2) then error("invalid number of arguments") end

  local p = argt[2] or '~'
  local pn = pathlib.normalize(p, path)
  if (not OS.is_catalog(pn)) then error("no such directory found") end
  set_path(pn)
end

return cd