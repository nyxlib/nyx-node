#!/usr/bin/env python3
########################################################################################################################

import requests

########################################################################################################################

MONGOOSE_VERSION = '7.19'

DOXYGEN_AWESOME_CSS_VERSION = '2.4.1'

########################################################################################################################

def patch(s):

    return f'#if !defined(ARDUINO)\n{s}\n#endif // ARDUINO\n'

########################################################################################################################

def download_mongoose():

    for filename in ['mongoose.c', 'mongoose.h']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/cesanta/mongoose/{MONGOOSE_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code == 200:

            with open(f'src/stacks/external/{filename}', 'wt') as f:

                f.write(patch(response.content.decode('UTF-8')))

        else:

            raise IOError(f'Cannot download `{filename}`')

########################################################################################################################

def download_doxygen_awesome():

    for filename in ['doxygen-awesome.css', 'doxygen-awesome-darkmode-toggle.js']:

        ################################################################################################################

        response = requests.get(f'https://raw.githubusercontent.com/jothepro/doxygen-awesome-css/v{DOXYGEN_AWESOME_CSS_VERSION}/{filename}')

        ################################################################################################################

        if response.status_code == 200:

            with open(f'docs/{filename}', 'wt') as f:

                f.write(response.content.decode('UTF-8').replace('--content-maxwidth: 1050px;', '--content-maxwidth: 1250px;'))

        else:

            raise IOError(f'Cannot download `{filename}`')

########################################################################################################################

if __name__ == '__main__':

    download_mongoose()

    download_doxygen_awesome()

########################################################################################################################
