local infile = arg[1]
local outfile = arg[2]
local varname = arg[3]

local f = io.open(infile, "rb")
if not f then
  os.exit(1)
end
local content = f:read("*a")
f:close()

local out = io.open(outfile, "w")
out:write("static const unsigned char " .. varname .. "[] = {\n")
for i = 1, #content do
  out:write(string.format("0x%02x, ", string.byte(content, i)))
  if i % 12 == 0 then out:write("\n") end
end
out:write("0x00\n};\n")
out:write("static const unsigned int " .. varname .. "_SIZE = " .. #content .. ";\n")
out:close()
