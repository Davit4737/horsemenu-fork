#include "game/features/players/PlayerPedControl.hpp"
#include "game/features/world/WorldEventUtils.hpp"

#include "game/backend/ScriptMgr.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Ped.hpp"

#include <cstdint>

namespace YimMenu::Features
{
	namespace
	{
		Ped g_Stalker{0};

		// 0 = use the target player's current model.
		// Set this to any valid model hash later if you want to force a specific model.
		uint32_t g_StalkerModel = 0;

		void CleanupStalker()
		{
			if (g_Stalker && g_Stalker.IsValid())
				g_Stalker.Delete();

			g_Stalker = Ped(0);
		}

		bool SpawnStalker(Player player)
		{
			auto target = player.GetPed();
			if (!target.IsValid())
				return false;

			if (!EnsurePedControl(target))
				return false;

			CleanupStalker();

			const auto targetPos = target.GetPosition();
			const auto offset    = WorldEvents::RandomHorizOffset(45.0f, 80.0f);

			float groundZ = targetPos.z;
			if (!WorldEvents::TryGetGroundZ(targetPos.x + offset.x, targetPos.y + offset.y, targetPos.z + 250.0f, groundZ))
				groundZ = targetPos.z;

			const rage::fvector3 spawn{
			    targetPos.x + offset.x,
			    targetPos.y + offset.y,
			    groundZ + 0.4f};

			const uint32_t model = g_StalkerModel != 0 ? g_StalkerModel : static_cast<uint32_t>(target.GetModel());

			auto stalker = Ped::Create(model, spawn, 0.0f);
			if (!stalker.IsValid())
				return false;

			const auto stalkerHandle = stalker.GetHandle();
			const auto targetHandle  = target.GetHandle();

			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(stalkerHandle, true, true);
			ENTITY::SET_ENTITY_VISIBLE(stalkerHandle, true);
			ENTITY::SET_ENTITY_INVINCIBLE(stalkerHandle, true);
			ENTITY::SET_ENTITY_COLLISION(stalkerHandle, true, true);
			ENTITY::SET_ENTITY_DYNAMIC(stalkerHandle, true);

			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(stalkerHandle, true);
			PED::SET_PED_KEEP_TASK(stalkerHandle, true);
			PED::SET_PED_CAN_RAGDOLL(stalkerHandle, false);

			TASK::TASK_FOLLOW_TO_OFFSET_OF_ENTITY(
			    stalkerHandle,
			    targetHandle,
			    0.0f,
			    -6.0f,
			    0.0f,
			    1.1f,
			    -1,
			    6.0f,
			    true,
			    true,
			    true,
			    true,
			    true,
			    true);

			g_Stalker = stalker;
			return true;
		}
	}

	class ShadowCloneStalker : public PlayerCommand
	{
	public:
		ShadowCloneStalker() :
		    PlayerCommand("shadowclonestalker", "Shadow Clone Stalker", "Spawns a creepy clone that follows the target player")
		{
		}

		virtual void OnCall(Player player) override
		{
			SpawnStalker(player);
		}
	};

	static ShadowCloneStalker _ShadowCloneStalker;
}