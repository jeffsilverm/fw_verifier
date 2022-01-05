# fw_verifier
A tool for verifying that a firewall is working properly,

Currently, the only way to verify that a firewall is blocking everything it is supposed to is to run an application that listens on a port and then verify that it
cannot be connected to when the firewall and can be connected to when the firewall is off.  The fw_verifier listens on all ports at the same so you may verify that a port is open (or closed) without having an application running that listens to that port.

## The roadmap is:


| version | Features |
| --- | --- |
| 0.0 | Loop over, say, half a dozen ports. |
| 0.1 | Multithreading |
| 0.2 | Open ports for TCP/IP with a socket call. |
| 0.3 | CLI |
| 0.4 | Reporting mechanism |


