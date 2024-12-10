local io = require("io")
local table = require("table")

local function man(name, description, args, flags)
  flags = flags or {}
  local help_flag_exists = false
  for _, flag in ipairs(flags) do
      if flag.long == "help" or flag.short == "h" then
          help_flag_exists = true
          break
      end
  end
  if not help_flag_exists then
    table.insert(flags, {
        short = "h",
        long = "help",
        description = "show this help message"
    })
  end

  return {
    program_name = name,
    description = description,
    args = args or {},
    flags = flags
  }
end

local function man_print(man_obj)
  IO.write("Usage:")

  local usage = man_obj.program_name
  for _, arg in ipairs(man_obj.args) do
      local ellipsis = ""
      if (arg.multiple) then ellipsis = "..." end
      if arg.required then
          usage = usage .. " <" .. arg.name .. ellipsis .. ">"
      else
          usage = usage .. " [" .. arg.name .. ellipsis .. "]"
      end
  end
  print(" " .. usage)
  if (man_obj.description) then print(man_obj.description) end

  if #man_obj.args > 0 then
      print("\nArgs:")
      for _, arg in ipairs(man_obj.args) do
          local ellipsis = ""
          if (arg.multiple) then ellipsis = "..." end
          local name = arg.required and ("<" .. arg.name .. ellipsis .. ">") or ("[" .. arg.name .. ellipsis .. "]")
          print("  " .. name .. "\t" .. arg.description)
      end
  end
  if #man_obj.flags > 0 then
      print("\nFlags:")
      for _, flag in ipairs(man_obj.flags) do
          local flag_name = flag.short and "-" .. flag.short .. ", " or ""
          flag_name = flag_name .. "--" .. flag.long
          print("  " .. flag_name .. "\t" .. flag.description)
      end
  end
end

function man_try_print(argt, man_obj)
  if (argt['help'] or argt['h']) then 
    man_print(man_obj)
    return true
  end
  return false
end

local export = {
  man = man,
  print = man_print,
  try_print = man_try_print
}
return export