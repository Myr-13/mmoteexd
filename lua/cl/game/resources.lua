function LoadAllTextures()
	Draw.LoadTexture("hud_heart", "lua/cl/textures/hud_heart.png")
	Draw.LoadTexture("hud_book", "lua/cl/textures/hud_book.png")
end


Hook.Add("OnInit", "LoadAllTextures", LoadAllTextures)
