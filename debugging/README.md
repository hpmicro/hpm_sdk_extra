# Debugging Related

## Overview

Here, you can find debug related information about HPMicro devices.

### [debug_key](debug_key)
When device lifecycle has been programmed to "SECURE" stage, by default, debugger
can no longer be connected to device. In order to make debug work again, the following
"unlock" procedure must be done via JTAG:
- send AUTHEN instruction to IR
- send 128 bit data to DR, expect device UID will be read 
- send 128 bit debug_key to DR

corresponding example for [openocd config file](debug_key/debug_key.cfg) and [jlink script](debug_key/debug_key.jlinkscript) can be found in "debug_key"


