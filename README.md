## Intro

The goal of this volatility plugin is to extract a screenshot of all
open X windows from a memory dump.

## Previous work

The volatility plugin `linux_xwindows` realised by Adam Bridge
(https://github.com/bridgeythegeek/linux_xwindows) extracts various
metadata from the window objects registered to X, which are identified
by walking the data structures of the X server. For example, for each
window it lists the window coordinates, the width and height
dimensions, and the window identifier. Adam's
[README](https://github.com/bridgeythegeek/linux_xwindows) has a good
discussion on these details, so we suggest to read them if you are
interested!

As a reference, here you can find a figure representing the core
structures of X and their inter-dependencies:
![Alt text](https://github.com/pagabuc/xfore/blob/master/xorg-structures.png?raw=true "Xorg-mappings")

## Overview

To extract these screenshot we re-use part of the X server code.

The plugin first dumps the X server memory mappings. These mappings
are then given in input to a C program (`loader`), along with the
output of Adam's plugin. This C program `mmap`s (with the `MAP_FIXED`
flag) the memory mappings of Xorg into its own address space. In this
way we recereate the address space of Xorg, ensuring that no reference
between code and data is broken. The loader then calls a function
(`compGetImage`) contained in the Xorg code and saves the result to
file. We identified this "magic" function by looking at how X serves a
request for screenshot during normal usage. Fortunately, this function
can always be found accurately since `struct _Screen` contains a
pointer to it.

Another good feature of this approach is that it is able to retreive
the screenshot even if the screen was locked at the time of dumping,
and even of windows hidden behind other windows!
    
## Usage

After cloning this repository, compile the loader:
    
`gcc -Wall -o loader loader.c`

To use the plugins:

`vol.py --plugins=$PWD/plugins/ --profile=XXX -f ./vbox.dmp linux_screenshot_xwindows --out-dir /tmp/xwds/`

To convert the results from `xwd` to `png`:
    
`find /tmp/xwds/ -type f -name "*.xwd" -exec convert {} {}.png \;`

## Tested version of Xorg

We successuly retrived the screenshot from the following setups:

    
1) Ubuntu 14.04 LTS - X.Org X Server 1.15.1 (Release Date: 2014-04-13)

2) Ubuntu 16-04 LTS - X.Org X Server 1.18.4 (Release Date: 2016-07-19)
 
3) Debian 9 Testing - X.Org X Server 1.19.6 (Release Date: 2017-12-20)

4) Kubuntu 18.04 LTS - X.Org X Server 1.19.6 (Release Date: 2017-12-20)

    
## Notes
A few random but important notes:
    
1) In order to limit the process to only “drawable” images we select
in the volatility plugin only the images with reasonable size.

2) This tools works *only* if Xorg uses software rendering. This is
not usually the case on physical machines but it was used by default
on the VirtualBox machines we tested.

3) Don't forget that you are seamlessly running code extracted from a
memory dump. It is probably not difficult for an attacker to tamper
the dump and gain code execution on your box. So take the necessary
precautions.

    
## Authors
This project has been completed during a Semester Project in Fall 2017
at Eurecom, and realised by two Eurecom students: Hamdi Ammar and
Ahmed Mkadem. It was supervised by [Fabio
Pagani](http://s3.eurecom.fr/~pagabuc/) and [Davide
Balzarotti](http://s3.eurecom.fr/~balzarot/)

