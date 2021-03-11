// Parsing, creating and managing of the very basic console
#include "../game.h"

// Console currently just holds the user input textbox and the one last result from command (or info)

static struct TextObject *result;
static struct UIObject *console; // Panel that our textbox and label will sit on

static char resulttext[512];

bool ConsoleVisible;

void ShowConsole()
{
    if (ConsoleVisible)
        return;

    int height = 100;
    console = malloc(sizeof(struct UIObject));
    CreateUIObject(console, 0, screen_height - height, screen_width, height, "");
    
    result = malloc(sizeof(struct TextObject));
    NewText(result, ConsoleFont,resulttext, White, 4, screen_height - 100);

    CreateTextBox(screen_width, 30, 0, screen_height - 30);
    ConsoleVisible = true;

}

void HideConsole()
{
    if (!ConsoleVisible)
        return;
    ConsoleVisible = false;
    
    FreeText(result);
    FreeTextBox();
    FreeUIObject(console);
}

void UpdateConsole(const char *msg)
{
    memset(resulttext, 0, strlen(resulttext));
    strcpy(resulttext, msg);

    if (ConsoleVisible)
        UpdateText(result, msg, White);
}