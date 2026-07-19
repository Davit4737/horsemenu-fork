#pragma once
#include <string>
#include <vector>

// Coat/breed palette for horse customisation. Every entry is a real horse model that
// already ships in g_PedModels, so picking one is guaranteed to load. "Reds &
// Chestnuts" is listed first because a fiery red horse is the most requested look.

namespace YimMenu::Data
{
	struct HorseCoat
	{
		std::string model;
		std::string name;
	};

	struct HorseCoatGroup
	{
		std::string name;
		std::vector<HorseCoat> coats;
	};

	inline const std::vector<HorseCoatGroup>& GetHorseCoatGroups()
	{
		static const std::vector<HorseCoatGroup> groups = {
		    {"Reds & Chestnuts",
		        {
		            {"A_C_Horse_Arabian_RedChestnut", "Arabian Red Chestnut"},
		            {"A_C_Horse_Breton_RedRoan", "Breton Red Roan"},
		            {"A_C_Horse_Breton_Sorrel", "Breton Sorrel"},
		            {"A_C_Horse_Ardennes_StrawberryRoan", "Ardennes Strawberry Roan"},
		            {"A_C_Horse_Belgian_BlondChestnut", "Belgian Blond Chestnut"},
		            {"A_C_Horse_Belgian_MealyChestnut", "Belgian Mealy Chestnut"},
		            {"A_C_Horse_HungarianHalfbred_FlaxenChestnut", "Hungarian Flaxen Chestnut"},
		            {"A_C_Horse_HungarianHalfbred_LiverChestnut", "Hungarian Liver Chestnut"},
		            {"A_C_Horse_Morgan_FlaxenChestnut", "Morgan Flaxen Chestnut"},
		            {"A_C_Horse_KentuckySaddle_ChestnutPinto", "Kentucky Chestnut Pinto"},
		            {"A_C_Horse_Criollo_Sorrelovero", "Criollo Sorrel Overo"},
		        }},
		    {"Bays & Browns",
		        {
		            {"A_C_Horse_Andalusian_DarkBay", "Andalusian Dark Bay"},
		            {"A_C_Horse_Morgan_Bay", "Morgan Bay"},
		            {"A_C_Horse_Morgan_BayRoan", "Morgan Bay Roan"},
		            {"A_C_Horse_Ardennes_BayRoan", "Ardennes Bay Roan"},
		            {"A_C_Horse_Breton_MealyDappleBay", "Breton Mealy Dapple Bay"},
		            {"A_C_Horse_DutchWarmblood_ChocolateRoan", "Dutch Chocolate Roan"},
		            {"A_C_Horse_Mustang_TigerStripedBay", "Mustang Tiger Striped Bay"},
		        }},
		    {"Blacks & Greys",
		        {
		            {"A_C_Horse_Arabian_Black", "Arabian Black"},
		            {"A_C_Horse_KentuckySaddle_Black", "Kentucky Black"},
		            {"A_C_Horse_Kladruber_Black", "Kladruber Black"},
		            {"A_C_Horse_Arabian_Grey", "Arabian Grey"},
		            {"A_C_Horse_Andalusian_RoseGray", "Andalusian Rose Grey"},
		            {"A_C_Horse_Ardennes_IronGreyRoan", "Ardennes Iron Grey Roan"},
		            {"A_C_Horse_Kladruber_Grey", "Kladruber Grey"},
		            {"A_C_Horse_HungarianHalfbred_DarkDappleGrey", "Hungarian Dapple Grey"},
		        }},
		    {"Whites & Palominos",
		        {
		            {"A_C_Horse_Arabian_White", "Arabian White"},
		            {"A_C_Horse_Kladruber_White", "Kladruber White"},
		            {"A_C_Horse_GypsyCob_WhiteBlagdon", "Gypsy Cob White Blagdon"},
		            {"A_C_Horse_Morgan_Palomino", "Morgan Palomino"},
		            {"A_C_Horse_AmericanStandardbred_PalominoDapple", "Palomino Dapple"},
		            {"A_C_Horse_GypsyCob_PalominoBlagdon", "Gypsy Cob Palomino"},
		            {"A_C_Horse_AmericanStandardbred_Buckskin", "Standardbred Buckskin"},
		        }},
		    {"Spotted & Exotic",
		        {
		            {"A_C_Horse_Appaloosa_Leopard", "Appaloosa Leopard"},
		            {"A_C_Horse_Appaloosa_BrownLeopard", "Appaloosa Brown Leopard"},
		            {"A_C_Horse_Appaloosa_BlackSnowflake", "Appaloosa Black Snowflake"},
		            {"A_C_Horse_AmericanPaint_Tobiano", "American Paint Tobiano"},
		            {"A_C_Horse_MissouriFoxTrotter_Blacktovero", "Missouri Fox Trotter Tovero"},
		            {"A_C_Horse_Mustang_GoldenDun", "Mustang Golden Dun"},
		            {"A_C_Horse_Criollo_Blueroanovero", "Criollo Blue Roan Overo"},
		            {"A_C_Horse_Arabian_RoseGreyBay", "Arabian Rose Grey Bay"},
		        }},
		};
		return groups;
	}
}
