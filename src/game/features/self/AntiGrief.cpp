#include "core/commands/LoopedCommand.hpp"
#include "game/backend/Self.hpp"
#include "game/rdr/Natives.hpp"

namespace YimMenu::Features
{
	// A light, safe self-defence layer for the Protection page. It does not touch the
	// network hooks (those already guard against kicks/crashes) - it just undoes the
	// most common on-foot griefing: being set on fire and being burned/chipped down to
	// death by explosions. Off by default so it never surprises you.
	class AntiGrief : public LoopedCommand
	{
		using LoopedCommand::LoopedCommand;

		virtual void OnTick() override
		{
			auto ped = Self::GetPed();
			if (!ped || !ped.IsValid() || ped.IsDead())
				return;

			const int handle = ped.GetHandle();

			if (FIRE::IS_ENTITY_ON_FIRE(handle))
				FIRE::STOP_ENTITY_FIRE(handle, 0);

			// Don't let a griefer whittle you to death - keep a healthy floor.
			const int maxHealth = ped.GetMaxHealth();
			if (maxHealth > 0 && ped.GetHealth() < maxHealth / 2)
				ped.SetHealth(maxHealth);
		}
	};

	static AntiGrief _AntiGrief{"antigrief", "Anti-Grief (Fire/Health)", "Puts out fires on you and tops your health if a griefer burns or chips you down"};
}
