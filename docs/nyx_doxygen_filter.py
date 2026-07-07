#!/usr/bin/env python3
# -*- coding: utf-8 -*-
########################################################################################################################

import ast
import sys
import pathlib

########################################################################################################################

def _keyword_value(node: ast.Call, name: str) -> ast.expr | None:

    for keyword in node.keywords:

        if keyword.arg == name:

            return keyword.value

    return None

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

    value = _keyword_value(node, 'doc')

    if (
        isinstance(value, ast.Constant)
        and
        isinstance(value.value, str)
    ):

        return value.value

    return None

########################################################################################################################

def _module_name(path: pathlib.Path) -> str:

    parts: list[str] = []

    if path.name != '__init__.py':

        parts.append(path.stem)

    parent = path.parent

    while (parent / '__init__.py').is_file():

        parts.append(parent.name)

        parent = parent.parent

    return '.'.join(reversed(parts))

########################################################################################################################

def _imported_names(tree: ast.Module, module_name: str) -> dict[str, str]:

    imports: dict[str, str] = {}

    package_name = module_name.rpartition('.')[0]
    package_parts = package_name.split('.') if package_name else []

    for node in tree.body:

        if isinstance(node, ast.Import):

            for alias in node.names:

                name = alias.asname or alias.name.split('.', 1)[0]

                if alias.asname is not None:

                    imports[name] = alias.name

                else:

                    imports[name] = name

        elif isinstance(node, ast.ImportFrom):

            if node.level:

                parts = package_parts[:len(package_parts) - node.level + 1]

                if node.module is not None:

                    parts.extend(node.module.split('.'))

                imported_module = '.'.join(parts)

            else:

                imported_module = node.module or ''

            for alias in node.names:

                if alias.name == '*':

                    continue

                name = alias.asname or alias.name

                if imported_module:

                    imports[name] = f'{imported_module}.{alias.name}'

                else:

                    imports[name] = alias.name

    return imports

########################################################################################################################

def _kind(node: ast.Call, imports: dict[str, str], module_name: str) -> tuple[str, str]:

    if len(node.args) >= 3:

        value = node.args[2]

    else:

        value = _keyword_value(node, 'kind')

    if value is None:

        return 'str', 'str'

    value_text = ast.unparse(value)
    value_parts = value_text.split('.')
    kind = value_parts[-1]

    if value_parts[0] in imports:

        kind_ref = '.'.join([
            imports[value_parts[0]],
            *value_parts[1:],
        ])

    elif len(value_parts) == 1 and kind.startswith('Nyx'):

        kind_ref = f'{module_name}.{kind}'

    else:

        kind_ref = value_text

    return kind, kind_ref

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

def _property_lines(name: str, doc: str | None, kind: str, kind_ref: str, indent: str) -> list[str]:

    ####################################################################################################################

    if kind.startswith('Nyx'):

        kind_doc = f'@ref {kind_ref} "{kind}"'

    else:

        kind_doc = kind

    if not doc:

        doc = f'Gets or sets the @c {name} property of this object (type: {kind_doc}).'

    ####################################################################################################################

    doc_lines = [line.strip() for line in doc.strip().splitlines()]

    if doc_lines[0].startswith('@brief '):

        doc_lines[0] = doc_lines[0][len('@brief '):]

    ####################################################################################################################

    result = [f'{indent}## @brief {doc_lines[0]}\n']

    ####################################################################################################################

    for line in doc_lines[1:]:

        if line:

            result.append(f'{indent}## {line}\n')

        else:

            result.append(f'{indent}##\n')

    ####################################################################################################################

    result.extend([
        f'{indent}self.{name} = property()\n',
        '\n',
    ])

    ####################################################################################################################

    return result

########################################################################################################################

def main(path: pathlib.Path) -> int:

    ####################################################################################################################

    lines = path.read_text(encoding = 'utf-8').splitlines(keepends = True)

    ####################################################################################################################

    tree = ast.parse(''.join(lines), filename = str(path))

    module_name = _module_name(path)
    imports = _imported_names(tree, module_name)

    ####################################################################################################################

    insertions: dict[int, list[str]] = {}

    for class_node in ast.walk(tree):

        if not isinstance(class_node, ast.ClassDef):

            continue

        properties: list[tuple[str, str | None, str, str]] = []

        for decorator in class_node.decorator_list:

            if not _is_nyx_property(decorator):

                continue

            name = decorator.args[0].value
            doc = _doc(decorator)
            kind, kind_ref = _kind(decorator, imports, module_name)

            properties.append((name, doc, kind, kind_ref))

            _comment_decorator(lines, decorator)

        if not properties:

            continue

        constructor = _constructor(class_node)
        insert_index = _insert_index(constructor)
        indent = _indent(lines[constructor.body[0].lineno - 1])

        generated_lines: list[str] = []

        for name, doc, kind, kind_ref in properties:

            generated_lines.extend(_property_lines(
                name,
                doc,
                kind,
                kind_ref,
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

    raise SystemExit(main(pathlib.Path(sys.argv[1])))

########################################################################################################################
