# SGLobbyLink v1.0 - by Mr Peck

Copies your current Steam lobby URL to your clipboard, so you can paste it into Discord (or similar) to invite people to your game. Play sets against people who aren't on your friends list, without lots of clicks!


# STEAM WEB API KEY NOTE (WHY THIS PROGRAM NO LONGER WORKS):

I have revoked my Steam Web API key for security reasons so this program will no longer work. If you want to build your own personal version of the program, add your Steam Web API key to line 21 of sg_lobby_link.c (#define STEAM_API_KEY "[REDACTED]").


# How to use:

- Create or join a Skullgirls lobby on Steam.
- Run the program. It copies your 'Join Game' link to your clipboard.
- Paste the link in Discord!

NOTE: The first time you run the program, it asks for your Steam ID and saves it to steam_id.txt. Once it has saved your Steam ID, it can instantly get your lobby link without having to ask you for anything.


# Troubleshooting:

Things that will cause this program to not work:

- Your Steam profile is private
- You entered an incorrect Steam ID (try deleting steam_id.txt and trying again)
- Steam is down
- You forgot to create the lobby before running the program
- I messed up somehow. This is the first time I've coded web stuff...


# External libraries used:

- libcurl https://curl.haxx.se/libcurl/
- json.h https://github.com/sheredom/json.h
