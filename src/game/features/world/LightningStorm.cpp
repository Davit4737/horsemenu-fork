#include "WorldEventUtils.hpp"

#include "core/commands/LoopedCommand.hpp"
#include "game/backend/ScriptMgr.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Enums.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	namespace
	{
		int g_TickCounter = 0;

		// Time (in ticks) between lightning strikes near you.
		constexpr int kStrikeInterval = 20;

		void StrikeNear(const rage::fvector3& center)
		{
			const auto offset = WorldEvents::RandomHorizOffset(8.0f, 45.0f);
			const float x     = center.x + offset.x;
			const float y     = center.y + offset.y;

			float groundZ = center.z;
			WorldEvents::TryGetGroundZ(x, y, center.z + 60.0f, groundZ);

			// Whole-sky flash plus a localised bolt right at the strike point.
			MISC::FORCE_LIGHTNING_FLASH();
			MISC::_FORCE_LIGHTNING_FLASH_AT_COORDS(x, y, groundZ + 40.0f, 1.0f);

			// A bright bluish-white column of light to sell the bolt.
			GRAPHICS::DRAW_LIGHT_WITH_RANGE(x, y, groundZ + 3.0f, 180, 210, 255, 20.0f, 8.0f);

			FIRE::ADD_EXPLOSION(x, y, groundZ + 0.5f, static_cast<int>(ExplosionTypes::MEDIUM_EXPLOSION), 1.0f, true, false, 1.0f);

			WorldEvents::PlayFireBurstAt(x, y, groundZ, 1.1f);
		}
	}

	class LightningStorm : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnEnable() override
		{
			g_TickCounter = 0;
			WorldEvents::RequestPtfxAsset("anm_fire");
		}

		virtual void OnTick() override
		{
			auto self = Self::GetPed();
			if (!self || !self.IsValid())
				return;

			if (++g_TickCounter < kStrikeInterval)
				return;

			g_TickCounter = 0;

			// One or two bolts per burst for a stormy feel.
			StrikeNear(self.GetPosition());
			if (WorldEvents::RandomRange(0.0f, 1.0f) > 0.55f)
				StrikeNear(self.GetPosition());
		}

		virtual void OnDisable() override
		{
			g_TickCounter = 0;
		}
	};

	static LightningStorm _LightningStorm{"lightningstorm", "Lightning Storm", "Lightning bolts crash down around you with flashes, light, and explosions"};
}
