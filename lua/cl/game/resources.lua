function LoadAllTextures()
	Draw.LoadTexture("hud_heart", "lua/cl/textures/hud_heart.png")
	Draw.LoadTexture("hud_book", "lua/cl/textures/hud_book.png")
	Draw.LoadTexture("hud_user", "lua/cl/textures/hud_user.png")
	Draw.LoadTextureNoFilter("hud_coins", "lua/cl/textures/hud_coins.png")

	local ItemsSet = Draw.CreateSpriteSet("lua/cl/textures/items.png", 16, 16)
	Draw.LoadSpriteTexture("item_carrot", ItemsSet, 0, 0)
	Draw.LoadSpriteTexture("item_potato", ItemsSet, 1, 0)
	Draw.LoadSpriteTexture("item_tomato", ItemsSet, 2, 0)
	Draw.DeleteSpriteSet(ItemsSet)
end


Hook.Add("OnInit", "LoadAllTextures", LoadAllTextures)
