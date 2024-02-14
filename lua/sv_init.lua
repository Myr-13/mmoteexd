local function AddCSDir(Dir, IncDir)
	for _, v in pairs(list_dir(Dir)) do
		local Inc = IncDir .. "/" .. v[2]

		if v[1] == "dir" then
			AddCSDir(Dir .. "/" .. v[2], Inc)
		else
			AddCSFile(Inc)
			print("added " .. Inc .. " to client files")
		end
	end
end

-- Client files
AddCSDir("lua/cl", "cl")
AddCSDir("lua/sh", "sh")

include("sv/main.lua")
