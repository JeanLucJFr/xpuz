# xpuz
A fork of xjig 2.5, the jigsaw puzzle : https://xjigsaw.net/

If the original build instructions fail due to a static link as it did for me, try the following Makefile I did for AUR :
https://aur.archlinux.org/cgit/aur.git/tree/Makefile?h=xpuz

Then :	
cp xpuz.man xpuz.6
gzip xpuz.6
make
make install
