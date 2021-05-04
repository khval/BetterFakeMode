# BetterFakeMode
Make small patch to replace not so nice-looking fake modes in AmigaOS4.

# The problems, I like to solve.

* Aspect ratio is often wrong, it’s not stretched / scaled correct.

* Many games and programs only work with 40 or 80 bytes per row,
and will not work with anything else. (Providing the correct size for video mode's
will allow old programs and games to be displayed correct.)

* Noticed problems with double buffered screens.

* Too often AmigaOS4.1 thinks screens are true color screens when they are planar screens.

# Current status.

Many of problems might not be OpenScreen actually it can be in definitions of the fake modes as well.

The rendering engine is pretty stupid right now.

Currently it will render on one screen, but not open the screen only display it in a window, it will not scale it, 
Only the first fake screen you open will be displayed, 2en screen you open will be ignored, when you close the first screen, it will display the 2en screen.
This is experiment, Ad Hoc development, I do not know what I’m doing.

This project is not for the end user, this project might be interesting to someone who likes to contribute or know how to solve some of problems listed under issues, or have better ideas on how solve some of the problems, then what I have come up with.

For the users, I put in some work to make sure does not  crash too badly, 
but don’t expect it to work on anything else then the tests provided.

# Priority

Ok here is native plan for this project.

1st

Because this is an Ad Hoc development project, first thing is to find out if this is doable? So, first order of business is quickly proto type something, that shows if we are able open some screens, if get windows working etc, if I can get windows to not crash, and I can get normally draw commands to work. then 2en phase can start.

2en

Work on user input, events, make sure windows and user input is forwarded to the correct fake window inside the fake screen.

3rd

When we can see if this project might work, the work on updating the code, so that it displays correct. remove any graphic glitches,
Updated the engine: focus now shifts to making it display in full screen mode, fix the aspect ratio, preferences, and so on.
