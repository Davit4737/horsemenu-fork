#pragma once

namespace YimMenu::Features
{
	// Always-on handler that lets an animal player model attack (and fly, when the
	// model is a bird). It is driven every tick from the feature loop, so there is
	// no toggle to switch it on - it simply reacts to you being an animal.
	void TickAnimalAbilities();
}
