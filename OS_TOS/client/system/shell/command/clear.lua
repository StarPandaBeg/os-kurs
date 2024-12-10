require("util")
require("os")

local man_obj = man.man("clear", "Clear screen")

local function clear(argc, argv)
  local argt = parse_args(argv)
  if (man.try_print(argt, man_obj)) then return end
  if (#argt ~= 1) then error("invalid number of arguments") end
  OS.clear()
end

return clear