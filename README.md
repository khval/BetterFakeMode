# BetterFakeMode
Make small patch to replace not so nice-looking fake modes in AmigaOS4.

# The problems, I like to solve.

* Aspect ratio is often wrong, it’s not stretched / scaled correct.

* Many games and programs only work with 40 or 80 bytes per row,
and will not work with anything else. (Providing the correct size for video mode's
will allow old programs and games to be displayed correct.)

* Noticed problems with double buffered screens.

# Current status.

Many of problems might not be OpenScreen actually it can be in definitions of the fake modes as well.

The rendering engine is pretty stupid right now.

Currently it will render on one screen, but not open the screen only display it in a window, it will not scale it, 
Only the first fake screen you open will be displayed, 2en screen you open will be ignored, when you close the first screen, it will display the 2en screen.
This is experiment, Ad Hoc development, I do not know what I’m doing.

This project is not for the end user, this project might be interesting to someone who likes to contribute or know how to solve some of problems listed under issues, or have better ideas on how solve some of the problems, then what I have come up with.

For the users, I put in some work to make sure does not  crash too badly, 
but don’t expect it to work on anything else then the tests provided.
