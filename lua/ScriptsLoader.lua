local ffi = require("ffi")

return function(folder)
	local mask = folder .. "/*.lua"
	local find_data = ffi.new("WIN32_FIND_DATAA")

	local hFind = ffi.C.FindFirstFileA(mask, find_data)

	if hFind == ffi.cast("void*", -1) then
		print("[Loader] No scripts found.")
		return
	end

	repeat
		local file_name = ffi.string(find_data.cFileName)

		if file_name ~= "" then
			local first_char = file_name:sub(1, 1)

			if file_name ~= "." and file_name ~= ".." and first_char ~= "." and first_char ~= "_" then
				local module = file_name:sub(1, -5)

				print("[Loader] Loading " .. file_name)

				local ok, err = pcall(require, module)

				if not ok then
					print("[Loader] Error loading " .. file_name .. ":")
					print(err)
				end
			else
				print("[Loader] Skipped " .. file_name)
			end
		end

	until not ffi.C.FindNextFileA(hFind, find_data)

	ffi.C.FindClose(hFind)
end
