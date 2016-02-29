# bmsampler

Sampler for rendering samples at a specified time to 32bit Float wave files.
For use with `bms-renderer` to speed things up.

Note that the constraint is still there:
That all samples must be 44100hz Streo.
You can use SoX to convert any audio file using `sox in.wav -r 44.1k -c 2 out.ogg`.
