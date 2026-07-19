#include "HorseCustomization.hpp"

#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Ped.hpp"
#include "game/rdr/data/HorseCoats.hpp"
#include "util/Joaat.hpp"

namespace YimMenu::Submenus
{
	namespace
	{
		int g_GroupIndex   = 0;
		int g_CoatIndex    = 0;
		float g_HorseScale = 1.0f;
		bool g_Invincible  = true;

		const Data::HorseCoat& SelectedCoat()
		{
			const auto& groups = Data::GetHorseCoatGroups();
			const auto& group  = groups[g_GroupIndex];
			return group.coats[g_CoatIndex];
		}

		void SpawnAndRide(const std::string& model)
		{
			FiberPool::Push([model] {
				auto self = Self::GetPed();
				if (!self || !self.IsValid())
					return;

				const auto coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(self.GetHandle(), 0.0f, 2.0f, 0.0f);
				auto horse        = Ped::Create(Joaat(model), coords, self.GetRotation().z);
				if (!horse)
					return;

				if (g_Invincible)
					horse.SetInvincible(true);

				if (g_HorseScale != 1.0f)
					horse.SetScale(g_HorseScale);

				horse.SetHealth(horse.GetMaxHealth());
				self.SetInMount(horse);
			});
		}

		void ApplyToCurrentMount()
		{
			FiberPool::Push([] {
				auto mount = Self::GetMount();
				if (!mount || !mount.IsValid())
					return;

				mount.SetInvincible(g_Invincible);
				mount.SetScale(g_HorseScale);
				mount.SetHealth(mount.GetMaxHealth());
			});
		}
	}

	void RenderHorseCustomizationMenu()
	{
		ImGui::PushID("horse_customization"_J);

		const auto& groups = Data::GetHorseCoatGroups();
		if (g_GroupIndex >= static_cast<int>(groups.size()))
			g_GroupIndex = 0;

		ImGui::TextWrapped("Pick a coat and ride out on a fresh horse - the 'Reds & Chestnuts' set covers the fiery red look.");

		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::BeginCombo("Coat Type", groups[g_GroupIndex].name.c_str()))
		{
			for (int i = 0; i < static_cast<int>(groups.size()); i++)
			{
				const bool selected = (i == g_GroupIndex);
				if (ImGui::Selectable(groups[i].name.c_str(), selected))
				{
					g_GroupIndex = i;
					g_CoatIndex  = 0;
				}
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		const auto& coats = groups[g_GroupIndex].coats;
		if (g_CoatIndex >= static_cast<int>(coats.size()))
			g_CoatIndex = 0;

		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::BeginCombo("Coat", coats[g_CoatIndex].name.c_str()))
		{
			for (int i = 0; i < static_cast<int>(coats.size()); i++)
			{
				const bool selected = (i == g_CoatIndex);
				if (ImGui::Selectable(coats[i].name.c_str(), selected))
					g_CoatIndex = i;
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SliderFloat("Size", &g_HorseScale, 0.5f, 2.5f, "%.2fx");
		ImGui::Checkbox("Invincible", &g_Invincible);

		ImGui::Separator();

		if (ImGui::Button("Spawn & Ride This Horse"))
			SpawnAndRide(SelectedCoat().model);

		if (ImGui::Button("Apply Size / Invincible To Current Horse"))
			ApplyToCurrentMount();

		ImGui::PopID();
	}
}
