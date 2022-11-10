#include "../core/include/include.h"

#include "../entity/include/player.h"   
#include "include/include.h"

static FE_UI_Label *update_label;
static FE_UI_Label *event_label;
static FE_UI_Label *render_label;
static FE_UI_Label *fps_label;

static FE_UI_Label *coord_label;
static FE_UI_Label *velocity_label;

static bool UI_Created = false;

static void CreateUI()
{
    update_label = FE_UI_CreateLabel(PresentGame->Client->Font, "Update: ", rel_w(20), 0, 0, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, update_label);
    update_label->showbackground = true;

    event_label = FE_UI_CreateLabel(PresentGame->Client->Font, "Event: ", rel_w(20), 0, 25, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, event_label);
    event_label->showbackground = true;

    render_label = FE_UI_CreateLabel(PresentGame->Client->Font, "Render: ", rel_w(20), 0, 50, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, render_label);
    render_label->showbackground = true;

    fps_label = FE_UI_CreateLabel(PresentGame->Client->Font, "FPS: ", rel_w(20), 0, 75, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, fps_label);
    fps_label->showbackground = true;

    coord_label = FE_UI_CreateLabel(PresentGame->Client->Font, "X: Y:", rel_w(20), 0, 100, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, coord_label);
    coord_label->showbackground = true;

    velocity_label = FE_UI_CreateLabel(PresentGame->Client->Font, "Velocity: ", rel_w(20), 0, 125, COLOR_BLACK);
    FE_UI_AddElement(FE_UI_LABEL, velocity_label);
    velocity_label->showbackground = true;

    UI_Created = true;
}

void FE_DebugUI_Destroy()
{
    if (UI_Created) {
        FE_UI_DestroyLabel(update_label, true);
        FE_UI_DestroyLabel(event_label, true);
        FE_UI_DestroyLabel(render_label, true);
        FE_UI_DestroyLabel(fps_label, true);
        FE_UI_DestroyLabel(coord_label, true);
        FE_UI_DestroyLabel(velocity_label, true);
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
            if (PresentGame->config->Vsync)
                sprintf(fps_text, "FPS: %i (VSYNC)", FE_FPS);
            else
                sprintf(fps_text, "FPS: %i", FE_FPS);
            sprintf(coord_text, "X: %i Y: %i", player->player->PhysObj->body.x, player->player->PhysObj->body.y);
            sprintf(velocity_text, "Velocity: (%.2f, %.2f)", player->player->PhysObj->velocity.x, player->player->PhysObj->velocity.y);

            FE_UI_UpdateLabel(update_label, update_text);
            FE_UI_UpdateLabel(event_label, event_text);
            FE_UI_UpdateLabel(render_label, render_text);
            FE_UI_UpdateLabel(fps_label, fps_text);
            FE_UI_UpdateLabel(coord_label, coord_text);
            FE_UI_UpdateLabel(velocity_label, velocity_text);
        }
    } else {
        if (UI_Created)
            FE_DebugUI_Destroy();
    }
}
