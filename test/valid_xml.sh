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

test_xml()
{
    echo "$1"
    "${BUILD}/check_xml" "$1"
    if [[ $? == 1 ]]
    then
      exit 1
    fi
}

########################################################################################################################

test_xml '<foo></foo>'
test_xml '<foo:bar></foo:bar>'
test_xml '<foo bar="baz"></foo>'
test_xml '<foo bar="baz" qux="quux"></foo>'
test_xml '<foo />'
test_xml '<foo>bar</foo>'
test_xml '<foo><bar>baz</bar></foo>'
test_xml '<foo><bar /></foo>'
test_xml '<foo><bar baz="qux">foobar</bar></foo>'
test_xml '<foo>   <bar>baz</bar>   </foo>'
test_xml '<foo>&lt;bar&gt;</foo>'
test_xml '<foo><![CDATA[<bar>baz</bar>]]></foo>'
test_xml '<foo bar="baz &amp; qux"></foo>'
test_xml '<foo><!-- comment --></foo>'

########################################################################################################################
