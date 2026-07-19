#include "AnimalPlayerAbilities.hpp"

#include "game/backend/Self.hpp"
#include "game/frontend/GUI.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

#include <cmath>

// When you SET_PLAYER_MODEL to an animal (wolf, cougar, bird, ...) the game hands
// you the animal ped but leaves it unable to fight: the melee task is disabled and
// birds keep their normal gravity so they can never take off. This handler runs
// every frame from the feature loop and, whenever your ped is an animal, quietly
// re-enables the attack move set and - for birds - swaps in velocity based flight.
// There is deliberately no menu toggle; it just works while you are an animal.

namespace YimMenu::Features
{
	namespace
	{
		constexpr float kDegToRad = 0.017453292f;

		int g_ConfiguredModel = 0; // the model we last set up, so we only touch flags on change
		int g_AttackCooldown  = 0; // frames to wait before re-issuing the lunge task

		bool IsBird(int handle)
		{
			return PED::_GET_IS_BIRD(handle);
		}

		// Kills the wildlife "run away from the player" tuning so the animal stays put
		// and obeys you instead of constantly fleeing.
		void DisableAnimalFlee(int handle)
		{
			static constexpr int fleeParams[] = {104, 105, 10, 146, 113, 114, 115, 116, 117, 118, 119, 111, 107};
			for (const auto param : fleeParams)
				FLOCK::SET_ANIMAL_TUNING_FLOAT_PARAM(handle, param, 0.0f);
		}

		// Clears the flags that stop an animal player model from meleeing and makes it
		// combat capable so the attack tasks actually play out.
		void MakeCombatReady(int handle)
		{
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::DisableMelee, false);
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::DisableMeleeHitReactions, false);
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::DisableMeleeKnockout, false);
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::DisableMeleeTargetSwitch, false);
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::CanAttackFriendly, true);
			PED::SET_PED_CONFIG_FLAG(handle, (int)PedConfigFlag::TreatAsPlayerDuringTargeting, true);

			PED::SET_PED_COMBAT_ATTRIBUTES(handle, 46, true); // can fight armed peds while unarmed
			PED::SET_PED_COMBAT_ATTRIBUTES(handle, 5, true);  // always fight
			PED::SET_PED_COMBAT_ATTRIBUTES(handle, 1, true);
			PED::SET_PED_COMBAT_ABILITY(handle, 2);
			PED::SET_PED_COMBAT_MOVEMENT(handle, 2);
			PED::SET_PED_COMBAT_RANGE(handle, 2);
			PED::SET_PED_FLEE_ATTRIBUTES(handle, 0, false);
			PED::SET_PED_CAN_RAGDOLL(handle, true);
			FLOCK::_SET_ANIMAL_IS_WILD(handle, false);
		}

		bool IsValidAttackTarget(int handle, int selfHandle)
		{
			if (!handle || handle == selfHandle)
				return false;
			if (!ENTITY::DOES_ENTITY_EXIST(handle) || !ENTITY::IS_ENTITY_A_PED(handle))
				return false;
			if (PED::IS_PED_DEAD_OR_DYING(handle, true))
				return false;
			return true;
		}

		// Prefer whatever you are aiming at / locked onto, so you can pick a fight
		// deliberately.
		int GetAimedPedTarget(int playerId, int selfHandle)
		{
			int target = 0;
			if (PLAYER::GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &target) && IsValidAttackTarget(target, selfHandle))
				return target;

			if (PLAYER::GET_PLAYER_TARGET_ENTITY(playerId, &target) && IsValidAttackTarget(target, selfHandle))
				return target;

			return 0;
		}

		// Fallback so you can just mash attack and lunge at the nearest thing, with no
		// need to lock on first.
		int GetNearestAttackTarget(int selfHandle, rage::fvector3 pos)
		{
			int closest = 0;
			if (PED::GET_CLOSEST_PED(pos.x, pos.y, pos.z, 14.0f, true, true, &closest, false, false, false, -1)
			    && IsValidAttackTarget(closest, selfHandle))
				return closest;
			return 0;
		}

		void TickAttack(int selfHandle, int playerId, rage::fvector3 pos)
		{
			if (g_AttackCooldown > 0)
			{
				g_AttackCooldown--;
				return;
			}

			const bool wantsAttack = PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_ATTACK)
			    || PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_MELEE_ATTACK)
			    || PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_ATTACK2);
			if (!wantsAttack)
				return;

			int target = GetAimedPedTarget(playerId, selfHandle);
			if (!target)
				target = GetNearestAttackTarget(selfHandle, pos);
			if (!target)
				return;

			if (PED::IS_PED_IN_COMBAT(selfHandle, target))
				return;

			if (IsBird(selfHandle))
				TASK::TASK_COMBAT_PED(selfHandle, target, 0, 16);
			else
				TASK::TASK_COMBAT_ANIMAL_CHARGE_PED(selfHandle, target, true, 0, 0, 0, 0);

			g_AttackCooldown = 24; // let the strike play before re-issuing
		}

		void TickBirdFlight(Ped ped)
		{
			const int handle = ped.GetHandle();

			PED::SET_PED_GRAVITY(handle, false);
			ENTITY::SET_ENTITY_HAS_GRAVITY(handle, false);
			ped.SetRagdoll(false);

			const float moveY = -PAD::GET_CONTROL_NORMAL(0, (Hash)NativeInputs::INPUT_MOVE_UD); // forward positive
			const float moveX = PAD::GET_CONTROL_NORMAL(0, (Hash)NativeInputs::INPUT_MOVE_LR);

			float up = 0.0f;
			if (PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_JUMP))
				up += 1.0f;
			if (PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_DUCK))
				up -= 1.0f;

			const bool sprint = PAD::IS_CONTROL_PRESSED(0, (Hash)NativeInputs::INPUT_SPRINT);
			const float speed = sprint ? 24.0f : 12.0f;

			if (std::abs(moveX) < 0.05f && std::abs(moveY) < 0.05f && up == 0.0f)
			{
				// no input: hover in place and gently bleed off momentum
				const auto v = ped.GetVelocity();
				ped.SetVelocity({v.x * 0.85f, v.y * 0.85f, 0.0f});
				return;
			}

			const auto camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
			const float yaw    = camRot.z * kDegToRad;
			const float pitch  = camRot.x * kDegToRad;
			const float cosP   = std::cos(pitch);

			const rage::fvector3 forward{-std::sin(yaw) * cosP, std::cos(yaw) * cosP, std::sin(pitch)};
			const rage::fvector3 right{std::cos(yaw), std::sin(yaw), 0.0f};

			rage::fvector3 dir{
			    forward.x * moveY + right.x * moveX,
			    forward.y * moveY + right.y * moveX,
			    forward.z * moveY + up,
			};

			const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
			if (len > 0.001f)
			{
				dir.x /= len;
				dir.y /= len;
				dir.z /= len;
			}

			ped.SetVelocity({dir.x * speed, dir.y * speed, dir.z * speed});
			ENTITY::SET_ENTITY_HEADING(handle, camRot.z);
		}
	}

	void TickAnimalAbilities()
	{
		auto ped = Self::GetPed();
		if (!ped || !ped.IsValid())
			return;

		const int handle = ped.GetHandle();

		if (!ped.IsAnimal())
		{
			// We stopped being an animal (e.g. changed back to Arthur): undo the flight
			// gravity tweak once so a human model behaves normally again.
			if (g_ConfiguredModel != 0)
			{
				PED::SET_PED_GRAVITY(handle, true);
				ENTITY::SET_ENTITY_HAS_GRAVITY(handle, true);
				g_ConfiguredModel = 0;
			}
			return;
		}

		const int model = ped.GetModel();
		if (model != g_ConfiguredModel)
		{
			ped.ForceControl();
			MakeCombatReady(handle);
			DisableAnimalFlee(handle);
			PED::SET_PED_KEEP_TASK(handle, true);
			g_ConfiguredModel = model;
		}

		// Don't fight the menu for control while it is open.
		if (GUI::IsOpen())
			return;

		if (IsBird(handle))
			TickBirdFlight(ped);

		TickAttack(handle, Self::GetPlayer().GetId(), ped.GetPosition());
	}
}
