#ifndef LUFA_CONFIG_H
#define LUFA_CONFIG_H

#define USE_STATIC_OPTIONS
#define FIXED_CONTROL_ENDPOINT_SIZE 8
#define FIXED_NUM_CONFIGURATIONS 1
#define KEYBOARD_IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define KEYBOARD_EPSIZE 8

#endif
