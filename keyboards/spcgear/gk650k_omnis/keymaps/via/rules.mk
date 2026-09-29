ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes

# The SignalRGB module turns the console on, but never prints anything, and the
# SN32F240B has no endpoint left for it next to VIA, NKRO and extra keys. The
# module's rules are read after these, so a plain assignment would be undone.
override CONSOLE_ENABLE = no
