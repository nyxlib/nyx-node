#!/bin/bash

########################################################################################################################

THIS_SCRIPT="${BASH_SOURCE[0]:-$0}"

while [[ -n "$(readlink "${THIS_SCRIPT}")" ]]
do
    THIS_SCRIPT="$(readlink "${THIS_SCRIPT}")"
done

########################################################################################################################

BUILD="$(cd "$(dirname "${THIS_SCRIPT}")" && pwd)/../build"

########################################################################################################################

test_json()
{
    echo "$1"
    "${BUILD}/check_json" "$1"
    if [[ $? == 1 ]]
    then
      exit 1
    fi
}

########################################################################################################################

test_json '{}'
test_json '{"key1": "value1", "key2": 123, "key3": null}'
test_json '{"key1": ["value1", 123, null], "key2": ["value1", 123, null]}'

test_json '[]'
test_json '["value1", 123, null]'
test_json '[{"key1": "value1", "key2": 123, "key3": null}, {"key1": "value1", "key2": 123, "key3": null}]'

test_json 'true'
test_json 'false'

test_json '1'
test_json '1.0'
test_json '1.0e+9'
test_json '-1'
test_json '-1.0'
test_json '-1.0e+9'

test_json '"\"\\\/\b\f\n\r\t\u8888"'
test_json '{"unicode_chars": "こんにちは世界"}'

########################################################################################################################
