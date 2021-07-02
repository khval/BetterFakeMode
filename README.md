# BetterFakeMode
Make small patch to replace not so nice-looking fake modes in AmigaOS4.

# The problems, I like to solve.

* Aspect ratio is often wrong, it’s not stretched / scaled correct.

* Many games and programs only work with 40 or 80 bytes per row,
and will not work with anything else. (Providing the correct size for video mode's
will allow old programs and games to be displayed correct.)

* Noticed problems with double buffered screens.

* Too often AmigaOS4.1 thinks screens are true color screens when they are planar screens.

* AmigaOS 4.1 does not allow you to use "Screen -> BitMap", only "Screen -> RastPort -> BitMap"
(Old programs might crash if they poke into Screen -> BitMap)

* Lots of missing modes like Porductivity Modes, DBPAL, DBNTSC

# Current status.

Many of problems might not be OpenScreen actually it can be in definitions of the fake modes as well.

The rendering engine is pretty stupid right now.

Currently it will render on one screen, but not open the screen only display it in a window, it will not scale it, 
Only the first fake screen you open will be displayed, 2en screen you open will be ignored, when you close the first screen, it will display the 2en screen.
This is experiment, Ad Hoc development, I do not know what I’m doing.

This project is not for the end user, this project might be interesting to someone who likes to contribute or know how to solve some of problems listed under issues, or have better ideas on how solve some of the problems, then what I have come up with.

For the users, I put in some work to make sure does not  crash too badly, 
but don’t expect it to work on anything else then the tests provided.

# What will work with this patch?

Mostly nothing will work with this patch, don't use it :-)

* Some events might not be sendt, 
(pleae report problem if you find any.)

* Double buffering is broken.
(Never used double buffer myself, so not sure how this should work, its not often used in programs, 
because it hard to do correct, even if done correct, it can crash, at exit. So maybe not so importent.)

* Intuition Windows are not rendered correct yet.
(Resize window, close window, depth gadget.)

* There pointers that are not set, that should be set, but I do not know what they are for.
(Unexpected crashes are likely, need more test cases, code snippets.)

* Do not try to open MUI, Reaction, or GadTools based programs.
(Some internal structures are not well documented, and I need to simulate this for this to work, 
or I need to fool the program completely. replacing Gadtools with my own fake one.)

# Demos I know works with this Patch.

Dkd-Chaotic, Dkd-swnf, dkd-jaeg.

(There will be a DSI error when the demo starts, press ignore.
I like to know what goes wrong, but don’t have the source code to the demos, 
nor to the OS, so stacktrace logs don't help, no symbols to help me.)

# Progams I know work with this patch

Vista Pro V3

# What is working.

* it is able to catch programs, trying to open old screen modes.
 
* it is able to display planar screens in a window. 

* It does scale fake screen to fit the window.

* It does send some events.

# How can you help.

End users are useless here, but if you are a developer:

* IF you have some example code, that you know is not working correct, then that will be really helpful.
(Not interested in 1000 of lines of code, only small example of 100 to 200 lines or so.)

* You also help directly contributing to the code, help fix bugs, and add missing features.

# How does it work?

Well, think of BetterFakeMode as an engine, it redirects things normal the OS does, and takes over, so in other words, the OS does not know what has happened, the OS does not know about BetterFakeMode fake screens, or fake windows, there is a chance this will cause confusion, any place where the OS get confused, we must take over. At this point in time, we don’t know what part of the OS that does not like this or what part that does. This why test cases are essential.

If it's not fake screen/fake bitmap, fake window, it's handeled by the OS.

# Priority

Ok here is native plan for this project.

1st - Done

Because this is an Ad Hoc development project, first thing is to find out if this is doable? So, first order of business is quickly proto type something, that shows if we are able open some screens, if get windows working etc, if I can get windows to not crash, and I can get normally draw commands to work. then 2en phase can start.

2en

Work on user input, events, make sure windows and user input is forwarded to the correct fake window inside the fake screen.

3rd - in progress 

A fake gadtools hack, to make old gadtool based porgrams work.

4th

When we can see if this project might work, the work on updating the code, so that it displays correct. remove any graphic glitches,
Updated the engine: focus now shifts to making it display in full screen mode, fix the aspect ratio, preferences, and so on.

# You found bugs 

If you find bugs while using this software, do NOT report this to Hyperion, they are not to blame, bugs as result of using this software should be reported in the bug tracker on GitHub. As I wrote before only interested in testable cases. if you have the source code to the program that is not working with this hack, then that’s wherry helpful.

And if you find anything that works better with this hack, please tell me.

