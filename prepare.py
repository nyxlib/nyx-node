#!/usr/bin/env python3
########################################################################################################################

import requests

########################################################################################################################

MONGOOSE_VERSION = '7.14'

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

def download_mongoose():

    for filename in ['mongoose.c', 'mongoose.h']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/cesanta/mongoose/{MONGOOSE_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code != 200:

            raise IOError(f'Cannot download `{filename}`')

        with open(f'src/mongoose/{filename}', 'wt') as f:

            f.write(response.content.decode('UTF-8'))

########################################################################################################################

def download_doxygen_awesome():

    for filename in ['doxygen-awesome.css']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v{DOXYGEN_AWESOME_CSS_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code != 200:

            raise IOError(f'Cannot download `{filename}`')

        with open(f'./.{filename}', 'wt') as f:

            f.write(response.content.decode('UTF-8'))

########################################################################################################################

if __name__ == '__main__':

    compile_schema()

    download_mongoose()

    download_doxygen_awesome()

########################################################################################################################
