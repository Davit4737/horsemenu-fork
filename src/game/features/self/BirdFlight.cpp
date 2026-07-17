#include "core/commands/FloatCommand.hpp"
#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

#include <cmath>

namespace YimMenu::Features
{
	static constexpr NativeInputs flightControls[] = {
	    NativeInputs::INPUT_SPRINT,
	    NativeInputs::INPUT_MOVE_UP_ONLY,
	    NativeInputs::INPUT_MOVE_DOWN_ONLY,
	    NativeInputs::INPUT_MOVE_LEFT_ONLY,
	    NativeInputs::INPUT_MOVE_RIGHT_ONLY,
	    NativeInputs::INPUT_DUCK,
	    NativeInputs::INPUT_JUMP,
	};

	static FloatCommand _BirdFlightSpeed{"birdflightspeed", "Flight Speed", "Features", 1.0f, 40.0f, 12.0f};

	// This is NOT the game's flight task -- RDR2 has no player-drivable one for bird
	// peds. This drives the ped with velocity instead of teleporting it (the way
	// Noclip does), so collision, landing and perching still behave normally.
	class BirdFlight : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		Ped m_Ped{nullptr};
		bool m_Applied = false;

		void RestorePed(Ped ped)
		{
			if (!ped)
				return;

			ENTITY::SET_ENTITY_HAS_GRAVITY(ped.GetHandle(), true);
			PED::SET_PED_GRAVITY(ped.GetHandle(), true);
			PED::SET_PED_CAN_RAGDOLL(ped.GetHandle(), true);
		}

		virtual void OnTick() override
		{
			auto ped = Self::GetPed();

			if (!ped)
				return;

			// only animal models -- flying Arthur looks like a bug, not a feature
			if (!ped.IsAnimal())
				return;

			// clean up if we swapped ped since last tick
			if (m_Ped != ped)
			{
				RestorePed(m_Ped);
				m_Ped     = ped;
				m_Applied = false;
			}

			const auto handle = ped.GetHandle();

			if (!m_Applied)
			{
				ENTITY::SET_ENTITY_HAS_GRAVITY(handle, false);
				PED::SET_PED_GRAVITY(handle, false);
				PED::SET_PED_CAN_RAGDOLL(handle, false);
				m_Applied = true;
			}

			for (const auto& control : flightControls)
				PAD::DISABLE_CONTROL_ACTION(0, static_cast<int>(control), true);

			rage::fvector3 vel{};

			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_MOVE_UP_ONLY))
				vel.y += 1.0f;
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_MOVE_DOWN_ONLY))
				vel.y -= 1.0f;
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_MOVE_LEFT_ONLY))
				vel.x -= 1.0f;
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_MOVE_RIGHT_ONLY))
				vel.x += 1.0f;
			// Space -- climb
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_JUMP))
				vel.z += 1.0f;
			// Ctrl -- dive
			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_DUCK))
				vel.z -= 1.0f;

			// point the bird where the camera is looking so it banks into turns
			const auto camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
			ped.SetRotation({camRot.x * 0.5f, 0.0f, camRot.z});

			if (vel.x == 0.f && vel.y == 0.f && vel.z == 0.f)
			{
				// hover in place rather than sinking
				ped.SetVelocity({});
				return;
			}

			float speed = _BirdFlightSpeed.GetState();

			if (PAD::IS_DISABLED_CONTROL_PRESSED(0, (int)NativeInputs::INPUT_SPRINT))
				speed *= 2.0f;

			// convert the local input vector into a world-space direction
			const auto position = ped.GetPosition();
			const auto offset   = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(handle, vel.x, vel.y, vel.z);

			rage::fvector3 dir{offset.x - position.x, offset.y - position.y, offset.z - position.z};

			const float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
			if (length > 0.0001f)
			{
				dir.x /= length;
				dir.y /= length;
				dir.z /= length;
			}

			ped.SetVelocity({dir.x * speed, dir.y * speed, dir.z * speed});
		}

		virtual void OnDisable() override
		{
			RestorePed(m_Ped ? m_Ped : Self::GetPed());
			m_Ped     = Ped(nullptr);
			m_Applied = false;
		}
	};

	static BirdFlight _BirdFlight{"birdflight", "Bird Flight", "Fly when your player model is a bird or other animal"};
}
