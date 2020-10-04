#!/bin/sh
if test -d /usr/local/share/aclocal; then
ACLOCAL_FLAGS="-I m4 -I /usr/local/share/aclocal"
else
ACLOCAL_FLAGS="-I m4"
fi

LIBTOOLIZE=`which glibtoolize`
if test "x$LIBTOOLIZE" = "x"; then
  LIBTOOLIZE=`which libtoolize`
fi

if test "x$LIBTOOLIZE" = "x"; then
  echo "No libtoolize command found"
  exit 1
fi

echo -n "doing aclocal..."
aclocal $ACLOCAL_FLAGS

if test $? != "0"; then
  echo "failed"
  exit 1
fi

echo "done"


echo -n "doing libtoolize..."
$LIBTOOLIZE --automake --copy --force
# $LIBTOOLIZE --copy --force

if test $? != "0"; then
  echo "failed"
  exit 1
fi

echo "done"
echo -n "doing autoheader..."
autoheader

if test $? != "0"; then
  echo "failed"
  exit 1
fi

echo "done"
echo -n "doing autoconf..."
autoconf

if test $? != "0"; then
  echo "failed"
  exit 1
fi

echo "done"
echo -n "doing automake..."
automake -a
if test $? != "0"; then
  echo -n "automake failed in first try (broken version). Trying once more..."
  $LIBTOOLIZE --automake --copy --force
  automake -a
  if test $? != "0"; then
    echo "failed"
  exit 1
  else 
    echo "Success :)"
  fi
fi
echo "done"

echo "Doing autogen.sh for xaos"
cd lib/xaos
./autogen.sh
cd ../..

echo "You can now run ./configure"
