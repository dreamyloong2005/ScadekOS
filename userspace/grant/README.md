# ScadekOS Grant Demo

`grant.S` is loaded by SCDK's M24 grant-test service. It creates a read-only
grant over a user buffer, sends the grant capability through an endpoint
message, revokes it, and verifies the revoked access path.
