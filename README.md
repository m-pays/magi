Magi development tree - 1.2.1.1+armv7l
===

Magi is a hybrid PoW/PoS-based cryptocurrency. Magi is also a CPU coin.

Fork Information
===

This branch is an unofficial offshoot that allows building for armv7l
architecture CPUs. It was made for the convenience of people that either 
use an ARM desktop or run an ARM server and who want to keep their 
wallet open to mint coins via POS or want to run an energy-efficient 
node for solo mining.

This fork really only adds a few lines of changes to the Makefile and 
Qt's version of a makefile. It might even be a drop-in solution for the 
future, if neither have significant changes made to them. (Either that
or the Magi team will merge it.)

<b>CAUTION:</b> This build uses libdb5.1 ! Essentially, this means that
you can't swap the blockchain database that these binaries create (or 
have opened) back to an official wallet/server build because they use 
libdb4.8. This was done to simplify the build process dramatically (just
an extra apt-get entry) instead of installing and relying on an 
unsupported libdb4.8-armhf binary.

Information for building is located in:
[doc/build-unix-arm.txt](doc/build-unix-arm.txt) for headless and
[doc/readme-qt-arm.rst](doc/readme-qt-arm.rst) for the Qt GUI.

Development process
===========================

Developers work in their own trees, then submit pull requests when
they think their feature or bug fix is ready.

The patch will be accepted if there is broad consensus that it is a
good thing.  Developers should expect to rework and resubmit patches
if they don't match the project's coding conventions (see coding.txt)
or are controversial.

The master branch is regularly built and tested, but is not guaranteed
to be completely stable. Tags are regularly created to indicate new
stable release versions of Magi.

Feature branches are created when there are major new features being
worked on by several people.

From time to time a pull request will become outdated. If this occurs, and
the pull is no longer automatically mergeable; a comment on the pull will
be used to issue a warning of closure. The pull will be closed 15 days
after the warning if action is not taken by the author. Pull requests closed
in this manner will have their corresponding issue labeled 'stagnant'.

Issues with no commits will be given a similar warning, and closed after
15 days from their last activity. Issues closed in this manner will be 
labeled 'stale'.

Magi names: Coin Magi, Coin of the Magi.
Symbol: XMG

Bitcointalk thread: https://bitcointalk.org/index.php?topic=735170.0
Website: http://www.cryptomagic.com/

Coins Source Verification
