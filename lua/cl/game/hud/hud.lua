local ScreenAspect = 0
local _CollectedItems = {}


local function Convert(x)
	return x * ScreenAspect
end


local function SideBar()
	local FontSize = 8
	local x = Convert(270)
	local y = 300 / 2 - 50 / 2

	-- Background
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(x, y, Convert(30), 50, 3, CORNER_L)

	x = x + 2
	y = y + 2

	-- Account name
	Draw.SetTexture("hud_user")
	Draw.SetColor(255, 255, 255)
	Draw.Rect(x, y + 0.5, FontSize, FontSize)

	Draw.Text(x + FontSize + 1, y, FontSize, TEXT_ALIGN_RIGHT, "%s", Account.Name)
	y = y + FontSize + 2

	-- Money
	Draw.SetTexture("hud_coins")
	Draw.SetColor(255, 255, 255)
	Draw.Rect(x, y + 0.5, FontSize, FontSize)

	Draw.RichText(x + FontSize + 1, y, FontSize, "%s<y>$", Utils.FormatNumber(Account.Money))
end


local function HealthAndMana()
	local HPRatio = (Account.Health / Account.MaxHealth)
	local MaxWidth = Convert(40)
	local HPX = Convert(100)
	local HPWidth = MaxWidth * HPRatio
	local MPWidth = MaxWidth * (Account.Mana / Account.MaxMana)
	local MPX = Convert(200) - MPWidth
	local MPXClear = Convert(200)
	local Offset = 1

	local HealthColor = { Math.Lerp(50, 255, (1 - HPRatio)), 255 * HPRatio, 50 * HPRatio }
	local ManaColor = { 50, 50, 200 }

	-- TODO: Add function for progress bars?
	-- Health bar
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(HPX - Offset, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2, 2, CORNER_ALL)
	Draw.SetColorTable(Utils.ColorMinus(HealthColor, { 70, 70, 70 }))
	Draw.Rect(HPX, 250, HPWidth, 10, 2, CORNER_ALL)
	Draw.SetColorTable(HealthColor)
	Draw.Rect(HPX, 250, HPWidth, 8.5, 2, CORNER_ALL)

	Draw.SetTexture("hud_heart")
	Draw.Rect(HPX + 2, 252, 6, 6)

	Draw.Text(HPX, 243, 5, TEXT_ALIGN_RIGHT, "%s/%s", Utils.FormatNumber(Account.Health), Utils.FormatNumber(Account.MaxHealth))

	-- Mana bar
	Draw.ClearTexture()
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(Convert(200) - MaxWidth - Offset, 250 - Offset, MaxWidth + Offset * 2, 10 + Offset * 2, 2, CORNER_ALL)
	Draw.SetColorTable(Utils.ColorMinus(ManaColor, { 20, 20, 20 }))
	Draw.Rect(MPX, 250, MPWidth, 10, 2, CORNER_ALL)
	Draw.SetColorTable(ManaColor)
	Draw.Rect(MPX, 250, MPWidth, 8.5, 2, CORNER_ALL)

	Draw.SetTexture("hud_book")
	Draw.SetColor(255, 255, 255)
	Draw.Rect(MPXClear - 8, 252, 6, 6)

	Draw.Text(MPXClear, 243, 5, TEXT_ALIGN_LEFT, "%s/%s", Utils.FormatNumber(Account.Mana), Utils.FormatNumber(Account.MaxMana))
end


local function LevelBar()
	local Width = Convert(100)
	local PosX = Convert(100)
	local PosY = 270
	local ExpRatio = Account.Exp / Account.NeedExp
	local Offset = 1

	Draw.ClearTexture()

	-- Bar
	Draw.SetColor(0, 0, 0, 100)
	Draw.Rect(PosX - Offset, PosY - Offset, Width + Offset * 2, 5 + Offset * 2, 2, CORNER_ALL)
	Draw.SetColor(15, 195, 15)
	Draw.Rect(PosX, PosY, Width * ExpRatio, 5, 2, CORNER_ALL)
	Draw.SetColor(75, 235, 75)
	Draw.Rect(PosX, PosY, Width * ExpRatio, 4, 2, CORNER_ALL)

	-- Text
	Draw.Text(PosX + Width / 2, PosY - 7, 6, TEXT_ALIGN_CENTER, "%d", Account.Level)
	Draw.Text(PosX + Width / 2, PosY + 7, 4, TEXT_ALIGN_CENTER, "%s/%s", Utils.FormatNumber(Account.Exp), Utils.FormatNumber(Account.NeedExp))
end


local function CollectedItems()
	local PosX = Convert(230)
	local OffsetY = 0
	local FontSize = 8

	for k, v in pairs(_CollectedItems) do
		OffsetY = OffsetY + FontSize
		local y = 300 - OffsetY
		Draw.Text(PosX, y, FontSize, TEXT_ALIGN_RIGHT, v.Text)

		Draw.RichText(PosX, y, FontSize, v.Text .. " <i:" .. v.StrID .. ">")

		if Game.Client.LocalTime > v.EndTime then
			table.remove(_CollectedItems, k)
		end
	end
end


local function MainBar()
	HealthAndMana()
	LevelBar()
end


Hook.Add("RenderLevel26", "MMOHud", function()
	ScreenAspect = Game.Graphics.ScreenAspect
	Game.Graphics:MapScreen(0, 0, 300 * ScreenAspect, 300)

	SideBar()
	MainBar()
	CollectedItems()
end)

Network.RegisterCallback("collect_item@hud", function(Data)
	table.insert(_CollectedItems, {
		Text = string.format("+ %s x%d", Data.Name, Data.Count),
		EndTime = Game.Client.LocalTime + 3,
		StrID = Data.StrID
	})
end)
