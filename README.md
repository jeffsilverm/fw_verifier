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


## Testing the fw_verifier with Linux command line tools: firewall off (passes traffic)
This is an incomplete description of what is available.  Ther literature on how to diagnose network
troubles using linux tools is vast, but see, for example [LinuxFest Northwest 2017: Troubleshooting network problems in local area networks](https://www.youtube.com/watch?v=bmOjugZ01-o).

For this test, look at just 5 ports, 2048,2049, 2050, 2051 and 2052.  Port 2048 is free, there is nothing listening to, and for this test, nothing is going to use it.  2049 is in use (but I am not sure by what.  lsof doesn't say.
It is the port for NFS so I speculate that the kernel grabbed it). For this test, I grabbed port 2050 with the netcat command `nc -l -k 2050`.  This makes it unavailable for the `fw_verifier`. So the fw_verifier 
will wind up grabbing ports 2050 and 2051.

### Start the firewall verifier
I start the `fw_verifier` with the command
```
jeffs@jeffs-desktop:~/software/fw_verifier$ ./fw_verifier 2048 2051
Created thread 0
Created thread 1
Binding to port 2048
Created thread 2
Binding to port 2050
Binding to port 2049
Created thread 3
Waiting for you to press enter
bind failed on port 2049: Address already in use
bind failed on port 2050: Address already in use
Binding to port 2051
```
(In this example, there is a race condition between the message that the `fw_verifier` is waiting
for you to push the return key and some of the status message).

The `fw_verifier` has bound to ports 2048 and 2051.  It couldn't bind to port 2049 because something
(what?) bound to it already.  It couldn't bind to port 2050 because `netcat` (`nc`) bound to it already.

### lsof
What processes own which ports?  `lsof` tells us.

```
jeffs@jeffs-desktop:~$ <kbd>sudo lsof -i :2047-2052</kbd><samp>
COMMAND     PID  USER   FD   TYPE   DEVICE SIZE/OFF NODE NAME
nc        77545 jeffs    3u  IPv4 15511411      0t0  TCP *:2050 (LISTEN)
fw_verifi 79718 jeffs    3u  IPv6 15780502      0t0  TCP *:2048 (LISTEN)
fw_verifi 79718 jeffs    7u  IPv6 15777698      0t0  TCP *:2051 (LISTEN)</samp>
jeffs@jeffs-desktop:~$ 
```

The `nc` (`netcat`) command is listening to port 2050.  The `fw_verifier` is listening
to ports 2048 and 2051.  There is nothing listening to ports 2047 or 2052.  When I 
ran the `fw_verifier` above, it tried and failed to listen to port 2049.  My hypothesis is
the kernel, which is where the Network File Services (NFS) daemon lives these days, has
grabbed port 2049 but since the kernel isn't a process, `lsof` doesn't know about it.


### tcpdump
```
<kbd>tcpdump -n tcp portrange 2047-2052</kbd>
<kbd>tcpdump -n tcp portrange 2048-2050 and host <var>HOST</var></kbd>
```

note: tcpdump might not work as you would think if the client and the server are on the same machine.



### nmap

`nmap` is a great tool for scanning a machine, looking for vulnerabilities.

```
pi@raspberrypi:~ $ <kbd>sudo nmap -O -sT 192.168.0.149 -p 2047-2052</kbd><samp>
pi@raspberrypi:~ $ sudo nmap -O -sT 192.168.0.149 -p 2047-2052
Starting Nmap 7.70 ( https://nmap.org ) at 2022-01-09 22:52 PST
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
WARNING: RST from 192.168.0.149 port 2048 -- is this port really open?
Nmap scan report for jeffs-desktop (192.168.0.149)
Host is up (0.00051s latency).

PORT     STATE  SE</samp>
pi@raspberrypi:~ $ 
```
s-monitor
2049/tcp open   nfs
2050/tcp open   av-emb-config
2051/tcp open   epnsdp
2052/tcp closed clearvisn
MAC Address: BC:5F:F4:21:AA:60 (ASRock Incorporation)
Device type: general purpose
Running: Linux 2.4.X
OS CPE: cpe:/o:linux:linux_kernel:2.4.21
OS details: Linux 2.4.21
Network Distance: 1 hop
</samp>
pi@raspberrypi:~ $ 
```

### tcpdump
`tcpdump` is a (very) low level network analysis program that you will never need to use - until you do.
This example only begins to show off what `tcpdump` can do.  In the interests of brevity, this example
has been edited a little:

```
root@jeffs-desktop:~# <kbd>tcpdump -n tcp portrange 2047-2052</kbd><samp>
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on enp5s0, link-type EN10MB (Ethernet), capture size 262144 bytes
23:01:13.408989 IP 192.168.0.159.49176 > 192.168.0.149.2047: Flags [S], seq 283256682, win 64240, options [mss 1460,sackOK,TS val 2056746390 ecr 0,nop,wscale 7], length 0
23:01:13.409063 IP 192.168.0.149.2047 > 192.168.0.159.49176: Flags [R.], seq 0, ack 283256683, win 0, length 0
23:01:13.409071 IP 192.168.0.159.55554 > 192.168.0.149.2049: Flags [S], seq 3535609932, win 64240, options [mss 1460,sackOK,TS val 2056746390 ecr 0,nop,wscale 7], length 0
23:01:13.409098 IP 192.168.0.149.2049 > 192.168.0.159.55554: Flags [S.], seq 1656183225, ack 3535609933, win 65160, options [mss 1460,sackOK,TS val 1081809914 ecr 2056746390,nop,wscale 7], length 0
23:01:13.409102 IP 192.168.0.159.58162 > 192.168.0.149.2052: Flags [S], seq 661879440, win 64240, options [mss 1460,sackOK,TS val 2056746390 ecr 0,nop,wscale 7], length 0
23:01:13.409115 IP 192.168.0.149.2052 > 192.168.0.159.58162: Flags [R.], seq 0, ack 661879441, win 0, length 0
23:01:13.409118 IP 192.168.0.159.46598 > 192.168.0.149.2051: Flags [S], seq 3412612866, win 64240, options [mss 1460,sackOK,TS val 2056746390 ecr 0,nop,wscale 7], length 0
23:01:13.409129 IP 192.168.0.149.2051 > 192.168.0.159.46598: Flags [R.], seq 0, ack 3412612867, win 0, length 0
23:01:13.409132 IP 192.168.0.159.41470 > 192.168.0.149.2048: Flags [S], seq 1039994221, win 64240, options [mss 1460,sachttps://datatracker.ietf.org/doc/html/rfc793#section-3.546 IP 192.168.0.159.39278 > 192.168.0.149.2050: Flags [S], seq 768467549, win 64240, options [mss 1460,sackOK,TS val 2056746390 ecr 0,nop,wscale 7], length 0
23:01:13.409159 IP 192.168.0.149.2050 > 192.168.0.159.39278: Flags [S.], seq 3883553031, ack 768467550, win 65160, options [mss 1460,sackOK,TS val 1081809914 ecr 2056746390,nop,wscale 7], length 0
23:01:13.409322 IP 192.168.0.159.55554 > 192.168.0.149.2049: Flags [.], ack 1, win 502, options [nop,nop,TS val 2056746390 ecr 1081809914], length 0
23:01:13.409373 IP 192.168.0.159.55554 > 192.168.0.149.2049: Flags [R.], seq 1, ack 1, win 502, options [nop,nop,TS val 20https://datatracker.ietf.org/doc/html/rfc793#section-3.545 IP 192.168.0.159.39278 > 192.168.0.149.2050: Flags [R.], seq 1, ack 1, win 502, options [nop,nop,TS val 2056746391 ecr 1081809914], length 0
23:01:13.509616 IP 192.168.0.159.56304 > 192.168.0.149.2049: Flags [S], seq 74326304, win 1, options [wscale 10,nop,mss 1460,TS val 4294967295 ecr 0,sackOK], length 0
23:01:13.509698 IP 192.168.0.149.2049 > 192.168.0.159.56304: Flags [S.], seq 2903787406, ack 74326305, win 65160, options [mss 1460,sackOK,TS val 1081810014 ecr 4294967295,nop,wscale 7], length 0
23:01:13.509902 IP 192.168.0.159.56304 > 192.168.0.149.2049: Flags [R], seq 74326305, win 0, length 0
23:01:13.609733 IP 192.168.0.159.56305 > 192.168.0.149.2049: Flags [S], seq 74326305, win 63, options [mss 1400,wscale 0,sackOK,TS val 4294967295 ecr 0,eol], length 0
23:01:13.609816 IP 192.168.0.149.2049 > 192.168.0.159.56305: Flags [S.], seq 2381009888, ack 74326306, win 65160, options [mss 1460,sackOK,TS val 1081810115 ecr 4294967295,nop,wscale 7], length 0
23:01:13.610005 IP 192.168.0.159.56305 > 192.168.0.149.2049: Flags [R], seq 74326306, win 0, length 0
23:01:13.709859 IP 192.168.0.159.56306 > 192.168.0.149.2049: Flags [S], seq 74326306, win 4, options [TS val 4294967295 ecrhttps://datatracker.ietf.org/doc/html/rfc793#section-3.51 IP 192.168.0.159.56306 > 192.168.0.149.2049: Flags [R], seq 74326307, win 0, length 0
23:01:13.810174 IP 192.168.0.159.56307 > 192.168.0.149.2049: Flags [S], seq 74326307, win 4, options [sackOK,TS val 4294967295 ecr 0,wscale 10,eol], length 0
23:01:13.810261 IP 192.168.0.149.2049 > 192.168.0.159.56307: Flags [S.], seq 3000693987, ack 74326308, win 65160, options [mss 1460,sackOK,TS val 1081810315 ecr 4294967295,nop,wscale 7], length 0
23:01:13.810508 IP 192.168.0.159.56307 > 192.168.0.149.2049: Flags [R], seq 74326308, win 0, length 0
23:01:13.910314 IP 192.168.0.159.56308 > 192.168.0.149.2049: Flags [S], seq 74326308, win 16, options [mss 536,sackOK,TS val 4294967295 ecr 0,wscale 10,eol], length 0
23:01:13.910395 IP 192.168.0.149.2049 > 192.168.0.159.56308: Flags [S.], seq 3874296099, ack 74326309, win 65160, options [mss 1460,sackOK,TS val 1081810415 ecr 4294967295,nop,wscale 7], length 0
23:01:13.910651 IP 192.168.0.159.56308 > 192.168.0.149.2049: Flags [R], seq 74326309, win 0, length 0
23:01:14.010500 IP 192.168.0.159.56309 > 192.168.0.149.2049: Flags [S], seq 74326309, win 512, options [mss 265,sackOK,TS val 4294967295 ecr 0], length 0
23:01:14.010583 IP 192.168.0.149.2049 > 192.168.0.159.56309: Flags [S.], seq 1475481086, ack 74326310, win 65160, options [mss 1460,sackOK,TS val 1081810515 ecr 4294967295], length 0
23:01:14.010838 IP 192.168.0.159.56309 > 192.168.0.149.2049: Flags [R], seq 74326310, win 0, length 0
...
</samp>
```

One could (and people have) written thick books on tcpdump and even thicker books on networks that use
tcpdump.  This capture shows what is happening at the TCP level, which is documented in
[RFC 793](https://datatracker.ietf.org/doc/html/rfc793) and [RFC 1122](https://datatracker.ietf.org/doc/html/rfc1122).  [Full disclosure: these are incredibly dull and boring documents.  I recomend that you
do not read them - until you have to.]

Each line begins with a time stamp in local time, precise to a microsecond. IP means TCP/IP 
version 4 (IP6 means IPv6, but as of this writing, I don't have IPv6 working yet).

The next 3 fields are the source address/port, the direction the packet is moving, and the destination address/port.

Next come the TCP flags.  I found a reasonably good explanation of them in [Understanding TCP Flags SYN ACK RST FIN URG PSH](https://www.howtouselinux.com/post/tcp-flags)
S = SYN used only during the 3 way handshake between a TCP client and a TCP server
. = ACK (Acknoledge)
P = PUSH When this bit is set, the sender is telling the server to release all buffered traffic to the application
U = URGENT
E 











## Failure mode: a firewall has blocked this port.
If you see TCP inbound packets coming with the SYN bit [S] set, but nothing coming out, then this particular
port is blocked by a firewall.

In this test, I am using the firewall to block access to ports 2047 (nothing listening to it), port 2048 (`fw_verifier` listening to it), and port 2049 (I don't know what is listening to it).  Ports 2050 and
ports 2051 are not affected.
```
root@jeffs-desktop:~# <kbd>iptables -I INPUT 1 -p tcp --sport 2047 -j DROP</kbd>
root@jeffs-desktop:~# <kbd>iptables -I INPUT 1 -p tcp --sport 2048 -j DROP</kbd>
root@jeffs-desktop:~# <kbd>iptables -I INPUT 1 -p tcp --sport 2049 -j DROP</kbd>
root@jeffs-desktop:~# <kbd>iptables -L INPUT<samp>
Chain INPUT (policy DROP)
target     prot opt source               destination         
DROP       tcp  --  anywhere             anywhere             tcp spt:nfs
DROP       tcp  --  anywhere             anywhere             tcp spt:2048
DROP       tcp  --  anywhere             anywhere             tcp spt:2047
ACCEPT     all  --  raspberry            anywhere            
...

```
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
```
#### Failure mode: nothing is listening on that port.
If the firewall is passing packets but nothing is listening on the port, then tcpdump will
show TCP packets with the RESET bit set
```
jeffs@jeffs-desktop:~/software/fw_verifier$ <kbd>sudo tcpdump -n tcp portrange 8567-8569</kbd><samp>
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on enp5s0, link-type EN10MB (Ethernet), capture size 262144 bytes
21:14:21.412609 IP 192.168.0.159.42538 > 192.168.0.149.8569: Flags [S], seq 3359576664, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412688 IP 192.168.0.149.8569 > 192.168.0.159.42538: Flags [R.], seq 0, ack 3359576665, win 0, length 0
21:14:21.412696 IP 192.168.0.159.56432 > 192.168.0.149.8567: Flags [S], seq 3239392529, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412709 IP 192.168.0.149.8567 > 192.168.0.159.56432: Flags [R.], seq 0, ack 3239392530, win 0, length 0
21:14:21.412712 IP 192.168.0.159.46580 > 192.168.0.149.8568: Flags [S], seq 1318266902, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412722 IP 192.168.0.149.8568 > 192.168.0.159.46580: Flags [R.], seq 0, ack 1318266903, win 0, length 0
21:14:21.588341 IP 192.168.0.159.56055 > 192.168.0.149.8567: Flags [S], seq 1128992667, win 31337, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.588399 IP 192.168.0.149.8567 > 192.168.0.159.56055: Flags [R.], seq 0, ack 1128992668, win 0, length 0
21:14:21.613411 IP 192.168.0.159.56056 > 192.168.0.149.8567: Flags [.], ack 3764636647, win 32768, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.613474 IP 192.168.0.149.8567 > 192.168.0.159.56056: Flags [R], seq 3764636647, win 0, length 0
21:14:21.638449 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 1128992667, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.638481 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1128992668, win 0, length 0
21:14:23.087594 IP 192.168.0.159.56055 > 192.168.0.149.8567: Flags [S], seq 2441507715, win 31337, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.087656 IP 192.168.0.149.8567 > 192.168.0.159.56055: Flags [R.], seq 0, ack 1312515049, win 0, length 0
21:14:23.112671 IP 192.168.0.159.56056 > 192.168.0.149.8567: Flags [.], ack 3370547257, win 32768, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.112726 IP 192.168.0.149.8567 > 192.168.0.159.56056: Flags [R], seq 2840216607, win 0, length 0
21:14:23.137793 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 2441507715, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.137837 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1312515049, win 0, length 0
21:14:23.238483 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 2441507715, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:2jeffs@jeffs-desktop:~/software/fw_verifier$ sudo tcpdump -n tcp portrange 8567-8569
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on enp5s0, link-type EN10MB (Ethernet), capture size 262144 bytes
21:14:21.412609 IP 192.168.0.159.42538 > 192.168.0.149.8569: Flags [S], seq 3359576664, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412688 IP 192.168.0.149.8569 > 192.168.0.159.42538: Flags [R.], seq 0, ack 3359576665, win 0, length 0
21:14:21.412696 IP 192.168.0.159.56432 > 192.168.0.149.8567: Flags [S], seq 3239392529, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412709 IP 192.168.0.149.8567 > 192.168.0.159.56432: Flags [R.], seq 0, ack 3239392530, win 0, length 0
21:14:21.412712 IP 192.168.0.159.46580 > 192.168.0.149.8568: Flags [S], seq 1318266902, win 64240, options [mss 1460,sackOK,TS val 2050334390 ecr 0,nop,wscale 7], length 0
21:14:21.412722 IP 192.168.0.149.8568 > 192.168.0.159.46580: Flags [R.], seq 0, ack 1318266903, win 0, length 0
21:14:21.588341 IP 192.168.0.159.56055 > 192.168.0.149.8567: Flags [S], seq 1128992667, win 31337, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.588399 IP 192.168.0.149.8567 > 192.168.0.159.56055: Flags [R.], seq 0, ack 1128992668, win 0, length 0
21:14:21.613411 IP 192.168.0.159.56056 > 192.168.0.149.8567: Flags [.], ack 3764636647, win 32768, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.613474 IP 192.168.0.149.8567 > 192.168.0.159.56056: Flags [R], seq 3764636647, win 0, length 0
21:14:21.638449 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 1128992667, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:21.638481 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1128992668, win 0, length 0
21:14:23.087594 IP 192.168.0.159.56055 > 192.168.0.149.8567: Flags [S], seq 2441507715, win 31337, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.087656 IP 192.168.0.149.8567 > 192.168.0.159.56055: Flags [R.], seq 0, ack 1312515049, win 0, length 0
21:14:23.112671 IP 192.168.0.159.56056 > 192.168.0.149.8567: Flags [.], ack 3370547257, win 32768, options [wscale 10,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.112726 IP 192.168.0.149.8567 > 192.168.0.159.56056: Flags [R], seq 2840216607, win 0, length 0
21:14:23.137793 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 2441507715, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.137837 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1312515049, win 0, length 0
21:14:23.238483 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 2441507715, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.238560 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1312515049, win 0, length 0
21:14:23.339160 IP 192.168.0.159.56057 > 192.168.0.149.8567: Flags [FPU], seq 2441507715, win 65535, urg 0, options [wscale 15,nop,mss 265,TS val 4294967295 ecr 0,sackOK], length 0
21:14:23.339230 IP 192.168.0.149.8567 > 192.168.0.159.56057: Flags [R.], seq 0, ack 1312515049, win 0, length 0</samp>

```








### nmap

nmap -sT ADDRESS -p 2048-2050


