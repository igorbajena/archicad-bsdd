#ifndef BSDDSERVICE_HPP
#define BSDDSERVICE_HPP

#include "ACAPinc.h"

namespace BSDDService {

	struct SearchResult {
		GS::UniString label;
		GS::UniString codeOrUri;
		GS::UniString dictionary;
	};

	struct DictionaryInfo {
		GS::UniString displayName;

		GS::UniString name;
		GS::UniString code;
		GS::UniString uri;
		GS::UniString version;

		GS::UniString organizationNameOwner;
		GS::UniString organizationCodeOwner;
		GS::UniString defaultLanguageCode;
		GS::UniString license;
		GS::UniString licenseUrl;
		GS::UniString qualityAssuranceProcedure;
		GS::UniString qualityAssuranceProcedureUrl;
		GS::UniString status;
		GS::UniString moreInfoUrl;
		GS::UniString releaseDate;
		GS::UniString lastUpdatedUtc;
		GS::UniString availableLanguagesText;

		bool isLatestVersion = false;
		bool isVerified = false;
		bool isPrivate = false;
	};

	GS::Array<SearchResult> GetMockSearchResults(const GS::UniString& query);
	bool TryLoadDictionaries(GS::Array<DictionaryInfo>& dictionaries, GS::UniString& errorMessage, UInt32 limit = 20);

}

#endif