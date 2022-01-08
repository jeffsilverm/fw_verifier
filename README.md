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
| 0.3 | Integrating multithreading with open ports 
| 0.4 | CLI |
| 0.4 | Reporting mechanism |


## Testing the fw_verifier with Linux command line tools
This is an incomplete description of what is available.
I was trying to look at just 3 ports, 2048,2049, and 2050.  2049 is in use. 2048 and 2050 normally are free.
### netcat or nc
nc ADDRESS PORT
where 

### tcpdump
<kbd>tcpdump -n tcp portrange 2048-2050</kbd>
<kbd>tcpdump -n tcp portrange 2048-2050 and host HOST</kbd>

note: tcpdump might not work as you would think if the client and the server are on the same machine.

#### Failure mode: a firewall has blocked this port.
If you see TCP inbound packets coming with the SYN bit [S] set, but nothing coming out, then this particular
port is blocked by a firewall.

jeffs@jeffs-desktop:~/fw_verifier$ <kbd>sudo tcpdump -n tcp portrange 2048-2050 and host 192.168.0.149</kbd><samp>
[sudo] password for jeffs: 
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on enp5s0, link-type EN10MB (Ethernet), capture size 262144 bytes
00:47:58.756283 IP 192.168.0.159.55174 > 192.168.0.149.2049: Flags [S], seq 1698982370, win 64240, options [mss 1460,sackOK,TS val 1890351736 ecr 0,nop,wscale 7], length 0
00:47:58.756358 IP 192.168.0.159.41086 > 192.168.0.149.2048: Flags [S], seq 2847899385, win 64240, options [mss 1460,sackOK,TS val 1890351736 ecr 0,nop,wscale 7], length 0
00:47:58.756373 IP 192.168.0.159.38894 > 192.168.0.149.2050: Flags [S], seq 1526558738, win 64240, options [mss 1460,sackOK,TS val 1890351736 ecr 0,nop,wscale 7], length 0
00:48:00.757988 IP 192.168.0.159.38896 > 192.168.0.149.2050: Flags [S], seq 2376835721, win 64240, options [mss 1460,sackOK,TS val 1890353738 ecr 0,nop,wscale 7], length 0
00:48:00.758087 IP 192.168.0.159.41092 > 192.168.0.149.2048: Flags [S], seq 3528916545, win 64240, options [mss 1460,sackOK,TS val 1890353738 ecr 0,nop,wscale 7], length 0
00:48:00.758186 IP 192.168.0.159.55184 > 192.168.0.149.2049: Flags [S], seq 2603460365, win 64240, options [mss 1460,sackOK,TS val 1890353738 ecr 0,nop,wscale 7], length 0
</samp>

#### Failure mode: nothing is listening on that port.




### lsof
What processes own which ports?
lsof -i 2048-2050

### nmap
nmap -sT ADDRESS -p 2048-2050

