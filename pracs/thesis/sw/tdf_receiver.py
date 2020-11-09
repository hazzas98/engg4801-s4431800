#!/usr/bin/env python3
"""

"""
__author__ = "Harry Stokes"

import argparse
import datetime

import tdf3
import ListenClient
import tago

from PacpMessage import PayloadType, DecryptionError
from NodeFilter import NodeFilter


def demo_receiver(base_host, base_port, tdf_server):
    # TDF decoding state
    tdfparse = tdf3.Tdf()
    tdfparse.loadTdf(tdf_server, timeout=10.0)
    parser = tdfparse.parseTdf16

    # Baselisten connection
    listener = ListenClient.ListenClient(base_host, base_port)
    listener.connect()

    MY_DEVICE_TOKEN = '4005b6a7-e9bd-4315-9b41-5fe527d7ebec'
    my_device = tago.Device(MY_DEVICE_TOKEN)

    try:
        # Loop forever for packets from baselisten
        while True:
            try:
                packet = listener.read(timeout=None)
            except ConnectionResetError:
                print("Connection to baselisten lost...")
                return
            except NotImplementedError:
                continue
            
            # Loop over every payload in the serial packet
            for payload_type, route, payload in packet.iter_payloads():

                now = datetime.datetime.now()

                first_hop = route[-1]
                pkt_addr = first_hop.address_fmt
                pkt_rssi = "{:d}dBm".format( first_hop.rssi)
                print("From {:s}, {:d} bytes, RSSI {:s}".format(pkt_addr, len(payload), pkt_rssi))

                # This demo is only concerned with TDF payloads
                if payload_type != PayloadType.PAYLOAD_TDF3:
                    print("\tPacket was not a TDF ({:})".format(payload_type))
                    continue
                
                # Payload is a TDF
                for point in parser(payload, datetime.datetime.utcnow(), debug=False, combine=True):
                    print(point)
                    
                    if point.get('sensor') == 'LPS22HB_ALL':

                        data_to_insert = {
                            'variable': point.get('sensor'),
                            'time': now.strftime("%Y-%m-%d %H:%M:%S"),
                            'metadata' : {'pressure' : point.get('phenomena').get('pressure').get('formatted')}
                        }

                    if point.get('sensor') == '3D_POSE':

                        data_to_insert = {
                            'variable': point.get('sensor'),
                            'time': now.strftime("%Y-%m-%d %H:%M:%S"),
                            'metadata' : {'roll' : point.get('phenomena').get('roll').get('formatted'), 'pitch' : point.get('phenomena').get('pitch').get('formatted')}
                        }

                    if point.get('sensor') == 'HEIGHT_MSL':

                        data_to_insert = {
                            'variable': point.get('sensor'),  
                            'time': now.strftime("%Y-%m-%d %H:%M:%S"),  
                            'metadata' : {'height' : point.get('phenomena').get('height').get('formatted'), 'raw' : point.get('phenomena').get('height').get('raw')}    
                        }

                    if point.get('sensor') == 'RANGE_MM':

                        data_to_insert = {
                            'variable': point.get('sensor'),
                            'time': now.strftime("%Y-%m-%d %H:%M:%S"),
                            'metadata' : {'range' : point.get('phenomena').get('range').get('formatted'),  'raw' : point.get('phenomena').get('range').get('raw')}    
                        }
                
                
                    my_device.insert(data_to_insert) 

    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='TDF listener')
    parser.add_argument('--host', dest='base_host', type=str, default="localhost", help='Hostname for baselisten')
    parser.add_argument('--port', dest='base_port', type=int, default=9001, help='Port for baselisten')
    parser.add_argument('--tdf', dest='tdf_server', type=str, default=None, help='Hostname for TDF server')

    args = parser.parse_args()

    demo_receiver(**vars(args))
