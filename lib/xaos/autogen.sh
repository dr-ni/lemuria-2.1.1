#!/bin/sh
if test -d /usr/local/share/aclocal; then
ACLOCAL_FLAGS="-I /usr/local/share/aclocal"
else
ACLOCAL_FLAGS=""
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
  echo "failed"
  exit 1
fi
echo "done"

