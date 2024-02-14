Draw = {}
Draw._RenderColor = {r = 1.0, g = 1.0, b = 1.0, a = 1.0}
Draw._Textures = {}


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


Draw.SetColor = function(r, g, b, a)
	if not a then
		a = 255
	end

	Draw._RenderColor.r = r / 255
	Draw._RenderColor.g = g / 255
	Draw._RenderColor.b = b / 255
	Draw._RenderColor.a = a / 255
end


Draw.Rect = function(x, y, w, h)
	Draw.RectCentred(x + w / 2, y + h / 2, w, h)
end


Draw.RectCentred = function(x, y, w, h)
	Game.Graphics:QuadsBegin()
	Game.Graphics:SetColor(Draw._RenderColor.r, Draw._RenderColor.g, Draw._RenderColor.b, Draw._RenderColor.a)
	Game.Graphics:QuadsDraw({
		QuadItem(x, y, w, h)
	})
	Game.Graphics:QuadsEnd()
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


Hook.Add("OnShutdown", "UnloadAllTextures", function()
	for k, _ in pairs(Draw._Textures) do
		Draw.UnloadTexture(k)
	end
end)
