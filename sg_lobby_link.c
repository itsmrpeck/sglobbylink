/*
SGLobbyLink by Mr Peck (2018)

This is crappy throwaway source code for a quick useful utility, I know it sucks. Please don't judge it.
*/

#include "stdafx.h"
#include "Windows.h"

// EXTERNAL LIBRARY: https://curl.haxx.se/libcurl/
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#define CURL_STATICLIB 
#include <curl/curl.h>

// EXTERNAL LIBRARY: https://github.com/sheredom/json.h
#include "json.h"


#define STEAM_API_KEY "[REDACTED]"

#define FAIL_QUIT(a) { printf("Failure - " a); curl_easy_cleanup(curl); Sleep(10000); return; }
#define FAIL_QUIT_NO_CURL(a) { printf("Failure - " a); Sleep(10000); return; }

#define WEB_BUF_LEN 5000
#define BUF_LEN 300

#define STEAM_ID_FILE "steam_id.txt"

static CURL *curl;
static char webBuf[WEB_BUF_LEN];
static size_t webBufSize;

size_t write_web_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    sprintf_s(webBuf, WEB_BUF_LEN, "%s", (char*)buffer);
    webBufSize = size * nmemb;

    return webBufSize;
}

struct json_value_s *json_get_elem_by_name(struct json_object_s *rootObj, const char *elemName, size_t nameLen)
{
    if(rootObj)
    {
        struct json_object_element_s* rootElem = rootObj->start;

        while(rootElem)
        {
            if(rootElem->name->string_size == nameLen && !_strnicmp(rootElem->name->string, elemName, nameLen))
                return rootElem->value;

            rootElem = rootElem->next;
        }
    }

    return 0;
}

int main()
{
    printf(
        "SGLobbyLink v1.0 - by Mr Peck\n"
        "-----------------------------\n\n");

    // Open cURL
    
    CURLcode res;

    curl = curl_easy_init();
    if(!curl)
        FAIL_QUIT_NO_CURL("Failed to open cURL!\n");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_web_data);

    // Get steam ID
    
    char sanitizedNameBuf[BUF_LEN];
    const char *steamID = 0;


    FILE *steamIDFileRead = fopen(STEAM_ID_FILE, "r");
    if(steamIDFileRead)
    {
        fseek(steamIDFileRead, 0, SEEK_END);
        long fsize = ftell(steamIDFileRead);
        fseek(steamIDFileRead, 0, SEEK_SET);

        char *steamIDHeapMem = malloc(fsize + 1); // never freeing this btw. It is a single tiny string for a program that runs once and quits.
        
        fread(steamIDHeapMem, fsize, 1, steamIDFileRead);
        fclose(steamIDFileRead);

        steamIDHeapMem[fsize] = 0;

        steamID = steamIDHeapMem;
    }
    else
    {
        // No file found. Let's get our steam ID and make one!

        puts("First time setup: let's find your steam ID.\n"
            "Enter your 64-bit Steam ID, or the name in your Steam profile URL\n"
            "(e.g. if your profile is http://steamcommunity.com/id/dave, type dave):\n");

        char consoleNameBuf[BUF_LEN];
        fgets(consoleNameBuf, BUF_LEN, stdin);

        size_t i = 0;
        char *raw = consoleNameBuf;
        char *sanitized = sanitizedNameBuf;
        int allNumbers = 1;
        while(*raw)
        {
            char ch = *raw;
            if((ch >= '0' && ch <= '9') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= 'A' && ch <= 'Z'))
            {
                *sanitized = *raw;
                sanitized++;

                if(ch < '0' || ch > '9')
                    allNumbers = 0;
            }

            raw++;
        }
        *sanitized = 0;

        size_t sanitizedLength = strlen(sanitizedNameBuf);
        if(!sanitizedLength)
            FAIL_QUIT("Failed to find Steam ID");

        if(allNumbers)
        {
            steamID = sanitizedNameBuf;
        }
        else
        {
            char steamIDRequestBuf[BUF_LEN];
            sprintf_s(steamIDRequestBuf, BUF_LEN, "http://api.steampowered.com/ISteamUser/ResolveVanityURL/v0001/?key=" STEAM_API_KEY "&vanityurl=%s", sanitizedNameBuf);
            curl_easy_setopt(curl, CURLOPT_URL, steamIDRequestBuf);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                FAIL_QUIT("cURL ResolveVanityURL() call failed.");
            }
            struct json_value_s *steamIDJson = json_parse(webBuf, webBufSize);
            if(!steamIDJson)
                FAIL_QUIT("Failed to parse JSON from ResolveVanityURL");

            struct json_object_s* root = (struct json_object_s*)steamIDJson->payload;
            struct json_value_s* response = json_get_elem_by_name(root, "response", 8);
            if(!response || response->type != json_type_object)
                FAIL_QUIT("Failed to parse \"response\" from ResolveVanityURL");

            // Get "steamid"
            struct json_object_s* responseObj = (struct json_object_s*)response->payload;
            struct json_value_s* jSteamID = json_get_elem_by_name(responseObj, "steamid", 7);
            if(!jSteamID || jSteamID->type != json_type_string)
                FAIL_QUIT("Failed to find Steam ID");

            steamID = ((struct json_string_s*)jSteamID->payload)->string;
        }

        // Save to file
        if(steamID)
        {
            FILE *steamIDFileWrite = fopen(STEAM_ID_FILE, "w");
            if(!steamIDFileWrite)
            {
                printf("Failed to write Steam ID to steam_id.txt - do you have admin permissions for this folder?\n");
            }
            else
            {
                fwrite(steamID, sizeof(char), strlen(steamID), steamIDFileWrite);
                fclose(steamIDFileWrite);
            }
        }
    }

    // Get app ID and lobby ID via GetPlayerSummaries
    char playerSummaryRequestBuf[BUF_LEN];
    sprintf_s(playerSummaryRequestBuf, BUF_LEN, "http://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=" STEAM_API_KEY "&steamids=%s", steamID);
    curl_easy_setopt(curl, CURLOPT_URL, playerSummaryRequestBuf);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        FAIL_QUIT("cURL GetPlayerSummaries() call failed.");
    }
    struct json_value_s *playerSummaryJson = json_parse(webBuf, webBufSize);
    if(!playerSummaryJson)
        FAIL_QUIT("Failed to parse JSON from GetPlayerSummaries");

    // Now find the values we want in the JSON
    { 
        struct json_object_s* root = (struct json_object_s*)playerSummaryJson->payload;

        struct json_value_s* response = json_get_elem_by_name(root, "response", 8);
        if(!response || response->type != json_type_object)
            FAIL_QUIT("Failed to parse \"response\" from GetPlayerSummaries");

        // Get first element of "players"
        struct json_object_s* responseObj = (struct json_object_s*)response->payload;
        struct json_value_s* players = json_get_elem_by_name(responseObj, "players", 7);
        if(!players || players->type != json_type_array)
            FAIL_QUIT("Failed to parse \"players\" from GetPlayerSummaries(1)");

        struct json_array_s* playersArr = (struct json_array_s*)players->payload;
        if(playersArr->length != 1)
            FAIL_QUIT("Failed to parse \"players\" from GetPlayerSummaries(2)");

        struct json_object_s* playerOne = (struct json_object_s*)playersArr->start->value->payload;

        // Get gameid and lobbysteamid
        struct json_value_s* jGameID = json_get_elem_by_name(playerOne, "gameid", 6);
        if(!jGameID || jGameID->type != json_type_string)
            FAIL_QUIT("Could not find currently played game. Is your Steam profile public, and are you in a game?");
        const char *appID = ((struct json_string_s*)jGameID->payload)->string;

        struct json_value_s* jLobbyID = json_get_elem_by_name(playerOne, "lobbysteamid", 12);
        if(!jLobbyID || jLobbyID->type != json_type_string)
            FAIL_QUIT("Could not find lobby ID. Are you in a lobby and is your Steam profile public?");

        const char *lobbyID = ((struct json_string_s*)jLobbyID->payload)->string;

        // Copy lobby link to clipboard
        char joinLobbyBuf[BUF_LEN];
        sprintf_s(joinLobbyBuf, BUF_LEN, "steam://joinlobby/%s/%s/%s", appID, lobbyID, steamID);

        const size_t len = strlen(joinLobbyBuf) + 1;
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
        memcpy(GlobalLock(hMem), joinLobbyBuf, len);
        GlobalUnlock(hMem);
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();

        printf("Success - lobby link copied to clipboard!\n");
        curl_easy_cleanup(curl);
        Sleep(2000);
    }

    return 0;
}
