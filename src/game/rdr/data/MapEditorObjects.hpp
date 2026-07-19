#pragma once
#include <string>
#include <vector>

// Curated, categorised palette for the Map Editor. Every model here is also present
// in g_ObjectModels (the set the Object Spawner already uses), so the editor only
// ever hands the game models it knows how to spawn - you pick from a list instead of
// typing a name.

namespace YimMenu::Data
{
	struct MapEditorObject
	{
		std::string model;
		std::string name;
	};

	struct MapEditorCategory
	{
		std::string name;
		std::vector<MapEditorObject> objects;
	};

	inline const std::vector<MapEditorCategory>& GetMapEditorCategories()
	{
		static const std::vector<MapEditorCategory> categories = {
		    {"Barriers & Structures",
		        {
		            {"p_fence01x", "Fence"},
		            {"p_gate01x", "Gate"},
		            {"p_wood_wall01x", "Wood Wall"},
		            {"p_wood_post01x", "Wood Post"},
		            {"p_hitchingpost01x", "Hitching Post"},
		            {"p_sign01x", "Sign"},
		            {"p_sign02x", "Sign 2"},
		            {"p_lantern_post01x", "Lantern Post"},
		        }},
		    {"Crates & Storage",
		        {
		            {"p_barrel01x", "Barrel"},
		            {"p_barrel02x", "Barrel 2"},
		            {"p_barrel03x", "Barrel 3"},
		            {"p_crate01x", "Crate"},
		            {"p_crate02x", "Crate 2"},
		            {"p_crate03x", "Crate 3"},
		            {"p_box01x", "Box"},
		            {"p_box02x", "Box 2"},
		            {"p_woodcrate01x", "Wood Crate"},
		            {"p_woodcrate02x", "Wood Crate 2"},
		            {"p_beerbarrel01x", "Beer Barrel"},
		            {"p_waterbarrel01x", "Water Barrel"},
		            {"p_sack01x", "Sack"},
		            {"p_basket01x", "Basket"},
		            {"p_basket02x", "Basket 2"},
		        }},
		    {"Camp",
		        {
		            {"p_campfire01x", "Campfire"},
		            {"p_campfire02x", "Campfire 2"},
		            {"p_tent01x", "Tent"},
		            {"p_tent02x", "Tent 2"},
		            {"p_cot01x", "Cot"},
		            {"p_cot02x", "Cot 2"},
		            {"p_bed01x", "Bed"},
		            {"p_chair01x", "Chair"},
		            {"p_chair02x", "Chair 2"},
		            {"p_chair03x", "Chair 3"},
		            {"p_stool01x", "Stool"},
		            {"p_bench01x", "Bench"},
		            {"p_bench02x", "Bench 2"},
		            {"p_table01x", "Table"},
		            {"p_table02x", "Table 2"},
		            {"p_lantern01x", "Lantern"},
		            {"p_lantern02x", "Lantern 2"},
		            {"p_candle01x", "Candle"},
		        }},
		    {"Nature & Wood",
		        {
		            {"p_rock01x", "Rock"},
		            {"p_haybale01x", "Hay Bale"},
		            {"p_haybale02x", "Hay Bale 2"},
		            {"p_woodpile01x", "Woodpile"},
		            {"p_woodpile02x", "Woodpile 2"},
		            {"p_woodpile03x", "Woodpile 3"},
		            {"p_woodpile04x", "Woodpile 4"},
		            {"p_firewood01x", "Firewood"},
		            {"p_coalpile01x", "Coal Pile"},
		            {"p_plant01x", "Plant"},
		            {"p_sawhorse01x", "Sawhorse"},
		            {"p_wheelbarrow01x", "Wheelbarrow"},
		        }},
		    {"Props",
		        {
		            {"p_chest01x", "Chest"},
		            {"p_trunk01x", "Trunk"},
		            {"p_coffin01x", "Coffin"},
		            {"p_locker01x", "Locker"},
		            {"p_shelf01x", "Shelf"},
		            {"p_stove01x", "Stove"},
		            {"p_mirror01x", "Mirror"},
		            {"p_rug01x", "Rug"},
		            {"p_pot01x", "Pot"},
		            {"p_pan01x", "Pan"},
		            {"p_bucket01x", "Bucket"},
		            {"p_bucket02x", "Bucket 2"},
		        }},
		};
		return categories;
	}
}
