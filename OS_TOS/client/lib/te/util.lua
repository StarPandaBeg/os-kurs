local kb = require("keyboard")

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

function is_switch_insert_mode(key1, key2)
  if (key1 ~= kb.keys.KEY_EXTENDED) then return nil end
  return key2 == kb.keys.KEY_E_INSERT
end