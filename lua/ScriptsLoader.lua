local ffi = require("ffi")

return function(folder)
  local mask = folder .. "/*.lua"
  local find_data = ffi.new("WIN32_FIND_DATAA")

  local hFind = ffi.C.FindFirstFileA(mask, find_data)

  if hFind == ffi.cast("void*", -1) then
    print("[Loader] Nothing found!")
    return
  end

  repeat
    local file_name = ffi.string(find_data.cFileName)
    local first_char = file_name:sub(1, 1)

    if file_name ~= "." and file_name ~= ".." and first_char ~= "." and first_char ~= "_" then
      local mod_name = file_name:sub(1, -5)

      local require_path = mod_name

      print("[Loader] Loading: " .. file_name .. "...")

      local ok, err = pcall(require, require_path)
      if not ok then
        print("[Loader] Error in " .. file_name .. ":" .. err)
      end
    else
      print("[Loader] Skipped file: " .. file_name)
    end
  until not ffi.C.FindNextFileA(hFind, find_data)
  ffi.C.FindClose(hFind)
end
