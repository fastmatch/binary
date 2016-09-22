#!/usr/bin/ksh 

#
#    This file is part of Fastmatch binary market data and order flow examples.
#
#    Fastmatch binary market data and order flow examples are free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Fastmatch binary market data and order flow examples are distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Fastmatch binary market data examples.  If not, see <http:#www.gnu.org/licenses/>.
#

# Specifies spinning time
export EF_SPIN_USEC=20000000

export EF_UDP_RECV_SPIN=1
export EF_UDP_SEND_SPIN=1
export EF_TCP_RECV_SPIN=1
export EF_TCP_SEND_SPIN=1
export EF_BUZZ_USEC=1000000
export EF_SOCK_LOCK_BUZZ=1
export EF_STACK_LOCK_BUZZ=1
#export EF_INT_DRIVEN=1

# Disables global sninning. Spinning will be enabled on per-thread basis.
export EF_POLL_USEC=0

# Spin in accept only in thread(s) where spinning is enabled
#export EF_TCP_ACCEPT_SPIN=1

# Disable UDP acceleration
#export EF_UDP=0

#export EF_UL_POLL=0
export EF_UL_EPOLL=1
export EF_UL_SELECT=1
export EF_PIPE=0

export EF_UDP=1
export EF_TCP=1
 
# Disable FASTSTART when connection is new or has been idle for a while.
# The additional acks it causes add latency on the receive path.
#export EF_TCP_FASTSTART_INIT=0
#export EF_TCP_FASTSTART_IDLE=0
 
# Use a large initial congestion window so that the slow-start algorithm
# doesn't cause delays.  We don't enable this by default because it breaks
# the TCP specs, and could cause congestion in your network.  Uncomment if
# you think you need this.
#
#export EF_TCP_INITIAL_CWND=10000
export EF_TX_PUSH=1
 
# When TCP_NODELAY is used, always kick packets out immediately.  This is
# not enabled by default because most apps benefit from the default
# behaviour.
#
#export EF_NONAGLE_INFLIGHT_MAX=65535

export EF_DONT_ACCELERATE=1

export EF_USE_HUGE_PAGES=0

#export EF_STACK_PER_THREAD=1
#export EF_MAX_ENDPOINTS=50
#export EF_MAX_TX_PACKETS=65536
#export EF_USE_HUGE_PAGES=1
#export EF_MIN_FREE_PACKETS=4096
#export EF_TXQ_SIZE=2048
#export EF_TCP_SNDBUF=8388608
#export EF_UDP_RCVBUF=67108864

