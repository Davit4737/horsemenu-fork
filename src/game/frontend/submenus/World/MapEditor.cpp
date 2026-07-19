#include "MapEditor.hpp"

#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/data/MapEditorObjects.hpp"
#include "util/Joaat.hpp"
#include "util/SpawnObject.hpp"

#include <vector>

namespace YimMenu::Submenus
{
	namespace
	{
		// Handles of everything we placed this session, so the editor can undo/clear.
		std::vector<int> g_PlacedObjects;
		int g_SelectedCategory = 0;
		float g_PlaceDistance  = 3.0f;

		void PlaceObject(const std::string& model)
		{
			FiberPool::Push([model] {
				auto ped = Self::GetPed();
				if (!ped || !ped.IsValid())
					return;

				const auto coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(ped.GetHandle(), 0.0f, g_PlaceDistance, 0.0f);
				auto entity       = SpawnObjectEntity(Joaat(model), coords, ped.GetRotation().z);

				if (entity)
					g_PlacedObjects.push_back(entity.GetHandle());
			});
		}

		void UndoLast()
		{
			FiberPool::Push([] {
				while (!g_PlacedObjects.empty())
				{
					const int handle = g_PlacedObjects.back();
					g_PlacedObjects.pop_back();

					if (ENTITY::DOES_ENTITY_EXIST(handle))
					{
						Entity(handle).Delete();
						break;
					}
				}
			});
		}

		void ClearPlaced()
		{
			FiberPool::Push([] {
				for (const int handle : g_PlacedObjects)
				{
					if (ENTITY::DOES_ENTITY_EXIST(handle))
						Entity(handle).Delete();
				}
				g_PlacedObjects.clear();
			});
		}
	}

	void RenderMapEditorMenu()
	{
		ImGui::PushID("map_editor"_J);

		ImGui::TextWrapped("Pick an object and it spawns in front of you - no typing. Use Undo to remove the last piece.");

		ImGui::SetNextItemWidth(160.0f);
		ImGui::SliderFloat("Distance", &g_PlaceDistance, 1.0f, 15.0f, "%.1f m");

		const auto& categories = Data::GetMapEditorCategories();
		if (g_SelectedCategory >= static_cast<int>(categories.size()))
			g_SelectedCategory = 0;

		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::BeginCombo("Category", categories[g_SelectedCategory].name.c_str()))
		{
			for (int i = 0; i < static_cast<int>(categories.size()); i++)
			{
				const bool selected = (i == g_SelectedCategory);
				if (ImGui::Selectable(categories[i].name.c_str(), selected))
					g_SelectedCategory = i;
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		int column = 0;
		for (const auto& object : categories[g_SelectedCategory].objects)
		{
			if (ImGui::Button(object.name.c_str(), ImVec2(130.0f, 0.0f)))
				PlaceObject(object.model);

			if (++column % 3 != 0)
				ImGui::SameLine();
		}
		if (column % 3 != 0)
			ImGui::NewLine();

		ImGui::Separator();
		ImGui::Text("Placed objects: %d", static_cast<int>(g_PlacedObjects.size()));
		if (ImGui::Button("Undo Last"))
			UndoLast();
		ImGui::SameLine();
		if (ImGui::Button("Clear Placed"))
			ClearPlaced();

		ImGui::PopID();
	}
}
