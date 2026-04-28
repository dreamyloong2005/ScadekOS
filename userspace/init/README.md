# ScadekOS `/init`

This directory contains the ScadekOS-owned initial flat user program.

For `0.1.0-devpreview.4`, `/init` receives the SCDK proc endpoint as its
bootstrap capability and requests startup of `/runner`, `/bin/hello`,
`/ring-test`, and `/prompt` through `libscadek` endpoint/message calls.
