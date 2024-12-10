require("util")

local man_obj = man.man("pwd", "Print current working directory")

local function pwd(argc, argv)
  local argt = parse_args(argv)
  if (man.try_print(argt, man_obj)) then return end
  if (#argt ~= 1) then error("invalid number of arguments") end
  print(path)
end

return pwd