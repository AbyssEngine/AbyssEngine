-- ---------------------------------------------------------------------- --
-- THIS FILE WAS AUTO-GENERATED BY generate.py -  DO NOT HAND MODIFY!     --
-- ---------------------------------------------------------------------- --
---@meta
---version: 0.1

---@class Button
Button = {
---No description set in button.h:23.
---@type boolean
checked = false,

---No description set in button.h:25.
---@type boolean
disabled = false,

}

---No description set in button.h:19.
---@param x number @ No description set in button.h:19.
---@param y number @ No description set in button.h:19.
function Button:setSegments(x, y) end

---No description set in button.h:20.
---@param x number @ No description set in button.h:20.
---@param y number @ No description set in button.h:20.
function Button:setFixedSize(x, y) end

---No description set in button.h:21.
---@param x number @ No description set in button.h:21.
---@param y number @ No description set in button.h:21.
function Button:setPressedOffset(x, y) end

---No description set in button.h:26.
---@param frameType string @ No description set in button.h:26.
---@param index number @ No description set in button.h:26.
function Button:setFrameIndex(frameType, index) end

---No description set in button.h:27.
---@param luaActivateCallback function @ No description set in button.h:27.
function Button:onActivate(luaActivateCallback) end

---No description set in button.h:28.
---@param luaPressCallback function @ No description set in button.h:28.
function Button:onPressed(luaPressCallback) end

---No description set in button.h:29.
---@param _luaMouseEnterCallback function @ No description set in button.h:29.
function Button:onMouseEnter(_luaMouseEnterCallback) end

---No description set in button.h:30.
---@param _luaMouseLeaveCallback function @ No description set in button.h:30.
function Button:onMouseLeave(_luaMouseLeaveCallback) end
