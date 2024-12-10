require("os")
require("te/util")
local io = require("io")
local kb = require("keyboard")
local math = require("math")
local table = require("table")
local string = require("string")
local strutil = require("string-util")

local editor = {
  buffer = {""},
  cursor = {x = 1, y = 1},
  offset = {x = 0, y = 0},
  insert = true,
  readonly = false,
  filename = nil,
}
local exit = false
local notify_save = function (buffer) end
local saved = false

local function move_cursor(dx, dy)
  local new_x = editor.cursor.x + dx
  local new_y = editor.cursor.y + dy

  if new_x < 1 then
      new_x = 1
      editor.offset.x = editor.offset.x - 1
  elseif new_x > 120 then
      new_x = 120
      editor.offset.x = editor.offset.x + 1
  end

  if new_y < 1 then
      new_y = 1
      editor.offset.y = editor.offset.y - 1
  elseif new_y > 26 then
      new_y = 26
      editor.offset.y = editor.offset.y + 1
  end

  editor.cursor.x = new_x
  editor.cursor.y = new_y
end

local function process_letter(letter)
  local letterIndex = editor.cursor.x + editor.offset.x
  local lineIndex = editor.cursor.y + editor.offset.y
  local line = editor.buffer[lineIndex]
  if (editor.insert) then
    line = strutil.insert_char(line, letter, letterIndex)
  else
    line = strutil.replace_char(line, letter, letterIndex)
  end
  editor.buffer[lineIndex] = line
  move_cursor(1, 0)
end

local function process_backspace()
  local letterIndex = editor.cursor.x + editor.offset.x - 1
  local lineIndex = editor.cursor.y + editor.offset.y
  local line = editor.buffer[lineIndex]
  if (letterIndex == 0) then 
    if (lineIndex == 1) then return end
    if (#line ~= 0) then return end
    table.remove(editor.buffer, lineIndex)
    move_cursor(0, -1)
    editor.cursor.x = #editor.buffer[lineIndex - 1] + 1
    return 
  end
  if (#line == letterIndex) then
    line = line:sub(1, letterIndex - 1)
  else
    line = strutil.remove_char(line, letterIndex)
  end
  editor.buffer[lineIndex] = line
  move_cursor(-1, 0)
end

local function process_return()
  local letterIndex = editor.cursor.x + editor.offset.x
  local lineIndex = editor.cursor.y + editor.offset.y
  local line = editor.buffer[lineIndex]

  local linePart = line:sub(letterIndex)
  line = line:sub(1, letterIndex - 1)
  editor.buffer[lineIndex] = line

  table.insert(editor.buffer, lineIndex + 1, linePart or "")
  move_cursor(0, 1)
  editor.cursor.x = 1
end

local function process_arrows(direction)
  local letterIndex = editor.cursor.x + editor.offset.x
  local lineIndex = editor.cursor.y + editor.offset.y
  local line = editor.buffer[lineIndex]
  local totalLineLength = #line

  if (direction == "left") then
    move_cursor(-1,0)
  elseif (direction == "right") then
    if (letterIndex > totalLineLength) then return end
    move_cursor(1,0)
  elseif (direction == "up") then
    if (lineIndex == 1) then return end
    move_cursor(0,-1)
    local line1 = editor.buffer[lineIndex - 1]
    editor.cursor.x = math.min(editor.cursor.x, #line1 + 1)
  elseif (direction == "down") then
    if (lineIndex >= #editor.buffer) then return end
    move_cursor(0,1)
    local line1 = editor.buffer[lineIndex + 1]
    editor.cursor.x = math.min(editor.cursor.x, #line1 + 1)
  end
end

local function draw()
  OS.clear()
  for i=1,28,1 do
    local lineIndex = i + editor.offset.y
    if (lineIndex <= #editor.buffer) then
      print(editor.buffer[lineIndex])
    end
  end

  local insertMode = "<REPLACE> "
  local saveMode = "*"
  if (editor.insert) then
    insertMode = "<INSERT>  "
  end
  if (editor.readonly) then
    insertMode = "<READONLY>"
  end
  if (saved) then saveMode = " " end

  io.IO.write("\27[27;1H")
  print(string.rep("-", 120))
  print(strutil.rpad(editor.filename .. saveMode, 120))
  print("^S Save     ^Q Exit     [arrows] Move cursor                                                                  " .. insertMode)
  io.IO.write("\27[" .. editor.cursor.y .. ";" .. editor.cursor.x .. "H")
end

local function read_input()
  local k1, k2 = io.IO.getch()

  if (io.IO.isprint(k1)) then 
    if (editor.readonly) then return end;
    process_letter(string.char(k1))
  elseif (k1 == kb.keys.KEY_BACKSPACE) then
    if (editor.readonly) then return end;
    process_backspace()
  elseif (k1 == kb.keys.KEY_RETURN) then
    if (editor.readonly) then return end;
    process_return()
  elseif (k1 == kb.keys.KEY_DC1) then
    exit = true
  elseif (k1 == kb.keys.KEY_XOFF) then
    if (editor.readonly) then return end;
    notify_save(editor.buffer)
    saved = true
  elseif (is_arrow(k1, k2)) then
    process_arrows(to_arrow(k1, k2))
  elseif (is_switch_insert_mode(k1, k2)) then
    if (editor.readonly) then return end;
    editor.insert = not editor.insert
  end
end

function on_save(callback)
  notify_save = callback
end

function run_editor(initial, readonly, filename, is_new)
  initial = initial or {""}
  readonly = readonly or false
  filename = filename or "unknown"
  is_new = is_new or true

  editor.buffer = initial
  editor.readonly = readonly
  editor.filename = filename
  saved = not is_new

  while not exit do
    draw()
    read_input()
  end
  OS.clear()
end