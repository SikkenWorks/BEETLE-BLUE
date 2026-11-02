# Packaging reference

`packaging-base.fwsc` is a read-only historical RC12 firmware package required by the existing FM1 firmware packager to preserve the verified header and OTA bytes. **It is NOT the V1.1 release.** The current downloadable firmware is `../BEETLE-BLUE-V1.1.fwsc`. Never publish the packaging reference as V1.1. Do not change the inherited `uboot.boot` metadata without separate verification.
