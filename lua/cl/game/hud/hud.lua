local ScreenAspect = 0


local function Convert(x)
	return x * ScreenAspect
end


local function SideBar()
	local FontSize = 6
	local x = Convert(270)
	local y = 300 / 2 - 50 / 2

	-- Background
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(x, y, Convert(30), 50)

	x = x + 2
	y = y + 2

	Draw.Text(x, y, FontSize, "Account: %s", Account.Name)
	y = y + FontSize
	Draw.Text(x, y, FontSize, "Level: %d", Account.Level)
	y = y + FontSize

	local Exp = Account.Exp
	local NeedExp = Account.NeedExp
	local Width = Convert(Exp / NeedExp * 25)

	-- TODO: Add cool progress bar xd

	Draw.Text(x, y, FontSize, "Exp: %d/%d", Exp, NeedExp)
	y = y + FontSize
	Draw.Text(x, y, FontSize, "Money: %d", Account.Money)
	y = y + FontSize
end


local function MainBar()
	local HPRatio = (Account.Health / Account.MaxHealth)
	local MaxWidth = Convert(40)
	local HPX = Convert(100)
	local HPWidth = MaxWidth * HPRatio
	local MPWidth = MaxWidth * (Account.Mana / Account.MaxMana)
	local MPX = Convert(200) - MPWidth
	local Offset = 2

	-- Health bar
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(HPX - Offset, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2)
	Draw.SetColor(Math.Lerp(50, 255, (1 - HPRatio)), 255 * HPRatio, 50 * HPRatio)
	Draw.Rect(HPX, 250, HPWidth, 10)

	Draw.SetTexture("hud_heart")
	Draw.SetColor(255, 255, 255)
	Draw.Rect(HPX + 2, 252, 6, 6)

	Draw.Text(HPX, 243, 5, "%d/%d", Account.Health, Account.MaxHealth)

	-- Mana bar
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(Convert(200) - MaxWidth - Offset, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2)
	Draw.SetColor(50, 50, 200)
	Draw.Rect(MPX, 250, MPWidth, 10)

	Draw.TextLeft(Convert(200), 243, 5, "%d/%d", Account.Mana, Account.MaxMana)
end


Hook.Add("RenderLevel26", "MMOHud", function()
	ScreenAspect = Game.Graphics.ScreenAspect
	Game.Graphics:MapScreen(0, 0, 300 * ScreenAspect, 300)

	SideBar()
	MainBar()
end)
