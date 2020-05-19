BUILD_DIR=build
SRC_DIR=src

CC=tcc

SRC=`find $SRC_DIR/ -name '*.c'`
HDR=`find $SRC_DIR/ -name '*.h'`
OBJ=${SRC:$SRC_DIR/%.c=$BUILD_DIR/%.o}

CFLAGS=-Wall -Werror -Wextra -std=c11 -g
LDFLAGS=

all:V: vulkan-fuckery

clean:V:
  rm -f vulkan-fuckery &&
  rm -rf $BUILD_DIR

vulkan-fuckery: $OBJ
  $CC -o $target $prereq $LDFLAGS

$BUILD_DIR/%.o: $SRC_DIR/%.c $HDR
  mkdir -p $(dirname $target) &&
  $CC -c $CFLAGS -o $target $SRC_DIR/$stem.c
