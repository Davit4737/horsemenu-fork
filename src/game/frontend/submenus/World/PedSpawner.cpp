#include "PedSpawner.hpp"

#include "CompanionDefense.hpp"
#include "PedOutfitVariations.hpp"
#include "SpawnedPedTracker.hpp"

#include "core/commands/HotkeySystem.hpp"
#include "game/backend/FiberPool.hpp"
#include "game/backend/NativeHooks.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/frontend/items/Items.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/data/PedModels.hpp"

namespace YimMenu::Submenus
{
namespace
{
    static bool IsPedModelInList(const std::string& model)
    {
        return Data::g_PedModels.contains(Joaat(model));
    }

    static int PedSpawnerInputCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
        {
            std::string newText{};
            std::string inputLower = data->Buf;
            std::transform(inputLower.begin(), inputLower.end(), inputLower.begin(), ::tolower);

            for (const auto& [key, model] : Data::g_PedModels)
            {
                std::string modelLower = model;
                std::transform(modelLower.begin(), modelLower.end(), modelLower.begin(), ::tolower);
                if (modelLower.find(inputLower) != std::string::npos)
                {
                    newText = model;
                }
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

    void GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH(rage::scrNativeCallContext* ctx)
    {
        if (ctx->GetArg<int>(0) == "mp_intro"_J)
        {
            ctx->SetReturnValue<int>(1);
        }
        else
        {
            ctx->SetReturnValue<int>(NETWORK::NETWORK_AWARD_HAS_REACHED_MAXCLAIM(ctx->GetArg<int>(0)));
        }
    }

    void _GET_META_PED_TYPE(rage::scrNativeCallContext* ctx)
    {
        ctx->SetReturnValue<int>(4);
    }
}

void RenderPedSpawnerMenu()
{
    CompanionDefense::EnsureRunning();
    ImGui::PushID("peds"_J);

    static auto model_hook = ([]() {
        NativeHooks::AddHook("long_update"_J, NativeIndex::GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH, GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH);
        NativeHooks::AddHook("long_update"_J, NativeIndex::_GET_META_PED_TYPE, _GET_META_PED_TYPE);
        return true;
    }());

    static std::string pedModelBuffer;
    static float scale = 1.0f;
    static bool dead, invis, godmode, freeze, companion, sedated;
    static int formation;

    InputTextWithHint("##pedmodel", "Ped Model", &pedModelBuffer, ImGuiInputTextFlags_CallbackCompletion, nullptr, PedSpawnerInputCallback).Draw();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Press Tab to auto fill");

    if (!pedModelBuffer.empty() && !IsPedModelInList(pedModelBuffer))
    {
        ImGui::BeginListBox("##pedmodels", ImVec2(250, 100));

        std::string bufferLower = pedModelBuffer;
        std::transform(bufferLower.begin(), bufferLower.end(), bufferLower.begin(), ::tolower);

        for (const auto& [hash, model] : Data::g_PedModels)
        {
            std::string pedModelLower = model;
            std::transform(pedModelLower.begin(), pedModelLower.end(), pedModelLower.begin(), ::tolower);
            if (pedModelLower.find(bufferLower) != std::string::npos && ImGui::Selectable(model))
            {
                pedModelBuffer = model;
            }
        }

        ImGui::EndListBox();
    }

    ImGui::Checkbox("Spawn Dead", &dead);
    ImGui::Checkbox("Sedated", &sedated);
    ImGui::Checkbox("Invisible", &invis);
    ImGui::Checkbox("GodMode", &godmode);
    ImGui::Checkbox("Frozen", &freeze);
    ImGui::Checkbox("Companion", &companion);

    if (companion)
    {
        if (ImGui::BeginCombo("Formation", groupFormations[formation]))
        {
            for (const auto& [num, name] : groupFormations)
            {
                bool is_selected = (formation == num);
                if (ImGui::Selectable(name, is_selected))
                {
                    formation = num;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Companions follow you and attack nearby threats.");
    }

    ImGui::SliderFloat("Scale", &scale, 0.1f, 10.0f);

    if (ImGui::Button("Spawn"))
    {
        FiberPool::Push([=] {
            auto ped = Ped::Create(Joaat(pedModelBuffer), Self::GetPed().GetPosition());

            if (!ped)
                return;

            ped.SetFrozen(freeze);

            if (dead)
                ped.Kill();

            ped.SetInvincible(godmode);
            ped.SetVisible(!invis);

            if (scale != 1.0f)
                ped.SetScale(scale);

            ped.SetConfigFlag(PedConfigFlag::IsTranquilized, sedated);

            SpawnedPedTracker::Add(ped);

            if (companion)
                CompanionDefense::RegisterCompanion(ped, formation);
        });
    }

    ImGui::SameLine();
    if (ImGui::Button("Set Model"))
    {
        FiberPool::Push([] {
            auto model = Joaat(pedModelBuffer);

            for (int i = 0; i < 30 && !STREAMING::HAS_MODEL_LOADED(model); i++)
            {
                STREAMING::REQUEST_MODEL(model, false);
                ScriptMgr::Yield();
            }

            PLAYER::SET_PLAYER_MODEL(Self::GetPlayer().GetId(), model, false);
            Self::Update();
            PED::_SET_RANDOM_OUTFIT_VARIATION(Self::GetPed().GetHandle(), true);
            STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
        });
    }

    ImGui::SameLine();
    if (ImGui::Button("Cleanup Peds"))
    {
        FiberPool::Push([] {
            CompanionDefense::ClearAll();

            for (auto it = SpawnedPedTracker::GetSpawned().begin(); it != SpawnedPedTracker::GetSpawned().end();)
            {
                if (it->IsValid())
                {
                    if (it->GetMount())
                    {
                        it->GetMount().ForceControl();
                        it->GetMount().Delete();
                    }

                    it->ForceControl();
                    it->Delete();
                }
                it = SpawnedPedTracker::GetSpawned().erase(it);
            }

            SpawnedPedTracker::Clear();
        });
    }

    RenderPedOutfitVariationMenu();

    ImGui::PopID();
}
}
