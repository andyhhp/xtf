#!/bin/sh -e

[ $# -ne 0 ] && { echo "Interactive script to create new XTF tests"; exit 1; }

cd $(dirname $(readlink -f $0))

# Test name
echo -n "Test name: "
read NAME

[ -z "$NAME" ] && { echo "No name given"; exit 1; }
[ -e tests/$NAME ] && { echo "Test $NAME already exists"; exit 1; }
mkdir -p tests/$NAME

# Category
echo -n "Category [utility]: "
read CATEGORY

if [ -z "$CATEGORY" ];
then
    CATEGORY="utility"
fi

# Environments
echo -n "Environments [hvm32]: "
read ENVS

if [ -z "$ENVS" ];
then
    ENVS="hvm32"
fi

# Optional extra config
echo -n "Extra xl.cfg? [y/N]: "
read EXTRA

if [ -z "$EXTRA" ];
then
    EXTRA=n
fi

if [ "$EXTRA" = "Y" ];
then
    EXTRA=y
fi

# Write Makefile
echo "Writing default tests/$NAME/Makefile"
{
cat <<EOF
MAKEFLAGS += -r
ROOT := \$(abspath \$(CURDIR)/../..)

include \$(ROOT)/build/common.mk

NAME      := $NAME
CATEGORY  := $CATEGORY
TEST-ENVS := $ENVS

EOF

[ "$EXTRA" = "y" ] && cat <<EOF
TEST-EXTRA-CFG := extra.cfg.in

EOF

cat <<EOF
obj-perenv += main.o

include \$(ROOT)/build/gen.mk
EOF
} > tests/$NAME/Makefile

# Possibly insert an empty extra.cfg file
if [ "$EXTRA" = "y" ];
then
    echo "Writing default tests/$NAME/extra.cfg.in"
    echo "" > tests/$NAME/extra.cfg.in
fi

# Write main.c
echo "Writing default tests/$NAME/main.c"
{
cat <<EOF
/**
 * @file tests/$NAME/main.c
 * @ref test-$NAME - TODO.
 *
 * @page test-$NAME TODO
 *
 * @see tests/$NAME/main.c
 */
#include <xtf.h>

void test_main(void)
{
    printk("Test $NAME\n");

    xtf_success(NULL);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
EOF
} > tests/$NAME/main.c
