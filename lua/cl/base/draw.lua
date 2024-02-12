Draw = {}
Draw._RenderColor = {r = 1.0, g = 1.0, b = 1.0, a = 1.0}


Draw.ClearTexture = function()
	Game.Graphics:TextureClear()
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
	Game.Graphics:QuadsBegin()
	Game.Graphics:SetColor(Draw._RenderColor.r, Draw._RenderColor.b, Draw._RenderColor.b, Draw._RenderColor.a)
	Game.Graphics:QuadsDraw({
		QuadItem(x, y, w, h)
	})
	Game.Graphics:QuadsEnd()
end
