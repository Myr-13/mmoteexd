#include "menus_imgui.h"

#include <engine/shared/lua/lua_state.h>
#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/client/imgui/imgui_custom.h>
#include <engine/serverbrowser.h>
#include <engine/client.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>

void CMenusImGui::AddBack(int Menu)
{
	AddChangeMenu(Menu, "Back");
}

void CMenusImGui::AddChangeMenu(int Menu, const char *pText)
{
	if(ImGui::ButtonFull(pText, 30))
		m_Menu = Menu;
}

void CMenusImGui::ToggleMenu()
{
	m_ShowMenu ^= 1;

	if(m_ShowMenu)
	{
		Input()->MouseModeAbsolute();

		int x = Graphics()->ScreenWidth() / 2;
		int y = Graphics()->ScreenHeight() / 2;
		Graphics()->SetMousePosition(x, y);
	}
	else
		Input()->MouseModeRelative();
}

void CMenusImGui::OnInit()
{
	m_Menu = MENU_START;
	m_OnlyShowMMOServers = true;
	m_ShowMenu = true;

	// Init style
	// https://github.com/simongeilfus/Cinder-ImGui
	ImGuiStyle &Style = ImGui::GetStyle();

	Style.WindowMinSize        = ImVec2(160, 20);
	Style.FramePadding         = ImVec2(4, 2);
	Style.ItemSpacing          = ImVec2(6, 2);
	Style.ItemInnerSpacing     = ImVec2(6, 4);
	Style.Alpha                = 0.95f;
	Style.WindowRounding       = 4.0f;
	Style.FrameRounding        = 2.0f;
	Style.IndentSpacing        = 6.0f;
	Style.ItemInnerSpacing     = ImVec2(2, 4);
	Style.ColumnsMinSpacing    = 50.0f;
	Style.GrabMinSize          = 14.0f;
	Style.GrabRounding         = 16.0f;
	Style.ScrollbarSize        = 12.0f;
	Style.ScrollbarRounding    = 16.0f;

	Style.Colors[ImGuiCol_Text]                  = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
	Style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
	Style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	Style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
	Style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	Style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	Style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	Style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	Style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
	Style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
	Style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	Style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
	Style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	Style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
	Style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	Style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
	Style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	Style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
	Style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
	Style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
	Style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	Style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
	Style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
	Style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
	Style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
	Style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
	Style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.22f, 0.27f, 0.90f);
	Style.Colors[ImGuiCol_Tab]                   = ImVec4(0.70f, 0.10f, 0.15f, 1.00f);
	Style.Colors[ImGuiCol_TabActive]             = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
	Style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
}

void CMenusImGui::OnRender()
{
	MainMenu();
}

bool CMenusImGui::OnInput(const IInput::CEvent &Event)
{
	if(Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
		ToggleMenu();

	LUA_FIRE_EVENT("OnInput", Event)

	return false;
}

void CMenusImGui::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_OFFLINE)
	{
		Input()->MouseModeRelative();
		m_Menu = MENU_INGAME;
		m_ShowMenu = false;
	}
	else
	{
		Input()->MouseModeAbsolute();
		m_Menu = MENU_START;
		m_ShowMenu = true;
	}
}

void CMenusImGui::MainMenu()
{
	if(!m_ShowMenu)
		return;

	ImGui::Begin("Main menu", 0x0, ImGuiWindowFlags_NoTitleBar);

	switch(m_Menu)
	{
	case MENU_START: RenderStartMenu(); break;
	case MENU_INGAME: RenderInGameMenu(); break;
	case MENU_SERVERS: RenderServers(); break;
	case MENU_SETTINGS: RenderSettings(); break;
	}

	ImGui::End();
}

void CMenusImGui::RenderServerList()
{
	if(ServerBrowser()->IsGettingServerlist())
	{
		ImGui::Text("Refreshing...");
		return;
	}

	const int NumServers = ServerBrowser()->NumSortedServers();

	if(ImGui::BeginTable("servers_table", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Ping");
		ImGui::TableSetupColumn("Actions");
		ImGui::TableHeadersRow();

		for(int i = 0; i < NumServers; i++)
		{
			const CServerInfo *pItem = ServerBrowser()->SortedGet(i);

			if(m_OnlyShowMMOServers && str_comp_nocase(pItem->m_aGameType, "MMOTee") != 0)
				continue;

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", pItem->m_aName);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%d", pItem->m_Latency);

			ImGui::TableSetColumnIndex(2);
			ImGui::PushID(i);
			if(ImGui::Button("Connect"))
				Client()->Connect(pItem->m_aAddress);
			ImGui::PopID();
		}

		ImGui::EndTable();
	}
}

void CMenusImGui::RenderServers()
{
	AddBack(MENU_START);

	if(ImGui::ButtonFull("Refresh", 30))
		ServerBrowser()->Refresh(ServerBrowser()->GetCurrentType());
	ImGui::Checkbox("Show only mmotee servers", &m_OnlyShowMMOServers);

	if(ImGui::BeginTabBar("##server_tabs"))
	{
		if(ImGui::BeginTabItem("Internet"))
		{
			if(ServerBrowser()->GetCurrentType() != IServerBrowser::TYPE_INTERNET)
				ServerBrowser()->Refresh(IServerBrowser::TYPE_INTERNET);

			RenderServerList();

			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("LAN"))
		{
			if(ServerBrowser()->GetCurrentType() != IServerBrowser::TYPE_LAN)
				ServerBrowser()->Refresh(IServerBrowser::TYPE_LAN);

			RenderServerList();

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void CMenusImGui::RenderSettings()
{
	AddBack((Client()->State() == IClient::STATE_ONLINE) ? MENU_INGAME : MENU_START);

	if(ImGui::CollapsingHeader("Player"))
	{
		ImGui::InputText("Player name", g_Config.m_PlayerName, sizeof(g_Config.m_PlayerName));
	}

	if(ImGui::CollapsingHeader("Customization"))
	{
		if(ImGui::BeginTabBar("##customization"))
		{
			if(ImGui::BeginTabItem("Sizes"))
			{
				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Colors"))
			{
				ImGuiStyle &Style = ImGui::GetStyle();

				for(int i = 0; i < ImGuiCol_COUNT; i++)
				{
					ImGui::PushID(i);
					ImGui::ColorEdit4("##color", (float *)&Style.Colors[i], ImGuiColorEditFlags_AlphaBar);
					ImGui::SameLine();
					ImGui::TextUnformatted(ImGui::GetStyleColorName(i));
					ImGui::PopID();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}

void CMenusImGui::RenderStartMenu()
{
	AddChangeMenu(MENU_SERVERS, "Play");
	AddChangeMenu(MENU_SETTINGS, "Settings");
	if(ImGui::ButtonFull("Quit", 30))
		Client()->Quit();
}

void CMenusImGui::RenderInGameMenu()
{
	AddChangeMenu(MENU_SETTINGS, "Settings");

	if(ImGui::ButtonFull("Disconnect", 30))
		Client()->Disconnect();
	if(ImGui::ButtonFull("Quit", 30))
		Client()->Quit();
}
