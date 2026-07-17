#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	class Godmode : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			auto ped = Self::GetPed();
			if (!ped)
				return;

			auto player = Self::GetPlayer();
			if (!player.IsValid())
				return;

			const auto handle = ped.GetHandle();

			if (ped.IsDead())
			{
				ENTITY::SET_ENTITY_CAN_BE_DAMAGED(handle, true);
				return;
			}

			ENTITY::SET_ENTITY_CAN_BE_DAMAGED(handle, false);

			if (ped.GetHealth() < ped.GetMaxHealth())
				ped.SetHealth(ped.GetMaxHealth());

			ped.SetTargetActionDisableFlag(13, true);
			ped.SetTargetActionDisableFlag(16, true);
			ped.SetTargetActionDisableFlag(17, true);
		}

		virtual void OnDisable() override
		{
			auto ped = Self::GetPed();
			if (!ped)
				return;

			const auto handle = ped.GetHandle();

			ENTITY::SET_ENTITY_CAN_BE_DAMAGED(handle, true);

			ped.SetTargetActionDisableFlag(13, false);
			ped.SetTargetActionDisableFlag(16, false);
			ped.SetTargetActionDisableFlag(17, false);
		}
	};

	static Godmode _Godmode{"godmode", "God Mode", "Blocks all incoming damage"};
}