#ifndef H_INIT
#define H_INIT

#include "../core/include/fedoraengine.h"

/* Cleans all resources possible without exiting */
void FE_CleanAll();

/* Returns a FedoraEngine init configuration with default values */
FE_InitConfig *FE_NewInitConfig();

/* Initializes the engine with the given configuration */
void FE_Init(FE_InitConfig *InitConfig);

/* Cleans all resources and exits cleanly */
void FE_Clean();

#endif