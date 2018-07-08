# SGLobbyLink v1.0 - by Mr Peck

Copies your current Steam lobby URL to your clipboard, so you can paste it into Discord (or similar) to invite people to your game. Play sets against people who aren't on your friends list, without lots of clicks!


# Download:

- Windows: Get the .exe here https://github.com/itsmrpeck/sglobbylink/releases
- Others: It's Windows only, sorry.


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

- libcurl (cURL): https://curl.haxx.se/libcurl/
- json.h: https://github.com/sheredom/json.h
