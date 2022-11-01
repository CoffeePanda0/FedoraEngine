#include "../core/include/include.h"
#include "include/console.h"

#define CONSOLE_FONT "Inconsolata"
#define CONSOLE_TEXTURE "game/ui/console.png"
#define CONSOLE_FONT_SIZE 20

#define CONSOLE_HEIGHT 100

static FE_Font *Font;

typedef struct {
    // output text
    SDL_Texture *output_label_text;
    SDL_Rect output_label_rect;

    // input text
    SDL_Texture *input_label_text;
    SDL_Rect input_label_rect;

    // console object
    SDL_Texture *console_texture;
    SDL_Rect console_rect;
} console;

static console Console;

static char *console_output;
static char *console_input;

// function to create console
int FE_Console_Init()
{
    if (TTF_Init() == -1)
    	error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());

    Font = FE_Font_Load(CONSOLE_FONT, CONSOLE_FONT_SIZE);

    console_output = mstrdup("");
    console_input = mstrdup("");

    Console.console_rect = (SDL_Rect){0, PresentGame->WindowHeight - CONSOLE_HEIGHT, PresentGame->WindowWidth, CONSOLE_HEIGHT};

    PresentGame->ConsoleVisible = false;
    info("Initialised Console");

    return 1;
}


// separate label functions so we don't end up using Label objects for console output
static void GenerateConsoleLabel()
{   
    if (!PresentGame->GameActive) return;

    if (Console.output_label_text)
        SDL_DestroyTexture(Console.output_label_text);

    SDL_Surface *text_surface = FE_Text_Render(Font, console_output, COLOR_WHITE); 
    Console.output_label_text = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);

    Console.output_label_rect.x = 0;
    Console.output_label_rect.y = (PresentGame->WindowHeight - CONSOLE_HEIGHT) + 10;
    SDL_QueryTexture(Console.output_label_text, NULL, NULL, &Console.output_label_rect.w, &Console.output_label_rect.h);
}

static void GenerateInputLabel()
{
    if (Console.input_label_text)
        SDL_DestroyTexture(Console.input_label_text);
    
    SDL_Surface *text_surface = FE_Text_Render(Font, console_input, COLOR_WHITE); 
    Console.input_label_text = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);

    Console.input_label_rect.x = 0;
    Console.input_label_rect.y = (PresentGame->WindowHeight - CONSOLE_HEIGHT) + CONSOLE_HEIGHT -40;
    SDL_QueryTexture(Console.input_label_text, NULL, NULL, &Console.input_label_rect.w, &Console.input_label_rect.h);
}

void FE_Console_UpdateInput(char *in)
{
    if (Font) {
        if (console_input)
            free(console_input);
        console_input = mstrdup(in);
        GenerateInputLabel();
        return;
    }
}

void FE_Console_SetText(const char *text)
{
    if (!text) {
        warn("Passing nullptr! (ConsoleSetText)");
        return;
    }

    if (!Font)
        return;

    if (console_output)
        free(console_output);
    
    console_output = mstrdup(text);
    if (PresentGame->ConsoleVisible)
        GenerateConsoleLabel();
}

void FE_Console_Show()
{
    return;
    PresentGame->ConsoleVisible = true;
    if (!Console.console_texture)
        Console.console_texture = FE_TextureFromFile(CONSOLE_TEXTURE);

    GenerateConsoleLabel();
    GenerateInputLabel();
}

void FE_Console_Hide()
{
    PresentGame->ConsoleVisible = false;

    if (Console.output_label_text) {
        SDL_DestroyTexture(Console.output_label_text);
        Console.output_label_text = 0;
    }
    if (Console.input_label_text) {
        SDL_DestroyTexture(Console.input_label_text);
        Console.input_label_text = 0;
    }
}

void FE_Console_Destroy()
{
    if (Console.output_label_text)
        SDL_DestroyTexture(Console.output_label_text);
    Console.output_label_text = 0;
    
    if (Console.input_label_text)
        SDL_DestroyTexture(Console.input_label_text);
    Console.input_label_text = 0;

    if (Console.console_texture)
        SDL_DestroyTexture(Console.console_texture);
    Console.console_texture = 0;

    if (Font)
        FE_Font_Destroy(Font);

    if (console_output)
        free(console_output);
    console_output = 0;
    if (console_input)
        free(console_input);
    console_input = 0;
}

void FE_Console_Render()
{
    if (PresentGame->ConsoleVisible) {
        SDL_RenderCopy(PresentGame->Renderer, Console.console_texture, NULL, &Console.console_rect); // console texture
        SDL_RenderCopy(PresentGame->Renderer, Console.output_label_text, NULL, &Console.output_label_rect); // output label
        SDL_RenderCopy(PresentGame->Renderer, Console.input_label_text, NULL, &Console.input_label_rect); // input label
    }
}