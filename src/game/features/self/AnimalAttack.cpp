#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "util/Joaat.hpp"

namespace YimMenu::Features
{
	// When you SET_PLAYER_MODEL to an animal you get the animal ped, but the game
	// leaves it in a state where the melee task never starts:
	//   - PedConfigFlag::DisableMelee is set on non-human player models
	//   - there is nothing in the weapon slot, so INPUT_MELEE_ATTACK gets eaten
	// Clearing the flags and forcing WEAPON_UNARMED in hand lets the animal's own
	// attack move set drive the strike.
	class AnimalAttack : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		int m_AppliedModel = 0;

		static void ApplyMeleeCapability(Ped ped)
		{
			const auto handle = ped.GetHandle();

			ped.SetConfigFlag(PedConfigFlag::DisableMelee, false);
			ped.SetConfigFlag(PedConfigFlag::DisableMeleeHitReactions, false);
			ped.SetConfigFlag(PedConfigFlag::DisableMeleeKnockout, false);
			ped.SetConfigFlag(PedConfigFlag::DisableMeleeTargetSwitch, false);

			// animals are not "friendly" to the player group by default, but without
			// this the lock-on refuses to acquire anything while you are one
			ped.SetConfigFlag(PedConfigFlag::CanAttackFriendly, true);
			ped.SetConfigFlag(PedConfigFlag::TreatAsPlayerDuringTargeting, true);

			// the animal move set hangs off the unarmed slot
			WEAPON::GIVE_WEAPON_TO_PED(handle, "WEAPON_UNARMED"_J, 0, true, false, 0, false, 0.5f, 1.0f, 0, false, 0.0f, false);
			WEAPON::SET_CURRENT_PED_WEAPON(handle, "WEAPON_UNARMED"_J, true, 0, false, false);

			PED::SET_PED_CAN_BE_TARGETTED(handle, true);
		}

		virtual void OnTick() override
		{
			auto ped = Self::GetPed();

			if (!ped)
				return;

			// only touch animal models, leave Arthur/John alone
			if (!ped.IsAnimal())
			{
				m_AppliedModel = 0;
				return;
			}

			// re-apply on model change only, spamming these every frame fights the game
			const auto model = ped.GetModel();
			if (model != m_AppliedModel)
			{
				ApplyMeleeCapability(ped);
				m_AppliedModel = model;
			}

			PAD::ENABLE_CONTROL_ACTION(0, static_cast<int>(NativeInputs::INPUT_MELEE_ATTACK), true);
		}

		virtual void OnDisable() override
		{
			auto ped = Self::GetPed();

			if (ped && ped.IsAnimal())
			{
				ped.SetConfigFlag(PedConfigFlag::CanAttackFriendly, false);
				ped.SetConfigFlag(PedConfigFlag::TreatAsPlayerDuringTargeting, false);
			}

			m_AppliedModel = 0;
		}
	};

	static AnimalAttack _AnimalAttack{"animalattack", "Animal Attack", "Lets animal player models use their melee attack"};
}
