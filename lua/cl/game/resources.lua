function LoadAllTextures()
	Draw.LoadTexture("hud_heart", "lua/cl/textures/checker.png")
end


Hook.Add("OnInit", "LoadAllTextures", LoadAllTextures)
