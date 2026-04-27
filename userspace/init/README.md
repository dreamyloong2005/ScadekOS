# ScadekOS `/init`

This directory contains the ScadekOS-owned initial flat user program.

For `0.1.0-dev.2`, `/init` receives the SCDK proc endpoint as its bootstrap
capability and requests startup of `/hello` and `/ring-test` through
SCDK-native endpoint/message calls.
