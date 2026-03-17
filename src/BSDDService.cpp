#include "BSDDService.hpp"

namespace {

	GS::UniString ToLowerCopy(const GS::UniString& input)
	{
		GS::UniString result = input;
		result.ToLowerCase();
		return result;
	}

}

namespace BSDDService {

	GS::Array<SearchResult> GetMockSearchResults(const GS::UniString& query)
	{
		GS::Array<SearchResult> allItems;
		allItems.Push({ "Wall", "mock:class/wall", "Mock Dictionary A" });
		allItems.Push({ "Door", "mock:class/door", "Mock Dictionary A" });
		allItems.Push({ "Window", "mock:class/window", "Mock Dictionary B" });

		GS::Array<SearchResult> filteredItems;
		GS::UniString loweredQuery = ToLowerCopy(query);

		for (UIndex i = 0; i < allItems.GetSize(); ++i) {
			GS::UniString loweredLabel = ToLowerCopy(allItems[i].label);

			if (loweredQuery.IsEmpty() || loweredLabel.Contains(loweredQuery)) {
				filteredItems.Push(allItems[i]);
			}
		}

		return filteredItems;
	}

}