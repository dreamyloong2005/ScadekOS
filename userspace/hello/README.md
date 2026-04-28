# ScadekOS `/hello`

This directory contains the small ScadekOS-owned console smoke-test program.

For `0.1.0-devpreview.4`, `/hello` receives the SCDK console endpoint as its
bootstrap capability, writes the ScadekOS version and boot marker through a
grant-backed `libscadek` console call, then exits.
