# BetterFakeMode
Make small patch to replace not so nice-looking fake modes in AmigaOS4.

Many games and programs only work with 40 or 80 bytes per row,
and will not work with anything else.

This is early days for the project, the only thing we do safely 
is to monitor the activities, from programs trying to open screens.

(We also use it simulate failure, that can be useful, to find unsuspected bugs in programs.)
