
/* Settings for smoothing functions and transformations commonly
   used to display FFT output */

/* The type of formula to use for weighting values when smoothing.
   Possible values:
   
   - circular     heavily rounded points
   - sinusoidal   rounded at both low and high weighted values
                    like a sine wave
   - linear       not rounded at all, just use linear distance
   */
#define ROUND_FORMULA circular

/* Factor used to scale frequencies. Lower values allows lower
   frequencies to occupy more space. */
#define SAMPLE_SCALE 8

/* The frequency range to sample. 1.0 would be the entire FFT output,
   and lower values reduce the displayed frequencies in a log-like
   scale. */
#define SAMPLE_RANGE 0.9

/* Factor for how to scale higher frequencies. Used in a linear equation
   which is multiplied by the result of the fft transformation. */
#request setfftscale 10.2

/* Cutoff for the bass end of the audio data when scaling frequencies.
   Higher values cause more of the bass frequencies to be skipped when
   scaling. */
#request setfftcutoff 0.3

/* How many frames to queue and run through the average function.
   Increasing this value will create latency between the audio and the
   animation, but will make for much smoother results. */
#request setavgframes 6

/* Whether to window frames ran through the average function (new & old
   frames are weighted less). This massively helps smoothing out
   spontaneous values in the animation. */
#request setavgwindow true

/* Gravity step, higher values means faster drops. The step is applied
   in a rate independant method like so:
   
   val -= (gravitystep) * (seconds per update) */
#request setgravitystep 4.2
