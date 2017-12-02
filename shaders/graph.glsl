
/* Distance (in pixels) for each fragment to sample the audio data */
#define SAMPLE_RANGE 0.2

/* Amount of samples for each fragment, using the above range for each sample */
#define SAMPLE_AMT 22

/* Inverse horizontal scale, larger means less higher frequencies displayed */
#define WSCALE 11

/* Vertical scale, larger values will amplify output */
#define VSCALE 300

/* Rendering direction, either -1 (inwards) or 1 (outwards). */
#define DIRECTION -1

/* Graph color logic. The shader will only use the `COLOR` macro definition for output. */

/* right color offset */
#define RCOL_OFF (gl_FragCoord.x / 3000)
/* left color offset */
#define LCOL_OFF ((screen.x - gl_FragCoord.x) / 3000)
/* vertical color step */
#define LSTEP (gl_FragCoord.y / 170)
/* actual color definition */
#define COLOR vec4((0.3 + RCOL_OFF) + LSTEP, 0.6 - LSTEP, (0.3 + LCOL_OFF) + LSTEP, 1)
