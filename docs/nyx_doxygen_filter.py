#!/usr/bin/env python3
# -*- coding: utf-8 -*-
########################################################################################################################

import ast
import sys
import pathlib

########################################################################################################################

def _is_nyx_property(node: ast.expr) -> bool:

    return (
            isinstance(node, ast.Call)
            and
            isinstance(node.func, ast.Attribute)
            and
            node.func.attr == 'nyx_property'
            and
            node.args
            and
            isinstance(node.args[0], ast.Constant)
            and
            isinstance(node.args[0].value, str)
    )

########################################################################################################################

def _doc(node: ast.Call) -> str | None:

    for keyword in node.keywords:

        if (
                keyword.arg == 'doc'
                and
                isinstance(keyword.value, ast.Constant)
                and
                isinstance(keyword.value.value, str)
        ):

            return keyword.value.value

    return None

########################################################################################################################

def main() -> int:

    ####################################################################################################################

    path = pathlib.Path(sys.argv[1])

    lines = path.read_text(encoding = 'utf-8').splitlines(keepends = True)

    ####################################################################################################################

    tree = ast.parse(''.join(lines), filename = str(path))

    ####################################################################################################################

    for class_node in ast.walk(tree):

        if not isinstance(class_node, ast.ClassDef):

            continue

        for decorator in class_node.decorator_list:

            if not _is_nyx_property(decorator):

                continue

            name = decorator.args[0].value
            doc = _doc(decorator)

            first = decorator.lineno - 1
            last = decorator.end_lineno or decorator.lineno
            indent = lines[first][:len(lines[first]) - len(lines[first].lstrip())]

            lines[first] = f'{indent}## @property {class_node.name}::{name}\n'

            if doc is not None and first + 1 < last:

                lines[first + 1] = f'{indent}#  {doc}\n'

            for index in range(first + 2, last):

                lines[index] = f'{indent}#\n'

    ####################################################################################################################

    sys.stdout.write(''.join(lines))

    ####################################################################################################################

    return 0

########################################################################################################################

if __name__ == '__main__':

    raise SystemExit(main())

########################################################################################################################
