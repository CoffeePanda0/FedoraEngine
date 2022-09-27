#ifndef H_TRIGGER
#define H_TRIGGER

typedef struct {
	vec2 position; // The position in the world of the trigger
	GPU_Rect range;
	void (*event)(); // The function to call once the rect reaches the trigger
    void *event_data;

	bool enabled;
	bool triggered; // Whether or not the trigger is currently being triggered

	bool visible;
	GPU_Rect display_rect;
	FE_Texture *display_texture;
} FE_Trigger;


/** Creates and adds a new trigger to the world
 * \param position The position of the trigger
 * \param range The trigger range of the trigger
 * \param event The function to call when the trigger is hit by a rect
 * \param event_data The data to pass to the callback
 * \param visible Whether or not the trigger is visible or invisible
 * \returns The created trigger
*/
FE_Trigger *FE_Trigger_Create(vec2 position, int range, void (*event)(), void *event_data, bool visible);


/** Renders all visible triggers to the screen
 * \param camera The camera to render the triggers to
*/
void FE_Trigger_Render(FE_Camera *camera);


/** Checks if a rect is colliding with a trigger, and if so, call the trigger event
 * \param rect The rect to check
 * \returns True if the the trigger was intersected, false otherwise
*/
bool FE_Trigger_Check(GPU_Rect *rect);


/** Destroys a trigger and frees memory
 * \param trigger The trigger to destroy
*/
void FE_Trigger_Destroy(FE_Trigger *trigger);


/* Cleans and destroys all triggers */
void FE_Trigger_Clean();


#endif