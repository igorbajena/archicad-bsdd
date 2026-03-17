#ifndef BSDDSERVICE_HPP
#define BSDDSERVICE_HPP

#include "ACAPinc.h"

namespace BSDDService {

	struct SearchResult {
		GS::UniString label;
		GS::UniString codeOrUri;
		GS::UniString dictionary;
	};

	GS::Array<SearchResult> GetMockSearchResults(const GS::UniString& query);

}

#endif