#include "core/frontend/Notifications.hpp"
#include "game/commands/PlayerCommand.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Ped.hpp"

namespace YimMenu::Features
{
	class RemoteBolas : public PlayerCommand
	{
		using PlayerCommand::PlayerCommand;

		virtual void OnCall(Player player) override
		{
			auto targetPed = player.GetPed();
			if (!targetPed || !targetPed.IsValid())
			{
				Notifications::Show("Remote Bolas", "Target player is invalid or out of range.", NotificationType::Error);
				return;
			}

			// Validate if the target can actually be lassoed/bolas'd
			bool canUseBolas = PED::_GET_PED_LASSO_HOGTIE_FLAG(targetPed.GetHandle(), (int)LassoFlags::LHF_CAN_BE_LASSOED)
			    && !PED::_GET_PED_LASSO_HOGTIE_FLAG(targetPed.GetHandle(), (int)LassoFlags::LHF_DISABLE_IN_MP);

			if (canUseBolas)
			{
				const auto targetPos = targetPed.GetPosition();
				
				// Fire from 2.5 meters directly above the player's head so it never gets blocked by walls/terrain
				const Vector3 spawnPos = Vector3(targetPos.x, targetPos.y, targetPos.z + 2.5f);

				MISC::SHOOT_SINGLE_BULLET_BETWEEN_COORDS(
					spawnPos.x, spawnPos.y, spawnPos.z,
					targetPos.x, targetPos.y, targetPos.z,
					100,                                 // Damage value (Adjust if you want it non-lethal, though bolas rely on script flags)
					true,                                // Target can be critically hit
					"weapon_thrown_bolas_ironspiked"_J,  // The Bolas projectile hash
					0,                                   // Owner: 0 means world ownership, preventing despawn bugs
					true,                                // Audible explosion/firing audio
					false,                               // Invisible projectile (false means it renders fully)
					150.0f,                              // Projectile velocity speed
					true                                 // Force create projectile
				);
			}
			else
			{
				Notifications::Show("Remote Bolas", "This player has bolas protections enabled!", NotificationType::Error);
			}
		}
	};

	static RemoteBolas _RemoteBolas{"remotebolas", "Remote Bolas", "Remotely strike the player with bolas"};
}