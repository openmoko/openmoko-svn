/*  openmoko-panel-usb.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 */
#include <libmokopanelui2/moko-panel-applet.h>

#include <sys/types.h>
#include <linux/limits.h>
#include <usb.h>

#include <gtk/gtkimage.h>
#include <time.h>

typedef struct {
    MokoPanelApplet* mpa;
    int dummy;
} UsbApplet;

static void
usb_applet_free (UsbApplet *applet)
{
    g_slice_free (UsbApplet, applet);
}

void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
    printf(" bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
    printf(" bmAttributes: %02xh\n", endpoint->bmAttributes);
    printf(" wMaxPacketSize: %d\n", endpoint->wMaxPacketSize);
    printf(" bInterval: %d\n", endpoint->bInterval);
    printf(" bRefresh: %d\n", endpoint->bRefresh);
    printf(" bSynchAddress: %d\n", endpoint->bSynchAddress);
}

void print_altsetting(struct usb_interface_descriptor *interface)
{
    int i;

    printf(" bInterfaceNumber: %d\n", interface->bInterfaceNumber);
    printf(" bAlternateSetting: %d\n", interface->bAlternateSetting);
    printf(" bNumEndpoints: %d\n", interface->bNumEndpoints);
    printf(" bInterfaceClass: %d\n", interface->bInterfaceClass);
    printf(" bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
    printf(" bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
    printf(" iInterface: %d\n", interface->iInterface);

    for (i = 0; i < interface->bNumEndpoints; i++)
        print_endpoint(&interface->endpoint[i]);
}

void print_interface(struct usb_interface *interface)
{
    int i;

    for (i = 0; i < interface->num_altsetting; i++)
        print_altsetting(&interface->altsetting[i]);
}

void print_configuration(struct usb_config_descriptor *config)
{
    int i;

    printf(" wTotalLength: %d\n", config->wTotalLength);
    printf(" bNumInterfaces: %d\n", config->bNumInterfaces);
    printf(" bConfigurationValue: %d\n", config->bConfigurationValue);
    printf(" iConfiguration: %d\n", config->iConfiguration);
    printf(" bmAttributes: %02xh\n", config->bmAttributes);
    printf(" MaxPower: %d\n", config->MaxPower);

    for (i = 0; i < config->bNumInterfaces; i++)
        print_interface(&config->interface[i]);
}

static void usb_applet_dump_usb_status()
{
    struct usb_bus *bus;
    struct usb_device *dev;
    printf("bus/device idVendor/idProduct\n");

    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            int ret, i;
            char string[256];
            usb_dev_handle *udev;

            printf("%s/%s %04X/%04X\n", bus->dirname, dev->filename,
                dev->descriptor.idVendor, dev->descriptor.idProduct);

            udev = usb_open(dev);
            if (udev) {
                if (dev->descriptor.iManufacturer) {
                    ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
                    if (ret > 0)
                        printf("- Manufacturer : %s\n", string);
                    else
                        printf("- Unable to fetch manufacturer string\n");
                }

                if (dev->descriptor.iProduct) {
                    ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
                    if (ret > 0)
                        printf("- Product : %s\n", string);
                    else
                        printf("- Unable to fetch product string\n");
                }

                if (dev->descriptor.iSerialNumber) {
                    ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
                    if (ret > 0)
                        printf("- Serial Number: %s\n", string);
                    else
                        printf("- Unable to fetch serial number string\n");
                }

                usb_close (udev);
            }

            if (!dev->config) {
                printf(" Couldn't retrieve descriptors\n");
                continue;
            }

            for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
                print_configuration(&dev->config[i]);
        }
    }
}

static void usb_applet_update_status( UsbApplet *applet )
{
    usb_init();
    int new_busses = usb_find_busses();
    g_debug( "usb_applet_update_status: %d new USBes found", new_busses );
    int new_devices = usb_find_devices();
    g_debug( "usb_applet_update_status: %d new USB devices found", new_devices );

    usb_applet_dump_usb_status();

    moko_panel_applet_set_icon( applet->mpa, PKGDATADIR "/Usb.png" );

}

G_MODULE_EXPORT GtkWidget*
mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    MokoPanelApplet* mokoapplet = moko_panel_applet_new();

    UsbApplet *applet;
    time_t t;
    struct tm *local_time;

    applet = g_slice_new( UsbApplet );
    applet->mpa = mokoapplet;

    usb_applet_update_status( applet );

    gtk_widget_show_all( GTK_WIDGET(mokoapplet) );
    return GTK_WIDGET(mokoapplet);
};
