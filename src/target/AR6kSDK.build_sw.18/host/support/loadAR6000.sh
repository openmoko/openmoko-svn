#!/bin/sh
# Loads and unloads the driver modules in the proper sequence
Help() {
    echo "Usage: $0 [options]"
    echo
    echo "With NO options, perform a default driver loading"
    echo
    echo "To bypass WMI: "
    echo "   $0 1"
    echo "To run Test commands:"
    echo "   $0 -t or $ --test"
    echo "To recover Flash using ROM BSP,ie,force target boot from ROM:"
    echo "   $0 -r or $0 --rom"
    echo" To enable console print:"
    echo "   $0 --enableuartprint"
    echo" To disable console print:"
    echo "   $0 --disableuartprint"
    echo" To specify the log file name:"
    echo "   $0 -l <log.file> or $0 --log <log.file>"
    echo "To see this help information:"
    echo "   $0 --help or $0 -h "
    echo "To unload all of the drivers:"
    echo "   $0 unloadall"
	exit 0
}
if [ -z "$WORKAREA" ]
then
	echo "Please set your WORKAREA environment variable."
	exit -1
fi
if [ -z "$ATH_PLATFORM" ]
then
	echo "Please set your ATH_PLATFORM environment variable."
	exit -1
fi

fromrom=0
slowbus=0
tmode=0
dounload=""
ar6000args=""
busclocksetting=""
bmi_enable=""
logfile="/tmp/dbglog.out"
NETIF=${NETIF:-eth1}
START_WLAN="TRUE"

echo "Platform set to $ATH_PLATFORM"

while [ "$#" -ne 0 ]
do
	case $1 in
		-t|--test )
        tmode=1
        ar6000args="$ar6000args testmode=1"
        shift
        ;;
        -r|--rom )
        fromrom=1
        slowbus=1
        ar6000args="$ar6000args skipflash=1"
        shift
        ;;
        -l|--log )
        shift
        logfile="$1"
        shift
        ;;
        -h|--help )
        Help
        ;;
        bmi)
	bmi_enable="yes"
        shift
        ;;
        bypasswmi)
        ar6000args="$ar6000args bypasswmi=1"
        shift
        ;;
        noresetok)
        ar6000args="$ar6000args resetok=0"
        shift
        ;;
        specialmode)
        ar6000args="$ar6000args bypasswmi=1, resetok=0"
	bmi_enable="yes"
        shift
        ;;
        unloadall)
        dounload="yes"
        shift
        ;;
        enableuartprint)
        ar6000args="$ar6000args enableuartprint=1"
        shift
        ;;
        disableuartprint)
        ar6000args="$ar6000args enableuartprint=0"
        shift
        ;;
        logwmimsgs)
        ar6000args="$ar6000args logWmiRawMsgs=1"
        shift
        ;;
        --nostart|-nostart|nostart)
        START_WLAN="FALSE"
        shift
        ;;
        * )
            echo "Unsupported argument"
            exit -1
        shift
    esac
done

if [ "$dounload" = "yes" ]
then
	echo "..unloading all"
	case $ATH_PLATFORM in
		LOCAL_i686-SDIO)
		/sbin/rmmod -w ar6000.ko
		/sbin/rmmod -w sdio_pcistd_hcd.ko
		/sbin/rmmod -w sdio_busdriver.ko
		/sbin/rmmod -w sdio_lib.ko
		killall recEvent
		;;
		LOCAL_i686-CF)
		/sbin/rmmod -w ar6000.ko
		/sbin/cardctl eject
		/sbin/rmmod -w ar6000_cs.ko
		/sbin/cardctl insert
		;;
		SANDGATEII_ARM-SDIO)
		/sbin/rmmod ar6000.ko
		/sbin/rmmod sdio_pxa270hcd.ko
		/sbin/rmmod sdio_busdriver.ko
		/sbin/rmmod sdio_lib.ko
		;;
		SANDGATEII_ARM-CF)
		/sbin/rmmod ar6000.ko
		/sbin/cardctl eject
		/sbin/rmmod ar6000_cs.ko
		/sbin/cardctl insert
		;;
		SGDK020_2-MSIO)
		/sbin/rmmod -w ar6000
		/sbin/rmmod -w mshost_drv
		;;
		OMAP2420-SPI2)
		/sbin/rmmod ar6000
		/sbin/rmmod athspi_omap2420_hcd
		/sbin/rmmod sdio_busdriver
		/sbin/rmmod sdio_lib
		;;
		OMAP2420-SPI)
		/sbin/rmmod ar6000
		/sbin/rmmod sdio_omap_raw_spi_hcd
		/sbin/rmmod sdio_busdriver
		/sbin/rmmod sdio_lib
		;;
		OMAP5912-SPI)
		/sbin/rmmod ar6000
		/sbin/rmmod sdio_omap_raw_spi_hcd
		/sbin/rmmod sdio_busdriver
		/sbin/rmmod sdio_lib
		;;
		OMAP2420-SDIO)
		/sbin/rmmod ar6000
		/sbin/rmmod sdio_omap_hcd
		/sbin/rmmod sdio_busdriver
		/sbin/rmmod sdio_lib		
		;;
		*)
		echo "Check your ATH_PLATFORM environment variable"
	esac

	exit 0
fi

# For AR6002 FPGA rather than silicon, uncomment these lines:
# wlanapp=${wlanapp:-$WORKAREA/target/AR6002/fpga/bin/athwlan.bin}
# busclocksetting="DefaultOperClock=12000000"
# slowbus=1

export IMAGEPATH=$WORKAREA/host/.output/$ATH_PLATFORM/image

case $ATH_PLATFORM in
	OMAP5912-SPI)
	if [ "$fromrom" -eq "1" ];then 
		/sbin/insmod $IMAGEPATH/ForceROM.o 
		/sbin/rmmod ForceROM
	fi
	/sbin/insmod $IMAGEPATH/sdio_lib.o debuglevel=0
	/sbin/insmod $IMAGEPATH/sdio_busdriver.o RequestListSize=300 debuglevel=0
	/sbin/insmod $IMAGEPATH/sdio_omap_raw_spi_hcd.o debuglevel=1 gpiodebug=0 powerupdelay=600
	;;
	OMAP2420-SPI)
	if [ "$fromrom" -eq "1" ];then 
		/sbin/insmod $IMAGEPATH/ForceROM.o
		/sbin/rmmod ForceROM
	fi
	/sbin/insmod $IMAGEPATH/sdio_lib.o debuglevel=0
	/sbin/insmod $IMAGEPATH/sdio_busdriver.o RequestListSize=300 debuglevel=0 HcdRCount=5
	if [ "$fromrom" -eq "1" ];then
		/sbin/insmod $IMAGEPATH/sdio_omap_raw_spi_hcd.o debuglevel=1 op_clock=12000000 gpiodebug=0
	else
		/sbin/insmod $IMAGEPATH/sdio_omap_raw_spi_hcd.o debuglevel=1 op_clock=24000000 gpiodebug=0 powerupdelay=600
	fi
	;;
	LOCAL_i686-SDIO)
        $IMAGEPATH/recEvent $logfile > /dev/null 2>&1 &
	/sbin/insmod $IMAGEPATH/sdio_lib.ko
	/sbin/insmod $IMAGEPATH/sdio_busdriver.ko debuglevel=7 RequestListSize=300 $busclocksetting
	/sbin/insmod $IMAGEPATH/sdio_pcistd_hcd.ko debuglevel=7
	;;
	LOCAL_i686-CF)
	/sbin/insmod $IMAGEPATH/ar6000_cs.ko
	/sbin/cardctl eject
	/sbin/cardctl insert
	sleep 2
	;;
	SANDGATEII_ARM-SDIO)
	/sbin/insmod $IMAGEPATH/sdio_lib.ko
	/sbin/insmod $IMAGEPATH/sdio_busdriver.ko $busclocksetting
	/sbin/insmod $IMAGEPATH/sdio_pxa270hcd.ko
	;;
	SANDGATEII_ARM-CF)
	/sbin/insmod $IMAGEPATH/ar6000_cs.ko
	/sbin/cardctl eject
	/sbin/cardctl insert
	sleep 2
	;;
	SGDK020_2-MSIO)
	/sbin/insmod $IMAGEPATH/mshost_drv.ko hostsel=1
	;;
	OMAP2420-SPI2)
	/sbin/insmod $IMAGEPATH/sdio_lib.o
	/sbin/insmod $IMAGEPATH/sdio_busdriver.o debuglevel=7 RequestListSize=300
	/sbin/insmod $IMAGEPATH/athspi_omap2420_hcd.o debuglevel=7 op_clock=12000000
	;;
	OMAP2420-SDIO)
	echo $IMAGEPATH
	/sbin/insmod $IMAGEPATH/sdio_lib.o
	/sbin/insmod $IMAGEPATH/sdio_busdriver.o debuglevel=7 RequestListSize=300 $busclocksetting
	/sbin/insmod $IMAGEPATH/sdio_omap_hcd.o debuglevel=7 builtin_card=1 async_irq=1
	;;
	*)
	echo "Check your ATH_PLATFORM environment variable"
	exit -1
esac

case $ATH_PLATFORM in
	LOCAL_i686-SDIO|LOCAL_i686-CF|SANDGATEII_ARM-SDIO|SANDGATEII_ARM-CF|SGDK020_2-MSIO)
	/sbin/insmod $IMAGEPATH/ar6000.ko bmienable=1 $ar6000args busspeedlow=$slowbus
	;;
	OMAP2420-SPI2|OMAP2420-SDIO|OMAP2420-SPI|OMAP5912-SPI)
	/sbin/insmod $IMAGEPATH/ar6000.o bmienable=1 $ar6000args busspeedlow=$slowbus
	;;
	*)
	echo "UNKNOWN PLATFORM"
esac

# At this point, all Host-side software is loaded.
# So we turn our attention to Target-side setup.


# If user requested BMI mode, he will handle everything himself.
# Otherwise, we load the Target firmware, load EEPROM and exit
# BMI mode.
exit_value=0
if [ "$bmi_enable" != "yes" ]
then

    # For speed, temporarily disable BMI debugging
    save_dbgbmi=`cat /sys/module/ar6000/debugbmi`
    echo 0 > /sys/module/ar6000/debugbmi

    eval export `$IMAGEPATH/bmiloader -i $NETIF --info | grep TARGET_TYPE`
    # echo TARGET TYPE is $TARGET_TYPE


    # TBDXXX: Eventually remove WAR
    if [ "$TARGET_TYPE" = "AR6002" ]
    then
	$WORKAREA/host/support/AR6002.war1.sh > /dev/null
    fi

    # ....and temporarily disable System Sleep on AR6002
    if [ "$TARGET_TYPE" = "AR6002" ]
    then
        old_sleep=`$IMAGEPATH/bmiloader -i $NETIF --get --address=0x40c4 | tail -1 | sed -e 's/.*: //'`
        new_sleep=$(($old_sleep | 1))
        $IMAGEPATH/bmiloader -i $NETIF --set --address=0x40c4 --param=$new_sleep > /dev/null
    fi

    # If no ROM Patch Distribution is specified but an
    # RPDF file is present, try to use it by default.
    if [ "$RPDF_FILE" = "" ]
    then
        if [ -r $WORKAREA/target/$TARGET_TYPE/patch.rpdf ]
        then
            export RPDF_FILE=$WORKAREA/target/$TARGET_TYPE/patch.rpdf
        fi
    fi

    if [ "$RPDF_FILE" != "" ]
    then
        echo "Install ROM Patch Distribution File, " $RPDF_FILE
        $IMAGEPATH/fwpatch --interface=$NETIF --file=$RPDF_FILE
        if [ $? -ne 0 ]
        then
            echo "Failed to load ROM Patch Distribution"
            exit_value=-1
        fi
    fi

    if [ "$EEPROM" != "" ]
    then
        echo "Load Board Data from $EEPROM"
        $IMAGEPATH/eeprom.$TARGET_TYPE --transfer --file=$EEPROM --interface=$NETIF
        if [ $? -ne 0 ]
        then
            echo "Failed to load Board Data from file."
            exit_value=-1
        fi
    else
        # Transfer Board Data from Target EEPROM to Target RAM
        # If EEPROM does not appear valid, this has no effect.
        $IMAGEPATH/eeprom.$TARGET_TYPE --transfer --interface=$NETIF 2> /dev/null

        # On AR6001, a failure to load from EEPROM is not fatal
        # because Board Data may be in flash.
        # On a standard driver load of AR6002, lack of Board
        # Data is fatal.
        if [ \( $? -ne 0 \) -a \( "$TARGET_TYPE" = "AR6002" \) ]
        then
            echo "Failed to load Board Data from EEPROM."
            exit_value=-1
        fi
    fi

    # For AR6002, download athwlan.bin to Target RAM.
    # For AR6001, athwlan is already in ROM or Flash.
    if [ "$TARGET_TYPE" = "AR6002" ]
    then
        # Download the Target application, usually athwlan.bin,
	# into Target RAM.

	wlanapp=${wlanapp:-$WORKAREA/target/AR6002/hw/bin/athwlan.bin}
        $IMAGEPATH/bmiloader -i $NETIF --write --address=0x502000 --file=$wlanapp
    fi

    if [ "$TARGET_TYPE" = "AR6001" ]
    then
        # Request maximum system performance (max clock rates)
        $IMAGEPATH/bmiloader -i $NETIF --set --address=0xac000020 --param=0x203 > /dev/null
        $IMAGEPATH/bmiloader -i $NETIF --set --address=0xac000024 --param=0x203 > /dev/null
    fi
    
    # Restore System Sleep on AR6002
    if [ "$TARGET_TYPE" = "AR6002" ]
    then
        $IMAGEPATH/bmiloader -i $NETIF --set --address=0x40c4 --param=$old_sleep > /dev/null
    fi

    # Restore BMI debug state.
    echo $save_dbgbmi > /sys/module/ar6000/debugbmi

    if [ \( $exit_value -eq 0 \) -a \( $START_WLAN = TRUE \) ]
    then
        # Leave BMI now and start the WLAN driver.
        $IMAGEPATH/bmiloader -i $NETIF --done
    fi
fi

exit $exit_value
