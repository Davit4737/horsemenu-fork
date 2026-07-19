#include "WorldEventUtils.hpp"

#include "core/commands/LoopedCommand.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

#include <vector>

namespace YimMenu::Features
{
	namespace
	{
		struct Fireball
		{
			rage::fvector3 position{};
			float groundZ = 0.0f;
			float speed   = 0.0f;
		};

		std::vector<Fireball> g_Fireballs;
		int g_TickCounter = 0;

		constexpr int kSpawnInterval = 10; // ticks between new fireballs
		constexpr int kMaxFireballs  = 12;

		void SpawnFireball(const rage::fvector3& center)
		{
			const auto offset = WorldEvents::RandomHorizOffset(10.0f, 55.0f);
			const float x     = center.x + offset.x;
			const float y     = center.y + offset.y;

			float groundZ = center.z;
			WorldEvents::TryGetGroundZ(x, y, center.z + 80.0f, groundZ);

			const float skyZ  = groundZ + WorldEvents::RandomRange(55.0f, 85.0f);
			const float speed = WorldEvents::RandomRange(18.0f, 30.0f);

			g_Fireballs.push_back({{x, y, skyZ}, groundZ, speed});
		}

		void UpdateFireballs()
		{
			const float deltaTime = MISC::GET_FRAME_TIME();

			for (auto it = g_Fireballs.begin(); it != g_Fireballs.end();)
			{
				auto& fb = *it;
				fb.position.z -= fb.speed * deltaTime;

				WorldEvents::PlayFireBurstAt(fb.position.x, fb.position.y, fb.position.z, 0.6f);
				GRAPHICS::DRAW_LIGHT_WITH_RANGE(fb.position.x, fb.position.y, fb.position.z, 255, 90, 20, 6.0f, 2.0f);

				if (fb.position.z <= fb.groundZ + 0.5f)
				{
					FIRE::ADD_EXPLOSION(fb.position.x, fb.position.y, fb.groundZ + 0.4f, static_cast<int>(ExplosionTypes::MOLOTOV_TYPE_EXPLOSION_FIRE), 0.8f, true, false, 0.8f);
					FIRE::START_SCRIPT_FIRE(fb.position.x, fb.position.y, fb.groundZ, 20, 1.5f, true, "", 0.0f, 0);
					it = g_Fireballs.erase(it);
					continue;
				}

				++it;
			}
		}
	}

	class RainOfFire : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnEnable() override
		{
			g_TickCounter = 0;
			g_Fireballs.clear();
			WorldEvents::RequestPtfxAsset("anm_fire");
		}

		virtual void OnTick() override
		{
			auto self = Self::GetPed();
			if (!self || !self.IsValid())
				return;

			UpdateFireballs();

			if (++g_TickCounter < kSpawnInterval)
				return;

			g_TickCounter = 0;

			if (static_cast<int>(g_Fireballs.size()) < kMaxFireballs)
				SpawnFireball(self.GetPosition());
		}

		virtual void OnDisable() override
		{
			g_TickCounter = 0;
			g_Fireballs.clear();
		}
	};

	static RainOfFire _RainOfFire{"rainoffire", "Rain of Fire", "Fireballs rain from the sky and set the ground ablaze around you"};
}
