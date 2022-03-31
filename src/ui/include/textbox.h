#ifndef H_TEXBOX
#define H_TEXBOX

typedef struct FE_TextBox {
    SDL_Texture *text;
    SDL_Rect r;
    char *content;
    FE_Label *label;
    bool active;
} FE_TextBox;

FE_TextBox *FE_CreateTextBox(int x, int y, int w, int h, char *value);
void FE_RenderTextBox();

int FE_DestroyTextBox(FE_TextBox *l);
int FE_CleanTextBoxes();

int FE_UpdateTextBox(char c);
char *FE_GetContent(FE_TextBox *t);
int FE_SetContent(FE_TextBox *t, char *value);

FE_TextBox *FE_GetActiveTextBox();
int FE_ForceActiveTextBox(FE_TextBox *t);

bool FE_TextBoxClick(int x, int y);

#endif