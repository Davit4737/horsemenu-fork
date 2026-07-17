#include "Players.hpp"

#include "core/frontend/widgets/imgui_colors.h"
#include "game/backend/PlayerData.hpp"
#include "game/backend/PlayerDatabase.hpp"
#include "game/backend/Players.hpp"
#include "game/features/Features.hpp"
#include "game/frontend/items/Items.hpp"

#include "Player/Helpful.hpp"
#include "Player/Info.hpp"
#include "Player/Kick.hpp"
#include "Player/Toxic.hpp"
#include "Player/Trolling.hpp"

#include <algorithm>
#include <vector>

namespace YimMenu::Submenus
{
	struct Tag
	{
		std::string Name;
		ImVec4 Color;
	};

	static std::vector<Tag> GetPlayerTags(YimMenu::Player player)
	{
		std::vector<Tag> tags;

		if (player.IsHost())
			tags.push_back({"HOST", ImGui::Colors::DeepSkyBlue});

		if (player.IsModder())
			tags.push_back({"MOD", ImGui::Colors::DeepPink});

		if (player.GetPed() && player.GetPed().IsInvincible())
			tags.push_back({"GOD", ImGui::Colors::Crimson});

		if (player.GetPed() && !player.GetPed().IsVisible())
			tags.push_back({"INVIS", ImGui::Colors::MediumPurple});

		return tags;
	}

	static void DrawPlayerList(bool external = true, float offset = 15.0f)
	{
		// Copy players into a vector so we can reliably sort them alphabetically
		std::vector<std::pair<uint8_t, YimMenu::Player>> sortedPlayers;
		for (const auto& [id, player] : YimMenu::Players::GetPlayers())
		{
			sortedPlayers.push_back({id, player});
		}

		// FIXED: Removed const from parameters so GetName() can be safely called
		std::sort(sortedPlayers.begin(), sortedPlayers.end(), [](auto& a, auto& b) {
			return a.second.GetName() < b.second.GetName();
		});

		if (external)
		{
			ImGui::SetNextWindowPos(
			    ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x + offset, ImGui::GetWindowPos().y));
			ImGui::SetNextWindowSize(ImVec2(230, ImGui::GetWindowSize().y)); // Slightly widened for tag room
			ImGui::Begin("Player List", nullptr, ImGuiWindowFlags_NoDecoration);

			ImGui::Checkbox("Spectate", &YimMenu::g_Spectating);
			ImGui::Separator();

			for (auto& [id, player] : sortedPlayers)
			{
				std::string display_name = player.GetName();

				ImGui::PushID(id);
				ImGui::BeginGroup(); // Grouping name + tags together for unified mouse hover tracking

				if (ImGui::Selectable(display_name.c_str(), (YimMenu::Players::GetSelected() == player), ImGuiSelectableFlags_AllowOverlap))
				{
					YimMenu::Players::SetSelected(id);
				}

				auto tags = GetPlayerTags(player);
				if (!tags.empty())
				{
					// Safe Style spacing implementation using PushStyleVar
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));
					for (auto& tag : tags)
					{
						ImGui::SameLine();
						ImGui::PushStyleColor(ImGuiCol_Text, tag.Color);
						ImGui::TextUnformatted(("[" + tag.Name + "]").c_str());
						ImGui::PopStyleColor();
					}
					ImGui::PopStyleVar();
				}

				ImGui::EndGroup();
				
				// Handle modder tooltip correctly across the entire item group area
				if (player.IsModder() && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
				{
					ImGui::BeginTooltip();
					for (auto detection : player.GetData().m_Detections)
					{
						ImGui::BulletText("%s", g_PlayerDatabase->ConvertDetectionToDescription(detection).c_str());
					}
					ImGui::EndTooltip();
				}

				ImGui::PopID();
			}
			ImGui::End();
		}
		else
		{
			if (ImGui::BeginCombo("Players", YimMenu::Players::GetSelected().GetName()))
			{
				for (auto& [id, player] : sortedPlayers)
				{
					if (ImGui::Selectable(player.GetName(), (YimMenu::Players::GetSelected() == player)))
					{
						YimMenu::Players::SetSelected(id);
					}
				}
				ImGui::EndCombo();
			}
		}
	}

	Players::Players() :
	    Submenu::Submenu("Players")
	{
		AddCategory(std::move(BuildInfoMenu()));
		AddCategory(std::move(BuildHelpfulMenu()));
		AddCategory(std::move(BuildTrollingMenu()));
		AddCategory(std::move(BuildToxicMenu()));
		AddCategory(std::move(BuildKickMenu()));

		for (auto& category : m_Categories)
		{
			category->PrependItem(std::make_shared<ImGuiItem>([] {
				DrawPlayerList();
			}));
		}
	}
}