MODE CON:cols=150 lines=80
color 0b
TITLE NCP1400
CLS
:start

CLS


: Burn hex file for Nordic fob onto unit
: -I Set ISP frequency to 100kHz
: -s Read signature bytes
: -wt Get current target voltage
: -e erase
: -ms Serial programming mode
: -q Read back fuse bytes
: -f DDC6 = external crystal, no internal 8 divider
: -EF8 is needed for complete fuse byte programming?
: -l -L Set and verify lock byte

:High Fuse DD
:Low Fuse E2
:STK500 -fDDE2, the DD is the low byte, E2 is the high byte!



"C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe" -e -dATmega168 -pf -vf -if"C:\Documents and Settings\Eng3-DW\Desktop\Workspace\SPI Shortcut\UCOM10\main.hex" -ms -q -cUSB -I100kHz -s -wt


pause

goto start



