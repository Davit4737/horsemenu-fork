#pragma once

#include "game/rdr/Ped.hpp"

namespace YimMenu::Features
{
	class AnimalPlayerAbilities
	{
	public:
		static void Init();
		static void ConfigurePlayerAsAnimal(Ped ped);
	};
}
