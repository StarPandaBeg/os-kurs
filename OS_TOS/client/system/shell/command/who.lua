require("util")

local man_obj = man.man("who", "Display current user info")

local function who(argc, argv)
  local argt = parse_args(argv)
  if (man.try_print(argt, man_obj)) then return end
  if (#argt ~= 1) then error("invalid number of arguments") end
  print(user().name .. " (uid " .. uid() .. ")")
end

return who