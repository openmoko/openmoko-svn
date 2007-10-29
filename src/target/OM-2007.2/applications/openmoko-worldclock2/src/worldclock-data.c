/*
 *  openmoko-worldclock -- OpenMoko Clock Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "worldclock-data.h"

/* Note, these are the same timezones as the 'main' zones in openmoko-dates2 */
 const WorldClockZoneData world_clock_tzdata[39] = {
	{ "Amsterdam", "Europe/Amsterdam", 52.367, 4.9, "The Netherlands" },
	{ "Auckland", "Pacific/Auckland", -36.85, 174.783, "New Zealand" },
	{ "Berlin", "Europe/Berlin", 52.52, 13.41, "Germany" },
	{ "Buenos Aires", "America/Argentina/Buenos_Aires", -34.667, -58.4, "Argentina" },
	{ "Cairo", "Africa/Cairo", 30.05, 31.3667, "Egypt" },
	{ "Calcutta", "Asia/Calcutta", 22.5656, 88.34667, "India" },
	{ "Chicago", "America/Chicago", 41.9, -87.65, "The United States" },
	{ "Denver", "America/Denver", 39.75, -104.9833, "The United States" },
	{ "Dublin", "Europe/Dublin", 53.34277, -6.2661, "Ireland" },
	{ "Hong Kong", "Asia/Hong_Kong", 22.2833, 114.1333, "Hong Kong" },
	{ "Honolulu", "Pacific/Honolulu", 21.30889, -157.8261, "The United States" },
	{ "Istanbul", "Europe/Istanbul", 41, 29, "Turkey" },
	{ "Jakarta", "Asia/Jakarta", -6.1833, 106.8333, "Indonesia" },
	{ "Karachi", "Asia/Karachi", 24.85, 67.01667, "Pakistan" },
	{ "Kinshasa", "Africa/Kinshasa", -4.2667, 15.2833, "The Democratic Republic of the Congo" },
	{ "Lagos", "Africa/Lagos", 6.5833, 3.3331, "Nigeria" },
	{ "Lima", "America/Lima", 12.0433, 0.02833, "Peru" },
	{ "London", "Europe/London", 51.50694, -0.1275, "The United Kingdom" },
	{ "Los Angeles", "America/Los_Angeles", 34.05, -118.25, "The United States" },
	{ "Madrid", "Europe/Madrid", 40.3833, -3.71667, "Spain" },
	{ "Manila", "Asia/Manila", 14.5833, 121, "The Philippines" },
	{ "Mexico City", "America/Mexico_City", 19.6931, -99.218611111, "Mexico" },
	{ "Montreal", "America/Montreal", 45.50889, -73.55417, "Canada" },
	{ "Moscow", "Europe/Moscow", 55.75222, 37.61556, "Russia" },
	{ "New York", "America/New_York", 43.14, -77.63944, "The United States" },
	{ "Paris", "Europe/Paris", 48.8667, 2.3331, "France" },
	{ "Rome", "Europe/Rome", 41.9, 12.4833, "Italy" },
	{ "Sao Paulo", "America/Sao_Paulo", -23.54333, -46.63306, "Brazil" },
	{ "Seoul", "Asia/Seoul", 37.521, 126.986, "South Korea" },
	{ "Shanghai", "Asia/Shanghai", 31.1667, 121.4667, "The People's Republic of China" },
	{ "Singapore", "Asia/Singapore", 1.2833, 103.85, "Singapore" },
	{ "Stockholm", "Europe/Stockholm", 59.35, 18.0667, "Sweden" },
	{ "Sydney", "Australia/Sydney", -33.86833, 151.20861, "Australia" },
	{ "Taipei", "Asia/Taipei", 25.0333, 121.6333, "The Republic of China" },
	{ "Tehran", "Asia/Tehran", 35.696189, 51.42296, "Iran" },
	{ "Tijuana", "America/Tijuana", 32.525, -117.0333, "Mexico" },
	{ "Tokyo", "Asia/Tokyo", 35.6833, 139.7667, "Japan" },
	{ "Toronto", "America/Toronto", 43.65, -79.38333, "Canada" },
	{ "Vancouver", "America/Vancouver", 49.26667, -123.1167, "Canada" },
 };
