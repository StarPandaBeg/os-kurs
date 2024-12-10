require("util")

local io = require("io")
local os = require("os")
local proc = require("proc")
local pathlib = require("path")
local locate = require("commands")
local strutil = require("string-util")

local do_loop = true

local function run(argc, argv)
  local callback = locate(argv[1])
  if (callback == nil) then return false end

  local status, err = pcall(function() callback(argc, argv) end)
  if (not status) then print_error(err) end
  return true
end

local function step()
  IO.write(shell_welcome())
  IO.write("$ ")
  local command = IO.read()
  if (command == nil or command == '') then return end
  if (command == 'exit') then
    do_loop = false
    return
  end
  
  local parts = strutil.split(command)
  if (not run(#parts, parts)) then
    print_error("Command not found")
  end
end

local function loop()
  path = user().home or '/'
  while (do_loop) do
    step()
  end
end

loop()