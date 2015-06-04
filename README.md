#### CSC209H1 Software Tools and Systems Programming (Winter 2015)
Several C codes (assignments/projects) from CSC209H1 course at University of Toronto.

##### addecho.c

> This is a simple C program that adds echo to a wav file.
> Usage: addecho [-d delay] [-v volume_scale] sourcewav destwav
> For example, if you want to add 12000 sampling delay and at the volume of 4, then you type:
> addecho -d 12000 -v 4 original.wav new.wav
> This will creat new.wav with echo sound.

##### remvocal.c

> Another simple C program that removes vocal from a wav file
> Usage: remvocals sourcewav destwav
> This program works on stereo sound only.
> How does this work? Well..., this program reads first 44 bytes of a wav file for the header, and keeps reading a sequence of `shorts` (two times) for left and right sound. And Write two copies of `(left - right) / 2` to the output. For some reasons, this algorithm removes vocal sound from the wav file. (Not completely, but it works.)



