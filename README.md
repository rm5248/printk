# printk
printf-like utility library for kernel development

This provides a nearly fully-functional printf-like functionality
for embedded systems that for some reason don't have a full C
library(e.g. a kernel).

This has a main method to facilitate debugging in a Linux
environment.

To use in a kernel:

1. Implement strlen
2. change the methods 'print' and 'printchar' to output data
 to an appropriate place.
3. implement the va_args and family macros.(note: if using GCC,
you can still probably include `<stddef.h>` depending on your
compiler settings)

Still to implement:
- floating point values
- the more obscure specifiers(n, g, G)
- The more obscure width and precision(\* width, \* precision)

## License

GPL V2
