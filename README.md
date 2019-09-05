# meloader
Linux i386 tool to load and execute ME modules.

The code as initially presented here is released under the GPLv2. 
(I reused code from an earlier project under that license) 
A commit adding license headers will soon follow.

~~Much of the later achievements demoed using this project were done on a local branch which
I lost, resetting the project to its progress at April 21st 2019. This also means that the
hardware register names and structures as used by this tool do not represent my current 
understanding of the ME hardware.~~ I have since rewritten all the features that I lost, and
it should now be able to do anything I've demonstrated using my local version of the tool.

Running the tool requires mmaping addresses from 0x1000 onward and so means that low mmap 
addresses should be enabled.

The chipset initially targetted by this tool is currently Sunrise Point (SPT, 100 series chipset),
although it has since been rewritten to allow full reconfiguration of the emulated peripherals
and interconnect.

It is provided as a interoperability tool to allow development of open alternative firmwares for
the CSME.

This tool requires a rom library dump from the ME to use. 
See https://github.com/ptresearch/IntelTXE-PoC for a means of acquiring one, though that
will yield a ROM for a different chipset (BXT). That chipset shares most core ME peripherals
with SPT so changing the code will mostly mean tweaking addresses.

