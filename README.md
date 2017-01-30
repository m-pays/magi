Coin Magi
====================

Copyright (c) 2009-2012 The Bitcoin Core developers

Copyright (c) 2012-2014 The PPCoin developers

Copyright (c) 2014-2017 The Magi Core developers

Coin Magi, derived from Bitcoin and PPCoin, is released under the terms of 
the MIT license. See COPYING for more information or see 
http://opensource.org/licenses/MIT.

Intro
---------------------
Coin Magi (XMG) is an online payment system, enabling instant payments to anyone in the world without using an intermediary. Magi coins can be minted by computational devices including personal computers and portable devices through mPoW and mPoS. Magi aims at fairness, cost effective and energy efficiency during coin minting. Magi is a hybrid PoW/PoS-based cryptocurrency that integrates two mechanisms: proof-of-work (PoW) and proof-of-stake (PoS) protocols. Magi is a CPU coin. 

Features
---------------------
- mPoW, the magi's proof-of-work (PoW) protocol, in addition to required computational works to be done to deter denial of service attacks, is also a network-dependent rewarding model system. The mPoW rewards participants who solve complicated cryptographical questions not only to validate transactions but also to create new blocks in order to generate coins. The coins mined via mPoW are adjusted and balanced by two primary mechanisms: 1) stimulating network activities by issuing rewards, and 2) mitigating redundant mining sources by reducing rewards.

- The particular designed block reward system to remove the competitive nature of 
mining and offer an even playing field for anyone looking to issue coins 
without expensive equipment - offering features such as energy saving, proof of 
mining.

- mPoS, the magi's proof-of-stake (PoS) protocol, aims to achieve distributed consensus through operations in addition to mPoW. mPoS is designed such that it rejects potential attacks, for example, through accumulating a large amount of coins or offline stake time. Magi hybridizes PoW with PoS, and integrate both consensus approaches in order to acquire benefits from the two mechanisms and create a more robust payment system. mPoS particularly enhances the security of XMG's staking system that distinguishes itself from the original concept developed by PPCoin. 

Development process
---------------------

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

Setup
---------------------
If you are just starting to explore magi, or upgrading wallet from versions prior to v1.3.0, the following procedure is recommended:  

1) Backup wallet.dat;

2) Remove the block-chain data under the .magi (unix-like system) or Magi (OS X or Windows) folder, except for wallet.dat;

3) Download latest block-chain data from here: http://coinmagi.org/bin/block-chain;

4) Unzip all the contents under "m-blockchain" into the .magi or Magi folder;

5) Launch the new wallet. 

- Windows: double click to install, or unpack the files and run the wallet directly;

- Mac OS: unpack the files and copy to the Application folder, and then run the wallet directly;

- Linux: unpack the files and run the wallet directly. 

Info
---------------------
- Website: http://www.coinmagi.org
- Bitcointalk thread: https://bitcointalk.org/index.php?topic=735170.0
- Forum: http://www.m-talk.org/
- Freenode IRC: #magi
