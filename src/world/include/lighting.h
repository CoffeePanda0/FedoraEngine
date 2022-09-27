#ifndef H_LIGHTING
#define H_LIGHTING

typedef struct FE_Light {
    GPU_Rect Rect;
    FE_Texture *Texture;
    uint8_t intensity;
    
    int16_t x_offset;
    int16_t y_offset;

    bool enabled;
} FE_Light;


/** Renders lights to screen (and ambient brightness)
 * \param camera The camera to render from
 * \param world The world to render
*/
void FE_Light_Render(FE_Camera *camera, GPU_Image *world);


/** Creates a new light effect and adds it to the world
 * \param rect The rectangle to render the light effect around
 * \param radius The radius of the light effect
 * \param texture The texture to render the light effect with
 * \returns The new light effect
*/
FE_Light *FE_Light_Create(GPU_Rect rect, int radius, const char *texture);


/** Destroys a light and frees memory
 * \param light The light to destroy
*/
void FE_Light_Destroy(FE_Light *light);


/* Destroys the lighting system */
void FE_Light_Clean();


/** Moves a light to a new location. Use this instead of modifiying the light directly.
 * \param light The light to move
 * \param x The new x location
 * \param y The new y location
*/
void FE_Light_Move(FE_Light *light, int x, int y);


/** Toggles a light on or off
 * \param light The light to toggle
*/
void FE_Light_Toggle(FE_Light *light);


/** Sets the intensity of a light
 * \param light The light to set the intensity of
 * \param intensity The new intensity
*/
void FE_Light_SetIntensity(FE_Light *light, uint8_t intensity);


#endif