#include "BSDDService.hpp"

#include <windows.h>
#include <winhttp.h>
#include <string>

#include <rapidjson/document.h>

#pragma comment(lib, "winhttp.lib")

namespace {

	class WinHttpHandle {
	public:
		WinHttpHandle() : handle(nullptr) {}
		explicit WinHttpHandle(HINTERNET h) : handle(h) {}

		~WinHttpHandle()
		{
			if (handle != nullptr) {
				WinHttpCloseHandle(handle);
				handle = nullptr;
			}
		}

		operator HINTERNET () const
		{
			return handle;
		}

		bool IsValid() const
		{
			return handle != nullptr;
		}

	private:
		HINTERNET handle;
	};


	GS::UniString ToLowerCopy(const GS::UniString& input)
	{
		GS::UniString result = input;
		result.ToLowerCase();
		return result;
	}


	GS::UniString ToUniString(const char* text)
	{
		if (text == nullptr) {
			return GS::UniString();
		}

		return GS::UniString(text);
	}


	GS::UniString GetStringMember(const rapidjson::Value& object, const char* memberName)
	{
		if (object.HasMember(memberName) && object[memberName].IsString()) {
			return ToUniString(object[memberName].GetString());
		}

		return GS::UniString();
	}


	bool GetBoolMember(const rapidjson::Value& object, const char* memberName, bool defaultValue = false)
	{
		if (object.HasMember(memberName) && object[memberName].IsBool()) {
			return object[memberName].GetBool();
		}

		return defaultValue;
	}


	GS::UniString JoinAvailableLanguages(const rapidjson::Value& arrayValue)
	{
		GS::UniString result;

		if (!arrayValue.IsArray()) {
			return result;
		}

		for (rapidjson::SizeType i = 0; i < arrayValue.Size(); ++i) {
			const rapidjson::Value& language = arrayValue[i];
			if (!language.IsObject()) {
				continue;
			}

			GS::UniString name = GetStringMember(language, "name");
			if (name.IsEmpty()) {
				name = GetStringMember(language, "code");
			}

			if (name.IsEmpty()) {
				continue;
			}

			if (!result.IsEmpty()) {
				result.Append(", ");
			}

			result.Append(name);
		}

		return result;
	}


	bool ReadResponseBody(HINTERNET request, std::string& body, GS::UniString& errorMessage)
	{
		body.clear();

		DWORD availableSize = 0;

		while (true) {
			availableSize = 0;

			if (!WinHttpQueryDataAvailable(request, &availableSize)) {
				errorMessage = "WinHttpQueryDataAvailable failed.";
				return false;
			}

			if (availableSize == 0) {
				break;
			}

			std::string chunk;
			chunk.resize(availableSize);

			DWORD downloadedSize = 0;
			if (!WinHttpReadData(request, chunk.data(), availableSize, &downloadedSize)) {
				errorMessage = "WinHttpReadData failed.";
				return false;
			}

			body.append(chunk.data(), downloadedSize);
		}

		return true;
	}


	bool QueryStatusCode(HINTERNET request, DWORD& statusCode, GS::UniString& errorMessage)
	{
		DWORD bufferSize = sizeof(statusCode);

		if (!WinHttpQueryHeaders(
			request,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&statusCode,
			&bufferSize,
			WINHTTP_NO_HEADER_INDEX))
		{
			errorMessage = "Could not read HTTP status code.";
			return false;
		}

		return true;
	}


	GS::UniString BuildDictionaryDisplayName(const BSDDService::DictionaryInfo& item)
	{
		GS::UniString text = item.name;

		if (!item.version.IsEmpty()) {
			if (!text.IsEmpty()) {
				text.Append(" ");
			}
			text.Append(item.version);
		}

		if (!item.code.IsEmpty()) {
			text.Append(" [");
			text.Append(item.code);
			text.Append("]");
		}

		return text;
	}

} // namespace


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


	bool TryLoadDictionaries(GS::Array<DictionaryInfo>& dictionaries, GS::UniString& errorMessage, UInt32 limit)
	{
		dictionaries.Clear();
		errorMessage.Clear();

		const std::wstring host = L"api.bsdd.buildingsmart.org";
		const std::wstring path =
			std::wstring(L"/api/Dictionary/v1?IncludeTestDictionaries=false&Offset=0&Limit=") +
			std::to_wstring(limit);

		WinHttpHandle session(
			WinHttpOpen(
				L"ArchicadBSDD/0.1",
				WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
				WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS,
				0
			)
		);

		if (!session.IsValid()) {
			errorMessage = "WinHttpOpen failed.";
			return false;
		}

		WinHttpHandle connection(
			WinHttpConnect(
				session,
				host.c_str(),
				INTERNET_DEFAULT_HTTPS_PORT,
				0
			)
		);

		if (!connection.IsValid()) {
			errorMessage = "WinHttpConnect failed.";
			return false;
		}

		WinHttpHandle request(
			WinHttpOpenRequest(
				connection,
				L"GET",
				path.c_str(),
				nullptr,
				WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES,
				WINHTTP_FLAG_SECURE
			)
		);

		if (!request.IsValid()) {
			errorMessage = "WinHttpOpenRequest failed.";
			return false;
		}

		const wchar_t* headers =
			L"Accept: application/json\r\n"
			L"X-User-Agent: ArchicadBSDD/0.1\r\n";

		if (!WinHttpAddRequestHeaders(request, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD)) {
			errorMessage = "WinHttpAddRequestHeaders failed.";
			return false;
		}

		if (!WinHttpSendRequest(
			request,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			0))
		{
			errorMessage = "WinHttpSendRequest failed.";
			return false;
		}

		if (!WinHttpReceiveResponse(request, nullptr)) {
			errorMessage = "WinHttpReceiveResponse failed.";
			return false;
		}

		DWORD statusCode = 0;
		if (!QueryStatusCode(request, statusCode, errorMessage)) {
			return false;
		}

		if (statusCode != 200) {
			errorMessage = "bSDD API returned HTTP status " + GS::ValueToUniString(statusCode) + ".";
			return false;
		}

		std::string responseBody;
		if (!ReadResponseBody(request, responseBody, errorMessage)) {
			return false;
		}

		rapidjson::Document document;
		document.Parse(responseBody.c_str());

		if (document.HasParseError() || !document.IsObject()) {
			errorMessage = "Could not parse JSON response.";
			return false;
		}

		if (!document.HasMember("dictionaries") || !document["dictionaries"].IsArray()) {
			errorMessage = "JSON response does not contain a valid 'dictionaries' array.";
			return false;
		}

		const rapidjson::Value& dictionariesJson = document["dictionaries"];

		for (rapidjson::SizeType i = 0; i < dictionariesJson.Size(); ++i) {
			const rapidjson::Value& itemJson = dictionariesJson[i];

			if (!itemJson.IsObject()) {
				continue;
			}

			DictionaryInfo item;

			item.name = GetStringMember(itemJson, "name");
			item.code = GetStringMember(itemJson, "code");
			item.uri = GetStringMember(itemJson, "uri");
			item.version = GetStringMember(itemJson, "version");

			item.organizationNameOwner = GetStringMember(itemJson, "organizationNameOwner");
			item.organizationCodeOwner = GetStringMember(itemJson, "organizationCodeOwner");
			item.defaultLanguageCode = GetStringMember(itemJson, "defaultLanguageCode");
			item.license = GetStringMember(itemJson, "license");
			item.licenseUrl = GetStringMember(itemJson, "licenseUrl");
			item.qualityAssuranceProcedure = GetStringMember(itemJson, "qualityAssuranceProcedure");
			item.qualityAssuranceProcedureUrl = GetStringMember(itemJson, "qualityAssuranceProcedureUrl");
			item.status = GetStringMember(itemJson, "status");
			item.moreInfoUrl = GetStringMember(itemJson, "moreInfoUrl");
			item.releaseDate = GetStringMember(itemJson, "releaseDate");
			item.lastUpdatedUtc = GetStringMember(itemJson, "lastUpdatedUtc");

			item.isLatestVersion = GetBoolMember(itemJson, "isLatestVersion");
			item.isVerified = GetBoolMember(itemJson, "isVerified");
			item.isPrivate = GetBoolMember(itemJson, "isPrivate");

			if (itemJson.HasMember("availableLanguages")) {
				item.availableLanguagesText = JoinAvailableLanguages(itemJson["availableLanguages"]);
			}

			item.displayName = BuildDictionaryDisplayName(item);
			dictionaries.Push(item);
		}

		return true;
	}

} // namespace BSDDService