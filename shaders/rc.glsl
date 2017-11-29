
/* The module to use. A module is a set of shaders used to produce
   the visualizer. The structure for a module is the following:
   
   module_name [directory]
       1.frag [file: fragment shader],
       2.frag [file: fragment shader],
       ...
       
   Shaders are loaded in numerical order, starting at '1.frag',
   continuing indefinitely. The results of each shader (except
   for the final pass) is given to the next shader in the list
   as a 2D sampler.
   
   See documentation for more details. */
#request mod graph

/* GLFW window hints */
#request setfloating  true
#request setdecorated true
#request setfocused   true
#request setmaximized false

/* GLFW window title */
#request settitle "GLava"

/* GLFW buffer swap interval (vsync), set to '0' to prevent
   waiting for refresh, '1' (or more) to wait for the specified
   amount of frames. */
#request setswap 1

/* Linear interpolation for audio data frames. Drastically
   improves smoothness with configurations that yield low UPS
   (`setsamplerate` and `setsamplesize`), or monitors that have
   high refresh rates.
   
   This feature itself, however, will effect performance as it
   will have to interpolate data every frame on the CPU.
   
   This will delay data output by two update frames, so it can
   desync audio with visual effects on low UPS configs. */
#request setinterpolate true

/* Frame limiter, set to the frames per second (FPS) desired or
   simple set to zero (or lower) to disable the frame limiter. */
#request setframerate 0

/* Enable/disable printing framerate every second. 'FPS' stands
   for 'Frames Per Second', and 'UPS' stands for 'Updates Per
   Second'. Updates are performed when new data is submitted
   by pulseaudio, and require transformations to be re-applied
   (thus being a good measure of how much work your CPU has to
   perform over time) */
#request setprintframes true

/* PulseAudio sample buffer size. Lower values result in more
   frequent audio updates (also depends on sampling rate), but
   will also require all transformations to be applied much 
   more frequently (CPU intensive).
   
   High (>2048, with 22050 Hz) values will decrease accuracy
   (as some signals can be missed by transformations like FFT)
   
   The following settings (@22050 Hz) produce the listed rates: 
   
   Sample    UPS                  Description
   - 2048 -> 43.0  (low accuracy, cheap), use with ~60 FPS
   - 1024 -> 86.1  (high accuracy, expensive), use with 120+ FPS
   -  512 -> 172.3 (extreme accuracy, very expensive), use only
                   for graphing accurate spectrum data with
                   custom modules. */
#request setsamplesize 1024

/* Audio buffer size to be used for processing and shaders. 
   Increasing this value can have the effect of adding 'gravity'
   to FFT output, as the audio signal will remain in the buffer
   longer.

   This value has a _massive_ effect on FFT performance and
   quality for some modules. */
#request setbufsize 4096

/* PulseAudio sample rate. Lower values can add 'gravity' to
   FFT output, but can also reduce accuracy. Most hardware
   samples at 44100Hz.
   
   Lower sample rates also can make output more choppy, when
   not using interpolation. It's generally OK to leave this
   value unless you have a strange PulseAudio configuration. */
#request setsamplerate 22050

/*                    ** DEPRECATED **
   Scale down the audio buffer before any operations are 
   performed on the data. Higher values are faster.
   
   This value can affect the output of various transformations,
   since it applies (crude) averaging to the data when shrinking
   the buffer. It is reccommended to use `setsamplerate` and
   `setsamplesize` to improve performance or accuracy instead. */
#request setbufscale 1

/* OpenGL context and GLSL shader versions, do not change unless
   you _absolutely_ know what you are doing. */
#request setversion 3 3
#request setshaderversion 330
