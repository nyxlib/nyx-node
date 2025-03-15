#!/usr/bin/env python3
########################################################################################################################

import re
import requests

########################################################################################################################

MONGOOSE_VERSION = '7.17'

DOXYGEN_AWESOME_CSS_VERSION = '2.3.3'

########################################################################################################################

def compile_schema():

    ####################################################################################################################

    with open('src/schema/nyx.xsd', 'rb') as f:

        data_in = bytearray(f.read())

    ####################################################################################################################

    data_len = len(data_in)
    data_out = (((('\n'))))

    for part in range(0, data_len, 16):

        data_out += f'\t{", ".join(["0x%02X" % b for b in data_in[part: part + 16]])},\n'

    ####################################################################################################################

    with open('src/nyx_node_schema.h', 'wt') as f:

        f.write(f'/* file: nyx.xsd */\n\n')

        f.write(f'#define NYX_NODE_XSD_SIZE {data_len}\n\n')

        f.write(f'char nyx_node_xsd_buff[] = {{{data_out}}};\n')

########################################################################################################################

MONGOOSE_CONFIG = '''
#  if defined(ARDUINO)
#    if defined(ESP8266)
#      include <errno.h>
#      include <stdarg.h>
#      include <stddef.h>
#      include <stdint.h>
#      include <stdbool.h>
#      include <time.h>
#      include <stdio.h>
#      include <stdlib.h>
#      include <string.h>
#      define MG_ARCH MG_ARCH_CUSTOM
#      define MG_ENABLE_FILESYSTEM 0
#      define MG_ENABLE_SOCKET 0
#      define MG_ENABLE_TCPIP 1
#      define MG_ENABLE_SSI 0
#    elif defined(ESP32)
#      define MG_ARCH MG_ARCH_ESP32
#      define MG_ENABLE_SSI 0
#    elif defined(PICO_BOARD)
#      define MG_ARCH MG_ARCH_PICOSDK
#      define MG_ENABLE_DRIVER_W5500 1
#      define MG_ENABLE_SOCKET 0
#      define MG_ENABLE_TCPIP 1
#      define MG_ENABLE_SSI 0
#      define MG_IO_SIZE 512
#    else
#      define MG_ARCH MG_ARCH_CUSTOM
#      define MG_ENABLE_DRIVER_W5500 1
#      define MG_ENABLE_SOCKET 0
#      define MG_ENABLE_TCPIP 1
#      define MG_ENABLE_SSI 0
#      define MG_IO_SIZE 128
#    endif
#  else
#    error("Only the Arduino platform is supported!")
#  endif
'''

def patch(s):

    return re.sub(r'#include\s+"mongoose_config\.h".*', MONGOOSE_CONFIG.strip(), s)

########################################################################################################################

def download_mongoose():

    for filename in ['mongoose.c', 'mongoose.h']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/cesanta/mongoose/{MONGOOSE_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code != 200:

            raise IOError(f'Cannot download `{filename}`')

        with open(f'src/stack/{filename}', 'wt') as f:

            f.write(patch(response.content.decode('UTF-8')))

########################################################################################################################

def download_doxygen_awesome():

    for filename in ['doxygen-awesome.css', 'doxygen-awesome-darkmode-toggle.js']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v{DOXYGEN_AWESOME_CSS_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code != 200:

            raise IOError(f'Cannot download `{filename}`')

        with open(f'.doxygen/{filename}', 'wt') as f:

            f.write(response.content.decode('UTF-8'))

########################################################################################################################

if __name__ == '__main__':

    compile_schema()

    download_mongoose()

    download_doxygen_awesome()

########################################################################################################################
