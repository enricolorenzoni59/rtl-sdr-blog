# Experimental 1090 MHz relative gain profiles

This branch adds opt-in metadata and a nearest-gain selector for **two measured
receivers**, not a universal V3/V4L calibration. Existing gain APIs and tuner
register programming are unchanged. No application automatically uses these
profiles. No sensitivity, noise figure or absolute input-power improvement has
been established.

## Measurement scope and provenance

Measured 2026-09-25: user-identified V3 (`Realtek` / `RTL2838UHIDIR`, serial
`00000002`) and V4L (`RTLSDRBlog` / `Blog V4L`, serial `00000001`). Serial numbers
are descriptive, not unique identifiers or a reliable model detector.

An ANTSDR E200 supplied a conducted CW signal through 16 dB attenuation and a
splitter whose ports the owner measured within 0.2 dB. Receivers were centered
at 1090 MHz; the tone was approximately +250 kHz from center. Settings: 2.4 MS/s,
requested 2 MHz bandwidth, manual combined LNA/mixer gain, fixed VGA index 8,
RTL digital AGC off, direct sampling off. No ports were swapped.

The estimator averaged 16 Hann-windowed 16384-point FFTs, integrated 13 tone
bins and subtracted the adjacent noise estimate. Accepted observations required
at least 20 dB tone-to-noise ratio in the integration band, no endpoint clipping
and tone power below -10 dBFS. Multiple source levels and low-gain confirmation
provided at least nine accepted observations per gain and unit. The largest
within-unit repeat range at 1090 MHz was approximately 0.215 dB. Hundredth-dB
storage is numerical resolution, not a claim of 0.01 dB accuracy.

`1090mhz.csv` preserves the full-precision summary, repeat ranges and counts.
Profiles subtract each unit's nominal-zero measurement and round to 0.01 dB.
Zero therefore means a relative reference, **not zero physical RF gain**.
Measurements at other frequencies differed; do not extrapolate this profile.

Campaign source snapshot: stream1090 commit
`908baaf9c85b8edda828e7922226ab97bc4aa3ca`, vendored `thirdparty/rtl-sdr-blog`.
The measurement build only changed forced bias-tee handling for the conducted
setup. Original experiment archive commit:
`e819c34` in the private firmware work repository. Raw captures are not included
here; this is a summary dataset, not an independently reproducible raw archive.
`expected-register-map.json` records source-derived register indices, not
hardware register readback. Combined-gain programming and fixed VGA in this
branch remain the same as the measurement source.

The V3's 48.0/49.6 nominal settings measured 47.48/47.45 dB relative gain.
That difference is within repeat variation. We preserve it rather than force
monotonicity; the selector consequently chooses nominal 48.0 for requests above
both values. Duplicates at nominal 43.9/44.5 select 43.9. These decisions minimize
measured distance, not noise figure or overload risk.

## API and application integration

The API is deliberately independent of a device handle: it accesses immutable
measurement data, does not validate hardware/configuration and does not write
registers. Applications must explicitly select a suitable profile and stop using
it after frequency, mode, bandwidth or stage configuration changes that leave
the measurement conditions. For other units, collect a new calibration.

```c
rtlsdr_gain_calibration_t point;
int rc = rtlsdr_select_calibrated_gain(
    RTLSDR_GAIN_PROFILE_V4L_00000001_1090, 4000, &point); /* relative 40 dB */
if (rc == 0) {
    rc = rtlsdr_set_tuner_gain_mode(dev, 1);
    if (rc == 0)
        rc = rtlsdr_set_tuner_gain(dev, point.nominal_gain_tenth_db);
}
/* Check rc. Configure the other measurement conditions separately. */
```

`rtlsdr_get_gain_calibration(profile, NULL, 0)` returns the required entry count.
Supply an array and its capacity to copy all entries in nominal order. Legacy
`rtlsdr_get_tuner_gains()` / `rtlsdr_get_tuner_gain()` still return nominal tenths
of a dB; never pass the measured hundredths directly to the legacy setter.
Stream1090 could use differences between measured points to estimate the effect
of a proposed gain change. That application integration is a separate change.

## Build and test

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The tests need no receivers. They cover all nominal entries, bounded copies,
errors without writes, duplicate/nonmonotonic selection, integer extremes and
an exhaustive request grid for both profiles. They test software behavior;
RF performance remains supported by the original experiment only.

Validation on 2026-09-25: macOS Apple silicon, AppleClang 21, libusb 1.0.30.
Full CMake shared/static library and utility build passed using the repository
CI warning flags (`-Wall -Wextra -Wno-unused-parameter -Wno-unused
-Wsign-compare -Wdeclaration-after-statement -Werror`); CTest passed. The new
module and tests also passed AddressSanitizer/UndefinedBehaviorSanitizer with
`-Wall -Wextra -Werror`, without unused-warning exclusions. All 58 entries read
through the built shared library matched the CSV after rounding. Source
comparison confirmed unchanged legacy API/tuner files and identical combined
gain/VGA functions to the measurement driver. No new RF run, Linux build or
Autotools build was performed for this branch.
