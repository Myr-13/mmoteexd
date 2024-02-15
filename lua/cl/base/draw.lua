Draw = {}
Draw._RenderColor = {r = 1.0, g = 1.0, b = 1.0, a = 1.0}
Draw._Textures = {}

CORNER_NONE = 0
CORNER_TL = 1
CORNER_TR = 2
CORNER_BL = 4
CORNER_BR = 8

CORNER_T = CORNER_TL + CORNER_TR
CORNER_B = CORNER_BL + CORNER_BR
CORNER_R = CORNER_TR + CORNER_BR
CORNER_L = CORNER_TL + CORNER_BL

CORNER_ALL = CORNER_T + CORNER_B


Draw.ClearTexture = function()
	Game.Graphics:TextureClear()
end


Draw.SetTexture = function(Name)
	local Texture = Draw._Textures[Name]
	if not Texture then
		return
	end

	Game.Graphics:TextureSet(Texture)
end


Draw.LoadTexture = function(Name, Path)
	Draw._Textures[Name] = Game.Graphics:LoadTexture(Path)
end


Draw.UnloadTexture = function(Name)
	local Texture = Draw._Textures[Name]
	if not Texture then
		return
	end

	Game.Graphics:UnloadTexture(Texture)
end


Draw.LoadTextureNoFilter = function(Name, Path)
	local Set = Draw.CreateSpriteSet(Path, 1, 1)
	Draw.LoadSpriteTexture(Name, Set, 0, 0)
	Draw.DeleteSpriteSet(Set)
end


Draw.CreateSpriteSet = function(Path, GridX, GridY)
	local ImgInfo = CImageInfo()

	if not Game.Graphics:LoadPNG(ImgInfo, Path) then
		return
	end

	return {
		PixPerGridX = ImgInfo.Width / GridX,
		PixPerGridY = ImgInfo.Height / GridY,
		Info = ImgInfo
	}
end


Draw.DeleteSpriteSet = function(SpriteSet)
	Game.Graphics:FreePNG(SpriteSet.Info)
end


Draw.LoadSpriteTexture = function(Name, SpriteSet, x, y, w, h)
	if not w then
		w = 1
	end

	if not h then
		h = 1
	end

	x = x * SpriteSet.PixPerGridX
	y = y * SpriteSet.PixPerGridY
	w = w * SpriteSet.PixPerGridX
	h = h * SpriteSet.PixPerGridY

	Draw._Textures[Name] = Game.Graphics:LoadSpriteTextureImpl(SpriteSet.Info, x, y, w, h, true)
end


Draw.SetColor = function(r, g, b, a)
	if not a then
		a = 255
	end

	Draw._RenderColor.r = r / 255
	Draw._RenderColor.g = g / 255
	Draw._RenderColor.b = b / 255
	Draw._RenderColor.a = a / 255
end


Draw.SetColorTable = function(tbl)
	Draw.SetColor(tbl[1], tbl[2], tbl[3], tbl[4])
end


Draw.Rect = function(x, y, w, h, r, Corners)
	if not r then
		r = 0
	end

	if not Corners then
		Corners = CORNER_NONE
	end

	
	Game.Graphics:QuadsBegin()
	Game.Graphics:SetColor(Draw._RenderColor.r, Draw._RenderColor.g, Draw._RenderColor.b, Draw._RenderColor.a)
	Game.Graphics:DrawRectExt(x, y, w, h, r, Corners)
	Game.Graphics:QuadsEnd()
end


Draw.RectCentred = function(x, y, w, h, r, Corners)
	Draw.Rect(x - w / 2, y - h / 2, w, h, r, Corners)
end


Draw.Text = function(x, y, Size, Format, ...)
	Game.TextRender:Text(x, y, Size, string.format(Format, ...), -1)
end


Draw.TextCenter = function(x, y, Size, Format, ...)
	local FinalText = string.format(Format, ...)
	Draw.Text(x - Game.TextRender:TextWidth(Size, FinalText) / 2, y, Size, FinalText)
end


Draw.TextLeft = function(x, y, Size, Format, ...)
	local FinalText = string.format(Format, ...)
	Draw.Text(x - Game.TextRender:TextWidth(Size, FinalText), y, Size, FinalText)
end


Draw.TextWidth = function(Size, Format, ...)
	return Game.TextRender:TextWidth(Size, string.format(Format, ...))
end


Hook.Add("OnShutdown", "UnloadAllTextures", function()
	for k, _ in pairs(Draw._Textures) do
		Draw.UnloadTexture(k)
	end
end)
