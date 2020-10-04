#!/bin/sh
SUBDIRS="src plugins plugins/gmerlin include lib lib/gradients lib/xaos lib/xaos/src lib/xaos/src/util lib/xaos/src/filter lib/xaos/src/include lib/xaos/src/include/config lib/xaos/src/include/mac lib/xaos/src/include/i386 lib/xaos/src/engine lib/xaos/src/ui-hlp lib/meshes lib/images"

make distclean

CLEANFILES="Makefile Makefile.in *.o *.lo *.la .libs .deps"

TOPCLEANFILES="aclocal.m4 config.guess config.status config.sub configure gmerlin.spec gmerlin.pc libtool ltmain.sh autom4te*.cache depcomp install-sh missing mkinstalldirs Makefile.in"

echo "Cleaning up..."
for i in $TOPCLEANFILES; do
echo "Removing $i"
rm -rf $i
done

echo "Cleaning up..."
for i in $TOPCLEANFILES; do
echo "Removing lib/xaos/$i"
rm -rf lib/xaos/$i
done

for i in $CLEANFILES; do
echo "Removing $i"
rm -rf $i
done

for i in $SUBDIRS; do
  for j in $CLEANFILES; do
  echo rm -rf $i/$j
  rm -rf $i/$j
  done
done

echo "You can now rerun autogen.sh"
