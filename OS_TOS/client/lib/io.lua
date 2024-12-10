local io = require("io-raw")
local io_core = require("io-real")

File = {}
File.__index = File

function File:open(path, mode)
  local f = {
    handle = io.open(path, mode),
    opened = true
  }
  local metatable = {
    __index = self,
    __gc = function(obj)
      obj:destroy()
    end
  }
  return setmetatable(f, metatable)
end

function File:destroy()
  if (self.opened) then
    io.close(self.handle)
  end
end

function File:close()
  io.close(self.handle)
  self.opened = false
end

function File:read(count)
  local data, read = io.read(self.handle, count)
  return data, read
end

function File:write(data)
  io.write(self.handle, data)
end

function File:seek(pos, origin)
  origin = origin or "set"
  io.seek(self.handle, pos, origin)
end

function File:tell()
  return io.tell(self.handle)
end

function File:size()
  return io.fend(self.handle)
end

function File:eof()
  return self:tell() >= self:size()
end

IO = {}
IO.__index = IO

function IO.read(format)
  format = format or "*l"
  return io_core.read(format)
end

function IO.write(data)
  io_core.write(data)
end

function IO.getch()
  return io.getch()
end

function IO.kbhit()
  return io.kbhit()
end

function IO.isprint(c)
  return io.isprint(c)
end

local export = {
  File=File,
  IO=IO
}
return export