# FedoraEngine
![](https://i.imgur.com/jBOrd1Y.png)  
The *most advanced game engine* to ever be released  

(still a very much work in progress so watch the page for actual features)

written in pure c

# Current features
FedoraEngine is still very much in development. Current groundbreaking features are:
 - Ability to create, render and manage multiple GameObjects and text on screen easily for game developers
 - A somewhat working physics and collision system
 - Particle system
 - Multiplayer UDP server and client system
 - Loading game config files
 - Lua Support, and a game console (although currently limited wrappers)
 - Loading maps and textures dynamicallly from text files
 - Custom dialogue system format, easily create dialogue text files and load them into your game
 - Pause menu
 
# Q&A's
1. What is this code useful for? A: good question
2. Will your code make me cry? A: Absoloutely, the code is a horror so prepare yourself

# Requirements
- A C compiler (e.g gcc)
- SDL2, SDL_TTF, SDL_Mixer and SDL_Image libraries
- Llua
- ~~a will to live~~
- TinyJson by (https://github.com/rafagafe/tiny-json) - Included in ext folder
- Znet by (https://github.com/lukaszle/zed_net) - Included in ext folder

# Todo
- everything

# Known bugs / limitations
- Netcode is not brilliant in client.c and multiple clients can cause lag
- Enemy system is currently broken, as their collision appears to break quantum mechanics


# Upcoming "features" (none of which are anywhere near completion)
- Animation
- Working enemy system
- maybe improving networking (grouping packets, username, chat)

# Credits
me (CoffeePanda0)
