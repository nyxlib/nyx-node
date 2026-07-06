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

def _indent(line: str) -> str:

    return line[:len(line) - len(line.lstrip())]

########################################################################################################################

def _is_docstring(node: ast.stmt) -> bool:

    return (
        isinstance(node, ast.Expr)
        and
        isinstance(node.value, ast.Constant)
        and
        isinstance(node.value.value, str)
    )

########################################################################################################################

def _constructor(node: ast.ClassDef) -> ast.FunctionDef:

    for child in node.body:

        if (
            isinstance(child, ast.FunctionDef)
            and
            child.name == '__init__'
        ):

            return child

    raise ValueError(f'{node.name} has @nyx_property decorators but no __init__() method')

########################################################################################################################

def _insert_index(node: ast.FunctionDef) -> int:

    first = node.body[0]

    if _is_docstring(first):

        return first.end_lineno or first.lineno

    return first.lineno - 1

########################################################################################################################

def _comment_decorator(lines: list[str], node: ast.Call) -> None:

    first = node.lineno - 1
    last = node.end_lineno or node.lineno
    indent = _indent(lines[first])

    for index in range(first, last):

        lines[index] = f'{indent}# {lines[index][len(indent):]}'

########################################################################################################################

def _property_lines(name: str, class_name: str, doc: str | None, indent: str) -> list[str]:

    if doc is None:

        doc = f'The @c {name} : @c str attribute of this {class_name} object.'

    doc_lines = [line.strip() for line in doc.strip().splitlines()]

    if doc_lines[0].startswith('@brief '):

        doc_lines[0] = doc_lines[0][len('@brief '):]

    result = [f'{indent}## @brief {doc_lines[0]}\n']

    for line in doc_lines[1:]:

        if line:

            result.append(f'{indent}## {line}\n')

        else:

            result.append(f'{indent}##\n')

    result.extend([
        f'{indent}self.{name} = \'\'\n',
        '\n',
    ])

    return result

########################################################################################################################

def main() -> int:

    ####################################################################################################################

    path = pathlib.Path(sys.argv[1])

    lines = path.read_text(encoding = 'utf-8').splitlines(keepends = True)

    ####################################################################################################################

    tree = ast.parse(''.join(lines), filename = str(path))

    ####################################################################################################################

    insertions: dict[int, list[str]] = {}

    for class_node in ast.walk(tree):

        if not isinstance(class_node, ast.ClassDef):

            continue

        properties: list[tuple[str, str | None]] = []

        for decorator in class_node.decorator_list:

            if not _is_nyx_property(decorator):

                continue

            name = decorator.args[0].value
            doc = _doc(decorator)

            properties.append((name, doc))

            _comment_decorator(lines, decorator)

        if not properties:

            continue

        constructor = _constructor(class_node)
        insert_index = _insert_index(constructor)
        indent = _indent(lines[constructor.body[0].lineno - 1])

        generated_lines: list[str] = []

        for name, doc in properties:

            generated_lines.extend(_property_lines(
                name,
                class_node.name,
                doc,
                indent,
            ))

        insertions.setdefault(insert_index, []).extend(generated_lines)

    ####################################################################################################################

    for index, line in enumerate(lines):

        if index in insertions:

            sys.stdout.write(''.join(insertions[index]))

        sys.stdout.write(line)

    ####################################################################################################################

    return 0

########################################################################################################################

if __name__ == '__main__':

    raise SystemExit(main())

########################################################################################################################
