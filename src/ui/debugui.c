#include "../core/include/fedoraengine.h"
#include "../core/include/timing.h"
#include "../entity/include/player.h"   
#include "include/ui.h"

static FE_Label *update_label;
static FE_Label *event_label;
static FE_Label *render_label;
static FE_Label *fps_label;

static FE_Label *coord_label;
static FE_Label *velocity_label;

static bool UI_Created = false;

static void CreateUI()
{
    update_label = FE_CreateLabel(PresentGame->font, "Update: ", FE_NewVector(0, 0), COLOR_BLACK);
    update_label->showbackground = true;

    event_label = FE_CreateLabel(PresentGame->font, "Event: ", FE_NewVector(0, 25), COLOR_BLACK);
    event_label->showbackground = true;

    render_label = FE_CreateLabel(PresentGame->font, "Render: ", FE_NewVector(0, 50), COLOR_BLACK);
    render_label->showbackground = true;

    fps_label = FE_CreateLabel(PresentGame->font, "FPS: ", FE_NewVector(0, 75), COLOR_BLACK);
    fps_label->showbackground = true;

    coord_label = FE_CreateLabel(PresentGame->font, "X: Y:", FE_NewVector(0, 100), COLOR_BLACK);
    coord_label->showbackground = true;

    velocity_label = FE_CreateLabel(PresentGame->font, "Velocity: ", FE_NewVector(0, 125), COLOR_BLACK);
    velocity_label->showbackground = true;

    UI_Created = true;
}

static void DestroyUI()
{
    if (UI_Created) {
        FE_DestroyLabel(update_label);
        FE_DestroyLabel(event_label);
        FE_DestroyLabel(render_label);
        FE_DestroyLabel(fps_label);
        FE_DestroyLabel(coord_label);
        FE_DestroyLabel(velocity_label);
    }
    UI_Created = false;
}

void FE_DebugUI_Update(FE_Player *player)
{
    if (PresentGame->DebugConfig.ShowTiming) {

        static float fps_timer = 0;
        fps_timer += FE_DT;
        if (fps_timer > 0.25f) {
            fps_timer = 0;

            if (!UI_Created)
                CreateUI();
            
            char update_text[64];
            char event_text[64];
            char render_text[64];
            char fps_text[64];
            char coord_text[64];
            char velocity_text[64];

            sprintf(update_text, "Update: %.2Lfms", PresentGame->Timing.UpdateTime);
            sprintf(event_text, "Event: %.2Lfms", PresentGame->Timing.EventTime);
            sprintf(render_text, "Render: %.2Lfms", PresentGame->Timing.RenderTime);
            if (PresentGame->config->vsync)
                sprintf(fps_text, "FPS: %i (VSYNC)", FE_FPS);
            else
                sprintf(fps_text, "FPS: %i", FE_FPS);
            sprintf(coord_text, "X: %i Y: %i", player->PhysObj->body.x, player->PhysObj->body.y);
            sprintf(velocity_text, "Velocity: (%.2f, %.2f)", player->PhysObj->velocity.x, player->PhysObj->velocity.y);

            FE_UpdateLabel(update_label, update_text);
            FE_UpdateLabel(event_label, event_text);
            FE_UpdateLabel(render_label, render_text);
            FE_UpdateLabel(fps_label, fps_text);
            FE_UpdateLabel(coord_label, coord_text);
            FE_UpdateLabel(velocity_label, velocity_text);
        }
    } else {
        if (UI_Created)
            DestroyUI();
    }
}
