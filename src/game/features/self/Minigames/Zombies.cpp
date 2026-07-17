#include "core/commands/LoopedCommand.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/features/Features.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"
#include "game/rdr/Ped.hpp"

#include <array>
#include <random>

namespace YimMenu::Features
{
	static BoolCommand _ZombiesLogging{"zombieslogging", "Zombies Logging", "Zombies Logging"};
	static BoolCommand _HardMode{"hardmode", "Hard Mode", "Hard Mode"};

	static constexpr int max_zombies_per_wave = 32;
	static constexpr float spawn_distance     = 65.f;
	static constexpr int max_round            = 20;
	static constexpr int max_dead_zombies     = 12;

	static std::vector<int> zombie_index;
	static std::vector<int> dead_zombies_index;

	static constexpr const std::array zombies_per_round =
		std::to_array({4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 35, 38, 41, 44, 47, 50});

	static int current_round    = 0;
	static int zombies_to_spawn = 0;

	static void CleanupZombiesIndex()
	{
		auto it = zombie_index.begin();

		while (it != zombie_index.end())
		{
			if (!ENTITY::DOES_ENTITY_EXIST(*it) || ENTITY::IS_ENTITY_DEAD(*it))
			{
				if (!ENTITY::IS_ENTITY_DEAD(*it))
					zombie_index.push_back(*it);

				it = zombie_index.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	static void CleanupDeadZombiesIndex()
	{
		auto it = dead_zombies_index.begin();

		while (it != dead_zombies_index.end())
		{
			if (ENTITY::DOES_ENTITY_EXIST(*it))
			{
				if (ENTITY::IS_ENTITY_DEAD(*it))
				{
					PED::DELETE_PED(&(*it));
					it = dead_zombies_index.erase(it);
				}
				else
				{
					++it;
				}
			}
			else
			{
				it = dead_zombies_index.erase(it);
			}
		}
	}

	static void SpawnZombies(const Vector3& player_pos, int player_ped)
	{
		std::random_device rd;
		std::mt19937 generator(rd());

		std::shuffle(
			zombie_index.begin(),
			zombie_index.end(),
			generator);

		std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

		for (int i = zombie_index.size(); i < zombies_to_spawn; ++i)
		{
			float x_offset = distribution(generator) * spawn_distance;
			float y_offset = distribution(generator) * spawn_distance;

			Vector3 spawn_pos =
				ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(
					player_ped,
					x_offset,
					y_offset,
					0.f);

			Ped zombie =
				Ped::Create(
					"G_M_M_UniBanditos_01"_J,
					spawn_pos,
					0.f);

			if (zombie.IsValid())
			{
				PED::SET_PED_KEEP_TASK(
					zombie.GetHandle(),
					true);

				PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(
					zombie.GetHandle(),
					true);

				TASK::TASK_COMBAT_PED(
					zombie.GetHandle(),
					player_ped,
					0,
					16);

				PED::SET_PED_RELATIONSHIP_GROUP_HASH(
					zombie.GetHandle(),
					"HATES_PLAYER"_J);

				WEAPON::GIVE_WEAPON_TO_PED(
					zombie.GetHandle(),
					"WEAPON_REVOLVER_CATTLEMAN"_J,
					999,
					false,
					true,
					0,
					false,
					0.0f,
					0.0f,
					0.0f,
					false,
					0.0f,
					false);

				PED::SET_PED_ACCURACY(
					zombie.GetHandle(),
					35);

				PED::SET_PED_COMBAT_ABILITY(
					zombie.GetHandle(),
					2);

				PED::SET_PED_COMBAT_MOVEMENT(
					zombie.GetHandle(),
					2);

				PED::SET_PED_COMBAT_RANGE(
					zombie.GetHandle(),
					2);

				if (_HardMode.GetState())
				{
					zombie.SetHealth(450);

					PED::ADD_ARMOUR_TO_PED(
						zombie.GetHandle(),
						150);

					PED::SET_PED_MOVE_RATE_OVERRIDE(
						zombie.GetHandle(),
						2.0f);

					PED::SET_PED_COMBAT_ATTRIBUTES(
						zombie.GetHandle(),
						5,
						true);
				}

				zombie_index.push_back(
					zombie.GetHandle());
			}
			else
			{
				if (_ZombiesLogging.GetState())
				{
					LOG(WARNING)
						<< "Failed to spawn a bandit ("
						<< spawn_pos.x << ", "
						<< spawn_pos.y << ", "
						<< spawn_pos.z << ")";
				}
			}
		}
	}

	class UndeadNightmare : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			static int previous_zombie_count = 0;

			Ped player_ped = Self::GetPed();
			Vector3 player_pos = player_ped.GetPosition();

			if (player_ped.GetHealth() <= 0)
			{
				CleanupZombiesIndex();
				CleanupDeadZombiesIndex();

				current_round = 0;
				zombies_to_spawn = 0;

				if (_ZombiesLogging.GetState())
					LOG(INFO) << "Bandit minigame reset!";

				ScriptMgr::Yield(750ms);

				return;
			}

			if (zombie_index.empty())
			{
				current_round =
					std::min(
						current_round + 1,
						max_round);

				zombies_to_spawn =
					zombies_per_round[
						std::min(
							current_round - 1,
							static_cast<int>(zombies_per_round.size()) - 1)];

				if (_ZombiesLogging.GetState())
					LOG(INFO)
						<< "Starting round "
						<< current_round;

				SpawnZombies(
					player_pos,
					player_ped.GetHandle());
			}

			CleanupZombiesIndex();

			if (dead_zombies_index.size() >= max_dead_zombies)
				CleanupDeadZombiesIndex();

			if (zombie_index.size() != previous_zombie_count)
			{
				if (_ZombiesLogging.GetState())
				{
					LOG(INFO)
						<< "Bandits remaining: "
						<< zombie_index.size();
				}

				previous_zombie_count =
					zombie_index.size();
			}
		}

		virtual void OnDisable() override
		{
			CleanupZombiesIndex();
			CleanupDeadZombiesIndex();

			current_round = 0;
			zombies_to_spawn = 0;

			if (_ZombiesLogging.GetState())
				LOG(INFO) << "Bandit minigame reset!";

			ScriptMgr::Yield(750ms);
		}
	};

	static UndeadNightmare _UndeadNightmare{
		"undeadnightmare",
		"Bandit Nightmare",
		"Bandit survival minigame"};
}