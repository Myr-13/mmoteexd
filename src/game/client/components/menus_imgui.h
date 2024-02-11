#ifndef GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H
#define GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H

#include <game/client/component.h>

class CMenusImGui : public CComponent
{
	enum
	{
		MENU_START,
		MENU_INGAME,
		MENU_SERVERS,
		MENU_SETTINGS
	};

	int m_Menu;
	bool m_OnlyShowMMOServers;
	bool m_ShowMenu;

	void AddBack(int Menu);
	void AddChangeMenu(int Menu, const char *pText);
	void ToggleMenu();

	void MainMenu();

	void RenderServers();
	void RenderServerList();
	void RenderSettings();
	void RenderStartMenu();
	void RenderInGameMenu();

public:
	virtual int Sizeof() const override { return sizeof(*this); }

	void OnInit() override;
	void OnRender() override;
	bool OnInput(const IInput::CEvent &Event) override;
	void OnStateChange(int NewState, int OldState) override;
};

#endif // GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H
