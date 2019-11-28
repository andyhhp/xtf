#!/bin/sh -e

fail () { echo "$1"; exit 1; }

[ $# -gt 1 ] && fail "Interactive script to create new XTF tests\n  Usage: $0 [\$NAME]"

cd $(dirname $(readlink -f $0))

# Test name
if [ $# -eq 1 ]
then
    NAME=$1
    echo "Test name: $NAME"
else
    echo -n "Test name: "
    read NAME
fi
NAME_UC=$(echo $NAME | tr a-z A-Z)

[ -z "$NAME" ] && fail "No name given"
[ -e tests/$NAME ] && fail "Test $NAME already exists"
mkdir -p tests/$NAME

# Category - Select default based on test name
if [ ${NAME#xsa-} != ${NAME} ];
then
    DEF_CATEGORY="xsa"
else
    DEF_CATEGORY="utility"
fi

echo -n "Category [$DEF_CATEGORY]: "
read CATEGORY
CATEGORY=${CATEGORY:-$DEF_CATEGORY}

# Environments
echo -n "Environments [hvm32]: "
read ENVS
ENVS=${ENVS:-"hvm32"}

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
 * @ref test-$NAME
 *
EOF

if [ $CATEGORY != "xsa" ]
then
    cat <<EOF
 * @page test-$NAME $NAME
EOF
else
    cat <<EOF
 * @page test-$NAME $NAME_UC
 *
 * Advisory: [$NAME_UC](https://xenbits.xen.org/xsa/advisory-${NAME#xsa-}.html)
EOF
fi

cat <<EOF
 *
 * @todo Docs for test-$NAME
 *
 * @see tests/$NAME/main.c
 */
#include <xtf.h>

EOF
[ $CATEGORY != "xsa" ] && \
    echo 'const char test_title[] = "Test '$NAME'";' || \
    echo 'const char test_title[] = "'$NAME_UC' PoC";'
cat <<EOF

void test_main(void)
{
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

# Update docs/all-tests.dox with a placeholder
echo "Adding placeholder to docs/all-tests.dox"
cat >> docs/all-tests.dox <<EOF
# Placeholder: Merge into the appropriate location above
@subpage test-$NAME - @todo title
EOF
