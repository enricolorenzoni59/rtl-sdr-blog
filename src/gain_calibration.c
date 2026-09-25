/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Individual-unit measurements, 2026-09-25; see calibration/README.md. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rtl-sdr.h"

static const rtlsdr_gain_calibration_t v3[] = {
	{ 0, 0 },
	{ 9, 319 },
	{ 14, 450 },
	{ 27, 839 },
	{ 37, 1027 },
	{ 77, 1390 },
	{ 87, 1558 },
	{ 125, 1836 },
	{ 144, 1978 },
	{ 157, 2144 },
	{ 166, 2283 },
	{ 197, 2569 },
	{ 207, 2715 },
	{ 229, 3004 },
	{ 254, 3149 },
	{ 280, 3458 },
	{ 297, 3601 },
	{ 328, 3786 },
	{ 338, 3931 },
	{ 364, 3976 },
	{ 372, 4125 },
	{ 386, 4178 },
	{ 402, 4313 },
	{ 421, 4385 },
	{ 434, 4513 },
	{ 439, 4584 },
	{ 445, 4584 },
	{ 480, 4748 },
	{ 496, 4745 },
};

static const rtlsdr_gain_calibration_t v4l[] = {
	{ 0, 0 },
	{ 9, 181 },
	{ 14, 315 },
	{ 27, 511 },
	{ 37, 698 },
	{ 77, 874 },
	{ 87, 1041 },
	{ 125, 1366 },
	{ 144, 1509 },
	{ 157, 1872 },
	{ 166, 2011 },
	{ 197, 2477 },
	{ 207, 2625 },
	{ 229, 2815 },
	{ 254, 2962 },
	{ 280, 3198 },
	{ 297, 3343 },
	{ 328, 3562 },
	{ 338, 3709 },
	{ 364, 3905 },
	{ 372, 4053 },
	{ 386, 4147 },
	{ 402, 4284 },
	{ 421, 4461 },
	{ 434, 4591 },
	{ 439, 4635 },
	{ 445, 4635 },
	{ 480, 4881 },
	{ 496, 4994 },
};

static const rtlsdr_gain_calibration_t *get_profile(enum rtlsdr_gain_profile profile)
{
 switch (profile) {
 case RTLSDR_GAIN_PROFILE_V3_00000002_1090: return v3;
 case RTLSDR_GAIN_PROFILE_V4L_00000001_1090: return v4l;
 default: return NULL;
 }
}

int rtlsdr_get_gain_calibration(enum rtlsdr_gain_profile profile,
 rtlsdr_gain_calibration_t *entries, uint32_t capacity)
{
 const rtlsdr_gain_calibration_t *table = get_profile(profile);
 const uint32_t count = (uint32_t)(sizeof(v3) / sizeof(v3[0]));
 if (!table || (!entries && capacity)) return -1;
 if (!entries) return (int)count;
 if (capacity < count) return -2;
 memcpy(entries, table, count * sizeof(*entries));
 return (int)count;
}

int rtlsdr_select_calibrated_gain(enum rtlsdr_gain_profile profile,
 int relative_gain_hundredth_db, rtlsdr_gain_calibration_t *selected)
{
 const rtlsdr_gain_calibration_t *table = get_profile(profile);
 size_t i, best = 0;
 int64_t distance, best_distance = INT64_MAX;
 if (!table || !selected) return -1;
 for (i = 0; i < sizeof(v3) / sizeof(v3[0]); ++i) {
  distance = (int64_t)table[i].relative_gain_hundredth_db -
   (int64_t)relative_gain_hundredth_db;
  if (distance < 0) distance = -distance;
  if (distance < best_distance) {
   best = i;
   best_distance = distance;
  }
 }
 *selected = table[best];
 return 0;
}
