#include "ObjectSpawner.hpp"

#include "game/backend/FiberPool.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/data/ObjectModels.hpp"
#include "util/SpawnObject.hpp"

#include <algorithm>

namespace YimMenu::Submenus
{
	namespace
	{
		static bool IsObjectModelInList(const std::string& model)
		{
			for (const auto& objectModel : Data::g_ObjectModels)
			{
				if (objectModel.model == model)
					return true;
			}
			return false;
		}

		static int ObjectSpawnerInputCallback(ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
			{
				std::string newText{};
				std::string inputLower = data->Buf;
				std::transform(inputLower.begin(), inputLower.end(), inputLower.begin(), ::tolower);
				for (const auto& objectModel : Data::g_ObjectModels)
				{
					std::string modelLower = objectModel.model;
					std::transform(modelLower.begin(), modelLower.end(), modelLower.begin(), ::tolower);
					if (modelLower.find(inputLower) != std::string::npos)
						newText = objectModel.model;
				}

				if (!newText.empty())
				{
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, newText.c_str());
				}

				return 1;
			}
			return 0;
		}
	}

	void RenderObjectSpawnerMenu()
	{
		ImGui::PushID("object_spawner"_J);

		static std::string objectModelBuffer;
		InputTextWithHint("##objectmodel", "Object Model", &objectModelBuffer, ImGuiInputTextFlags_CallbackCompletion, nullptr, ObjectSpawnerInputCallback)
			.Draw();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Press Tab to auto fill");

		if (!objectModelBuffer.empty() && !IsObjectModelInList(objectModelBuffer))
		{
			ImGui::BeginListBox("##objectmodels", ImVec2(250, 100));

			std::string bufferLower = objectModelBuffer;
			std::transform(bufferLower.begin(), bufferLower.end(), bufferLower.begin(), ::tolower);
			for (const auto& objectModel : Data::g_ObjectModels)
			{
				std::string modelLower = objectModel.model;
				std::transform(modelLower.begin(), modelLower.end(), modelLower.begin(), ::tolower);
				if (modelLower.find(bufferLower) != std::string::npos && ImGui::Selectable(objectModel.model.c_str()))
					objectModelBuffer = objectModel.model;
			}

			ImGui::EndListBox();
		}

		if (ImGui::Button("Spawn Object"))
		{
			// FIXED: We capture a local copy of the string here so the Fiber doesn't read corrupted memory across threads.
			std::string modelToSpawn = objectModelBuffer;

			FiberPool::Push([modelToSpawn] {
				auto ped = Self::GetPed();
				
				// Added a .IsValid() check just to be perfectly safe before requesting coordinates
				if (!ped || !ped.IsValid())
					return;

				auto spawnCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(ped.GetHandle(), 0.0f, 2.5f, 0.0f);
				
				// Hashing the thread-safe copy of our string
				(void)SpawnObjectEntity(Joaat(modelToSpawn), spawnCoords, ped.GetRotation().z);
			});
		}

		ImGui::PopID();
	}
}