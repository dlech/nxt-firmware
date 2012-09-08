#!/usr/bin/env python
#
# Copyright (C) 2011 the NxOS developers
#
# Module Developed by: Nicolas Schodet
#                      TC Wan
#
# See AUTHORS for a full list of the developers.
#
# See COPYING for redistribution license
#
# Exchange GDB messages with the NXT brick.
#
# Every message is encapsulated with the debug command and message length.
# This can be used by the firmware to make the distinction between debug
# messages and regular messages.
#
import nxt.locator
import socket
import optparse
import select
try:
    import pyfantom
except ImportError:
    import usb
import struct
import sys

CTRLC = chr(3)
NAKCHAR = '-'
ACKCHAR = '+'
STATUS_QUERY = "$?#3F"
DEFAULT_PORT = 2828
SELECT_TIMEOUT = 0.1
DEBUG = False
DEBUG2 = False
NXT_RECV_ERR = -1

# Libusb 0.12.x blocks on USB reads
LIBUSB_RECEIVE_BLOCKING = True

class NXTGDBServer:

    # Socket read size.
    recv_size = 1024

    # Maximum message size.
    pack_size = 61
    
    # Debug command header, no reply.
    debug_command = 0x8d

    def __init__ (self, port, nowait):
        """Initialise server."""
        self.nowait = nowait
        self.port = port
        self.in_buf = ''
        self.brick = None

    def pack (self, data, segment_no):
        """Return packed data to send to NXT."""
        # Insert command and length.
        assert len (data) <= self.pack_size
        return struct.pack ('BBB', self.debug_command, segment_no, len (data)) + data

    def unpack (self, data):
        """Return unpacked data from NXT."""
        # May be improved, for now, check command and announced length.
        if len (data) == 0:
            return '', 0  # No message, exit
        if len (data) < 3:
            return '', NXT_RECV_ERR
        header, body = data[0:3], data[3:]
        command, segment_no, length = struct.unpack ('BBB', header)
        if command != self.debug_command or length != len (body):
            return '', NXT_RECV_ERR
        return body, segment_no

    def segment (self, data):
        """Split messages in GDB commands and make segments with each command."""
        segs = [ ]
        self.in_buf += data

        # Find ACK '+' 
        end = self.in_buf.find (ACKCHAR)
        while end == 0:
            self.in_buf = self.in_buf[end+1:]   # Strip out any leading ACKCHAR
            if DEBUG2:
                print "stripped ACK, remain: ", self.in_buf
            end = self.in_buf.find (ACKCHAR)

        # Find NAK '-' 
        end = self.in_buf.find (NAKCHAR)
        if end == 0:
            msg, self.in_buf = self.in_buf[0:end+1], self.in_buf[end+1:]
            segs.append (self.pack (msg, 0))
            end = self.in_buf.find (NAKCHAR)

        # Find Ctrl-C (assumed to be by itself and not following a normal command)
        end = self.in_buf.find (CTRLC)
        if end >= 0:
            msg, self.in_buf = self.in_buf[0:end+1], self.in_buf[end+1:]
            assert len (msg) <= self.pack_size, "Ctrl-C Command Packet too long!"
            segs.append (self.pack (msg, 0))
            end = self.in_buf.find (CTRLC)
        
        end = self.in_buf.find ('#')
        # Is # found and enough place for the checkum?
        while end >= 0 and end < len (self.in_buf) - 2:
            msg, self.in_buf = self.in_buf[0:end + 3], self.in_buf[end + 3:]
            i = 0
            gdbprefix = msg[i]
            while gdbprefix in [ACKCHAR]:
                # Ignore any '+'
                i += 1
                gdbprefix = msg[i]
                if DEBUG2:
                    print "Checking '", gdbprefix, "'"
            assert gdbprefix == '$', "not a GDB command"
            # Make segments.
            seg_no = 0
            while msg:
                seg, msg = msg[0:self.pack_size], msg[self.pack_size:]
                seg_no += 1
                if not msg: # Last segment.
                    seg_no = 0
                segs.append (self.pack (seg, seg_no))
            # Look for next one.
            end = self.in_buf.find ('#')
        return segs
        
    def reassemble (self, sock):
        msg = ''
        prev_segno = 0
        segno = NXT_RECV_ERR                    # force initial pass through while loop
        while segno != 0:
            try:
                s, segno = self.unpack (sock.recv ())
                if len (s) == 0:
                    if segno == 0 and prev_segno == 0:
                        return ''               # No message pending
                    else:
                        segno = NXT_RECV_ERR    # Keep waiting for segments
                # Ignore error packets
                if segno >= 0:
                    # Check segno, if non-zero it must be monotonically increasing from 1, otherwise 0
                    if segno > 0:
                       assert segno == prev_segno + 1, "segno = %s, prev_segno = %s" % (segno, prev_segno)
                    prev_segno = segno
                    msg += s               
            except IOError as e:
                # Some pyusb are buggy, ignore some "errors".
                if e.args != ('No error', ):
                    raise e
        return msg
        
    def run (self):
        """Endless run loop."""
        # Create the listening socket.
        s = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind (('', self.port))
        s.listen (1)
        while True:
            # We should open the NXT connection first, otherwise Python startup delay
            # may cause GDB to misbehave
            if not self.nowait:
                dummy = raw_input('Waiting...Press <ENTER> when NXT GDB Stub is ready. ')
            # Open connection to the NXT brick.
            self.brick = nxt.locator.find_one_brick ()
            self.brick.sock.debug = DEBUG
            # Wait for a connection.
            print "Waiting for GDB connection on port %s..." % self.port
            client, addr = s.accept ()
            print "Client from", addr
            # Work loop, wait for a message from client socket or NXT brick.
            while client is not None:
                data = ''
                # Wait for a message from client or timeout.
                rlist, wlist, xlist = select.select ([ client ], [ ], [ ],
                        SELECT_TIMEOUT)
                for c in rlist:
                    assert c is client
                    # Data from client, read it and forward it to NXT brick.
                    data = client.recv (self.recv_size)
                    data = data.strip()
                    if len (data) > 0:
                        #if len (data) == 1 and data.find(CTRLC) >= 0:
                        #   print "CTRL-C Received!"
                        #   data = STATUS_QUERY
                        if DEBUG:
                            if data[0] == CTRLC:
                                print "[GDB->NXT] <CTRL-C>"
                            else:
                                print "[GDB->NXT] %s" % data
                        segments = self.segment (data)
                        data = ''
                        for seg in segments:
                            try:
                                self.brick.sock.send (seg)
                            except IOError as e:
                                # Some pyusb are buggy, ignore some "errors".
                                if e.args != ('No error', ):
                                    raise e
                        if segments != [] and LIBUSB_RECEIVE_BLOCKING:
                            if DEBUG2:
                                print "Accessing Blocking sock.recv()"
                            data = self.reassemble (self.brick.sock)
                    else:
                        client.close ()
                        client = None
                if not LIBUSB_RECEIVE_BLOCKING:
                    if DEBUG2:
                         print "Accessing Non-Blocking sock.recv()"
                    data = self.reassemble (self.brick.sock)
                    
                # Is there something from NXT brick?
                if data:
                    if DEBUG:
                        print "[NXT->GDB] %s" % data
                    if client:
                        client.send (data)
                    data = ''
            self.brick.sock.close()
            print "Connection closed."
            if self.nowait:
                break

if __name__ == '__main__':
    # Read options from command line.
    parser = optparse.OptionParser (description = """
    Gateway between the GNU debugger and a NXT brick.
    """)
    parser.add_option ('-p', '--port', type = 'int', default = DEFAULT_PORT,
            help = "server listening port (default: %default)", metavar = "PORT")
    parser.add_option ('-v', '--verbose', action='store_true', dest='verbose', default = False,
            help = "verbose mode (default: %default)")
    parser.add_option ('-n', '--nowait', action='store_true', dest='nowait', default = False,
            help = "Don't wait for NXT GDB Stub Setup before connecting (default: %default)")
    (options, args) = parser.parse_args ()
    if args:
        parser.error ("Too many arguments")
    # Run.
    try:
        DEBUG = options.verbose
        if DEBUG:
            print "Debug Mode Enabled!"
        server = NXTGDBServer (options.port, options.nowait)
        server.run ()
    except KeyboardInterrupt:
        print "\n\nException caught. Bye!"
        if server.brick is not None:
            server.brick.sock.close()
        sys.exit()
