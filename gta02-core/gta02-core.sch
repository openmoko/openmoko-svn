EESchema Schematic File Version 2  date Thu 23 Jul 2009 23:55:10 BST
LIBS:power,device,conn,gta02-core
EELAYER 24  0
EELAYER END
$Descr A3 16535 11700
Sheet 1 12
Title "GTA02-CORE"
Date "20 jun 2009"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Notes Line
	15000 6150 15000 10200
Wire Wire Line
	13450 10000 13650 10000
Wire Wire Line
	13450 9700 13650 9700
Wire Wire Line
	13450 8500 13650 8500
Wire Wire Line
	13450 8050 13650 8050
Wire Wire Line
	13650 7750 13450 7750
Wire Wire Line
	12850 6700 12850 6850
Wire Wire Line
	13650 7300 13450 7300
Wire Wire Line
	12850 6850 13650 6850
Wire Wire Line
	13650 7150 13450 7150
Wire Wire Line
	13650 7000 13450 7000
Wire Wire Line
	13650 7600 13450 7600
Wire Wire Line
	13650 7900 13450 7900
Wire Wire Line
	13450 8350 13650 8350
Wire Wire Line
	13450 9550 13650 9550
Wire Wire Line
	13450 9850 13650 9850
Wire Notes Line
	15000 6150 12350 6150
Wire Notes Line
	15000 10200 12350 10200
Wire Wire Line
	13450 8950 13650 8950
Wire Wire Line
	13450 8800 13650 8800
Wire Wire Line
	13450 9250 13650 9250
Wire Wire Line
	13450 9100 13650 9100
Wire Notes Line
	12350 10200 12350 6150
NoConn ~ 13450 9250
NoConn ~ 13450 9100
Text GLabel 13650 9250 2    60   Input ~ 0
MODEM_RX
Text GLabel 13650 9100 2    60   Output ~ 0
MODEM_TX
NoConn ~ 13450 8950
NoConn ~ 13450 8800
Text GLabel 13650 8950 2    60   Output ~ 0
MODEM_RTS
Text GLabel 13650 8800 2    60   Input ~ 0
MODEM_CTS
Text Notes 12450 6300 0    60   ~ 0
GSM labels unused while calypso status is unresolved
NoConn ~ 13450 10000
NoConn ~ 13450 9850
NoConn ~ 13450 9700
NoConn ~ 13450 9550
Text GLabel 13650 10000 2    60   Input ~ 0
MICIN
Text GLabel 13650 9850 2    60   Input ~ 0
MICIP
Text GLabel 13650 9700 2    60   Output ~ 0
EARN
Text GLabel 13650 9550 2    60   Output ~ 0
EARP
NoConn ~ 13450 8500
NoConn ~ 13450 8350
Text GLabel 13650 8500 2    60   Input ~ 0
MODEM_ON
Text GLabel 13650 8350 2    60   Input ~ 0
MODEM_RST
NoConn ~ 13450 8050
Text GLabel 13650 8050 2    60   Input ~ 0
DL_GSM
NoConn ~ 13450 7900
NoConn ~ 13450 7750
NoConn ~ 13450 7600
Text GLabel 13650 7600 2    60   Input ~ 0
AMP_SHUT
Text GLabel 13650 7750 2    60   Input ~ 0
HP_IN
Text GLabel 13650 7900 2    60   Input ~ 0
INTO
NoConn ~ 13450 7300
NoConn ~ 13450 7150
NoConn ~ 13450 7000
$Comp
L PWR_FLAG #FLG?
U 1 1 4A5C71E1
P 12850 6700
F 0 "#FLG?" H 12850 6970 30  0001 C CNN
F 1 "PWR_FLAG" H 12850 6930 30  0000 C CNN
	1    12850 6700
	1    0    0    -1  
$EndComp
Text GLabel 13650 7150 2    60   Output ~ 0
SIM_CLK
Text GLabel 13650 7000 2    60   Output ~ 0
SIM_RST
Text GLabel 13650 6850 2    60   Output ~ 0
SIM_V
Text GLabel 13650 7300 2    60   BiDi ~ 0
SIM_IO
$Sheet
S 5000 4500 1550 1500
U 4A4B8F03
F0 "SD-SIM" 60
F1 "sd-sim.sch" 60
$EndSheet
$Sheet
S 3000 2000 1500 1500
U 4A45E033
F0 "CPU-POWER" 60
F1 "cpu-power.sch" 60
$EndSheet
$Sheet
S 3000 4500 1500 1500
U 4A434A5F
F0 "LCM" 60
F1 "lcm.sch" 60
$EndSheet
$Sheet
S 8950 4500 1500 1500
U 4A424951
F0 "BT" 60
F1 "bt.sch" 60
$EndSheet
$Sheet
S 1000 2000 1500 1500
U 4A0AE5B6
F0 "CPU" 60
F1 "cpu.sch" 60
$EndSheet
$Sheet
S 7000 2000 1500 1500
U 4A0AE69D
F0 "MEMORY" 60
F1 "memory.sch" 60
$EndSheet
$Sheet
S 1000 4500 1500 1500
U 4A0AE5F3
F0 "IO" 60
F1 "io.sch" 60
$EndSheet
$Sheet
S 5000 2000 1500 1500
U 4A0AE5D4
F0 "PMU" 60
F1 "pmu.sch" 60
$EndSheet
$Sheet
S 8950 2000 1500 1500
U 4A0AE60B
F0 "AUDIO" 60
F1 "audio.sch" 60
$EndSheet
$Sheet
S 7000 4500 1500 1500
U 4A3D08B2
F0 "GPS" 60
F1 "gps.sch" 60
$EndSheet
$Sheet
S 10900 2000 1500 1500
U 4A3C2B62
F0 "USB" 60
F1 "usb.sch" 60
$EndSheet
$EndSCHEMATC
