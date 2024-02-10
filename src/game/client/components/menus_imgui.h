#ifndef GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H
#define GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H

#include <game/client/component.h>

class CMenusImGui : public CComponent
{
	void RenderServerList();

	void MainMenu();

	enum
	{
		MENU_START,
		MENU_SERVERS,
		MENU_SETTINGS
	};

	int m_Menu;

	void AddBack(int Menu);

	void RenderSettings();
	void RenderStartMenu();
	void RenderInGameMenu();

public:
	virtual int Sizeof() const override { return sizeof(*this); }

	void OnInit() override;
	void OnRender() override;
};

#endif // GAME_CLIENT_COMPONENTS_MENUS_IMGUI_H
