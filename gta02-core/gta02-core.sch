EESchema Schematic File Version 2  date Tue 14 Jul 2009 11:26:59 PM PDT
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
	12350 9650 12350 6350
Wire Notes Line
	12350 9650 15000 9650
Wire Notes Line
	15000 9650 15000 6350
Wire Notes Line
	15000 6350 12350 6350
Wire Wire Line
	13450 9300 13650 9300
Wire Wire Line
	13450 9000 13650 9000
Wire Wire Line
	13450 8550 13650 8550
Wire Wire Line
	13650 8100 13450 8100
Wire Wire Line
	13650 7800 13450 7800
Wire Wire Line
	13650 7200 13450 7200
Wire Wire Line
	13650 7350 13450 7350
Wire Wire Line
	13650 7050 12850 7050
Wire Wire Line
	13650 7500 13450 7500
Wire Wire Line
	12850 7050 12850 6900
Wire Wire Line
	13650 7950 13450 7950
Wire Wire Line
	13450 8250 13650 8250
Wire Wire Line
	13450 8700 13650 8700
Wire Wire Line
	13450 9150 13650 9150
Wire Wire Line
	13450 9450 13650 9450
Text Notes 12450 6500 0    60   ~ 0
GSM labels unused while calypso status is unresolved
NoConn ~ 13450 9450
NoConn ~ 13450 9300
NoConn ~ 13450 9150
NoConn ~ 13450 9000
Text GLabel 13650 9450 2    60   Input ~ 0
MICIN
Text GLabel 13650 9300 2    60   Input ~ 0
MICIP
Text GLabel 13650 9150 2    60   Output ~ 0
EARN
Text GLabel 13650 9000 2    60   Output ~ 0
EARP
NoConn ~ 13450 8700
NoConn ~ 13450 8550
Text GLabel 13650 8700 2    60   Input ~ 0
MODEM_ON
Text GLabel 13650 8550 2    60   Input ~ 0
MODEM_RST
NoConn ~ 13450 8250
Text GLabel 13650 8250 2    60   Input ~ 0
DL_GSM
NoConn ~ 13450 8100
NoConn ~ 13450 7950
NoConn ~ 13450 7800
Text GLabel 13650 7800 2    60   Input ~ 0
AMP_SHUT
Text GLabel 13650 7950 2    60   Input ~ 0
HP_IN
Text GLabel 13650 8100 2    60   Input ~ 0
INTO
NoConn ~ 13450 7500
NoConn ~ 13450 7350
NoConn ~ 13450 7200
$Comp
L PWR_FLAG #FLG?
U 1 1 4A5C71E1
P 12850 6900
F 0 "#FLG?" H 12850 7170 30  0001 C CNN
F 1 "PWR_FLAG" H 12850 7130 30  0000 C CNN
	1    12850 6900
	1    0    0    -1  
$EndComp
Text GLabel 13650 7350 2    60   Output ~ 0
SIM_CLK
Text GLabel 13650 7200 2    60   Output ~ 0
SIM_RST
Text GLabel 13650 7050 2    60   Output ~ 0
SIM_V
Text GLabel 13650 7500 2    60   BiDi ~ 0
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
