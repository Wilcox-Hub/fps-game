# Corrections before the recorded passes

The first build attempt lacked the fixture source because its staging Tests directory did not exist. After adding it, a stale build graph omitted the new CPP; using -gather together with -NoUBTMakefiles discovered it. A local variable named Role shadowed the inherited Actor member and was renamed. Final Editor and Shipping builds succeed.

The first four-peer execution discovered/joined all peers but did not launch: relative ServerTravel retained RidgefireMenu, causing ready input to be interpreted as browser input. Hosting now travels absolutely; InitGame also gives the lobby option priority over menu state. The affected four-peer test was rerun and passes two complete cycles. Final two-peer and four-peer host-loss checks follow the subsequent menu/gamepad fixes. No failed attempt is counted as a pass.
