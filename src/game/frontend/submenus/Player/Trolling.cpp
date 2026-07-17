#include "Trolling.hpp"

namespace YimMenu::Submenus
{
	std::shared_ptr<Category> BuildTrollingMenu()
	{
		auto menu = std::make_shared<Category>("Trolling");

		// 1 Column makes small groupings look much cleaner
		auto cages = std::make_shared<Group>("Cages", 1);
		cages->AddItem(std::make_shared<PlayerCommandItem>("cageplayersmall"_J));
		cages->AddItem(std::make_shared<PlayerCommandItem>("cageplayerlarge"_J));
		cages->AddItem(std::make_shared<PlayerCommandItem>("cageplayercircus"_J));

		// Reduced down to 1 column to avoid uneven 11-item layout grids
		auto attachments = std::make_shared<Group>("Attachments", 1);
		attachments->AddItem(std::make_shared<PlayerCommandItem>("rideonshoulders"_J));
		attachments->AddItem(std::make_shared<PlayerCommandItem>("spank"_J));
		attachments->AddItem(std::make_shared<CommandItem>("cancelattachment"_J));

		// Divides perfectly into a 2x2 button grid matrix
		auto animations = std::make_shared<Group>("Force Animations", 2);
		animations->AddItem(std::make_shared<ListCommandItem>("playeranimpreset"_J));
		animations->AddItem(std::make_shared<PlayerCommandItem>("forceplayeranim"_J));
		animations->AddItem(std::make_shared<PlayerCommandItem>("clearplayeranim"_J));
		animations->AddItem(std::make_shared<PlayerCommandItem>("ragdollplayer"_J));

		auto npcAttacks = std::make_shared<Group>("NPC Attacks", 1);
		npcAttacks->AddItem(std::make_shared<IntCommandItem>("npcattackcount"_J));
		npcAttacks->AddItem(std::make_shared<PlayerCommandItem>("spawnnpcattackers"_J));
		npcAttacks->AddItem(std::make_shared<PlayerCommandItem>("spawnanimalattackers"_J));

		auto extra = std::make_shared<Group>("Extra", 1);
		extra->AddItem(std::make_shared<PlayerCommandItem>("remotebolas"_J));

		menu->AddItem(cages);
		menu->AddItem(attachments);
		menu->AddItem(animations);
		menu->AddItem(npcAttacks);
		menu->AddItem(extra);

		return menu;
	}
}