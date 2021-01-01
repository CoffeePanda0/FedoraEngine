# FedoraEngine Documentation

~~be warned this code is a wreck, and will give you a new reason to hate life~~  

## Changing parameters:  
- Screen width and height (game.h)

## MAIN.C:
- This file is responsible for calling the main functions (bet you couldnt tell that)
- Most of the main loops for the game are in game.c

## Game objects:  
- Initialize it first with a name, e.g (struct GameObject NAME_HERE)
- use the function CreateObject(int xPos, int yPos, int width, int height, const char *texture_location, struct Gameobject *obj)
- DestroyObject(struct GameObject obj) frees the memory used by a game object
- The function RenderObjects() renders all GameObjects simultaneously and is already called in the Render() function

## Logging:  
- Logs are written to "log.txt" and to the console. Different logging features are:
Notice, Warn (Not urgent)
- Error: Displays the erorr message in a pop up on screen and exits the application
- cheers to Gibson (ypsvlq) for the code for lib.c and help with gameobjects

## Text:  
- Text creation works similar to the gameobject, and text is defined as a TextObject(). All text and UI code is currently in uitext.c 
- To create text, define the struct with a name, e.g (struct TextObject NAME_HERE). Initialise the text whenever you need with the function NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos)
- To update the text already on the screen, use the function UpdateText() as this does not add it to the list and should be used instead of creating a new texobject each time
- All text is automatically rendered whenever added and does not need to be rendered inividually, as long as RenderText() is called while the game is active


## Maps:
- Maps are loaded from a text file, and is cqalled in game.c with InitMap(char* file_path). See the example map in /map for details, and the readme to explain. Each int has to be separated by a space, and extra spaces can cause issues with loading maps.
Tile size is defined in map.c, and maps can now scroll across the screen and the camera follows the player as it moves accross these tiles. As long as you set a valid tile size in map.h scrolling works automatically. 

