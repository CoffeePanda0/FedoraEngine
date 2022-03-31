#ifndef H_CONSOLE
#define H_CONSOLE

extern bool FE_ConsoleVisible;

int FE_ConsoleInit();
void FE_ConsoleShow();
void FE_ConsoleHide();
void FE_DestroyConsole();
int FE_ConsoleUpdateInput(char *in);
void FE_RenderConsole();
int FE_ConsoleSetText(const char *text);

#endif
