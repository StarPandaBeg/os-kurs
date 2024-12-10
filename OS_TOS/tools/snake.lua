require("os")
local io = require("io")
local math = require("math")
local osb = require("os-raw")
local kb = require("keyboard") 
 
local function create_desk(w, h)
  local table = {}
  for i=1,h do
    table[i] = {}
    for j=1,w do
      table[i][j] = 0
    end
  end
  return table
end
 
local function fif(condition, if_true, if_false)
  if condition then return if_true else return if_false end
end
 
local function to_desk_char(value)
  if value == 0 then return " " end
  return "S"
end
 
local function print_desk(desk)
  local w = #desk[1]
  local h = #desk
  for i=1,w+2 do
    io.IO.write("#")
  end
  print()  
  for i=1,h do
    for j=1,w do
      if (j==1) then
        io.IO.write("#")
      end
      local char = to_desk_char(desk[i][j])
      io.IO.write(char)
      if (j == w) then
        io.IO.write("#\n")
      end 
    end
  end
  for i=1,w+2 do
    io.IO.write("#")
  end
  print()
  print("Score: 0")
end
  
local function update_cell(desk, x, y, value)
  desk[y][x] = value
  io.IO.write("\27[" .. y+1 .. ";" .. x+1 .. "H")
  io.IO.write(to_desk_char(value))
end
 
local function step(desk)
  local w = #desk[1]
  local h = #desk
 
  for i=1,h do
    for j=1,w do
      if desk[i][j] ~= 0 then
        update_cell(desk, j, i, desk[i][j] - 1)
      end
    end
  end
end
 
local function move(head, dir)
  head[1] = head[1] + dir[1]
  head[2] = head[2] + dir[2]
 
  head[1] = fif(head[1] > 0, head[1], 32)
  head[1] = fif(head[1] < 33, head[1], 1)
  head[2] = fif(head[2] > 0, head[2], 16)
  head[2] = fif(head[2] < 17, head[2], 1) 
 
  return head
end
local arrows = {
  left  = kb.keys.KEY_E_ARROW_LEFT,
  right = kb.keys.KEY_E_ARROW_RIGHT,
  up    = kb.keys.KEY_E_ARROW_UP,
  down  = kb.keys.KEY_E_ARROW_DOWN,
}
 
function is_arrow(key1, key2)
  if (key1 ~= kb.keys.KEY_EXTENDED) then return false end
  for _,keycode in pairs(arrows) do
    if (keycode == key2) then return true end
  end
  return false
end
 
function to_arrow(key1, key2)
  if (key1 ~= kb.keys.KEY_EXTENDED) then return nil end
  for dir,keycode in pairs(arrows) do
    if (keycode == key2) then return dir end
  end
  return false
end
 
function next_fruit()
  local x = math.random(32)
  local y = math.random(16)
  return {x, y}
end
function draw_fruit(pos)
  io.IO.write("\27[" .. pos[2]+1 .. ";" .. pos[1]+1 .. "H")
  io.IO.write("F")  
end
OS.clear()
 
local desk = create_desk(32, 16)
local head = {16, 8}
local dir = {1, 0}
local len = 3
local fruit = next_fruit()
local score = 0
 
print_desk(desk)
draw_fruit(fruit)
 
while 1 do
  if (io.IO.kbhit() ~= 0) then
    local k1, k2 = io.IO.getch()
    if (k1 == kb.keys.KEY_q) then goto gameend end
    if (is_arrow(k1, k2)) then
      local v = to_arrow(k1, k2)
      if v == "left" and dir[1] ~= 1 then dir = {-1, 0} end
      if v == "right" and dir[1] ~= -1 then dir = {1, 0} end
      if v == "up" and dir[2] ~= 1 then dir = {0, -1} end
      if v == "down" and dir[2] ~= -1 then dir = {0, 1} end
    end
  end
  if head[1] == fruit[1] and head[2] == fruit[2] then
    len = len + 1
    fruit = next_fruit()
    draw_fruit(fruit)
    
    score = score + 10
    io.IO.write("\27[19;0HScore: "..score)
  end
  step(desk)
  
  if desk[head[2]][head[1]] ~= 0 then
    io.IO.write("\27[21;0HGame Over! Press any key to exit!")
    io.IO.getch()
    goto gameend
  end
  update_cell(desk, head[1], head[2], len)
  head = move(head, dir)
  osb.sleep(math.max(100, 300 - score))
end
::gameend:: 
 
OS.clear()
print("Bye!")